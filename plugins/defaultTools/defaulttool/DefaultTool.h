/* This file is part of the KDE project

   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>

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

class KoInteractionStrategy;
class KoShapeMoveCommand;
class KoSelection;

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
    explicit DefaultTool( KoCanvasBase *canvas );
    virtual ~DefaultTool();

public:

    virtual bool wantsAutoScroll();
    virtual void paint( QPainter &painter, const KoViewConverter &converter );

    virtual void repaintDecorations();

    ///reimplemented
    virtual void copy() const;

    ///reimplemented
    virtual void deleteSelection();

    ///reimplemented
    virtual bool paste();
    ///reimplemented
    virtual QStringList supportedPasteMimeTypes() const;

    /**
     * Returns which selection handle is at params point (or NoHandle if none).
     * @return which selection handle is at params point (or NoHandle if none).
     * @param point the location (in pt) where we should look for a handle
     * @param innerHandleMeaning this boolean is altered to true if the point
     *   is inside the selection rectangle and false if it is just outside.
     *   The value of innerHandleMeaning is undefined if the handle location is NoHandle
     */
    KoFlake::SelectionHandle handleAt(const QPointF &point, bool *innerHandleMeaning = 0);

public slots:
    void activate(bool temporary = false);

private slots:
    void selectionAlignHorizontalLeft();
    void selectionAlignHorizontalCenter();
    void selectionAlignHorizontalRight();
    void selectionAlignVerticalTop();
    void selectionAlignVerticalCenter();
    void selectionAlignVerticalBottom();

    void selectionBringToFront();
    void selectionSendToBack();
    void selectionMoveUp();
    void selectionMoveDown();

public: // Events

    virtual void mousePressEvent( KoPointerEvent *event );
    virtual void mouseMoveEvent( KoPointerEvent *event );
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    virtual void mouseDoubleClickEvent( KoPointerEvent *event );

    virtual void keyPressEvent(QKeyEvent *event);

protected:
    QWidget* createOptionWidget();

    virtual KoInteractionStrategy *createStrategy(KoPointerEvent *event);

private:
    void setupActions();
    void recalcSelectionBox();
    void updateCursor();
    /// Returns rotation angle of given handle of the current selection
    double rotationOfHandle( KoFlake::SelectionHandle handle, bool useEdgeRotation );

    void selectionAlign(KoShapeAlignCommand::Align align);
    void selectionReorder(KoShapeReorderCommand::MoveShapeType order );
    bool moveSelection( int direction, Qt::KeyboardModifiers modifiers );

    QRectF handlesSize();

    // convenience method;
    KoSelection * koSelection();

    void resourceChanged( int key, const QVariant & res );


    KoFlake::SelectionHandle m_lastHandle;
    KoFlake::Position m_hotPosition;
    bool m_mouseWasInsideHandles;
    QPointF m_selectionBox[8];
    QPolygonF m_selectionOutline;
    QPointF m_lastPoint;
    KoShapeMoveCommand *m_moveCommand;
    QTime m_lastUsedMoveCommand;

    // TODO alter these 3 arrays to be static const instead
    QCursor m_sizeCursors[8];
    QCursor m_rotateCursors[8];
    QCursor m_shearCursors[8];
    double m_angle;
};

#endif
