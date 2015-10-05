
#include "hashManager.h"
#include <assert.h>

HashtableManager::HashtableManager()
{
	m_pmtx = new Mutex();
	assert(m_pmtx != NULL);
}

HashtableManager::~HashtableManager()
{
	if(m_pght)
		ght_finalize(m_pght);
	if(m_pmtx)
	{
		delete m_pmtx;
		m_pmtx = NULL;
	}
}	

int HashtableManager::Create(unsigned int hashnum)
{
	m_pght = ght_create(hashnum);
	assert(m_pght != NULL);
	return 0;
}

int HashtableManager::Insert(string key, string value)
{
	m_pmtx->Lock();
	int ret = ght_insert(m_pght, (void*)value.c_str(), key.length(), (char*)key.c_str());
	m_pmtx->Unlock();
	return ret;
}

int HashtableManager::Search(string key, string& value)
{
//	m_pmtx->Lock();
	void* v = NULL;
	if((v = ght_get(m_pght, key.length(), (char*)key.c_str())) == NULL)
	{
//		m_pmtx->Unlock();
		return -1;
	}
	value = (char*)v;
//	m_pmtx->Unlock();
	return 0;
}

int HashtableManager::Delete(string key)
{
	m_pmtx->Lock();
	void* v = NULL;
	if((v = ght_remove(m_pght, key.length(), (char*)key.c_str())) == NULL)
	{
		m_pmtx->Unlock();
		return -1;
	}
	m_pmtx->Unlock();
	return 0;
}
