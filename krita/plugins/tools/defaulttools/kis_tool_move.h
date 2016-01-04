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
#include <kis_icon.h>
#include <QKeySequence>
#include <QWidget>
#include <QGroupBox>
#include <QRadioButton>
#include <kconfig.h>
#include <kconfiggroup.h>


class KoCanvasBase;
class MoveToolOptionsWidget;

class KisToolMove : public KisTool
{
    Q_OBJECT
    Q_ENUMS(MoveToolMode);
    Q_PROPERTY(MoveToolMode moveToolMode READ moveToolMode WRITE setMoveToolMode NOTIFY moveToolModeChanged);
    Q_PROPERTY(bool moveInProgress READ moveInProgress NOTIFY moveInProgressChanged);
public:
    KisToolMove(KoCanvasBase * canvas);
    virtual ~KisToolMove();

public Q_SLOTS:
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    void deactivate();

public Q_SLOTS:
    void requestStrokeEnd();
    void requestStrokeCancellation();

protected Q_SLOTS:
    virtual void resetCursorStyle();

public:
    enum MoveToolMode {
        MoveSelectedLayer,
        MoveFirstLayer,
        MoveGroup
    };

    enum MoveDirection {
        Up,
        Down,
        Left,
        Right
    };

    void beginPrimaryAction(KoPointerEvent *event);
    void continuePrimaryAction(KoPointerEvent *event);
    void endPrimaryAction(KoPointerEvent *event);

    void beginAlternateAction(KoPointerEvent *event, AlternateAction action);
    void continueAlternateAction(KoPointerEvent *event, AlternateAction action);
    void endAlternateAction(KoPointerEvent *event, AlternateAction action);

    void startAction(KoPointerEvent *event, MoveToolMode mode);
    void continueAction(KoPointerEvent *event);
    void endAction(KoPointerEvent *event);

    virtual void paint(QPainter& gc, const KoViewConverter &converter);

    virtual QWidget* createOptionWidget();

    MoveToolMode moveToolMode() const;
    bool moveInProgress() const;
public Q_SLOTS:
    void setMoveToolMode(MoveToolMode newMode);
    void slotWidgetRadioToggled(bool checked);
    void moveDiscrete(MoveDirection direction);
    void slotSetMoveStep(int newMoveStep);

Q_SIGNALS:
    void moveToolModeChanged();
    void moveInProgressChanged();

private:
    void drag(const QPoint& newPos);
    void cancelStroke();
    QPoint applyModifiers(Qt::KeyboardModifiers modifiers, QPoint pos);

private Q_SLOTS:
    void endStroke();

private:

    MoveToolOptionsWidget* m_optionsWidget;

    QPoint m_dragStart;
    QPoint m_accumulatedOffset;

    KisStrokeId m_strokeId;

    MoveToolMode m_moveToolMode;
    bool m_moveInProgress;
    KisNodeSP m_currentlyProcessingNode;
    KConfigGroup configGroup;
    int m_moveStep;

    QAction * m_actionMoveUp;
    QAction * m_actionMoveDown;
    QAction * m_actionMoveLeft;
    QAction * m_actionMoveRight;
};


class KisToolMoveFactory : public KoToolFactoryBase
{

public:
    KisToolMoveFactory()
            : KoToolFactoryBase("KritaTransform/KisToolMove") {
        setToolTip(i18n("Move Tool"));
        setToolType(TOOL_TYPE_TRANSFORM);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setPriority(11);
        setIconName(koIconNameCStr("krita_tool_move"));
        setShortcut(QKeySequence( Qt::Key_T));
    }

    virtual ~KisToolMoveFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolMove(canvas);
    }

};

#endif // KIS_TOOL_MOVE_H_

