/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
