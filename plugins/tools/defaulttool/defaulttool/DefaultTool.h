/* This file is part of the KDE project

   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2008 Thomas Zander <zander@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef DEFAULTTOOL_H
#define DEFAULTTOOL_H

#include <KoInteractionTool.h>
#include <KoFlake.h>
#include <commands/KoShapeAlignCommand.h>
#include <commands/KoShapeReorderCommand.h>

#include <QPolygonF>
#include <QTime>

class QSignalMapper;
class KoInteractionStrategy;
class KoShapeMoveCommand;
class KoSelection;
class DefaultToolTabbedWidget;
class KisViewManager;

/**
 * The default tool (associated with the arrow icon) implements the default
 * interactions you have with flake objects.<br>
 * The tool provides scaling, moving, selecting, rotation and soon skewing of
 * any number of shapes.
 * <p>Note that the implementation of those different strategies are delegated
 * to the InteractionStrategy class and its subclasses.
 */
class DefaultTool : public KoInteractionTool
{
    Q_OBJECT
public:
    /**
     * Constructor for basic interaction tool where user actions are translated
     * and handled by interaction strategies of type KoInteractionStrategy.
     * @param canvas the canvas this tool will be working for.
     */
    explicit DefaultTool(KoCanvasBase *canvas);
    ~DefaultTool() override;

    enum CanvasResource {
        HotPosition = 1410100299
    };

public:

    bool wantsAutoScroll() const override;
    void paint(QPainter &painter, const KoViewConverter &converter) override;

    void repaintDecorations() override;

    ///reimplemented
    void copy() const override;

    ///reimplemented
    void deleteSelection() override;

    ///reimplemented
    bool paste() override;
    ///reimplemented
    KoToolSelection *selection() override;

    QMenu* popupActionsMenu() override;

    /**
     * Returns which selection handle is at params point (or NoHandle if none).
     * @return which selection handle is at params point (or NoHandle if none).
     * @param point the location (in pt) where we should look for a handle
     * @param innerHandleMeaning this boolean is altered to true if the point
     *   is inside the selection rectangle and false if it is just outside.
     *   The value of innerHandleMeaning is undefined if the handle location is NoHandle
     */
    KoFlake::SelectionHandle handleAt(const QPointF &point, bool *innerHandleMeaning = 0);

public Q_SLOTS:
    void activate(ToolActivation activation, const QSet<KoShape *> &shapes) override;
    void deactivate() override;

private Q_SLOTS:
    void selectionAlign(int _align);
    void selectionDistribute(int _distribute);

    void selectionBringToFront();
    void selectionSendToBack();
    void selectionMoveUp();
    void selectionMoveDown();

    void selectionGroup();
    void selectionUngroup();

    void selectionTransform(int transformAction);
    void selectionBooleanOp(int booleanOp);
    void selectionSplitShapes();

    void slotActivateEditFillGradient(bool value);
    void slotActivateEditStrokeGradient(bool value);

protected Q_SLOTS:
    /// Update actions on selection change
    void updateActions();

public: // Events

    void mousePressEvent(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;
    void mouseReleaseEvent(KoPointerEvent *event) override;
    void mouseDoubleClickEvent(KoPointerEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void explicitUserStrokeEndRequest() override;
protected:
    QList<QPointer<QWidget> > createOptionWidgets() override;

    KoInteractionStrategy *createStrategy(KoPointerEvent *event) override;

protected:
    friend class SelectionInteractionStrategy;
    virtual bool isValidForCurrentLayer() const;
    virtual KoShapeManager *shapeManager() const;
    virtual KoSelection *koSelection() const;

    /**
     * Enable/disable actions specific to the tool (vector vs. reference images)
     */
    virtual void updateDistinctiveActions(const QList<KoShape*> &editableShapes);

    void addTransformActions(QMenu *menu) const;
    QScopedPointer<QMenu> m_contextMenu;

private:
    class MoveGradientHandleInteractionFactory;

private:
    void setupActions();
    void recalcSelectionBox(KoSelection *selection);
    void updateCursor();
    /// Returns rotation angle of given handle of the current selection
    qreal rotationOfHandle(KoFlake::SelectionHandle handle, bool useEdgeRotation);

    void addMappedAction(QSignalMapper *mapper, const QString &actionId, int type);

    void selectionReorder(KoShapeReorderCommand::MoveShapeType order);
    bool moveSelection(int direction, Qt::KeyboardModifiers modifiers);

    /// Returns selection rectangle adjusted by handle proximity threshold
    QRectF handlesSize();


    void canvasResourceChanged(int key, const QVariant &res) override;

    KoFlake::SelectionHandle m_lastHandle;
    KoFlake::AnchorPosition m_hotPosition;
    bool m_mouseWasInsideHandles;
    QPointF m_selectionBox[8];
    QPolygonF m_selectionOutline;
    QPointF m_lastPoint;

    // TODO alter these 3 arrays to be static const instead
    QCursor m_sizeCursors[8];
    QCursor m_rotateCursors[8];
    QCursor m_shearCursors[8];
    qreal m_angle;
    KoToolSelection *m_selectionHandler;
    friend class SelectionHandler;

    DefaultToolTabbedWidget *m_tabbedOptionWidget;
};

#endif
