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

#ifndef KOREPORT_BOUNDEDTEXTITEM_H
#define KOREPORT_BOUNDEDTEXTITEM_H

#include <QGraphicsTextItem>
#include <QFont>

/**
 * @brief Subclass of QGraphicsTextItem which simply forces
 * its boundingRect to be the same as its parent.
 * By default a QGraphicsTextItem will size to its text and
 * we want it to size to the parent item.
 * 
 */

class BoundedTextItem : public QGraphicsTextItem
{
    Q_OBJECT
    
public:
    explicit BoundedTextItem(QGraphicsItem *parent);
    virtual QRectF boundingRect() const;
    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w);
    void setBackgroudColor(const QColor &bc);
    void setForegroundColor(const QColor &fc);
    void setBackgroudOpacity(int o);
    void setDisplayFont(const QFont &f);
    
    
protected:
    virtual void keyReleaseEvent ( QKeyEvent * event );
    
private:
    QColor m_backgroundColor;
    QColor m_foregroundColor;
    QFont m_font;
    
    int m_backgroundOpacity;
    
Q_SIGNALS:
    void exitEditMode();

};

#endif // KOREPORT_BOUNDEDTEXTITEM_H
