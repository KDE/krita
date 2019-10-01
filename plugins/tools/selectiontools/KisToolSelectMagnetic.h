/*
 *  Copyright (c) 2019 Kuntal Majumder <hellozee@disroot.org>
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

#ifndef KIS_TOOL_SELECT_MAGNETIC_H_
#define KIS_TOOL_SELECT_MAGNETIC_H_

#include <QPoint>
#include "KisSelectionToolFactoryBase.h"
#include <kis_tool_select_base.h>
#include <kis_icon.h>
#include "KisMagneticWorker.h"

class QPainterPath;

class KisToolSelectMagnetic : public KisToolSelect
{
    Q_OBJECT

public:
    KisToolSelectMagnetic(KoCanvasBase *canvas);
    ~KisToolSelectMagnetic() override = default;
    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void paint(QPainter& gc, const KoViewConverter &converter) override;

    void beginPrimaryDoubleClickAction(KoPointerEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void mouseMoveEvent(KoPointerEvent *event) override;

    void resetCursorStyle() override;
    void requestStrokeEnd() override;
    void requestStrokeCancellation() override;
    QWidget * createOptionWidget() override;

public Q_SLOTS:
    void deactivate() override;
    void activate(KoToolBase::ToolActivation activation, const QSet<KoShape *> &shapes) override;
    void undoPoints();
    void slotSetFilterRadius(qreal);
    void slotSetThreshold(int);
    void slotSetSearchRadius(int);
    void slotSetAnchorGap(int);

protected:
    using KisToolSelectBase::m_widgetHelper;

private:
    void finishSelectionAction();
    void updateFeedback();
    void updateContinuedMode();
    void updateCanvas();
    void updatePaintPath();
    void resetVariables();
    void drawAnchors(QPainter &gc);
    void checkIfAnchorIsSelected(QPointF pt);
    vQPointF computeEdgeWrapper(QPoint a, QPoint b);
    void reEvaluatePoints();

    QPainterPath m_paintPath;
    QVector<QPointF> m_points;
    QVector<QPoint> m_anchorPoints;
    bool m_continuedMode;
    QPointF m_lastCursorPos;
    QPoint m_lastAnchor;
    bool m_complete, m_selected, m_finished;
    KisMagneticWorker m_worker;
    int m_threshold, m_searchRadius, m_selectedAnchor, m_anchorGap;
    qreal m_filterRadius;
    QRectF m_snapBound;
    KConfigGroup m_configGroup;
    QVector<vQPointF> m_pointCollection;
};

class KisToolSelectMagneticFactory : public KisSelectionToolFactoryBase
{
public:
    KisToolSelectMagneticFactory()
        : KisSelectionToolFactoryBase("KisToolSelectMagnetic")
    {
        setToolTip(i18n("Magnetic Selection Tool"));
        setSection(TOOL_TYPE_SELECTION);
        setIconName(koIconNameCStr("tool_magnetic_selection"));
        setPriority(8);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    }

    ~KisToolSelectMagneticFactory() override { }

    KoToolBase * createTool(KoCanvasBase *canvas) override
    {
        return new KisToolSelectMagnetic(canvas);
    }

    QList<QAction *> createActionsImpl() override
    {
        KisActionRegistry *actionRegistry = KisActionRegistry::instance();
        QList<QAction *> actions = KisSelectionToolFactoryBase::createActionsImpl();

        actions << actionRegistry->makeQAction("undo_polygon_selection");

        return actions;
    }
};


#endif // __selecttoolmagnetic_h__
