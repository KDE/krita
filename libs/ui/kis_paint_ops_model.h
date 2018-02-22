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
#include <kritaui_export.h>
#include "kis_categorized_list_model.h"
#include <brushengine/kis_paintop_factory.h>

class KisPaintOpFactory;

struct KRITAUI_EXPORT KisPaintOpInfo
{
    KisPaintOpInfo() { }
    KisPaintOpInfo(const QString& _id, const QString& _name, const QString& _category, const QIcon& _icon, qint32 _priority):
        id(_id),  name(_name), category(_category), icon(_icon), priority(_priority) { }

    KisPaintOpInfo(const QString& _id):
        id(_id) { }

    bool operator==(const KisPaintOpInfo info) const{
        return (info.id == id);
    }

    QString id;
    QString name;
    QString category;
    QIcon icon;
    qint32  priority;
};

struct PaintOpInfoToQStringConverter {
    QString operator() (const KisPaintOpInfo &info) {
        return info.name;
    }
};

typedef KisCategorizedListModel<KisPaintOpInfo, PaintOpInfoToQStringConverter> BasePaintOpCategorizedListModel;

class KRITAUI_EXPORT KisPaintOpListModel : public BasePaintOpCategorizedListModel
{
public:
    KisPaintOpListModel(QObject *parent);
    QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
    void fill(const QList<KisPaintOpFactory*>& list);
};

class KRITAUI_EXPORT KisSortedPaintOpListModel : public KisSortedCategorizedListModel<KisPaintOpListModel>
{
public:
    KisSortedPaintOpListModel(QObject *parent)
        : KisSortedCategorizedListModel<KisPaintOpListModel>(parent),
          m_model(new KisPaintOpListModel(this))
    {
        initializeModel(m_model);
    }

    void fill(const QList<KisPaintOpFactory*> &list) {
        m_model->fill(list);
    }

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override {
        return lessThanPriority(left, right, KisPaintOpFactory::categoryStable());
    }

private:
    KisPaintOpListModel *m_model;
};

#endif //_KIS_PAINTOP_LIST_MODEL_H_
