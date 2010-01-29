/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                2003 Patrick Julien  <freak@codepimps.org>
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

#ifndef KIS_TOOL_MOVE_H_
#define KIS_TOOL_MOVE_H_

#include <KoToolFactoryBase.h>
#include <kis_types.h>
#include <kis_tool.h>
#include <flake/kis_node_shape.h>
#include <QWidget>
#include <QGroupBox>
#include <QRadioButton>

class KoCanvasBase;

#include "ui_wdgmovetool.h"

class MoveToolOptionsWidget : public QWidget, public Ui::WdgMoveTool
{
    Q_OBJECT

public:
    MoveToolOptionsWidget(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};



// XXX: Moving is not nearly smooth enough!
class KisToolMove : public KisTool
{

    Q_OBJECT

public:
    KisToolMove(KoCanvasBase * canvas);
    virtual ~KisToolMove();


public:

    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

    virtual QWidget* createOptionWidget();
    virtual QWidget* optionWidget();

private:
    void drag(const QPoint& pos);

private:

    MoveToolOptionsWidget* m_optionsWidget;
    QRect m_deviceBounds;
    QPoint m_dragStart;
    QPoint m_layerStart;
    QPoint m_layerPosition;
    bool m_dragging;
    KisNodeSP m_selectedNode;
    KisNodeSP m_targetLayer;
    KisSelectionSP m_selection;
};


class KisToolMoveFactory : public KoToolFactoryBase
{

public:
    KisToolMoveFactory(QObject *parent, const QStringList&)
            : KoToolFactoryBase(parent, "KritaTransform/KisToolMove") {
        setToolTip(i18n("Move a layer"));
        setToolType(TOOL_TYPE_TRANSFORM);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setPriority(11);
        setIcon("krita_tool_move");
        //setShortcut( QKeySequence( Qt::SHIFT + Qt::Key_V ) );
    }

    virtual ~KisToolMoveFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolMove(canvas);
    }

};

#endif // KIS_TOOL_MOVE_H_

