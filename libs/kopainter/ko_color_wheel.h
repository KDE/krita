/*
 * Copyright (c) 2004 Sven Langkamp <longamp@reallygood.de>
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

#ifndef KO_COLORWHEEL_H
#define KO_COLORWHEEL_H

#include <QPixmap>
//Added by qt3to4:
#include <QMouseEvent>
#include <QResizeEvent>

#include <kselector.h>
#include "koColor.h"

class QPainter;

class KOPAINTER_EXPORT KoOldColorWheel : public KXYSelector
{
  Q_OBJECT

public:
    KoOldColorWheel( QWidget *parent=0, const char *name=0 );

signals:
    void valueChanged(const KoOldColor& c);

public slots:
    virtual void slotSetValue(const KoOldColor& c);

protected:
    virtual void drawWheel( QPixmap *pixmap );
    virtual void resizeEvent( QResizeEvent * );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void drawContents( QPainter *painter );

private:
    QPixmap m_pixmap;
    KoOldColor m_color;
};

#endif
