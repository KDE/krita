/*
 * This file is copied from ksnapshot.
 *
 * Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef REGIONGRABBER_H
#define REGIONGRABBER_H

#include <qlabel.h>
#include <qpixmap.h>

class QTimer;

class SizeTip : public QLabel
{
public:
    SizeTip( QWidget *parent, const char *name=0 );
    ~SizeTip() {}

    void setTip( const QRect &rect );
    void positionTip( const QRect &rect );
};

class RegionGrabber : public QWidget
{
    Q_OBJECT

public:
    RegionGrabber();
    ~RegionGrabber();

protected slots:
    void initGrabber();
    void updateSizeTip();

    signals:
    void regionGrabbed( const QPixmap & );

protected:
    void mousePressEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void keyPressEvent( QKeyEvent *e );

    void drawRubber();

    bool mouseDown;
    QRect grabRect;
    QPixmap pixmap;

    SizeTip *sizeTip;
    QTimer *tipTimer;
};

#endif // REGIONGRABBER_H

