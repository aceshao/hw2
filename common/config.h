#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <string>
#include <strings.h>
#include <map>

using namespace std;
class Config 
{
public:
	~Config() {};
       static Config* Instance ();
       static void Destroy ();
	int GetIntVal(const char *sec, const char *key, int defaut =0);
	const char *GetStrVal (const char *sec, const char *key,const char *defaut = NULL);
	bool HasSection(const char *sec);
	bool HasKey(const char *sec, const char *key);
	int ParseConfig(const char *f=0, const char *s=0);	
private:
	struct nocase
	{
		bool operator()(const std::string &a, const std::string &b) const
		{ return strcasecmp(a.c_str(), b.c_str()) < 0; }
	};
	typedef map<string, string, nocase> keymap;
	typedef map<string, keymap, nocase> secmap;
	Config() {};
	string filename;
	secmap smap;
	static Config* config;
};


#endif

