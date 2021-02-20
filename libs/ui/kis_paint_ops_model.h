/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 * 
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    qint32  priority {0};
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
