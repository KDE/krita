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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qpair.h>
#include <qmap.h>
#include <qtl.h>
#include <qvaluelist.h>
#include "kis_types.h"
#include "kis_global.h"
#include "kisscopedlock.h"
#include "kistilemediator.h"

class KisTileMediatorSingleton {
	typedef QPair<KisTileMgrSP, Q_INT32> KisTileLink;
	typedef QValueList<KisTileLink> vKisTileLink;
	typedef vKisTileLink::iterator vKisTileLink_it;
	typedef QMap<KisTileSP, vKisTileLink> KisTileLinkMap;
	typedef KisTileLinkMap::iterator KisTileLinkMap_it;

public:
	KisTileMediatorSingleton();
	~KisTileMediatorSingleton();

public:
	void attach(KisTileSP tile, KisTileMgrSP mgr, Q_INT32 tilenum);
	void detach(KisTileSP tile, KisTileMgrSP mgr, Q_INT32 tilenum);
	void detachAll(KisTileMgr *mgr);
	Q_INT32 tileNum(KisTileSP tile, KisTileMgrSP mgr);

private:
	KisTileLinkMap m_links;
};

QMutex KisTileMediator::m_mutex;
KisTileMediatorSingleton *KisTileMediator::m_instance = 0;
Q_INT32 KisTileMediator::m_ref = 0;

KisTileMediator::KisTileMediator()
{
	KisScopedLock l(&KisTileMediator::m_mutex);

	if (KisTileMediator::m_ref == 0)
		KisTileMediator::m_instance = new KisTileMediatorSingleton;
	
	KisTileMediator::m_ref++;
}

KisTileMediator::~KisTileMediator()
{
	KisScopedLock l(&KisTileMediator::m_mutex);

	KisTileMediator::m_ref--;

	if (KisTileMediator::m_ref == 0) {
		delete KisTileMediator::m_instance;
		KisTileMediator::m_instance = 0;
	}
}

void KisTileMediator::attach(KisTileSP tile, KisTileMgrSP mgr, Q_INT32 tilenum)
{
	KisScopedLock l(&KisTileMediator::m_mutex);

	KisTileMediator::m_instance -> attach(tile, mgr, tilenum);
}

void KisTileMediator::detach(KisTileSP tile, KisTileMgrSP mgr, Q_INT32 tilenum)
{
	KisScopedLock l(&KisTileMediator::m_mutex);

	return KisTileMediator::m_instance -> detach(tile, mgr, tilenum);
}

Q_INT32 KisTileMediator::tileNum(KisTileSP tile, KisTileMgrSP mgr)
{
	KisScopedLock l(&KisTileMediator::m_mutex);

	return KisTileMediator::m_instance -> tileNum(tile, mgr);
}

void KisTileMediator::detachAll(KisTileMgr *mgr)
{
	KisScopedLock l(&KisTileMediator::m_mutex);

	KisTileMediator::m_instance -> detachAll(mgr);
}

KisTileMediatorSingleton::KisTileMediatorSingleton()
{
}

KisTileMediatorSingleton::~KisTileMediatorSingleton()
{
}

void KisTileMediatorSingleton::attach(KisTileSP tile, KisTileMgrSP mgr, Q_INT32 tilenum)
{
	if (m_links.count(tile) == 0)
		m_links[tile] = vKisTileLink();

	m_links[tile].push_back(qMakePair(mgr, tilenum));
}

void KisTileMediatorSingleton::detach(KisTileSP tile, KisTileMgrSP mgr, Q_INT32 tilenum)
{
	if (m_links.count(tile)) {
		vKisTileLink& l = m_links[tile];

		for (vKisTileLink_it it = l.begin(); it != l.end(); it++) {
			const KisTileLink& link = *it;

			if (link.first == mgr && link.second == tilenum)
				it = l.erase(it);
		}

		if (l.empty()) 
			m_links.erase(tile);
	}
}

void KisTileMediatorSingleton::detachAll(KisTileMgr *mgr)
{
	for (KisTileLinkMap_it it = m_links.begin(); it != m_links.end(); it++) {
		KisTileSP tile = it.key();
		vKisTileLink& l = it.data();

		for (vKisTileLink_it lit = l.begin(); lit != l.end(); lit++) {
			const KisTileLink& link = *lit;

			if (link.first.data() == mgr)
				lit = l.erase(lit);
		}

		if (l.empty())
			m_links.erase(it);
	}
}

Q_INT32 KisTileMediatorSingleton::tileNum(KisTileSP tile, KisTileMgrSP mgr)
{
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

