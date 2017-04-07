/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoLineStyleSelector.h"
#include "KoLineStyleModel_p.h"
#include "KoLineStyleItemDelegate_p.h"

#include <QPen>
#include <QPainter>

class Q_DECL_HIDDEN KoLineStyleSelector::Private
{
public:
    Private(QWidget *parent)
    : model(new KoLineStyleModel(parent))
    {
    }

    KoLineStyleModel *model;
};

KoLineStyleSelector::KoLineStyleSelector(QWidget *parent)
    : QComboBox(parent), d(new Private(this))
{
    setModel(d->model);
    setItemDelegate(new KoLineStyleItemDelegate(this));
}

KoLineStyleSelector::~KoLineStyleSelector()
{
    delete d;
}

void KoLineStyleSelector::paintEvent(QPaintEvent *pe)
{
    QComboBox::paintEvent(pe);

    QStyleOptionComboBox option;
    option.initFrom(this);
    option.frame = hasFrame();
    QRect r = style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, this);
    if (!option.frame) // frameless combo boxes have smaller margins but styles do not take this into account
        r.adjust(-14, 0, 14, 1);

    QPen pen = itemData(currentIndex(), Qt::DecorationRole).value<QPen>();

    QPainter painter(this);
    painter.setPen(pen);
    if (!(option.state & QStyle::State_Enabled)) {
        painter.setOpacity(0.5);
    }
    painter.drawLine(r.left(), r.center().y(), r.right(), r.center().y());
}

bool KoLineStyleSelector::addCustomStyle(const QVector<qreal> &style)
{
    return d->model->addCustomStyle(style);
}

void KoLineStyleSelector::setLineStyle(Qt::PenStyle style, const QVector<qreal> &dashes)
{
    int index = d->model->setLineStyle(style, dashes);
    if (index >= 0)
        setCurrentIndex(index);
}

Qt::PenStyle KoLineStyleSelector::lineStyle() const
{
    QPen pen = itemData(currentIndex(), Qt::DecorationRole).value<QPen>();
    return pen.style();
}

QVector<qreal> KoLineStyleSelector::lineDashes() const
{
    QPen pen = itemData(currentIndex(), Qt::DecorationRole).value<QPen>();
    return pen.dashPattern();
}
