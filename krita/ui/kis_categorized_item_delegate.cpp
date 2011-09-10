/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_categorized_item_delegate.h"
#include "kis_categorized_list_model.h"
#include <QPainter>

KisCategorizedItemDelegate::KisCategorizedItemDelegate(QAbstractListModel* model, bool indicateError):
    m_model(model),
    m_indicateError(indicateError),
    m_minimumItemHeight(0)
{
    m_errorIcon = QIcon::fromTheme("dialog-warning");
}

void KisCategorizedItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    qint32 border = 0;//4;
    qint32 xpos   = border + option.rect.x() + option.rect.height();
    qint32 ypos   = option.rect.y();
    qint32 size   = option.rect.height();
    QRect  rect(xpos, ypos, option.rect.width()-xpos, option.rect.height());
    
    painter->resetTransform();
    
    if(!m_model->data(index, IsHeaderRole).toBool()) {
// 		QStyledItemDelegate::paint(painter, option, index);
        QStyleOptionViewItem sovi = option;
        
        if(m_indicateError)
            sovi.rect = rect;
        
        QStyledItemDelegate::paint(painter, sovi, index);
        
        if(m_indicateError && !(m_model->flags(index) & Qt::ItemIsEnabled))
            m_errorIcon.paint(painter, 0, ypos, size, size, Qt::AlignCenter, QIcon::Normal, QIcon::On);
    }
    else {
        if(option.state & QStyle::State_MouseOver)
            painter->fillRect(option.rect, Qt::gray);
        else
            painter->fillRect(option.rect, Qt::lightGray);
        
        painter->drawText(option.rect, m_model->data(index).toString(), QTextOption(Qt::AlignVCenter|Qt::AlignHCenter));
        
        paintTriangle(
            painter,
            option.rect.x(),
            option.rect.y(),
            size,
            !m_model->data(index, ExpandCategoryRole).toBool()
        );
    }
    
    painter->resetTransform();
}

QSize KisCategorizedItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    //on first calling this calculates the mininmal height of the items
    if(m_minimumItemHeight == 0) {
        for(int i=0; i<index.model()->rowCount(); i++) {
            QSize indexSize = QStyledItemDelegate::sizeHint(option, index.model()->index(i, 0));
            m_minimumItemHeight = qMax(m_minimumItemHeight, indexSize.height());
        }
    }
    
    return QSize(QStyledItemDelegate::sizeHint(option, index).width(), m_minimumItemHeight);
}

void KisCategorizedItemDelegate::paintTriangle(QPainter* painter, qint32 x, qint32 y, qint32 size, bool rotate) const
{
    QPolygonF triangle;
    triangle.push_back(QPointF(-0.2,-0.2));
    triangle.push_back(QPointF( 0.2,-0.2));
    triangle.push_back(QPointF( 0.0, 0.2));
    
    painter->translate(x + size/2, y + size/2);
    painter->scale(size, size);
    
    if(rotate)
        painter->rotate(-90);
    
    painter->setBrush(QBrush(Qt::black));
    painter->drawPolygon(triangle);
}
