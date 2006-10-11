/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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
#ifndef KIS_QPAINTER_CANVAS_H
#define KIS_QPAINTER_CANVAS_H

#include <QWidget>

#include "kis_abstract_canvas_widget.h"

class QBrush;
class QImage;
class QPaintEvent;
class KisCanvas2;
class KoColorProfile;

/**
 *
 * KisQPainterCanvas is the widget that shows the actual image using arthur.
 *
 * @author Boudewijn Rempt <boud@valdyas.org>
*/
class KisQPainterCanvas : public QWidget, public KisAbstractCanvasWidget
{

    Q_OBJECT

public:

    KisQPainterCanvas( KisCanvas2 * canvas, QWidget * parent );

    virtual ~KisQPainterCanvas();


public: // QWidget

    void paintEvent ( QPaintEvent * event );

public: // KisAbstractCanvasWidget

    QWidget * widget() { return this; }

private:
    KoColorProfile *  m_monitorProfile;
    QImage * m_checkTexture;
    QBrush * m_checkBrush;
    KisCanvas2 * m_canvas;

};

#endif
