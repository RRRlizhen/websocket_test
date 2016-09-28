
#ifndef _redisCliMgr_h_
#define _redisCliMgr_h_

#include "redisclient.h"
#include "lock.h"
#include <stdint.h>
#include <list>
#include <vector>

using namespace redis;
using namespace boost;


class RedisCliMgr
{
public:

	RedisCliMgr(std::vector<connection_data> &vecCon)
	{
		assert(!vecCon.empty());
		m_vecCon = vecCon;
		
		for (int i = 0; i < 1; ++i)
		{
			boost::shared_ptr<client> cluster( new redis::client(m_vecCon.begin(), m_vecCon.end()) );
			assert(cluster);
			ScopeWLock lock(m_lock);
			m_listCli.push_back(cluster);
		}
	}

	boost::shared_ptr<client> get()
	{
		if (true)
		{
			ScopeWLock lock(m_lock);
			if (!m_listCli.empty())
			{
				boost::shared_ptr<client> dummy = *m_listCli.begin();
				assert(dummy);
				m_listCli.pop_front();
				return dummy;
			}
		}

		boost::shared_ptr<client> cluster( new redis::client(m_vecCon.begin(), m_vecCon.end()) );
		assert(cluster);
		return cluster;
	}

	void giveback(boost::shared_ptr<client> m)
	{
		ScopeWLock lock(m_lock);
		m_listCli.push_back(m);
	}

	class GiveBackGuard
	{
	public:
		GiveBackGuard(RedisCliMgr& mm, boost::shared_ptr<client> m) : _mm(mm), _m(m)	{assert(_m);}
		~GiveBackGuard(){_mm.giveback(_m);}
		RedisCliMgr&		_mm;
		boost::shared_ptr<client>	_m;
	};

private:
	std::vector<connection_data>		m_vecCon;
	std::list<boost::shared_ptr<client> >	m_listCli;
	RWLockable					m_lock;
};

#endif

