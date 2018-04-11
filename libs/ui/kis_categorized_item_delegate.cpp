/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *  Copyright (c) 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_categorized_item_delegate.h"
#include "kis_categorized_list_model.h"

#include <QPainter>
#include <QStyle>
#include <QStyleOptionMenuItem>
#include <QStyleOptionViewItem>
#include <QApplication>
#include <QTransform>

#include <KoIcon.h>

#include <kis_icon.h>
#include "kis_debug.h"

KisCategorizedItemDelegate::KisCategorizedItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
      m_minimumItemHeight(0)
{
}

void KisCategorizedItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->resetTransform();

    if(!index.data(__CategorizedListModelBase::IsHeaderRole).toBool()) {
        QStyleOptionViewItem sovi(option);

        if (index.data(__CategorizedListModelBase::isLockableRole).toBool()) {

            const int iconSize = qMax(16, m_minimumItemHeight - 2);
            bool locked = index.data(__CategorizedListModelBase::isLockedRole).toBool();


            QIcon icon = locked ? KisIconUtils::loadIcon(koIconName("layer-locked")) : KisIconUtils::loadIcon(koIconName("layer-unlocked"));

            // to be able to make an icon more transparent. we need to create a new image
            // and use the QPainter to make a more transparent version of it.
            QImage image(iconSize, iconSize, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent); // needs to happen, otherwise 'junk' pixels are used (looks bad)

            QPainter p(&image);
            p.setCompositionMode(QPainter::CompositionMode_Source);


            // make unlocked icons more hidden
            if (locked) {
                p.setOpacity(1.0);
            } else {
                p.setOpacity(0.14);
            }

            p.drawPixmap(0, 0, icon.pixmap(iconSize, QIcon::Normal) );
            p.end();

            icon = QIcon( QPixmap::fromImage(image) ); // the new icon is ready



            sovi.decorationPosition = QStyleOptionViewItem::Right;
            sovi.decorationAlignment = Qt::AlignRight;
            sovi.decorationSize = QSize(iconSize, iconSize);
            sovi.features |= QStyleOptionViewItem::HasDecoration;
            sovi.icon = icon;

        }

        QStyledItemDelegate::paint(painter, sovi, index);
        painter->setOpacity(1);
    }
    else {
        QPalette palette = QApplication::palette();
        if(option.state & QStyle::State_MouseOver)
            painter->fillRect(option.rect, palette.midlight());
        else
            painter->fillRect(option.rect, palette.button());

        painter->setBrush(palette.buttonText());
        painter->drawText(option.rect, index.data().toString(), QTextOption(Qt::AlignVCenter|Qt::AlignHCenter));

        paintTriangle(
            painter,
            option.rect.x(),
            option.rect.y(),
            option.rect.height(),
            !index.data(__CategorizedListModelBase::ExpandCategoryRole).toBool()
        );
    }
    painter->resetTransform();
}

QSize KisCategorizedItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    //on first calling this calculates the minimal height of the items
    if (m_minimumItemHeight == 0) {
        for(int i=0; i<index.model()->rowCount(); i++) {
            QSize indexSize = QStyledItemDelegate::sizeHint(option, index.model()->index(i, 0));
            m_minimumItemHeight = qMax(m_minimumItemHeight, indexSize.height());

            /**
             * Make all the items, including the ones having
             * checkboxes look the same.
             */
            QStyle *style = QApplication::style();
            QStyleOptionButton so;
            QSize size = style->sizeFromContents(QStyle::CT_CheckBox, &so, QSize(), 0);

            m_minimumItemHeight = qMax(size.height(), m_minimumItemHeight);
        }
    }

    int width = QStyledItemDelegate::sizeHint(option, index).width();

    if (index.data(__CategorizedListModelBase::isLockableRole).toBool()) {
        width += m_minimumItemHeight;
    }

    return QSize(width, m_minimumItemHeight);
}

void KisCategorizedItemDelegate::paintTriangle(QPainter* painter, qint32 x, qint32 y, qint32 size, bool rotate) const
{
    QPolygonF triangle;
    triangle.push_back(QPointF(-0.2,-0.2));
    triangle.push_back(QPointF( 0.2,-0.2));
    triangle.push_back(QPointF( 0.0, 0.2));

    QTransform transform;
    transform.translate(x + size/2, y + size/2);
    transform.scale(size, size);

    if(rotate)
        transform.rotate(-90);

    QPalette palette = QApplication::palette();
    painter->setBrush(palette.buttonText());
    painter->drawPolygon(transform.map(triangle));
}

