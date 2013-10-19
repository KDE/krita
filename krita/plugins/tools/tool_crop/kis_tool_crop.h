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

#include <KoIcon.h>

#include <KoToolFactoryBase.h>
#include "kis_tool.h"
#include "flake/kis_node_shape.h"
#include "ui_wdg_tool_crop.h"

class QRect;
class DecorationLine;

class WdgToolCrop : public QWidget, public Ui::WdgToolCrop
{
    Q_OBJECT

public:
    WdgToolCrop(QWidget *parent) : QWidget(parent) {
        setupUi(this);
        boolHeight->setIcon(koIcon("height_icon"));
        boolWidth->setIcon(koIcon("width_icon"));
        boolRatio->setIcon(koIcon("ratio_icon"));
        label_horizPos->setPixmap(koIcon("offset_horizontal").pixmap(16, 16));
        label_vertiPos->setPixmap(koIcon("offset_vertical").pixmap(16, 16));
    }
};

/**
 * Crop tool
 *
 * TODO: - crop from selection -- i.e, set crop outline to the exact bounds of the selection.
 *       - (when moving to Qt 4: replace rectangle with  darker, dimmer overlay layer
 *         like we have for selections right now)
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
    Q_PROPERTY(bool forceWidth READ forceWidth WRITE setForceWidth NOTIFY forceWidthChanged);
    Q_PROPERTY(int cropHeight READ cropHeight WRITE setCropHeight NOTIFY cropHeightChanged);
    Q_PROPERTY(bool forceHeight READ forceHeight WRITE setForceHeight NOTIFY forceHeightCHanged);
    Q_PROPERTY(double ratio READ ratio WRITE setRatio NOTIFY ratioChanged);
    Q_PROPERTY(bool forceRatio READ forceRatio WRITE setForceRatio NOTIFY forceRatioChanged);
    Q_PROPERTY(int decoration READ decoration WRITE setDecoration NOTIFY decorationChanged);

public:
    enum CropToolType {
        LayerCropType,
        ImageCropType
    };
    KisToolCrop(KoCanvasBase * canvas);
    virtual ~KisToolCrop();

    virtual QWidget* createOptionWidget();

    virtual void mousePressEvent(KoPointerEvent *e);
    virtual void mouseMoveEvent(KoPointerEvent *e);
    virtual void mouseReleaseEvent(KoPointerEvent *e);
    virtual void mouseDoubleClickEvent(KoPointerEvent *e);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual void canvasResourceChanged(int key, const QVariant &res);

    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    CropToolType cropType() const;
    bool cropTypeSelectable() const;
    int cropX() const;
    int cropY() const;
    int cropWidth() const;
    bool forceWidth() const;
    int cropHeight() const;
    bool forceHeight() const;
    double ratio() const;
    bool forceRatio() const;
    int decoration() const;

signals:
    void cropTypeChanged();
    void cropTypeSelectableChanged();
    void cropXChanged();
    void cropYChanged();
    void cropWidthChanged();
    void forceWidthChanged();
    void cropHeightChanged();
    void forceHeightCHanged();
    void ratioChanged();
    void forceRatioChanged();
    void decorationChanged();

public slots:

    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    virtual void deactivate();

    void crop();

    void setCropTypeLegacy(int cropType);
    void setCropType(CropToolType cropType);
    void setCropTypeSelectable(bool selectable);
    void setCropX(int x);
    void setCropY(int y);
    void setCropWidth(int x);
    void setForceWidth(bool force);
    void setCropHeight(int y);
    void setForceHeight(bool force);
    void setRatio(double ratio, bool updateOthers = true);
    void setForceRatio(bool force);
    void setDecoration(int i);

private:
    QRectF boundingRect();
    QRectF borderLineRect();
    QPainterPath handlesPath();
    void paintOutlineWithHandles(QPainter& gc);
    qint32 mouseOnHandle(const QPointF currentViewPoint);
    void setMoveResizeCursor(qint32 handle);
    void validateSelection(bool updateratio = true);
    void updateValues(bool updateratio = true);
    QRectF lowerRightHandleRect(QRectF cropBorderRect);
    QRectF upperRightHandleRect(QRectF cropBorderRect);
    QRectF lowerLeftHandleRect(QRectF cropBorderRect);
    QRectF upperLeftHandleRect(QRectF cropBorderRect);
    QRectF lowerHandleRect(QRectF cropBorderRect);
    QRectF rightHandleRect(QRectF cropBorderRect);
    QRectF upperHandleRect(QRectF cropBorderRect);
    QRectF leftHandleRect(QRectF cropBorderRect);
    void drawDecorationLine(QPainter *p, DecorationLine *decorLine, QRectF rect);

private:
    QRect m_rectCrop; // Is the coordinate of the region to crop.
    QPoint m_dragStart;

    qint32 m_handleSize;
    bool m_haveCropSelection;
    qint32 m_mouseOnHandleType;

    CropToolType m_cropType;
    bool m_cropTypeSelectable;
    int m_cropX;
    int m_cropY;
    int m_cropWidth;
    bool m_forceWidth;
    int m_cropHeight;
    bool m_forceHeight;
    double m_ratio;
    bool m_forceRatio;
    int m_decoration;

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
};

class KisToolCropFactory : public KoToolFactoryBase
{

public:
    KisToolCropFactory(const QStringList&)
            : KoToolFactoryBase("KisToolCrop") {
        setToolTip(i18n("Crop the image to an area"));
        setToolType(TOOL_TYPE_TRANSFORM);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
        setPriority(10);
        setIconName(koIconNameCStr("tool_crop"));
    }

    virtual ~KisToolCropFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolCrop(canvas);
    }

};



#endif // KIS_TOOL_CROP_H_

