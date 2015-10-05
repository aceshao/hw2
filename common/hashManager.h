#ifndef _HASHMANAGER_H
#define _HASHMANAGER_H

#include <string>
#include "thread.h"
#include <ght_hash_table.h>
using namespace std;

class HashtableManager
{
public:
	HashtableManager();
	~HashtableManager();

	int Create(unsigned int hashnum);
	int Insert(string key, string value);
	int Search(string key, string& value);
	int Delete(string key);

private:
	Mutex* m_pmtx;
	ght_hash_table_t* m_pght;

};


#endif
