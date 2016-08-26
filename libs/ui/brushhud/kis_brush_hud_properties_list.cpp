/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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
