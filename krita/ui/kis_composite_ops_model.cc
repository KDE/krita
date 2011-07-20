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

#include "kis_composite_ops_model.h"
#include <KoCompositeOp.h>

#include <kcategorizedsortfilterproxymodel.h>
#include "kis_debug.h"

struct CompositeOpModelInitializer
{
    CompositeOpModelInitializer() {
        model.addCategory(KoID("favorites", i18n("Favorites")));
        model.addEntries(KoCompositeOpRegistry::instance().getCompositeOps());
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
