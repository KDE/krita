/* This file is part of the KDE project

   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#include <KoUnit.h>

#include <QPointF>
#include <QPoint>
#include <QRectF>
#include <QRect>
#include <QWidget>

#include "flake_export.h"

class QGraphicsWidget;
class QUndoCommand;

class KoResourceManager;
class KoShapeManager;
class KoToolProxy;
class KoViewConverter;
class KoShapeController;
class KoShapeControllerBase;
class KoCanvasController;
class KoShape;
class KoSnapGuide;
class KoGuidesData;

/**
 * KoCanvasBase is the interface actual application canvas classes
 * should implement. Flake tools know about the canvas, so they can
 * do things like scroll, redraw, set a cursor etc.
 */
class FLAKE_EXPORT KoCanvasBase
{

public:

    /**
     * The constructor.
     * @param shapeControllerBase the implementation of the shapeController that the
     *   application provides to allow shapes to be added in multiple views.
     */
    explicit KoCanvasBase(KoShapeControllerBase *shapeControllerBase);
    virtual ~KoCanvasBase();

public:

    /**
     * @return true if opengl can be used directly on the canvas
     */
    virtual bool canvasIsOpenGL() { return false; }

    /**
     * retrieve the grid size setting.
     * The grid spacing will be provided in pt.
     * @param horizontal a pointer to a qreal that will be filled with the horizontal grid-spacing
     * @param vertical a pointer to a qreal that will be filled with the vertical grid-spacing
     */
    virtual void gridSize(qreal *horizontal, qreal *vertical) const = 0;

    /**
     * return if snap to grid is enabled.
     * @return if snap to grid is enabled.
     */
    virtual bool snapToGrid() const = 0;

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
    virtual void addCommand(QUndoCommand *command) = 0;

    /**
     * return the current shapeManager
     * @return the current shapeManager
     */
    virtual KoShapeManager *shapeManager() const = 0;

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
    virtual const KoViewConverter *viewConverter() const = 0;

    /**
     * Return the widget that will be added to the scrollArea.
     */
    virtual QWidget *canvasWidget() = 0;

    /**
     * Return the widget that will be added to the scrollArea.
     */
    virtual const QWidget *canvasWidget() const = 0;

    /**
     * Return the widget that will be added to the scrollArea.
     */
    virtual QGraphicsWidget *canvasItem() { return 0; }

    /**
     * Return the widget that will be added to the scrollArea.
     */
    virtual const QGraphicsWidget *canvasItem() const{ return 0; }

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
    KoResourceManager *resourceManager() const;

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

    /**
     * This factory method creates a new widget for the user to change
     * the snapping guide policies object from snapGuide().
     */
    QWidget *createSnapGuideConfigWidget() const;

    /**
     * Returns the guides data.
     *
     * Applications that want to have guides should reimplement this
     * function and return the KOGuideData object.
     * The default implementation returns 0.
     *
     * @return pointer to the guide data or zero if there is none
     */
    virtual KoGuidesData *guidesData();

    /**
     * Calling this will turn the canvas into a read/write or a read-only canvas.
     * Note that upon calling this method no actions will be enabled/disabled
     * as that is the responsibility of the KoToolManager. The KoToolManager
     * should use this variable at next tool switch.
     *
     * @see KoToolManager::updateReadWrite(), isReadWrite()
     */
    void setReadWrite(bool readWrite);

    /**
     * @return returns true if this canvas is marked as allowing content changing actions
     * @see setReadWrite(), KoToolManager::updateReadWrite()
     */
    bool isReadWrite() const;

    /// called by KoCanvasController to set the controller that handles this canvas.
    void setCanvasController(KoCanvasController *controller);

private:
    // we need a KoShapeControllerBase so that it can work
    KoCanvasBase();

    class Private;
    Private * const d;
};

#endif // KOCANVASBASE_H
