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

#ifndef _KIS_COMPOSITE_OPS_MODEL_H_
#define _KIS_COMPOSITE_OPS_MODEL_H_

#include <KoID.h>
#include <QAbstractListModel>
#include "kis_categorized_list_model.h"

class KoCompositeOp;
class KoColorSpace;

class KRITAUI_EXPORT KisCompositeOpListModel: public KisCategorizedListModel<KoID,KoID>
{
    typedef KisCategorizedListModel<KoID,KoID> BaseClass;

public:
    static KisCompositeOpListModel* sharedInstance();
    
    virtual QString  categoryToString(const KoID& val) const { return val.name(); }
    virtual QString  entryToString   (const KoID& val) const { return val.name(); }
    virtual bool     setData         (const QModelIndex& idx, const QVariant& value, int role=Qt::EditRole);
    virtual QVariant data            (const QModelIndex& idx, int role=Qt::DisplayRole) const;
    
    void validateCompositeOps(const KoColorSpace* colorSpace);
    void readFavoriteCompositeOpsFromConfig();
    void writeFavoriteCompositeOpsToConfig() const;
};

#endif
