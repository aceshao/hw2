#ifndef TOOLS_H
#define TOOLS_H

#include <stdio.h>

string  iTo4ByteString(int iNum)
{
	char szName[12] = {0};
	sprintf(szName,"%4d",iNum);
	return szName;
}
 
string trim(const char* data)
{
	if(data==NULL) return "";
	int bIndex = 0;
	int eIndex = strlen(data)-1;
	while(*(data+bIndex)==' ' || *(data+bIndex)== '\n') bIndex++;
	while(eIndex>=bIndex&&(*(data+eIndex)==' ' || *(data+eIndex)=='\n')) eIndex--;
	string value = "";
	value.assign(data+bIndex,eIndex-bIndex+1);
	return value;
}



#endif
