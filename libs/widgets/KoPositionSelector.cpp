/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoPositionSelector.h"

#include <QRadioButton>
#include <QGridLayout>
#include <QButtonGroup>
#include <QButtonGroup>
#include <QPainter>
#include <kdebug.h>
#include <QStyleOption>

#define GAP 0

class KoPositionSelector::Private {
public:
    Private()
        : position(KoFlake::TopLeftCorner)
    {
        topLeft = createButton(KoFlake::TopLeftCorner);
        topLeft->setChecked(true);
        topRight = createButton(KoFlake::TopRightCorner);
        center = createButton(KoFlake::CenteredPosition);
        bottomRight = createButton(KoFlake::BottomRightCorner);
        bottomLeft = createButton(KoFlake::BottomLeftCorner);
    }

    QRadioButton *createButton(int id) {
        QRadioButton *b = new QRadioButton();
        buttonGroup.addButton(b, id);
        return b;
    }

    QRadioButton *topLeft, *topRight, *center, *bottomRight, *bottomLeft;
    QButtonGroup buttonGroup;
    KoFlake::Position position;
};

class RadioLayout : public QLayout {
public:
    RadioLayout(QWidget *parent)
        : QLayout(parent)
    {
    }

    ~RadioLayout()
    {
        foreach( const Item & item, items )
            delete item.child;
        items.clear();
    }

    void setGeometry (const QRect &geom) {
        QSize prefSize = calcSizes();

        qreal columnWidth, rowHeight;
        if (geom.width() <= minimum.width())
            columnWidth = geom.width() / (qreal) maxCol;
        else
            columnWidth = prefSize.width() + GAP;
        if (geom.height() <= minimum.height())
            rowHeight = geom.height() / (qreal) maxRow;
        else
            rowHeight = prefSize.height() + GAP;
        // padding inside row and column so that radio button is centered
        QPoint padding( qRound(0.5 * (columnWidth - prefSize.width())), qRound(0.5 * (rowHeight - prefSize.height())));
        // offset so that all the radio button are centered within the widget
        qreal offsetX = 0.5 * (geom.width()- static_cast<qreal>(maxCol) * columnWidth);
        qreal offsetY = 0.5 * (geom.height() - static_cast<qreal>(maxRow) * rowHeight);
        QPoint offset( qRound(offsetX), qRound(offsetY));
        foreach(const Item & item, items) {
            QPoint point( qRound(item.column * columnWidth), qRound(item.row * rowHeight) );
            QRect rect(point + offset + padding + geom.topLeft(), prefSize);
            item.child->setGeometry(rect);
        }
    }

    QSize calcSizes() {
        QSize prefSize;
        maxRow = 0;
        maxCol = 0;
        foreach(const Item & item, items) {
            if(prefSize.isEmpty()) {
                QAbstractButton *but = dynamic_cast<QAbstractButton*> (item.child->widget());
                Q_ASSERT(but);
                QStyleOptionButton opt;
                opt.initFrom(but);
                prefSize = QSize(but->style()->pixelMetric(QStyle::PM_ExclusiveIndicatorWidth, &opt, but),
                        but->style()->pixelMetric(QStyle::PM_ExclusiveIndicatorHeight, &opt, but));
            }
            maxRow = qMax(maxRow, item.row);
            maxCol = qMax(maxCol, item.column);
        }
        maxCol++; maxRow++; // due to being zero-based.
        preferred = QSize(maxCol * prefSize.width() + (maxCol-1) * GAP, maxRow * prefSize.height() + (maxRow-1) * GAP);
        minimum = QSize(maxCol * prefSize.width(), maxRow * prefSize.height());
        return prefSize;
    }

    QLayoutItem *itemAt (int index) const {
        if( index < count() )
            return items.at(index).child;
        else
            return 0;
    }

    QLayoutItem *takeAt (int index) {
        Q_ASSERT(index < count());
        Item item = items.takeAt(index);
        return item.child;
    }

    int count () const {
        return items.count();
    }

    void addItem(QLayoutItem *) {
        Q_ASSERT(0);
    }

    QSize sizeHint() const {
        if(preferred.isEmpty())
            const_cast<RadioLayout*> (this)->calcSizes();
        return preferred;
    }

    QSize minimumSize() const {
        if(minimum.isEmpty())
            const_cast<RadioLayout*> (this)->calcSizes();
        return minimum;
    }

    void addWidget(QRadioButton *widget, int row, int column) {
        addChildWidget(widget);
        Item newItem;
        newItem.child = new QWidgetItem(widget);
        newItem.row = row;
        newItem.column = column;
        items.append(newItem);
    }

private:
    struct Item {
        QLayoutItem *child;
        int column;
        int row;
    };
    QList<Item> items;
    QSize preferred, minimum;
    int maxCol, maxRow;
};

KoPositionSelector::KoPositionSelector(QWidget *parent)
    : QWidget(parent),
    d(new Private())
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    RadioLayout *lay = new RadioLayout(this);
    lay->addWidget(d->topLeft, 0, 0);
    lay->addWidget(d->topRight, 0, 2);
    lay->addWidget(d->center, 1, 1);
    lay->addWidget(d->bottomRight, 2, 2);
    lay->addWidget(d->bottomLeft, 2, 0);
    setLayout(lay);

    connect(&d->buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(positionChanged(int)));
}

KoPositionSelector::~KoPositionSelector() {
    delete d;
}

KoFlake::Position KoPositionSelector::position() const {
    return d->position;
}

void KoPositionSelector::setPosition(KoFlake::Position position) {
    d->position = position;
    switch(d->position) {
        case KoFlake::TopLeftCorner:
            d->topLeft->setChecked(true);
            break;
        case KoFlake::TopRightCorner:
            d->topRight->setChecked(true);
            break;
        case KoFlake::CenteredPosition:
            d->center->setChecked(true);
            break;
        case KoFlake::BottomLeftCorner:
            d->bottomLeft->setChecked(true);
            break;
        case KoFlake::BottomRightCorner:
            d->bottomRight->setChecked(true);
            break;
    }
}

void KoPositionSelector::positionChanged(int position) {
    d->position = static_cast<KoFlake::Position> (position);
    emit positionSelected(d->position);
}

void KoPositionSelector::paintEvent (QPaintEvent *) {
    QPainter painter( this );
    QPen pen(Qt::black);
    int width;
    if (d->topLeft->width() %2 == 0)
        width = 2;
    else
        width = 3;
    pen.setWidth(width);
    painter.setPen(pen);
    painter.drawRect( QRect( d->topLeft->geometry().center(), d->bottomRight->geometry().center() ) );
    painter.end();
}
