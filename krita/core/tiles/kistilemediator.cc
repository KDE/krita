/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <assert.h>
#include <algorithm>
#include <list>
#include <map>
#include <utility>
#include "kis_types.h"
#include "kis_global.h"
#include "kisscopedlock.h"
#include "kistilemediator.h"

namespace {
	class KisTileMediatorSingleton {
		typedef std::pair<KisTileMgr *, Q_INT32> KisTileLink;
		typedef std::list<KisTileLink> vKisTileLink;
		typedef vKisTileLink::iterator vKisTileLink_it;
		typedef std::map<KisTile *, vKisTileLink> acKisTileLink;
		typedef acKisTileLink::iterator acKisTileLink_it;

	public:
		KisTileMediatorSingleton();
		~KisTileMediatorSingleton();

	public:
		void attach(KisTile *tile, KisTileMgr *mgr, Q_INT32 tilenum);
		void detach(KisTile *tile, KisTileMgr *mgr, Q_INT32 tilenum);
		void detachAll(KisTileMgr *mgr);
		Q_INT32 tileNum(KisTile *tile, KisTileMgr *mgr);

	public:
		static KisTileMediatorSingleton *singleton();

	private:
		QMutex m_mutex;
		acKisTileLink m_links;

	private:
		static KisTileMediatorSingleton *m_singleton;
	};

	KisTileMediatorSingleton *KisTileMediatorSingleton::m_singleton = 0;

	KisTileMediatorSingleton::KisTileMediatorSingleton()
	{
		assert(KisTileMediatorSingleton::m_singleton == 0);
		KisTileMediatorSingleton::m_singleton = this;
	}

	KisTileMediatorSingleton::~KisTileMediatorSingleton()
	{
		assert(KisTileMediatorSingleton::m_singleton);
	}

	void KisTileMediatorSingleton::attach(KisTile *tile, KisTileMgr *mgr, Q_INT32 tilenum) 
	{
		QMutexLocker lock(&m_mutex);

		if (m_links.count(tile) == 0)
			m_links[tile] = vKisTileLink();

		m_links[tile].push_back(std::make_pair(mgr, tilenum));
	}

	void KisTileMediatorSingleton::detach(KisTile *tile, KisTileMgr *mgr, Q_INT32 tilenum)
	{
		QMutexLocker lock(&m_mutex);

		if (m_links.count(tile)) {
			vKisTileLink& l = m_links[tile];

			for (vKisTileLink_it it = l.begin(); it != l.end();) {
				const KisTileLink& link = *it;

				if (link.first == mgr && link.second == tilenum) {
					it = l.erase(it);
				}
				else {
					it++;
				}
			}

			if (l.empty()) 
				m_links.erase(tile);
		}
	}

	void KisTileMediatorSingleton::detachAll(KisTileMgr *mgr)
	{
		QMutexLocker lock(&m_mutex);
		std::list<KisTile *> emptyEntries;

		for (acKisTileLink_it it = m_links.begin(); it != m_links.end(); it++) {
			KisTile *tile = it -> first;
			vKisTileLink& l = it -> second;

			for (vKisTileLink_it lit = l.begin(); lit != l.end();) {
				const KisTileLink& link = *lit;

				if (link.first == mgr) {
					lit = l.erase(lit);
				}
				else {
					lit++;
				}
			}

			if (l.empty()) {
				emptyEntries.push_back(tile);
			}
		}

		for (std::list<KisTile *>::iterator it = emptyEntries.begin(); it != emptyEntries.end(); it++) {
			m_links.erase(*it);
		}
	}

	Q_INT32 KisTileMediatorSingleton::tileNum(KisTile *tile, KisTileMgr *mgr)
	{
		QMutexLocker lock(&m_mutex);

		if (!m_links.count(tile))
			return TILE_NOT_ATTACHED;

		vKisTileLink& l = m_links[tile];
		
		for (vKisTileLink_it it = l.begin(); it != l.end(); it++) {
			const KisTileLink& link = *it;

			if (link.first == mgr)
			       return link.second;
		}

		return TILE_NOT_ATTACHED;
	}

	KisTileMediatorSingleton *KisTileMediatorSingleton::singleton()
	{
		assert(KisTileMediatorSingleton::m_singleton);
		return KisTileMediatorSingleton::m_singleton;
	}
}

KisTileMediatorSingleton moveMe; // XXX Where should we create singleton data in Krita?

KisTileMediator::KisTileMediator()
{
}

KisTileMediator::~KisTileMediator()
{
}

void KisTileMediator::attach(KisTile *tile, KisTileMgr *mgr, Q_INT32 tilenum)
{
	KisTileMediatorSingleton *mediator = KisTileMediatorSingleton::singleton();

	mediator -> attach(tile, mgr, tilenum);
}

void KisTileMediator::detach(KisTile *tile, KisTileMgr *mgr, Q_INT32 tilenum)
{
	KisTileMediatorSingleton *mediator = KisTileMediatorSingleton::singleton();

	return mediator -> detach(tile, mgr, tilenum);
}

Q_INT32 KisTileMediator::tileNum(KisTile *tile, KisTileMgr *mgr)
{
	KisTileMediatorSingleton *mediator = KisTileMediatorSingleton::singleton();

	return mediator -> tileNum(tile, mgr);
}

void KisTileMediator::detachAll(KisTileMgr *mgr)
{
	KisTileMediatorSingleton *mediator = KisTileMediatorSingleton::singleton();

	mediator -> detachAll(mgr);
}

