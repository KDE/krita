/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_brush_hud_properties_list.h"


struct KisBrushHudPropertiesList::Private
{
};

KisBrushHudPropertiesList::KisBrushHudPropertiesList(QWidget *parent)
    : QListWidget(parent),
      m_d(new Private)
{
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
}

KisBrushHudPropertiesList::~KisBrushHudPropertiesList()
{
}

void KisBrushHudPropertiesList::addProperties(const QList<KisUniformPaintOpPropertySP> &properties)
{
    int index = 0;
    Q_FOREACH (KisUniformPaintOpPropertySP prop, properties) {
        QListWidgetItem *item = new QListWidgetItem(prop->name(), this);
        item->setData(Qt::UserRole, prop->id());
        addItem(item);
        index++;
    }
}

QList<QString> KisBrushHudPropertiesList::selectedPropertiesIds() const
{
    QList<QString> ids;

    for (int i = 0; i < count(); i++) {
        ids << item(i)->data(Qt::UserRole).toString();
    }

    return ids;
}

Qt::DropActions KisBrushHudPropertiesList::supportedDropActions() const
{
    // we cannot change drop actions to Move only! It stops working
    // for some reason :(
    return QListWidget::supportedDropActions();
}
