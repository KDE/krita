/* This file is part of the KDE project

   Copyright (C) 2006, 2010 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2006, 2010 Thomas Zander <zander@kde.org>
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KOCANVASBASE_H
#define KOCANVASBASE_H

#include <QPoint>

#include "kritaflake_export.h"

class KUndo2Command;

class KoUnit;
class KoCanvasResourceProvider;
class KoShapeManager;
class KoToolProxy;
class KoViewConverter;
class KoShapeController;
class KoShapeControllerBase;
class KoCanvasController;
class KoShape;
class KoSnapGuide;
class KoSelectedShapesProxy;

class QWidget;
class QCursor;
class QPointF;
class QRectF;
class QSizeF;

#include <QObject>

/**
 * KoCanvasBase is the interface actual application canvas classes
 * should implement. Flake tools know about the canvas, so they can
 * do things like scroll, redraw, set a cursor etc.
 */
class KRITAFLAKE_EXPORT KoCanvasBase : public QObject
{
    Q_OBJECT
public:

    /**
     * The constructor.
     * @param shapeController the implementation of the shapeController that the
     *   application provides to allow shapes to be added in multiple views.
     */
    explicit KoCanvasBase(KoShapeControllerBase *shapeController, KoCanvasResourceProvider *sharedResourceManager = 0);
    ~KoCanvasBase() override;

public:

    /**
     * @return true if opengl can be used directly on the canvas
     */
    virtual bool canvasIsOpenGL() const { return false; }

    /**
     * retrieve the grid size setting.
     * The grid spacing will be provided in pt.
     * @param horizontal a pointer to a qreal that will be filled with the horizontal grid-spacing
     * @param vertical a pointer to a qreal that will be filled with the vertical grid-spacing
     */
    virtual void gridSize(QPointF *offset, QSizeF *spacing) const = 0;

    /**
     * return if snap to grid is enabled.
     * @return if snap to grid is enabled.
     */
    virtual bool snapToGrid() const = 0;


    /**
     * set the specified cursor on this canvas
     *
     * @param cursor the new cursor
     * @return the old cursor
     */
    virtual void setCursor(const QCursor &cursor) = 0;

    /**
     * Adds a command to the history. Call this for each @p command you create.
     * This will also execute the command.
     * This means, most of the application's code will look like
     *    MyCommand * cmd = new MyCommand( parameters );
     *    canvas.addCommand( cmd );
     *
     * Note that the command history takes ownership of the command, it will delete
     * it when the undo limit is reached, or when deleting the command history itself.
     * @param command the command to add
     */
    virtual void addCommand(KUndo2Command *command) = 0;

    /**
     * Return the current shapeManager. WARNING: the shape manager can switch
     * in time, e.g. when a layer is changed. Please don't keep any persistent
     * connections to it. Instead please use selectedShapesProxy(),
     * which is guaranteed to be the same throughout the life of the canvas.
     *
     * @return the current shapeManager
     */
    virtual KoShapeManager *shapeManager() const = 0;

    /**
     * @brief selectedShapesProxy() is a special interface for keeping a persistent connections
     * to selectionChanged() and selectionContentChanged() signals. While shapeManager() can change
     * throughout the life time of the cavas, selectedShapesProxy() is guaranteed to stay the same.
     * @return persistent KoSelectedShapesProxy object
     */
    virtual KoSelectedShapesProxy *selectedShapesProxy() const = 0;

    /**
     * Tell the canvas to repaint the specified rectangle. The coordinates
     * are document coordinates, not view coordinates.
     */
    virtual void updateCanvas(const QRectF &rc) = 0;

    /**
     * Return the proxy to the active tool (determining which tool
     * is really, really active is hard when tablets are involved,
     * so leave that to others.
     */
    virtual KoToolProxy *toolProxy() const = 0;

    /**
     * Return the viewConverter for this view.
     * @return the viewConverter for this view.
     */
    virtual KoViewConverter *viewConverter() const = 0;

    /**
     * Convert a coordinate in pixels to pt.
     * @param viewPoint the point in the coordinate system of the widget, or window.
     */
    virtual QPointF viewToDocument(const QPointF &viewPoint) const;

    /**
     * Return the widget that will be added to the scrollArea.
     */
    virtual QWidget *canvasWidget() = 0;

    /**
     * Return the widget that will be added to the scrollArea.
     */
    virtual const QWidget *canvasWidget() const = 0;

    /**
     * Return the unit of the current document for initialization of the widgets created
     * by the flake framework.
     * @see KoDocument::unit()
     */
    virtual KoUnit unit() const = 0;

    /**
     * Called when the user tries to move the argument shape to allow the application to limit the
     * users movement to stay within the document bounds.
     * An implementation can alter the parameter move to make sure that if the distance moved
     * is applied to the shape it will not become unreachable for the user.
     * The default implementation does not restrict movement.
     * @param shape the shape that will be moved soon.
     * @param move the distance the caller intends to move the shape.
     */
    virtual void clipToDocument(const KoShape *shape, QPointF &move) const;

    /**
     * Return the position of the document origin inside the canvas widget, in pixels.
     * By default the origin of the canvas widget and the position of the
     * document origin are coincident, thus an empty point is returned.
     */
    virtual QPoint documentOrigin() const {
        return QPoint(0, 0);
    }

    /**
     * This method should somehow call QWidget::updateMicroFocus() on the canvas widget.
     */
    virtual void updateInputMethodInfo() = 0;

    /**
     * disconnect the given QObject completely and utterly from any and all
     * connections it has to any QObject owned by the canvas. Do this in
     * the setCanvas of every KoCanvasObserver.
     */
    virtual void disconnectCanvasObserver(QObject *object);

    /**
     * Return a pointer to the resource manager associated with this
     * canvas. The resource manager contains per-canvas settings such
     * as current foreground and background color.
     * If instead of per-canvas resources you need per-document resources
     * you can by going via the shapeController instead;
     * @code
     *   canvasBase->shapeController()->resourceManager();
     * @endcode
     * @see KoShapeController::resourceManager()
     */
    KoCanvasResourceProvider *resourceManager() const;

    /**
     * Return the shape controller for this canvas.
     * A shape controller is used to create or delete shapes and show the relevant dialogs to the user.
     */
    KoShapeController *shapeController() const;

    /**
     * Return the canvas controller for this canvas.
     */
    KoCanvasController *canvasController() const;

    /**
     * @brief Scrolls the content of the canvas so that the given rect is visible.
     *
     * The rect is to be specified in document coordinates.
     *
     * @param rect the rectangle to make visible
     */
    virtual void ensureVisible(const QRectF &rect);

    /**
     * Returns the snap guide of the canvas
     */
    KoSnapGuide *snapGuide() const;

    /// called by KoCanvasController to set the controller that handles this canvas.
    void setCanvasController(KoCanvasController *controller);

private:
    // we need a KoShapeControllerBase so that it can work
    KoCanvasBase();

    class Private;
    Private * const d;
};

#endif // KOCANVASBASE_H
