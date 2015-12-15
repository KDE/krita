/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2013
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
#ifndef SKETCHDECLARATIVEVIEW_H
#define SKETCHDECLARATIVEVIEW_H

#include <QQuickView>
#include <QPointer>
#include <opengl/kis_opengl_canvas2.h>

#include "krita_sketch_export.h"

/**
 * @brief The SketchDeclarativeView class overrides QGraphicsView's drawBackground
 */
class KRITA_SKETCH_EXPORT SketchDeclarativeView : public QQuickView
{
    Q_OBJECT

    Q_PROPERTY(bool drawCanvas READ drawCanvas WRITE setDrawCanvas NOTIFY drawCanvasChanged);
    Q_PROPERTY(QWidget* canvasWidget READ canvasWidget WRITE setCanvasWidget NOTIFY canvasWidgetChanged);

public:
    explicit SketchDeclarativeView(QWindow *parent = 0);
    explicit SketchDeclarativeView(const QUrl &url, QWindow *parent = 0);
    virtual ~SketchDeclarativeView();

    QWidget* canvasWidget() const;
    void setCanvasWidget(QWidget *canvasWidget);

    bool drawCanvas() const;
    void setDrawCanvas(bool drawCanvas);

Q_SIGNALS:
    void canvasWidgetChanged();
    void drawCanvasChanged();

protected:
    void resizeEvent(QResizeEvent *event);
    virtual bool event(QEvent* event);
    virtual bool eventFilter(QObject* watched, QEvent* e);

protected Q_SLOTS:
    void initializeImageRenderer();
    void invalidateImageRenderer();
    void synchronizeImageRenderer();
    void renderImageOnBackground();

private:

    bool m_drawCanvas;
    QPointer<KisOpenGLCanvas2> m_canvasWidget;
    bool m_GLInitialized;
    Q_SLOT void resetInitialized();
};

#endif // SKETCHDECLARATIVEVIEW_H
