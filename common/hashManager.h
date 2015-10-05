#ifndef _HASHMANAGER_H
#define _HASHMANAGER_H

#include <string.h>
#include "thread.h"
#include <ght_hash_table.h>
using namespace std;

class HashtableManager
{
public:
	HashtableManager();
	~HashtableManager();

	int Create();
	int Insert(const string& key, const string& value);
	int Search(const string& key, string& value);
	int Delete(const string& key);

private:
	Mutex* m_pmtx;
	ght_hash_table_t* m_pght;

};


#endif