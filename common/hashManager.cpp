
#include "hashManager.h"
#include <assert.h>

HashtableManager::HashtableManager(unsigned int hashnum)
{
	Create(hashnum);
}

HashtableManager::~HashtableManager()
{
	if(m_pght)
		ght_finalize(m_pght);
}	

int HashtableManager::Create(unsigned int hashnum)
{
	m_pght = ght_create(hashnum);
	assert(m_pght != NULL);
	return 0;
}

int HashtableManager::Insert(const string& key, const string& value)
{
	return ght_insert(m_pght, &value, key.length(), &key);
}

int HashtableManager::Search(const string& key, string& value)
{
	void* v = NULL;
	if((v = ght_get(m_pght, key.length(), &key)) == NULL)
		return -1;
	value = (char*)v;
	return 0;
}

int HashtableManager::Delete(const string& key)
{
	void* v = NULL;
	if((v = ght_remove(m_pght, key.length(), &key)) == NULL)
		return -1;
	return 0;
}
