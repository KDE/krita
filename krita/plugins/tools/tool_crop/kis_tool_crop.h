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

#include <KoToolFactoryBase.h>
#include "kis_tool.h"
#include "flake/kis_node_shape.h"

#include "ui_wdg_tool_crop.h"

class QRect;

class WdgToolCrop : public QWidget, public Ui::WdgToolCrop
{
    Q_OBJECT

public:
    WdgToolCrop(QWidget *parent) : QWidget(parent) {
        setupUi(this);
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

public:

    KisToolCrop(KoCanvasBase * canvas);
    virtual ~KisToolCrop();

    virtual QWidget* createOptionWidget();
    virtual QWidget* optionWidget();

    virtual void mousePressEvent(KoPointerEvent *e);
    virtual void mouseMoveEvent(KoPointerEvent *e);
    virtual void mouseReleaseEvent(KoPointerEvent *e);
    virtual void mouseDoubleClickEvent(KoPointerEvent *e);

    virtual void paint(QPainter &painter, const KoViewConverter &converter);

public slots:

    virtual void activate(bool);
    virtual void deactivate();

private:
    QRectF boundingRect();
    QRectF borderLineRect();
    QPainterPath handlesPath();
    void paintOutlineWithHandles(QPainter& gc);
    qint32 mouseOnHandle(const QPointF currentViewPoint);
    void setMoveResizeCursor(qint32 handle);
    void validateSelection(bool updateratio = true);
    void setOptionWidgetX(qint32 x);
    void setOptionWidgetY(qint32 y);
    void setOptionWidgetWidth(qint32 x);
    void setOptionWidgetHeight(qint32 y);
    void setOptionWidgetRatio(double ratio);
    void updateWidgetValues(bool updateratio = true);
    QRectF lowerRightHandleRect(QRectF cropBorderRect);
    QRectF upperRightHandleRect(QRectF cropBorderRect);
    QRectF lowerLeftHandleRect(QRectF cropBorderRect);
    QRectF upperLeftHandleRect(QRectF cropBorderRect);
    QRectF lowerHandleRect(QRectF cropBorderRect);
    QRectF rightHandleRect(QRectF cropBorderRect);
    QRectF upperHandleRect(QRectF cropBorderRect);
    QRectF leftHandleRect(QRectF cropBorderRect);

private slots:

    void crop();
    void setCropX(int x);
    void setCropY(int y);
    void setCropWidth(int x);
    void setCropHeight(int y);
    void setRatio(double ratio);

private:
    QRect m_rectCrop; // Is the coordinate of the region to crop.
    bool m_selecting;
    QPoint m_dragStart;

    WdgToolCrop* m_optWidget;

    qint32 m_handleSize;
    bool m_haveCropSelection;
    qint32 m_mouseOnHandleType;

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
};

class KisToolCropFactory : public KoToolFactoryBase
{

public:
    KisToolCropFactory(QObject *parent, const QStringList&)
            : KoToolFactoryBase(parent, "KisToolCrop") {
        setToolTip(i18n("Crop the image to an area"));
        setToolType(TOOL_TYPE_TRANSFORM);
        //setActivationShapeId( KIS_NODE_SHAPE_ID );
        setPriority(10);
        setIcon("tool_crop");
    }

    virtual ~KisToolCropFactory() {}

    virtual KoToolBase * createTool(KoCanvasBase *canvas) {
        return new KisToolCrop(canvas);
    }

};



#endif // KIS_TOOL_CROP_H_

