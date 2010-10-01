/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef _KIS_PAINT_OPS_MODEL_H_
#define _KIS_PAINT_OPS_MODEL_H_

#include <QAbstractListModel>
#include <QPixmap>

class KisPaintOpFactory;

/**
 * This model can be use to show a list of paint ops in a list view.
 */
class KisPaintOpsModel : public QAbstractListModel
{
public:
    
    enum AdditionalRoles {
        PaintOpSortRole = 0xF1DFDB
    };
    
public:
    
    KisPaintOpsModel(const QList<KisPaintOpFactory*>& list);
    ~KisPaintOpsModel();
    
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    
    const QString& itemAt(const QModelIndex & index) const;
    QModelIndex indexOf(const KisPaintOpFactory*) const;
    /**
     * @return the index for the given paint op id
     */
    QModelIndex indexOf(const QString&) const;

    
private:
    struct PaintOpInfo {
        PaintOpInfo(QString _id, QString _name, QString _category, QPixmap _icon, int _priority) : id(_id), name(_name), category(_category), icon(_icon), priority(_priority) {}
        QString id;
        QString name;
        QString category;
        QPixmap icon;
        int priority;

        bool operator==(const PaintOpInfo info) const
        {
            return ((info.id == id) && (info.name == name) && (info.category == category) && (info.priority == priority));
        }
        
        bool operator<( const PaintOpInfo & other ) const{
            if (priority < other.priority) {
                return true;
            } else
            if ((priority == other.priority) && (name < other.name)) {
                return true;
            }
            return false;
        }
    };
    
    QList< PaintOpInfo > m_list;
};

#endif
