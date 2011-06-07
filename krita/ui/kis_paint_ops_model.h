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

#ifndef _KIS_PAINTOP_LIST_MODEL_H_
#define _KIS_PAINTOP_LIST_MODEL_H_

#include <QAbstractListModel>
#include <QPixmap>
#include <krita_export.h>
#include "kis_categorized_list_model.h"

class KisPaintOpFactory;

struct KRITAUI_EXPORT KisPaintOpInfo
{
    KisPaintOpInfo() { }
    KisPaintOpInfo(const KisPaintOpInfo& v):
        id(v.id),  name(v.name), category(v.category), icon(v.icon), priority(v.priority) { }
    
    KisPaintOpInfo(const QString& _id, const QString& _name, const QString& _category, const QPixmap& _icon, qint32 _priority):
        id(_id),  name(_name), category(_category), icon(_icon), priority(_priority) { }
    
    KisPaintOpInfo(const QString& _id):
        id(_id) { }
    
        KisPaintOpInfo(const QString& _id, const QString& _category) :
            id(_id), category(_category) { }
    
    bool operator==(const KisPaintOpInfo info) const{
        return (info.id == id);// && (info.category == category); //((info.id == id) && (info.name == name) && (info.category == category) && (info.priority == priority));
    }
    
    bool operator<( const KisPaintOpInfo& other ) const{
        if(priority < other.priority)
            return true;
        else if((priority == other.priority) && (name < other.name))
            return true;
    
        return false;
    }
    
    QString id;
    QString name;
    QString category;
    QPixmap icon;
    qint32  priority;
};

class KRITAUI_EXPORT KisPaintOpListModel: public KisCategorizedListModel<QString,KisPaintOpInfo>
{
    typedef KisCategorizedListModel<QString,KisPaintOpInfo> BaseClass;
    
public:
    virtual QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const;
    void fill(const QList<KisPaintOpFactory*>& list);
    int indexOf(const KisPaintOpFactory* op) const;
    using BaseClass::indexOf;
    
    virtual QString categoryToString (const QString&        val) const { return val;      }
    virtual QString entryToString    (const KisPaintOpInfo& val) const { return val.name; }
};

#endif //_KIS_PAINTOP_LIST_MODEL_H_
