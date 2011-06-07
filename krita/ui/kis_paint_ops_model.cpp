/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_paint_ops_model.h"

#include "kis_debug.h"
#include <kis_paintop_registry.h>
#include <kis_paintop_factory.h>
#include <kstandarddirs.h>
#include "kis_factory2.h"

bool categorySortFunc(const QString& a, const QString& b) {
    return a > b;
}

QVariant KisPaintOpListModel::data(const QModelIndex& idx, int role) const
{
    if(idx.isValid() && role == Qt::DecorationRole) {
        BaseClass::Index index = getIndex(idx.row());
        
        if(!BaseClass::isHeader(index))
            return BaseClass::m_categories[index.first].entries[index.second].data.icon;
        
        return QVariant();
    }
    
    return BaseClass::data(idx, role);
}

void KisPaintOpListModel::fill(const QList<KisPaintOpFactory*>& list)
{
    BaseClass::clear();
    
    typedef QList<KisPaintOpFactory*>::const_iterator Iterator;
    
    for(Iterator itr=list.begin(); itr!=list.end(); ++itr) {
        KisPaintOpFactory* op       = *itr;
        QString            fileName = KisFactory2::componentData().dirs()->findResource("kis_images", op->pixmap());
        QPixmap            pixmap(fileName);
        
        if(pixmap.isNull()){
            pixmap = QPixmap(22,22);
            pixmap.fill();
        }
        
        BaseClass::addEntry(op->category(), KisPaintOpInfo(op->id(), op->name(), op->category(), pixmap, op->priority()));
    }
    
    BaseClass::sortCategories(categorySortFunc);
    BaseClass::sortEntries();
}

int KisPaintOpListModel::indexOf(const KisPaintOpFactory* op) const
{
    return BaseClass::indexOf(KisPaintOpInfo(op->id(), op->category())).row();
}
