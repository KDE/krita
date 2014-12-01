/*
  KoReport report rendering library
  Copyright (C) 2014 Adam Pigg <adam@piggz.co.uk>
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "BoundedTextItem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QKeyEvent>
#include <QDebug>

BoundedTextItem::BoundedTextItem(QGraphicsItem* parent): QGraphicsTextItem(parent)
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
}

QRectF BoundedTextItem::boundingRect() const
{
    if (parentItem()) {
        return parentItem()->boundingRect();
    }
    
    return QGraphicsTextItem::boundingRect();
}

void BoundedTextItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w) 
{
    QColor bg = m_backgroundColor.isValid() ? m_backgroundColor : o->palette.base().color();
    bg.setAlphaF(m_backgroundOpacity * 0.01);
    
    QColor fc = m_foregroundColor.isValid() ? m_foregroundColor : o->palette.text().color();
    painter->setBrush(bg);
    painter->setPen(fc);

    painter->drawRect(boundingRect());
    
    QStyleOptionGraphicsItem opt(*o);
    opt.state &= ~QStyle::State_HasFocus;
    QGraphicsTextItem::paint(painter, &opt, w);
}  

void BoundedTextItem::setBackgroudColor(const QColor& bc)
{
    m_backgroundColor = bc;
}

void BoundedTextItem::setForegroundColor(const QColor& fc)
{
    m_foregroundColor = fc;
}

void BoundedTextItem::setDisplayFont(const QFont& f)
{
    m_font = f;
    setFont(m_font);
}

void BoundedTextItem::setBackgroudOpacity(int o)
{
    m_backgroundOpacity = o;
}

void BoundedTextItem::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        emit exitEditMode();
    } else {
        QGraphicsTextItem::keyReleaseEvent(event);
    }
}

#include "BoundedTextItem.moc"
