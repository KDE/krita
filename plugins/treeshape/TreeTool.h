/* This file is part of the KDE project

   Copyright (c) 2010 Cyril Oblikov <munknex@gmail.com>

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
class TreeShape;

/**
 * The Tree tool implements interactions you have with trees.
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
    explicit TreeTool(KoCanvasBase *canvas);
    virtual ~TreeTool();
    virtual void paint(QPainter &painter, const KoViewConverter &converter);
    virtual void repaintDecorations();
    virtual KoToolSelection* selection();

signals:
    void updateConfigWidget(TreeShape *tree);

public slots:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);

    void changeStructure(int index);
    void changeShape(int index);
    void changeConnectionType(int index);

private slots:
    /// Grabs TreeShapes from selection on selection change
    void grabTrees();

public: // Events

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    virtual void mouseDoubleClickEvent(KoPointerEvent *event);

    virtual void keyPressEvent(QKeyEvent *event);

protected:
    virtual KoInteractionStrategy *createStrategy(KoPointerEvent *event);
    virtual QMap<QString, QWidget *>  createOptionWidgets();

private:
    void setupActions();
    void updateConfigWidget();
    bool moveSelection(int direction, Qt::KeyboardModifiers modifiers);

    // convenience method;
    KoSelection * koSelection();

    QList<TreeShape*> m_selectedTrees;
    KoFlake::Position m_hotPosition;
    bool m_mouseWasInsideHandles;
    TreeShapeMoveCommand *m_moveCommand;

    KoToolSelection *m_selectionHandler;
    friend class SelectionHandler;
    KoInteractionStrategy * m_customEventStrategy;
};

#endif
