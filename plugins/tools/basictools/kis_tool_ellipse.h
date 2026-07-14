/*
 *  kis_tool_ellipse.h - part of Krayon
 *
 *  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@compuzone.net>
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_TOOL_ELLIPSE_H__
#define __KIS_TOOL_ELLIPSE_H__

#include "kis_tool_shape.h"
#include "kis_types.h"
#include "KisToolPaintFactoryBase.h"
#include "flake/kis_node_shape.h"
#include <kis_tool_ellipse_base.h>
#include <kis_icon.h>

#include <QVector>

class KoCanvasBase;
class QPainterPath;
class QWidget;

class KisToolEllipse : public KisToolEllipseBase
{
    Q_OBJECT

public:
    KisToolEllipse(KoCanvasBase * canvas);
    ~KisToolEllipse() override;

    bool supportsPaintingAssistants() const override;

protected Q_SLOTS:
    void resetCursorStyle() override;
    void slotPlacementModeChanged(int mode, bool checked);

protected:
    enum PlacementMode {
        BoundingBoxPlacement,
        ThreePointPlacement
    };

    QWidget *createOptionWidget() override;
    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void mouseMoveEvent(KoPointerEvent *event) override;
    void paint(QPainter& gc, const KoViewConverter &converter) override;
    KisPopupWidgetInterface* popupWidget() override;
    void requestStrokeEnd() override;
    void requestStrokeCancellation() override;

    void finishRect(const QRectF& rect, qreal roundCornersX, qreal roundCornersY) override;

private:
    bool isThreePointPlacement() const;
    bool canStartThreePointEllipse(KoPointerEvent *event);
    bool hasCompleteThreePointEllipse() const;
    QPointF threePointPreviewPoint() const;
    int threePointHandleAt(const QPointF &pos) const;
    QCursor threePointConfirmCursor() const;
    void updateThreePointPreview(const QPointF &pos);
    void updateThreePointCursor(const QPointF &pos);
    void resetThreePointState();
    void cancelThreePointEllipse();
    void commitThreePointEllipse();
    void paintThreePointEllipse(QPainter &gc);
    void paintThreePointHandles(QPainter &gc);
    bool threePointEllipsePath(const QPointF &thirdPoint, QPainterPath *path) const;
    QRectF threePointUpdateRect() const;
    void addThreePointEllipsePath(const QPainterPath &path);

private:
    PlacementMode m_placementMode {BoundingBoxPlacement};
    QVector<QPointF> m_threePointHandles;
    QPointF m_threePointHoverPoint;
    int m_threePointDraggedHandle {-1};
    bool m_threePointEllipseActive {false};
    bool m_threePointCommitInProgress {false};
    bool m_threePointCommittedOnPress {false};
};

class KisToolEllipseFactory : public KisToolPaintFactoryBase
{

public:
    KisToolEllipseFactory()
            : KisToolPaintFactoryBase("KritaShape/KisToolEllipse") {
        setToolTip(i18n("Ellipse Tool"));
        setSection(ToolBoxSection::Shape);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setIconName(koIconNameCStr("krita_tool_ellipse"));
        setPriority(3);
    }

    ~KisToolEllipseFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return  new KisToolEllipse(canvas);
    }

};


#endif //__KIS_TOOL_ELLIPSE_H__
