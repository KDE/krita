/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_GENERIC_REGISTRY_H_
#define _KIS_GENERIC_REGISTRY_H_

#include <map>

#include <qstring.h>

#include <kdebug.h>

template<typename _T>
class KisGenericRegistry {
	typedef std::map<QString, _T> storageMap;
public:
	virtual ~KisGenericRegistry() { };
protected:
	KisGenericRegistry() { };
public:
	void add(_T item)
	{
		m_storage.insert( typename storageMap::value_type( item->name(), item) );
		kdDebug() << "Added to registry: " << item -> name() << "\n";
	}
	_T get(const QString& name) const
	{
		_T p;
		typename storageMap::const_iterator it = m_storage.find(name);
		if (it != m_storage.end()) {
			p = it -> second;
		}
		Q_ASSERT(p);
		return p;
	}
	bool exist(const QString& name) const
	{
		typename storageMap::const_iterator it = m_storage.find(name);
		return (it != m_storage.end());
	}
	QStringList listKeys() const
	{
		QStringList list;
		typename storageMap::const_iterator it = m_storage.begin();
		typename storageMap::const_iterator endit = m_storage.end();
		while( it != endit )
		{
			list.append(it->first);
			++it;
		}
		return list;
	}

private:
	KisGenericRegistry(const KisGenericRegistry&) { };
	KisGenericRegistry operator=(const KisGenericRegistry&) { };
	storageMap m_storage;
};

#endif
