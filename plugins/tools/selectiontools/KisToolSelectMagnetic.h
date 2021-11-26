/*
 *  SPDX-FileCopyrightText: 2019 Kuntal Majumder <hellozee@disroot.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TOOL_SELECT_MAGNETIC_H_
#define KIS_TOOL_SELECT_MAGNETIC_H_

#include <QPoint>
#include "KisSelectionToolFactoryBase.h"
#include <kis_tool_select_base.h>
#include <kis_signal_compressor.h>
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

    //void beginPrimaryDoubleClickAction(KoPointerEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void mouseMoveEvent(KoPointerEvent *event) override;

    void resetCursorStyle() override;
    void requestStrokeEnd() override;
    void requestStrokeCancellation() override;
    QWidget *createOptionWidget() override;

Q_SIGNALS:
    void setButtonsEnabled(bool);

public Q_SLOTS:
    void deactivate() override;
    void activate(const QSet<KoShape *> &shapes) override;
    void undoPoints();
    void slotSetFilterRadius(qreal);
    void slotSetThreshold(int);
    void slotSetSearchRadius(int);
    void slotSetAnchorGap(int);
    void slotCalculateEdge();

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
    void calculateCheckPoints(vQPointF points);
    void deleteSelectedAnchor();
    void updateSelectedAnchor();
    int updateInitialAnchorBounds(QPoint pt);

    QPainterPath m_paintPath;
    QVector<QPointF> m_points;
    QVector<QPoint> m_anchorPoints;
    bool m_continuedMode;
    QPointF m_lastCursorPos, m_cursorOnPress;
    QPoint m_lastAnchor;
    bool m_complete, m_selected, m_finished;
    QScopedPointer<KisMagneticWorker> m_worker;
    int m_threshold, m_searchRadius, m_selectedAnchor, m_anchorGap;
    qreal m_filterRadius;
    QRectF m_snapBound;
    KConfigGroup m_configGroup;
    QVector<vQPointF> m_pointCollection;
    KisSignalCompressor m_mouseHoverCompressor;
};

class KisToolSelectMagneticFactory : public KisSelectionToolFactoryBase
{
public:
    KisToolSelectMagneticFactory()
        : KisSelectionToolFactoryBase("KisToolSelectMagnetic")
    {
        setToolTip(i18n("Magnetic Selection Tool"));
        setSection(ToolBoxSection::Select);
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
