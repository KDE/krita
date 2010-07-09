/* This file is part of the KDE project

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

#ifndef TREETOOL_H
#define TREETOOL_H

#include <KoInteractionTool.h>
#include <KoFlake.h>

#include <QPolygonF>
#include <QTime>

class KoInteractionStrategy;
class TreeShapeMoveCommand;
class KoSelection;

/**
 * The default tool (associated with the arrow icon) implements the default
 * interactions you have with flake objects.<br>
 * The tool provides scaling, moving, selecting, rotation and soon skewing of
 * any number of shapes.
 * <p>Note that the implementation of those different strategies are delegated
 * to the InteractionStrategy class and its subclasses.
 */
class TreeTool : public KoInteractionTool
{
    Q_OBJECT
public:
    /**
     * Constructor for basic interaction tool where user actions are translated
     * and handled by interaction strategies of type KoInteractionStrategy.
     * @param canvas the canvas this tool will be working for.
     */
    explicit TreeTool( KoCanvasBase *canvas );
    virtual ~TreeTool();

    enum CanvasResource {
        HotPosition = 1410100299
    };

public:

    virtual void paint( QPainter &painter, const KoViewConverter &converter );

    virtual void repaintDecorations();

    ///reimplemented
    virtual KoToolSelection* selection();

public slots:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);

public: // Events

    virtual void mousePressEvent( KoPointerEvent *event );
    virtual void mouseMoveEvent( KoPointerEvent *event );
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    virtual void mouseDoubleClickEvent( KoPointerEvent *event );

    virtual void keyPressEvent(QKeyEvent *event);

protected:
    virtual KoInteractionStrategy *createStrategy(KoPointerEvent *event);

private:
    bool moveSelection( int direction, Qt::KeyboardModifiers modifiers );

    // convenience method;
    KoSelection * koSelection();

    void resourceChanged( int key, const QVariant & res );

    KoFlake::SelectionHandle m_lastHandle;
    KoFlake::Position m_hotPosition;
    bool m_mouseWasInsideHandles;
    TreeShapeMoveCommand *m_moveCommand;

    KoToolSelection *m_selectionHandler;
    friend class SelectionHandler;
    KoInteractionStrategy * m_customEventStrategy;
};

#endif
