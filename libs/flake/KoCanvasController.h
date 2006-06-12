/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOCANVASVIEW_H
#define KOCANVASVIEW_H

#include <koffice_export.h>

#include "KoCanvasBase.h"

#include <QWidget>
#include <QScrollArea>

class QGridLayout;
class QPaintEvent;

/**
 * This widget is a wrapper around your canvas providing scrollbars.
 * Flake does not provide a canvas, the application will have to extend a QWidget
 * and implement that themselves; but Flake does make it a lot easier to do so.
 * One of those things is this widget that acts as a decorator around the canvas
 * widget and provides scrollbars and allows the canvas to be centered in the viewArea
 * <p>The using application can intantiate this class and add its canvas using the
 * setCanvas() call. Which is designed so it can be called multiple times for those
 * that wish to exchange one canvas widget for another.
 */
class FLAKE_EXPORT KoCanvasController : public QScrollArea {
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param parent the parent this widget will belong to
     */
    KoCanvasController(QWidget *parent);
    virtual ~KoCanvasController() {}

    /**
     * Set the new canvas to be shown as a child
     * Calling this will emit canvasRemoved() if there was a canvas before, and will emit
     * canvasSet() with the new canvas.
     * @param canvas the new canvas. The KoCanvasBase::canvas() will be called to retrieve the
     *        actual widget which will then be added as child of this one.
     */
    void setCanvas(KoCanvasBase *canvas);
    /**
     * Return the curently set canvas
     * @return the curently set canvas
     */
    KoCanvasBase* canvas() const;

    /**
     * return the amount of pixels vertically visible of the child canvas.
     * @return the amount of pixels vertically visible of the child canvas.
     */
    int visibleHeight() const;
    /**
     * return the amount of pixels horizontally visible of the child canvas.
     * @return the amount of pixels horizontally visible of the child canvas.
     */
    int visibleWidth() const;
    /**
     * return the amount of pixels that are not visible on the left side of the canvas.
     * The leftmost pixel that is shown is returned.
     */
    int canvasOffsetX() const;
    /**
     * return the amount of pixels that are not visible on the top side of the canvas.
     * The topmost pixel that is shown is returned.
     */
    int canvasOffsetY() const;

    /**
     * Set the canvas to be displayed centered in this widget.
     * In the case that the canvas widget is smaller then this one the canvas will be centered
     * and a contrasting color used for the background.
     * @param centered center canvas if true, or aligned to the left (LTR) if false.
     *        Centered is the default value.
     */
    void centerCanvas(bool centered);
    /**
     * return the canvas centering value.
     * @return the canvas centering value
     */
    bool isCanvasCentered() const;

signals:
    /**
     * Emitted when a previously added canvas is about to be removed.
     * @param cv this object
     */
    void canvasRemoved(KoCanvasController* cv);
    /**
     * Emitted when a canvas is set on this widget
     * @param cv this object
     */
    void canvasSet(KoCanvasController* cv);

private:
    class Viewport : public QWidget {
      public:
        Viewport();
        ~Viewport() {};
        void setCanvas(QWidget *canvas);
        void removeCanvas(QWidget *canvas);
        void centerCanvas(bool centered);
      private:
        QGridLayout *m_layout;
    };

private:
    KoCanvasBase *m_canvas;
    QWidget *m_canvasWidget;
    Viewport *m_viewport;
    bool m_centerCanvas;

};

#endif
