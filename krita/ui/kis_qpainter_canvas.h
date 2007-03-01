/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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
#ifndef KIS_QPAINTER_CANVAS_H
#define KIS_QPAINTER_CANVAS_H

#include <QWidget>

#include "kis_abstract_canvas_widget.h"

class QBrush;
class QImage;
class QPaintEvent;
class QMoveEvent;
class KisCanvas2;
class KoViewConverter;
class KoToolProxy;

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

    /// reimplemented method from superclass
    void keyPressEvent( QKeyEvent *e );

    /// reimplemented method from superclass
    void mouseMoveEvent(QMouseEvent *e);

    /// reimplemented method from superclass
    void mousePressEvent(QMouseEvent *e);

    /// reimplemented method from superclass
    void mouseReleaseEvent(QMouseEvent *e);

    /// reimplemented method from superclass
    void mouseDoubleClickEvent(QMouseEvent *e);

    /// reimplemented method from superclass
    void keyReleaseEvent (QKeyEvent *e);

    /// reimplemented method from superclass
    void paintEvent(QPaintEvent * ev);

    /// reimplemented method from superclass
    void tabletEvent( QTabletEvent *e );

    /// reimplemented method from superclass
    void wheelEvent( QWheelEvent *e );

    /// reimplemented method from superclass
    bool event(QEvent *event);

public: // KisAbstractCanvasWidget

    QWidget * widget() { return this; }

    KoToolProxy * toolProxy();

public slots:

    void parentSizeChanged( const QSize & size );

private:

    class Private;
    Private * m_d;
};

#endif
