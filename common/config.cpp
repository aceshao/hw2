#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "config.h"

int str2int (const char *strval, int def)
{
	int ret_code = def;
	if (isdigit (strval[0]) || (strval[0] == '-' && isdigit(strval[1])))
		return atoi (strval);
	if (!strcasecmp (strval, "On"))
		ret_code = 1;
	else if (!strcasecmp (strval, "Off"))
		ret_code = 0;
	else if (!strcasecmp (strval, "Yes"))
		ret_code = 1;
	else if (!strcasecmp (strval, "No"))
		ret_code = 0;
	else if (!strcasecmp (strval, "True"))
		ret_code = 1;
	else if (!strcasecmp (strval, "False"))
		ret_code = 0;
	else if (!strcasecmp (strval, "Enable"))
		ret_code = 1;
	else if (!strcasecmp (strval, "Disable"))
		ret_code = 0;
	else if (!strcasecmp (strval, "Enabled"))
		ret_code = 1;
	else if (!strcasecmp (strval, "Disabled"))
		ret_code = 0;
	return ret_code;
}
char *skipblank(char *p)
{
	while(isblank(*p))
		p++;
	return  p;
}

Config* Config::config = NULL;


Config* Config::Instance ()
{
	if(config) return config;
	else
	{
		config = new Config;
		return config;
	}
}

void Config::Destroy ()
{
 	if(config)
 	{
 		delete config;
 		config = NULL;
 	}    
}

int Config::ParseConfig (const char *fn, const char *defsec)
{
	char *buf, *start;
	int len, fd, ret_code = -1;

	if(fn == NULL)
		fn = filename.c_str();
	else if(filename.size() <= 0)
	{
		filename = fn;
	}
	if(filename[0]=='\0')
		return -1;

	fd = open (fn, O_RDONLY);
	if (fd < 0) 
	{
		return -1;
	}
	len = lseek (fd, 0L, SEEK_END);
	lseek (fd, 0L, SEEK_SET);
	buf = new char[len+1];
	read(fd, buf, len);
	buf[len] = '\0';
	ret_code = len + 1;
	close(fd);

	if(defsec==NULL)
		defsec = "";
	keymap *kmap = &smap[defsec];

	start = buf;
	int ln = 0;
	char *end = start;
	ret_code = 0;
	for (start=buf, ln=0; end && buf + len > start; start = end + 1)
	{
		end = strchr (start, '\n');
		if(end)
		{
			if (*end)
				*end = '\0';
			if(end>start && end[-1]=='\r')
				end[-1] = '\0';
		}

		std::string val("?");
		keymap *m = kmap;

		ln++;

		char *v = NULL;
		char *key = skipblank(start);
		// blank or comment line
		if(key[0]=='\0' || key[0]=='#' || key[0]=='?' || key[0]==';')
			continue;
		// key must printable
		if(!isprint(key[0]))
		{
			ret_code = -1;
			continue;
		}

		// find the equation
		start = key + strcspn(key, "= \t");
		if(*start!='\0')
		{
			char *p = start[0]=='=' ? start : skipblank(start+1);
			if(*p == '=')
			{
				v = skipblank(p+1);
			}
			else if(key[0]=='[')
				/*OK*/;
			else {
				ret_code = -1;
				continue;
			}
			*start = '\0';
		} else if(key[0] != '[')
		{
			ret_code = -1;
			continue;
		}

		if(key[0]=='[')
		{
			char *r = strchr(key, ']');
			if(!r)
			{
				ret_code = -1;
				continue;
			} else if(!((r[1]=='\0'&&v==NULL)||(r[1]=='.'&&v!=NULL)))
			{
				ret_code = -1;
				continue;
			} else {
				*r = '\0';
				m = &smap[key+1];
				if(r[1]=='\0')
				{
					kmap = m;
					continue;
				}
				key = r + 2;
			}
		}

		if(v==NULL)
			continue;

		switch(v[0])
		{
		case '(':
			start = strchr(v, ')');
			if(start==NULL) goto error;
			break;
		case '[':
			start = strchr(v, ']');
			if(start==NULL) goto error;
			break;
		case '{':
			start = strchr(v, '}');
			if(start==NULL) goto error;
			break;

		case '"':
			start = strrchr(v+1, '"');
			if(start==NULL) goto error;
			break;
		case '\'':
			start = strrchr(v+1, '\'');
			if(start==NULL) goto error;
			break;
		default:
			start = end ? end-1 : v+strlen(v)-1;
			if(*start=='\0') start--;
			while(start > v && isblank(*start))
				start--;
			break;
		error:
			ret_code = -1;
			continue;
		}
		start[1] = '\0';

		if(v[0])
			val.append(v);

		(*m)[key] = val;
	}
	delete [] buf;
	return ret_code;
}

bool Config::HasSection (const char *sec)
{
	secmap::iterator n = smap.find(sec);
	return n!=smap.end() && n->second.size()>0 ? true : false;
}

bool Config::HasKey (const char *sec, const char *key)
{
	secmap::iterator n = smap.find(sec);
	if(n==smap.end())
		return false;
	keymap &kmap = n->second;
	keymap::iterator m = kmap.find(key);
	return m != kmap.end() && m->second.size()>0  ? true : false;
}

const char* Config::GetStrVal (const char *sec, const char* key,const char* defaut)
{
	keymap &kmap = smap[sec];
	std::string &v = kmap[key];
	if(v.size()==0)
		return defaut;
	v[0] = ' ';
	return v.c_str() + 1;
}

int Config::GetIntVal (const char *sec, const char* key, int def)
{
	const char* val = GetStrVal(sec, key);
	if (val == NULL)
		return def;
	return str2int (val, def);
}

