/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <KoCompositeOp.h>

#include <KoIcon.h>

#include "kis_composite_ops_model.h"
#include "kis_debug.h"
#include "kis_config.h"

struct CompositeOpModelInitializer
{
    CompositeOpModelInitializer() {
        model.addEntries(KoCompositeOpRegistry::instance().getCompositeOps(), false, true);
		model.expandAllCategories(false);
        model.addCategory(KoID("favorites", i18n("Favorites")));
        model.readFavriteCompositeOpsFromConfig();
        model.expandCategory(KoID("favorites"), true);
    }
    
    KisCompositeOpListModel model;
};

KisCompositeOpListModel* KisCompositeOpListModel::sharedInstance()
{
    static CompositeOpModelInitializer initializer;
    return &initializer.model;
}

void KisCompositeOpListModel::validateCompositeOps(const KoColorSpace* colorSpace)
{
    typedef QList<Category>::iterator Itr;
    
    emit layoutAboutToBeChanged();
    
    for(Iterator cat=m_categories.begin(); cat!=m_categories.end(); ++cat) {
        for(int i=0; i<cat->entries.size(); ++i) {
            bool enable = KoCompositeOpRegistry::instance().colorSpaceHasCompositeOp(colorSpace, cat->entries[i].data);
            cat->entries[i].disabled = !enable;
        }
    }
    
    emit layoutChanged();
}

bool KisCompositeOpListModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
	KoID entry;
	bool result = BaseClass::setData(idx, value, role);
	
	if(role == Qt::CheckStateRole && BaseClass::entryAt(entry, idx.row())) {
		if(value.toInt() == Qt::Checked)
			BaseClass::addEntry(KoID("favorites"), entry);
		else
			BaseClass::removeEntry(KoID("favorites"), entry);
		
		writeFavoriteCompositeOpsToConfig();
	}
	
	return result;
}

QVariant KisCompositeOpListModel::data(const QModelIndex& idx, int role) const
{
    if(idx.isValid() && role == Qt::DecorationRole) {
        BaseClass::Index index = BaseClass::getIndex(idx.row());
        
        if(!BaseClass::isHeader(index) && BaseClass::m_categories[index.first].entries[index.second].disabled)
            return koIcon("dialog-warning");
    }
    
    return BaseClass::data(idx, role);
}

void KisCompositeOpListModel::readFavriteCompositeOpsFromConfig()
{
	KisConfig   config;
	QStringList compositeOps = config.favoriteCompositeOps();
	
	BaseClass::clearCategory(KoID("favorites"));
	
	for(QStringList::iterator i=compositeOps.begin(); i!=compositeOps.end(); ++i) {
		KoID entry = KoCompositeOpRegistry::instance().getKoID(*i);
		setData(BaseClass::indexOf(entry), Qt::Checked, Qt::CheckStateRole);
	}
}

void KisCompositeOpListModel::writeFavoriteCompositeOpsToConfig() const
{
	QList<KoID> compositeOps;
	
	if(BaseClass::getCategory(compositeOps, KoID("favorites"))) {
		QStringList list;
		KisConfig   config;
		
		for(QList<KoID>::iterator i=compositeOps.begin(); i!=compositeOps.end(); ++i)
			list.push_back(i->id());
		
		config.setFavoriteCompositeOps(list);
	}
}
