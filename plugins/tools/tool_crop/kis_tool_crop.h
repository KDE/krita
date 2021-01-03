/*
 *  kis_tool_crop.h - part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_TOOL_CROP_H_
#define KIS_TOOL_CROP_H_

#include <QPoint>
#include <QPainterPath>

#include <kis_icon.h>

#include <kconfig.h>
#include <kconfiggroup.h>

#include <QKeySequence>
#include <KoToolFactoryBase.h>
#include "kis_tool.h"
#include "flake/kis_node_shape.h"
#include "ui_wdg_tool_crop.h"
#include "kis_constrained_rect.h"
#include "kis_action.h"
#include "kistoolcropconfigwidget.h"

class QRect;
struct DecorationLine;


/**
 * Crop tool
 */
class KisToolCrop : public KisTool
{

    Q_OBJECT
    Q_ENUMS(CropToolType);
    Q_PROPERTY(CropToolType cropType READ cropType WRITE setCropType NOTIFY cropTypeChanged);
    Q_PROPERTY(bool cropTypeSelectable READ cropTypeSelectable WRITE setCropTypeSelectable NOTIFY cropTypeSelectableChanged);
    Q_PROPERTY(int cropX READ cropX WRITE setCropX NOTIFY cropXChanged);
    Q_PROPERTY(int cropY READ cropY WRITE setCropY NOTIFY cropYChanged);
    Q_PROPERTY(int cropWidth READ cropWidth WRITE setCropWidth NOTIFY cropWidthChanged);
    Q_PROPERTY(bool lockWidth READ lockWidth WRITE setLockWidth NOTIFY lockWidthChanged);
    Q_PROPERTY(int cropHeight READ cropHeight WRITE setCropHeight NOTIFY cropHeightChanged);
    Q_PROPERTY(bool lockHeight READ lockHeight WRITE setLockHeight NOTIFY lockHeightChanged);
    Q_PROPERTY(double ratio READ ratio WRITE setRatio NOTIFY ratioChanged);
    Q_PROPERTY(bool lockRatio READ lockRatio WRITE setLockRatio NOTIFY lockRatioChanged);
    Q_PROPERTY(int decoration READ decoration WRITE setDecoration NOTIFY decorationChanged);

public:
    enum CropToolType {
        LayerCropType,
        ImageCropType
    };
    KisToolCrop(KoCanvasBase * canvas);
    ~KisToolCrop() override;

    QWidget* createOptionWidget() override;

    void beginPrimaryAction(KoPointerEvent *event) override;
    void continuePrimaryAction(KoPointerEvent *event) override;
    void endPrimaryAction(KoPointerEvent *event) override;
    void beginPrimaryDoubleClickAction(KoPointerEvent *event) override;

    void mouseMoveEvent(KoPointerEvent *e) override;
    void canvasResourceChanged(int key, const QVariant &res) override;

    void paint(QPainter &painter, const KoViewConverter &converter) override;

    QMenu* popupActionsMenu() override;

    CropToolType cropType() const;
    bool cropTypeSelectable() const;
    int cropX() const;
    int cropY() const;
    int cropWidth() const;
    bool lockWidth() const;
    int cropHeight() const;
    bool lockHeight() const;
    double ratio() const;
    bool lockRatio() const;
    int decoration() const;
    bool growCenter() const;
    bool allowGrow() const;


Q_SIGNALS:
    void cropTypeSelectableChanged();
    void cropTypeChanged(int value);
    void decorationChanged(int value);

    void cropXChanged(int value);
    void cropYChanged(int value);
    void cropWidthChanged(int value);
    void cropHeightChanged(int value);

    void ratioChanged(double value);

    void lockWidthChanged(bool value);
    void lockHeightChanged(bool value);
    void lockRatioChanged(bool value);

    void canGrowChanged(bool value);
    void isCenteredChanged(bool value);

public Q_SLOTS:

    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) override;
    void deactivate() override;

    void requestStrokeEnd() override;
    void requestStrokeCancellation() override;

    void crop();

    void setCropTypeLegacy(int cropType);
    void setCropType(CropToolType cropType);
    void setCropTypeSelectable(bool selectable);
    void setCropX(int x);
    void setCropY(int y);
    void setCropWidth(int x);
    void setLockWidth(bool lock);
    void setCropHeight(int y);
    void setLockHeight(bool lock);
    void setRatio(double ratio);
    void setLockRatio(bool lock);
    void setDecoration(int i);
    void setAllowGrow(bool g);
    void setGrowCenter(bool g);

    void slotRectChanged();

private:
    void doCanvasUpdate(const QRect &updateRect);

private:
    void cancelStroke();
    QRectF boundingRect();
    QRectF borderLineRect();
    QPainterPath handlesPath();
    void paintOutlineWithHandles(QPainter& gc);
    qint32 mouseOnHandle(const QPointF currentViewPoint);
    void setMoveResizeCursor(qint32 handle);
    QRectF lowerRightHandleRect(QRectF cropBorderRect);
    QRectF upperRightHandleRect(QRectF cropBorderRect);
    QRectF lowerLeftHandleRect(QRectF cropBorderRect);
    QRectF upperLeftHandleRect(QRectF cropBorderRect);
    QRectF lowerHandleRect(QRectF cropBorderRect);
    QRectF rightHandleRect(QRectF cropBorderRect);
    QRectF upperHandleRect(QRectF cropBorderRect);
    QRectF leftHandleRect(QRectF cropBorderRect);
    void drawDecorationLine(QPainter *p, DecorationLine *decorLine, QRectF rect);

    bool tryContinueLastCropAction();

private:
    QPoint m_dragStart;

    qint32 m_handleSize;
    bool m_haveCropSelection;
    qint32 m_mouseOnHandleType;

    CropToolType m_cropType;
    bool m_cropTypeSelectable;

    int m_decoration;
    bool m_resettingStroke;
    QRect m_lastCanvasUpdateRect;

    KConfigGroup configGroup;
    KisToolCropConfigWidget* optionsWidget;

    QScopedPointer<QMenu> m_contextMenu;
    KisAction* applyCrop;
    KisAction* growToggleOption;
    KisAction* centerToggleOption;


    enum handleType {
        None = 0,
        UpperLeft = 1,
        UpperRight = 2,
        LowerLeft = 3,
        LowerRight = 4,
        Upper = 5,
        Lower = 6,
        Left = 7,
        Right = 8,
        Inside = 9
    };
    QList<DecorationLine *> m_decorations;

    KisConstrainedRect m_finalRect;
    QRect m_initialDragRect;
    QPointF m_dragOffsetDoc;
};

class KisToolCropFactory : public KoToolFactoryBase
{

public:
    KisToolCropFactory()
            : KoToolFactoryBase("KisToolCrop") {
        setToolTip(i18n("Crop Tool"));
        setSection(TOOL_TYPE_TRANSFORM);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setPriority(11);
        setIconName(koIconNameCStr("tool_crop"));
        setShortcut(QKeySequence("C"));
    }

    ~KisToolCropFactory() override {}

    KoToolBase * createTool(KoCanvasBase *canvas) override {
        return new KisToolCrop(canvas);
    }

};



#endif // KIS_TOOL_CROP_H_

