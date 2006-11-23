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

#ifndef KIS_CANVAS_H
#define KIS_CANVAS_H

#include <QObject>
#include <QWidget>

#include <KoCanvasBase.h>

#include <kis_types.h>

class KisView2;
class KoToolProxy;

class KisDummyShape;

enum KisCanvasType {
    QPAINTER,
    OPENGL,
    MITSHM
};

class KoViewConverter;

/**
 * KisCanvas2 is not an actual widget class, but rather an adapter for
 * the widget it contains, which may be either a QPainter based
 * canvas, or an OpenGL based canvas: that are the real widgets.
 */
class KisCanvas2 : public QObject, public KoCanvasBase
{

    Q_OBJECT

public:

    /**
     * Create a new canvas. The canvas manages a widget that will do
     * the actual painting: the canvas itself is not a widget.
     *
     * @param viewConverter the viewconverter for converting between
     *                       window and document coordinates.
     * @param canvasType determines which kind of canvas widget the
     *                   canvas initially creates.
     */
    KisCanvas2(KoViewConverter * viewConverter, KisCanvasType canvasType, KisView2 * view, KoShapeControllerBase * sc);

    virtual ~KisCanvas2();

    void setCanvasWidget(QWidget * widget);

public: // KoCanvasBase implementation

    virtual void gridSize(double *horizontal, double *vertical) const;

    virtual bool snapToGrid() const;

    virtual void addCommand(KCommand *command, bool execute = true);

    virtual KoShapeManager *shapeManager() const;

    virtual void updateCanvas(const QRectF& rc);

    virtual KoViewConverter *viewConverter();

    virtual QWidget* canvasWidget();

    virtual KoUnit::Unit unit();

    virtual KoToolProxy* toolProxy();

public: // KisCanvas2 methods

    void setCanvasSize(int w, int h);

    KisImageSP image();

public slots:

    /// Update the entire canvas area
    void updateCanvas();

    /// Update the given rect (in document coordinates)
    void updateCanvas( const QRect rc );

private:

    KisCanvas2(const KisCanvas2&);

private:

    class KisCanvas2Private;
    KisCanvas2Private * m_d;
};

#endif
