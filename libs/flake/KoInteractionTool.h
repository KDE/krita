/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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

#ifndef KOINTERACTIONTOOL_H
#define KOINTERACTIONTOOL_H

#include "KoTool.h"
#include "KoSelection.h"

#include <koffice_export.h>

class KoInteractionStrategy;

/**
 * The default tool (associated with the arrow icon) implements the default
 * interactions you have with flake objects.<br>
 * The tool provides scaling, moving, selecting, rotation and soon skewing of
 * any number of shapes.
 * <p>Note that the implementation of those different strategies are delegated
 * to the InteractionStrategy class and its subclasses.
 */
class FLAKE_EXPORT KoInteractionTool : public KoTool
{
public:
    /**
     * Constructor for basic interaction tool where user actions are translated
     * and handled by interaction strategies of type KoInteractionStrategy.
     * @param name ??
     * @param id ??
     * @param type ??
     * @param canvas the canvas this tool will be working for.
     */
    KoInteractionTool(const QString & name, const QString & id, const QString & type, KoCanvasBase *canvas );
    KoInteractionTool( KoCanvasBase *canvas );
    virtual ~KoInteractionTool();


public: // Identification

    virtual quint32 priority()
    {
        return 0;
    }

    virtual QString quickHelp()
    {
        return  QString( "" );
    }

    virtual void setup(KActionCollection *ac) {
        Q_UNUSED(ac);
    }

    virtual KRadioAction * action()
    {
        return m_action;
    }

public:

    virtual bool wantsAutoScroll();
    virtual QCursor cursor( const QPointF &position );
    virtual void paint( QPainter &painter, KoViewConverter &converter );

    void repaintDecorations();
    /**
     * Returns which selection handle is at params point (or NoHandle if none).
     * @return which selection handle is at params point (or NoHandle if none).
     * @param point the location (in pt) where we should look for a handle
     * @param innerHandleMeaning this boolean is altered to true if the point
     *   is inside the selection rectangle and false if it is just outside.
     *   The value of innerHandleMeaning is undefined if the handle location is NoHandle
     */
    KoFlake::SelectionHandle handleAt(const QPointF &point, bool *innerHandleMeaning = 0);

public: // Events

    virtual void mousePressEvent( KoGfxEvent *event );
    virtual void mouseMoveEvent( KoGfxEvent *event );
    virtual void mouseReleaseEvent( KoGfxEvent *event );

    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

signals:

    void sigActivateTool( const QString &id );
    void sigActivateTemporary(const QString &);
    void sigDone();


protected:
    KoInteractionStrategy *m_currentStrategy;

private:
    void recalcSelectionBox();

    KoInteractionTool(const KoInteractionTool&);
    KoInteractionTool& operator=(const KoInteractionTool&);

    QRectF handlesSize();

    // convenience method;
    KoSelection * selection();
    KoFlake::SelectionHandle m_lastHandle;
    bool m_drawHandles;
    static QPointF m_handleDiff[8];
    QPointF m_selectionBox[8];
    QPointF m_lastPoint;
};

/**
 * The SelectionDecorator is used to paint extra user-interface items on top of a selection.
 */
class SelectionDecorator {
public:
    /**
     * Constructor.
     * @param bounds the rectangle that the selection occupies; the decorations will be
        drawn around that rect.
     * @param arrows the direction that needs highlighting. (currently unused)
     * @param rotationHandles if true; the rotation handles will be drawn
     * @param shearHandles if true; the shearhandles will be drawn
     */
    SelectionDecorator(QRectF bounds, KoFlake::SelectionHandle arrows, bool rotationHandles, bool shearHandles);
    ~SelectionDecorator() {}

    /**
     * paint the decortations.
     * @param painter the painter to paint to.
     * @param converter to convert between internal and view coordinates.
     */
    void paint(QPainter &painter, KoViewConverter &converter);
private:
    bool m_rotationHandles, m_shearHandles;
    KoFlake::SelectionHandle m_arrows;
    KoSelection *m_selection;
    QRectF m_bounds;
};

#endif /* KOINTERACTIONTOOL_H */
