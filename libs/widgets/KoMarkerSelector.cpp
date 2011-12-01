/* This file is part of the KDE project
 * Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoMarkerSelector.h"

#include "KoMarker.h"
#include "KoMarkerModel.h"
#include "KoMarkerItemDelegate.h"
#include "KoPathShape.h"

#include <QList>
#include <QComboBox>
#include <QPainter>
#include <QPainterPath>
#include <QTransform>

class KoMarkerSelector::Private
{
public:
    Private(KoMarkerData::MarkerPosition position, QWidget *parent)
    : model(new KoMarkerModel(QList<KoMarker*>(), position, parent))
    {}

    KoMarkerModel *model;
};

KoMarkerSelector::KoMarkerSelector(KoMarkerData::MarkerPosition position, QWidget *parent)
: QComboBox(parent)
, d(new Private(position, this))
{
    setModel(d->model);
    setItemDelegate(new KoMarkerItemDelegate(this));
}

KoMarkerSelector::~KoMarkerSelector()
{
    delete d;
}

void KoMarkerSelector::paintEvent(QPaintEvent *pe)
{
    QComboBox::paintEvent(pe);

    QStyleOptionComboBox option;
    option.initFrom(this);
    option.frame = hasFrame();
    QRect r = style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, this);
    if (!option.frame) { // frameless combo boxes have smaller margins but styles do not take this into account
        r.adjust(-14, 0, 14, 1);
    }

    QPainter painter(this);
    bool antialiasing = painter.testRenderHint(QPainter::Antialiasing);
    if (!antialiasing) {
        painter.setRenderHint(QPainter::Antialiasing, true);
    }

    KoPathShape *pathShape = itemData(currentIndex(), Qt::DisplayRole).value<KoPathShape*>();
    if (pathShape != 0) {
        // paint marker
        QPen pen(option.palette.text(), 2);
        painter.setPen(pen);
        QPainterPath path = pathShape->pathStroke(pen);
        painter.fillPath(path, pen.brush());
        delete pathShape;
    }

    if (!antialiasing) {
        painter.setRenderHint(QPainter::Antialiasing, false);
    }
}

void KoMarkerSelector::setMarker(KoMarker *marker)
{
    int index = d->model->markerIndex(marker);
    if (index >= 0) {
        setCurrentIndex(index);
    }
}

KoMarker *KoMarkerSelector::marker() const
{
    return itemData(currentIndex(), Qt::DecorationRole).value<KoMarker*>();
}

void KoMarkerSelector::updateMarkers(const QList<KoMarker*> markers)
{
    KoMarkerModel *model = new KoMarkerModel(markers,d->model->position(), this);
    d->model = model;
    // this deletes the old model
    setModel(model);
}

QVariant KoMarkerSelector::itemData(int index, int role) const
{
    if (role == Qt::DisplayRole) {
        KoMarker *marker = d->model->marker(index, Qt::DecorationRole).value<KoMarker*>();
        KoPathShape *pathShape = new KoPathShape();
        pathShape->moveTo(QPointF(10, 15));
        pathShape->lineTo(QPointF(70, 15));
        pathShape->setMarker(marker, d->model->position());
        return QVariant::fromValue<KoPathShape*>(pathShape);
    }
    return d->model->marker(index, role);
}
