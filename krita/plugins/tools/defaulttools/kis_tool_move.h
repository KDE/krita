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
#include <KoIcon.h>
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

private:
    void drag(const QPoint& newPos);

    QPoint applyModifiers(Qt::KeyboardModifiers modifiers, QPoint pos);

private:

    MoveToolOptionsWidget* m_optionsWidget;

    QPoint m_dragStart;
    QPoint m_lastDragPos;

    KisStrokeId m_strokeId;
};


class KisToolMoveFactory : public KoToolFactoryBase
{

public:
    KisToolMoveFactory(const QStringList&)
            : KoToolFactoryBase("KritaTransform/KisToolMove") {
        setToolTip(i18n("Move a layer"));
        setToolType(TOOL_TYPE_TRANSFORM);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setPriority(11);
        setIconName(koIconNameCStr("krita_tool_move"));
        //setShortcut( QKeySequence( Qt::SHIFT + Qt::Key_V ) );
    }

    virtual ~KisToolMoveFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolMove(canvas);
    }

};

#endif // KIS_TOOL_MOVE_H_

