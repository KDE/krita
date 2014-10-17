/*
 *  kis_tool_crop.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (C) 2007 Adrian Page <adrian@pagenet.plus.com>
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

#include "kis_tool_crop.h"
#include "kistoolcropconfigwidget.h"


#include <QCheckBox>
#include <QComboBox>
#include <QObject>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QRect>
#include <QVector>

#include <kis_debug.h>
#include <klocale.h>

#include <KoCanvasBase.h>
#include <kis_global.h>
#include <kis_painter.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <KoPointerEvent.h>
#include <kis_selection.h>
#include <kis_layer.h>
#include <kis_canvas2.h>
#include <kis_view2.h>
#include <kis_floating_message.h>
#include <kis_group_layer.h>
#include <kis_resources_snapshot.h>


struct DecorationLine
{
    QPointF start;
    QPointF end;
    enum Relation
    {
        Width,
        Height,
        Smallest,
        Largest
    };
    Relation startXRelation;
    Relation startYRelation;
    Relation endXRelation;
    Relation endYRelation;
};

DecorationLine decors[20] =
{
    //thirds
    {QPointF(0.0, 0.3333),QPointF(1.0, 0.3333), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height},
    {QPointF(0.0, 0.6666),QPointF(1.0, 0.6666), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height},
    {QPointF(0.3333, 0.0),QPointF(0.3333, 1.0), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height},
    {QPointF(0.6666, 0.0),QPointF(0.6666, 1.0), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height},

    //fifths
    {QPointF(0.0, 0.2),QPointF(1.0, 0.2), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height},
    {QPointF(0.0, 0.4),QPointF(1.0, 0.4), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height},
    {QPointF(0.0, 0.6),QPointF(1.0, 0.6), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height},
    {QPointF(0.0, 0.8),QPointF(1.0, 0.8), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height},
    {QPointF(0.2, 0.0),QPointF(0.2, 1.0), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height},
    {QPointF(0.4, 0.0),QPointF(0.4, 1.0), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height},
    {QPointF(0.6, 0.0),QPointF(0.6, 1.0), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height},
    {QPointF(0.8, 0.0),QPointF(0.8, 1.0), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height},

    // Passport photo
    {QPointF(0.0, 0.45/0.35),QPointF(1.0, 0.45/0.35), DecorationLine::Width, DecorationLine::Width, DecorationLine::Width, DecorationLine::Width},
    {QPointF(0.2, 0.05/0.35),QPointF(0.8, 0.05/0.35), DecorationLine::Width, DecorationLine::Width, DecorationLine::Width, DecorationLine::Width},
    {QPointF(0.2, 0.40/0.35),QPointF(0.8, 0.40/0.35), DecorationLine::Width, DecorationLine::Width, DecorationLine::Width, DecorationLine::Width},
    {QPointF(0.25, 0.07/0.35),QPointF(0.75, 0.07/0.35), DecorationLine::Width, DecorationLine::Width, DecorationLine::Width, DecorationLine::Width},
    {QPointF(0.25, 0.38/0.35),QPointF(0.75, 0.38/0.35), DecorationLine::Width, DecorationLine::Width, DecorationLine::Width, DecorationLine::Width},
    {QPointF(0.35/0.45, 0.0),QPointF(0.35/0.45, 1.0), DecorationLine::Height, DecorationLine::Height, DecorationLine::Height, DecorationLine::Height},

    //Crosshair
    {QPointF(0.0, 0.5),QPointF(1.0, 0.5), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height},
    {QPointF(0.5, 0.0),QPointF(0.5, 1.0), DecorationLine::Width, DecorationLine::Height, DecorationLine::Width, DecorationLine::Height}
};

#define DECORATION_COUNT 5
const int decorsIndex[DECORATION_COUNT] = {0,4,12,18,20};

KisToolCrop::KisToolCrop(KoCanvasBase * canvas)
        : KisTool(canvas, KisCursor::load("tool_crop_cursor.png", 6, 6))
{
    setObjectName("tool_crop");
    m_rectCrop = QRect(0, 0, 0, 0);
    m_handleSize = 13;
    m_haveCropSelection = false;
    m_lastCropSelectionWasReset = false;
    m_cropTypeSelectable = false;
    m_cropType = ImageCropType;
    m_cropX = 0;
    m_cropY = 0;
    m_cropWidth = 0;
    m_forceWidth = false;
    m_cropHeight = 0;
    m_forceHeight = false;
    m_ratio = 0;
    m_forceRatio = false;
    m_growCenter = false;
    m_grow = true;
    m_decoration = 1;

    configGroup = KGlobal::config()->group("cropTool"); // save settings to kritarc
}

KisToolCrop::~KisToolCrop()
{
}

void KisToolCrop::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{

    KisTool::activate(toolActivation, shapes);

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image(), 0, this->canvas()->resourceManager());


    // load settings from configuration
    setGrowCenter(configGroup.readEntry("growCenter", false));
    setForceRatio(configGroup.readEntry("forceRatio", false));
    setAllowGrow(configGroup.readEntry("allowGrow", true));
    setDecoration(configGroup.readEntry("decoration", 0));

    // can't save Enum values, so we ened to convert it to int.
    setCropType(configGroup.readEntry("cropType") == 0 ? LayerCropType : ImageCropType);



    KisSelectionSP sel = resources->activeSelection();
    if (sel) {
        sel->updateProjection();
        m_rectCrop = sel->selectedExactRect();
        m_haveCropSelection = true;
        validateSelection();
    }
    useCursor(cursor());
    updateCanvasPixelRect(image()->bounds());

    //pixel layer
    if(resources->currentNode() && resources->currentNode()->paintDevice()) {
        setCropTypeSelectable(true);
    }
    //vector layer
    else {
        setCropTypeSelectable(false);
    }
}

void KisToolCrop::cancelStroke()
{
    m_haveCropSelection = false;
    m_rectCrop = QRect();
    validateSelection();
    updateCanvasPixelRect(image()->bounds());
}

void KisToolCrop::deactivate()
{
    cancelStroke();
    KisTool::deactivate();
}

void KisToolCrop::requestStrokeEnd()
{
    if (m_haveCropSelection) crop();
}

void KisToolCrop::requestStrokeCancellation()
{
    cancelStroke();
}

void KisToolCrop::canvasResourceChanged(int key, const QVariant &res)
{
    KisTool::canvasResourceChanged(key, res);

    //pixel layer
    if(currentNode() && currentNode()->paintDevice()) {
        setCropTypeSelectable(true);
    }
    //vector layer
    else {
        setCropType(ImageCropType);
        setCropTypeSelectable(false);
    }
}

void KisToolCrop::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    paintOutlineWithHandles(painter);
}

void KisToolCrop::beginPrimaryAction(KoPointerEvent *event)
{
    setMode(KisTool::PAINT_MODE);

    QPoint pos = convertToPixelCoord(event).toPoint();

    m_dragStart = pos;

    if (!m_haveCropSelection) { //if the selection is not set
        m_rectCrop = QRect();
        updateCanvasPixelRect(image()->bounds());
    } else {
        m_mouseOnHandleType = mouseOnHandle(pixelToView(convertToPixelCoord(event)));
        if (m_mouseOnHandleType != None) {
            m_center = m_rectCrop.center(); //store the center of the original so when we grow we are sure to have the same center
            m_originalRatio = m_ratio; //this is the ratio we want to match some areas it is impossible to get this ratio because of integer width and height
            //so if we update this as we go we will run into problems of drift in value. so set target on mouse click
        } else {
            m_lastCropSelectionWasReset = true;
            m_haveCropSelection = false;
            m_rectCrop = QRect();
            updateCanvasPixelRect(image()->bounds());
        }
    }
}

void KisToolCrop::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    QPointF pos = convertToPixelCoord(event);
    bool needForceRatio = this->forceRatio() != bool(event->modifiers() & Qt::ShiftModifier);

    QRectF updateRect = boundingRect();
    if (!m_haveCropSelection) { //if the cropSelection is not yet set
        m_rectCrop = QRect(m_dragStart, pos.toPoint());
    } else { //if the crop selection is set
        QPoint dragStop = pos.toPoint();
        QPoint drag = dragStop - m_dragStart;

        if (m_mouseOnHandleType != None && m_dragStart != dragStop) {
            if(!m_growCenter){ //normal don't grow outwards from center
                if (m_mouseOnHandleType == Inside) {
                    m_rectCrop.translate(drag);
                } else if (needForceRatio) {
                    if (!m_forceWidth && !m_forceHeight) {
                        QRect newRect = m_rectCrop;
                        switch (m_mouseOnHandleType) {
                        case(UpperLeft): {
                            qint32 dep = (drag.x() + drag.y()) / 2;
                            newRect.setTop(newRect.top() + dep);
                            newRect.setLeft(newRect.right() - m_originalRatio * newRect.height());
                        }
                            break;
                        case(LowerRight): {
                            qint32 dep = (drag.x() + drag.y()) / 2;
                            newRect.setBottom(newRect.bottom() + dep);
                            newRect.setWidth((int)(m_originalRatio * newRect.height()));
                            break;
                        }
                        case(UpperRight): {
                            qint32 dep = (drag.x() - drag.y()) / 2;
                            newRect.setTop(newRect.top() - dep);
                            newRect.setWidth((int)(m_originalRatio * newRect.height()));
                            break;
                        }
                        case(LowerLeft): {
                            qint32 dep = (drag.x() - drag.y()) / 2;
                            newRect.setBottom(newRect.bottom() - dep);
                            newRect.setLeft((int)(newRect.right() - m_originalRatio * newRect.height()));
                            break;
                        }
                        case(Upper):
                            newRect.setTop(pos.y());
                            newRect.setWidth((int)(newRect.height() * m_originalRatio));
                            break;
                        case(Lower):
                            newRect.setBottom(pos.y());
                            newRect.setWidth((int)(newRect.height() * m_originalRatio));
                            break;
                        case(Left):
                            newRect.setLeft(pos.x());
                            newRect.setHeight((int)(newRect.width() / m_originalRatio));
                            break;
                        case(Right):
                            newRect.setRight(pos.x());
                            newRect.setHeight((int)(newRect.width() / m_originalRatio));
                            break;
                        case(Inside):  // never happen
                            break;
                        }
                        m_rectCrop = newRect;
                    }
                } else {
                    if (m_forceWidth) {
                        m_rectCrop.setWidth(m_cropWidth);
                    } else {
                        switch (m_mouseOnHandleType) {
                        case(LowerLeft):
                        case(Left):
                        case(UpperLeft):
                            m_rectCrop.setLeft(pos.x());
                        break;
                        case(Right):
                        case(UpperRight):
                        case(LowerRight):
                            m_rectCrop.setRight(pos.x());
                        break;
                        default:
                            break;
                        }
                    }
                    if (m_forceHeight) {
                        m_rectCrop.setHeight(m_cropHeight);
                    } else {
                        switch (m_mouseOnHandleType) {
                        case(UpperLeft):
                        case(Upper):
                        case(UpperRight):
                            m_rectCrop.setTop(pos.y());
                        break;
                        case(LowerRight):
                        case(LowerLeft):
                        case(Lower):
                            m_rectCrop.setBottom(pos.y());
                        break;
                        default:
                            break;
                        }
                    }
                }

            }else{ //grow from center
                
                if (m_mouseOnHandleType == Inside) {
                    m_rectCrop.translate(drag);
                } else if (needForceRatio) {
                    if (!m_forceWidth && !m_forceHeight) {
                        QRect newRect = m_rectCrop;
                        switch (m_mouseOnHandleType) {
                        case(UpperLeft):{
                            qint32 depy = -(drag.x()+drag.y())/2;
                            qint32 depx2 = (m_originalRatio*(2*depy+m_cropHeight)-m_cropWidth);
                            qint32 depx = depx2/2;
                            newRect.adjust(-depx,-depy,depx2-depx,depy); //does not preserve center because of odd widths accumulating
                            newRect.moveCenter(m_center); //fix straying center
                            break;
                        }
                        case(LowerRight): {
                            qint32 depy = (drag.x()+drag.y())/2;
                            qint32 depx2 = (m_originalRatio*(2*depy+m_cropHeight)-m_cropWidth);
                            qint32 depx = depx2/2;
                            newRect.adjust(-depx,-depy,depx2-depx,depy);
                            newRect.moveCenter(m_center);
                            break;
                        }
                        case(UpperRight): {
                            qint32 depy = (drag.x()-drag.y())/2;
                            qint32 depx2 = (m_originalRatio*(2*depy+m_cropHeight)-m_cropWidth);
                            qint32 depx = depx2/2;
                            newRect.adjust(-depx,-depy,depx2-depx,depy);
                            newRect.moveCenter(m_center);
                            break;

                        }
                        case(LowerLeft): {

                            qint32 depy = (drag.y()-drag.x())/2;
                            qint32 depx2 = (m_originalRatio*(2*depy+m_cropHeight)-m_cropWidth);
                            qint32 depx = depx2/2;
                            newRect.adjust(-depx,-depy,depx2-depx,depy);
                            newRect.moveCenter(m_center);
                            break;
                        }
                        case(Upper):{
                            qint32 depy = -drag.y();
                            qint32 depx2 = (m_originalRatio*(2*depy+m_cropHeight)-m_cropWidth);
                            qint32 depx = depx2/2;
                            newRect.adjust(-depx,-depy,depx2-depx,depy);
                            newRect.moveCenter(m_center);
                            break;
                        }
                        case(Lower):{
                            qint32 depy = drag.y();
                            qint32 depx2 = (m_originalRatio*(2*depy+m_cropHeight)-m_cropWidth);
                            qint32 depx = depx2/2;
                            newRect.adjust(-depx,-depy,depx2-depx,depy);
                            newRect.moveCenter(m_center);
                            break;
                        }
                        case(Left):{
                            qint32 depx = -drag.x();
                            qint32 depy2 = ((2*depx+m_cropWidth)/m_originalRatio-m_cropHeight);
                            qint32 depy = depy2/2;
                            newRect.adjust(-depx,-depy,depx,depy2-depy);
                            newRect.moveCenter(m_center);
                            break;
                        }
                        case(Right):{
                            qint32 depx = drag.x();
                            qint32 depy2 = ((2*depx+m_cropWidth)/m_originalRatio-m_cropHeight);
                            qint32 depy = depy2/2;
                            newRect.adjust(-depx,-depy,depx,depy2-depy);
                            newRect.moveCenter(m_center);
                            break;
                        }
                        case(Inside):  // never happen
                            break;
                        }
                        m_rectCrop = newRect;
                    }
                } else {
                    if (m_forceWidth) {
                        m_rectCrop.setWidth(m_cropWidth);
                    } else {
                        switch (m_mouseOnHandleType) {
                        case(LowerLeft):
                        case(Left):
                        case(UpperLeft):
                            m_rectCrop.adjust(drag.x(),0,-drag.x(),0);
                        break;
                        case(Right):
                        case(UpperRight):
                        case(LowerRight):
                            m_rectCrop.adjust(-drag.x(),0,drag.x(),0);
                        break;
                        default:
                            break;
                        }
                    }
                    if (m_forceHeight) {
                        m_rectCrop.setHeight(m_cropHeight);
                    } else {
                        switch (m_mouseOnHandleType) {
                        case(UpperLeft):
                        case(Upper):
                        case(UpperRight):
                            m_rectCrop.adjust(0,drag.y(),0,-drag.y());
                        break;
                        case(LowerRight):
                        case(LowerLeft):
                        case(Lower):
                            m_rectCrop.adjust(0,-drag.y(),0,drag.y());
                        break;
                        default:
                            break;
                        }
                    }
                }

            }
            m_dragStart = dragStop;
        }
    }

    validateSelection();
    updateRect |= boundingRect();
    updateCanvasViewRect(updateRect);
}

void KisToolCrop::endPrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    setMode(KisTool::HOVER_MODE);

    m_rectCrop = m_rectCrop.normalized();

    QRectF viewCropRect = pixelToView(m_rectCrop);

    if (viewCropRect.width() < m_handleSize ||
        viewCropRect.height() < m_handleSize) {

        if (m_lastCropSelectionWasReset) {
            m_rectCrop = QRect();
        } else {
            m_rectCrop = image()->bounds();
        }
    }

    m_lastCropSelectionWasReset = false;
    m_haveCropSelection = !m_rectCrop.isEmpty();

    validateSelection();

    QRectF updateRect = boundingRect();

    updateRect |= boundingRect();
    updateCanvasViewRect(updateRect);
    qint32 type = mouseOnHandle(pixelToView(convertToPixelCoord(event)));
    setMoveResizeCursor(type);
}

void KisToolCrop::mouseMoveEvent(KoPointerEvent *event)
{
    QPointF pos = convertToPixelCoord(event);

    if (m_haveCropSelection) {  //if the crop selection is set
        //set resize cursor if we are on one of the handles
        if(mode() == KisTool::PAINT_MODE) {
            //keep the same cursor as the one we clicked with
            setMoveResizeCursor(m_mouseOnHandleType);
        }else{
            //hovering
            qint32 type = mouseOnHandle(pixelToView(pos));
            setMoveResizeCursor(type);
        }
    }
}

void KisToolCrop::beginPrimaryDoubleClickAction(KoPointerEvent *event)
{
    if (m_haveCropSelection) crop();

    // this action will have no continuation
    event->ignore();
}

//now updates all values regardless (to get rid of the recursive nature of the previous implementation that got into infinite loops)
void KisToolCrop::validateSelection(bool updateratio)
{
    if (canvas() && image()) {
        if(!m_grow){
            m_rectCrop &= image()->bounds();
        }

        QRect r = m_rectCrop.normalized();
        m_cropX = r.x();
        m_cropY = r.y();
        m_cropWidth = r.width();
        m_cropHeight = r.height();
        if(updateratio){
            m_ratio = (double)r.width()/(double)r.height();
        }
        emit cropChanged(updateratio);
    }
}

#define FADE_AREA_OUTSIDE_CROP 1

#if FADE_AREA_OUTSIDE_CROP
#define BORDER_LINE_WIDTH 0
#define HALF_BORDER_LINE_WIDTH 0
#else
#define BORDER_LINE_WIDTH 1
#define HALF_BORDER_LINE_WIDTH (BORDER_LINE_WIDTH / 2.0)
#endif

#define HANDLE_BORDER_LINE_WIDTH 1

QRectF KisToolCrop::borderLineRect()
{
    QRectF borderRect = pixelToView(m_rectCrop.normalized());

    // Draw the border line right next to the crop rectangle perimeter.
    borderRect.adjust(-HALF_BORDER_LINE_WIDTH, -HALF_BORDER_LINE_WIDTH, HALF_BORDER_LINE_WIDTH, HALF_BORDER_LINE_WIDTH);

    return borderRect;
}

#define OUTSIDE_CROP_ALPHA 200

void KisToolCrop::paintOutlineWithHandles(QPainter& gc)
{
    if (canvas() && (mode() == KisTool::PAINT_MODE || m_haveCropSelection)) {
        gc.save();

#if FADE_AREA_OUTSIDE_CROP

        QRectF wholeImageRect = pixelToView(image()->bounds());
        QRectF borderRect = borderLineRect();

        QPainterPath path;

        path.addRect(wholeImageRect);
        path.addRect(borderRect);
        gc.setPen(Qt::NoPen);
        gc.setBrush(QColor(0, 0, 0, OUTSIDE_CROP_ALPHA));
        gc.drawPath(path);

        // Handles
        QPen pen(Qt::SolidLine);
        pen.setWidth(HANDLE_BORDER_LINE_WIDTH);
        pen.setColor(Qt::black);
        gc.setPen(pen);
        gc.setBrush(Qt::yellow);
        gc.drawPath(handlesPath());

        gc.setClipRect(borderRect, Qt::IntersectClip);

        if (m_decoration > 0) {
            for (int i = decorsIndex[m_decoration-1]; i<decorsIndex[m_decoration]; i++) {
                drawDecorationLine(&gc, &(decors[i]), borderRect);
            }
        }

#else
        QPen pen(Qt::SolidLine);
        pen.setWidth(BORDER_LINE_WIDTH);
        gc.setPen(pen);

        QRectF borderRect = borderLineRect();

        //  Border
        gc.setBrush(Qt::NoBrush);
        gc.drawRect(borderRect);

        // Handles
        gc.setBrush(Qt::black);
        gc.drawPath(handlesPath());

        //draw guides
        gc.drawLine(borderRect.bottomLeft(), QPointF(-canvas()->canvasWidget()->width(), borderRect.bottom()));
        gc.drawLine(borderRect.bottomLeft(), QPointF(borderRect.left(), canvas()->canvasWidget()->height()));
        gc.drawLine(borderRect.topRight(), QPointF(canvas()->canvasWidget()->width(), borderRect.top()));
        gc.drawLine(borderRect.topRight(), QPointF(borderRect.right(), -canvas()->canvasWidget()->height()));
#endif
        gc.restore();
    }
}

void KisToolCrop::crop()
{
    KIS_ASSERT_RECOVER_RETURN(currentImage());
    if (m_rectCrop.isEmpty()) return;

    if (m_cropType == LayerCropType) {
        //Cropping layer
        if (!nodeEditable()) {
            return;
        }
    }

    m_haveCropSelection = false;
    useCursor(cursor());

    QRect cropRect = m_rectCrop.normalized();

    // The visitor adds the undo steps to the macro
    if (m_cropType == LayerCropType && currentNode()->paintDevice()) {
        currentImage()->cropNode(currentNode(), cropRect);
    } else {
        currentImage()->cropImage(cropRect);
    }
    m_rectCrop = QRect();
}

void KisToolCrop::setCropTypeLegacy(int cropType)
{
    setCropType(cropType == 0 ? LayerCropType : ImageCropType);
}

void KisToolCrop::setCropType(KisToolCrop::CropToolType cropType)
{
    if(m_cropType == cropType)
        return;
    m_cropType = cropType;

    // can't save LayerCropType, so have to convert it to int for saving
    configGroup.writeEntry("cropType", cropType == LayerCropType ? 0 : 1);

    emit cropTypeChanged();
}

KisToolCrop::CropToolType KisToolCrop::cropType() const
{
    return m_cropType;
}

void KisToolCrop::setCropTypeSelectable(bool selectable)
{
    if(selectable == m_cropTypeSelectable)
        return;
    m_cropTypeSelectable = selectable;
    emit cropTypeSelectableChanged();
}

bool KisToolCrop::cropTypeSelectable() const
{
    return m_cropTypeSelectable;
}

int KisToolCrop::decoration() const
{
    return m_decoration;
}

void KisToolCrop::setDecoration(int i)
{
    // This shouldn't happen, but safety first
    if(i < 0 || i > DECORATION_COUNT)
        return;
    m_decoration = i;
    emit decorationChanged();
    updateCanvasViewRect(boundingRect());

    configGroup.writeEntry("decoration", i);
}

void KisToolCrop::setCropX(int x)
{
    if(x == m_cropX)
        return;

    QRectF updateRect;

    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
        m_rectCrop = QRect(x, 0, 1, 1);
        updateRect = pixelToView(image()->bounds());
    } else {
        updateRect = boundingRect();
        m_rectCrop.moveLeft(x);
    }

    m_cropX = m_rectCrop.normalized().x();

    validateSelection();

    updateRect |= boundingRect();
    updateCanvasViewRect(updateRect);
}

int KisToolCrop::cropX() const
{
    return m_cropX;
}

void KisToolCrop::setCropY(int y)
{
    if(y == m_cropY)
        return;

    QRectF updateRect;

    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
        m_rectCrop = QRect(0, y, 1, 1);
        updateRect = pixelToView(image()->bounds());
    } else {
        updateRect = boundingRect();
        m_rectCrop.moveTop(y);
    }

    m_cropY = m_rectCrop.normalized().y();

    validateSelection();

    updateRect |= boundingRect();
    updateCanvasViewRect(updateRect);
}

int KisToolCrop::cropY() const
{
    return m_cropY;
}

void KisToolCrop::setCropWidth(int w)
{
    if(w == m_cropWidth)
        return;

    QRectF updateRect;

    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
        m_rectCrop = QRect(0, 0, w, 1);
        m_center = m_rectCrop.center();
        updateRect = pixelToView(image()->bounds());
    } else {
        updateRect = boundingRect();
        m_center = m_rectCrop.center();
        m_rectCrop.setWidth(w);
    }

    if (forceRatio()) {
        m_rectCrop.setHeight((int)(w / m_ratio));
    }

    if(m_growCenter){
        m_rectCrop.moveCenter(m_center);
    }

    m_cropWidth = m_rectCrop.normalized().width();

    validateSelection();

    updateRect |= boundingRect();
    updateCanvasViewRect(updateRect);
}

int KisToolCrop::cropWidth() const
{
    return m_cropWidth;
}

void KisToolCrop::setForceWidth(bool force)
{
    m_forceWidth = force;
}

bool KisToolCrop::forceWidth() const
{
    return m_forceWidth;
}

void KisToolCrop::setCropHeight(int h)
{
    if(h == m_cropHeight)
        return;

    QRectF updateRect;

    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
        m_rectCrop = QRect(0, 0, 1, h);
        m_center = m_rectCrop.center();
        updateRect = pixelToView(image()->bounds());
    } else {
        updateRect = boundingRect();
        m_center = m_rectCrop.center();
        m_rectCrop.setHeight(h);
    }

    if (forceRatio()) {
        m_rectCrop.setWidth((int)(h * m_ratio));
    }

    m_cropHeight = m_rectCrop.normalized().height();

    if(m_growCenter){
        m_rectCrop.moveCenter(m_center);
    }

    validateSelection();

    updateRect |= boundingRect();
    updateCanvasViewRect(updateRect);
}

int KisToolCrop::cropHeight() const
{
    return m_cropHeight;
}

void KisToolCrop::setForceHeight(bool force)
{
    if(force == m_forceHeight)
        return;

    m_forceHeight = force;
//    emit forceHeightChanged();
}

bool KisToolCrop::forceHeight() const
{
    return m_forceHeight;
}

void KisToolCrop::setAllowGrow(bool g)
{
    m_grow = g;
    configGroup.writeEntry("allowGrow", g);
}

bool KisToolCrop::allowGrow() const
{
    return m_grow;
}

void KisToolCrop::setGrowCenter(bool g)
{
    m_growCenter = g;
    configGroup.writeEntry("growCenter", g);
}

bool KisToolCrop::growCenter() const
{
    return m_growCenter;
}

void KisToolCrop::setRatio(double ratio)
{
    if(ratio == m_ratio)
        return;

    if (!(m_forceWidth && m_forceHeight)) {
        m_ratio = ratio;
        //emit ratioChanged();

        QRectF updateRect;

        if (!m_haveCropSelection) {
            m_haveCropSelection = true;
            m_rectCrop = QRect(0, 0, 1, 1);
            updateRect = pixelToView(image()->bounds());
        } else {
            updateRect = boundingRect();
        }
        m_center=m_rectCrop.center();
        if (m_forceWidth) {
            m_rectCrop.setHeight((int)(m_rectCrop.width() / m_ratio));
        } else if (m_forceHeight) {
            m_rectCrop.setWidth((int)(m_rectCrop.height() * m_ratio));
        } else {
            int newwidth = (int)(m_ratio * m_rectCrop.height());
            newwidth = (newwidth + m_rectCrop.width()) / 2;
            m_rectCrop.setWidth(newwidth);
            m_rectCrop.setHeight((int)(newwidth / m_ratio));
        }

        if(m_growCenter){
            m_rectCrop.moveCenter(m_center);
        }

        validateSelection(false);
        updateRect |= boundingRect();
        updateCanvasViewRect(updateRect);
    }
}

double KisToolCrop::ratio() const
{
    return m_ratio;
}

void KisToolCrop::setForceRatio(bool force)
{
    if(force == m_forceRatio)
        return;
    m_forceRatio = force;
    configGroup.writeEntry("forceRatio", force);

    emit forceRatioChanged();
}

bool KisToolCrop::forceRatio() const
{
    return m_forceRatio;
}

QWidget* KisToolCrop::createOptionWidget()
{
    KisToolCropConfigWidget* optionsWidget = new KisToolCropConfigWidget(0, this);
    // See https://bugs.kde.org/show_bug.cgi?id=316896
    QWidget *specialSpacer = new QWidget(optionsWidget);
    specialSpacer->setObjectName("SpecialSpacer");
    specialSpacer->setFixedSize(0, 0);
    optionsWidget->layout()->addWidget(specialSpacer);

    Q_CHECK_PTR(optionsWidget);
    optionsWidget->setObjectName(toolId() + " option widget");

    connect(optionsWidget->bnCrop, SIGNAL(clicked()), this, SLOT(crop()));

    connect(optionsWidget, SIGNAL(cropTypeChanged(int)), this, SLOT(setCropTypeLegacy(int)));
    connect(optionsWidget, SIGNAL(cropXChanged(int)), this, SLOT(setCropX(int)));
    connect(optionsWidget, SIGNAL(cropYChanged(int)), this, SLOT(setCropY(int)));
    connect(optionsWidget, SIGNAL(cropHeightChanged(int)), this, SLOT(setCropHeight(int)));
    connect(optionsWidget, SIGNAL(forceHeightChanged(bool)), this, SLOT(setForceHeight(bool)));
    connect(optionsWidget, SIGNAL(cropWidthChanged(int)), this, SLOT(setCropWidth(int)));
    connect(optionsWidget, SIGNAL(forceWidthChanged(bool)), this, SLOT(setForceWidth(bool)));
    connect(optionsWidget, SIGNAL(ratioChanged(double)), this, SLOT(setRatio(double)));
    connect(optionsWidget, SIGNAL(forceRatioChanged(bool)), this, SLOT(setForceRatio(bool)));
    connect(optionsWidget, SIGNAL(decorationChanged(int)), this, SLOT(setDecoration(int)));
    connect(optionsWidget, SIGNAL(allowGrowChanged(bool)), this, SLOT(setAllowGrow(bool)));
    connect(optionsWidget, SIGNAL(growCenterChanged(bool)), this, SLOT(setGrowCenter(bool)));

    optionsWidget->setFixedHeight(optionsWidget->sizeHint().height());

    return optionsWidget;
}

QRectF KisToolCrop::lowerRightHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.right() - m_handleSize / 2.0, cropBorderRect.bottom() - m_handleSize / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::upperRightHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.right() - m_handleSize / 2.0 , cropBorderRect.top() - m_handleSize / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::lowerLeftHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.left() - m_handleSize / 2.0 , cropBorderRect.bottom() - m_handleSize / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::upperLeftHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.left() - m_handleSize / 2.0, cropBorderRect.top() - m_handleSize / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::lowerHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.left() + (cropBorderRect.width() - m_handleSize) / 2.0 , cropBorderRect.bottom() - m_handleSize / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::rightHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.right() - m_handleSize / 2.0 , cropBorderRect.top() + (cropBorderRect.height() - m_handleSize) / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::upperHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.left() + (cropBorderRect.width() - m_handleSize) / 2.0 , cropBorderRect.top() - m_handleSize / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::leftHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.left() - m_handleSize / 2.0, cropBorderRect.top() + (cropBorderRect.height() - m_handleSize) / 2.0, m_handleSize, m_handleSize);
}

QPainterPath KisToolCrop::handlesPath()
{
    QRectF cropBorderRect = borderLineRect();
    QPainterPath path;

    path.addRect(upperLeftHandleRect(cropBorderRect));
    path.addRect(upperRightHandleRect(cropBorderRect));
    path.addRect(lowerLeftHandleRect(cropBorderRect));
    path.addRect(lowerRightHandleRect(cropBorderRect));
    path.addRect(upperHandleRect(cropBorderRect));
    path.addRect(lowerHandleRect(cropBorderRect));
    path.addRect(leftHandleRect(cropBorderRect));
    path.addRect(rightHandleRect(cropBorderRect));

    return path;
}

qint32 KisToolCrop::mouseOnHandle(QPointF currentViewPoint)
{
    QRectF borderRect = borderLineRect();
    qint32 handleType = None;

    if (upperLeftHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = UpperLeft;
    } else if (lowerLeftHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = LowerLeft;
    } else if (upperRightHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = UpperRight;
    } else if (lowerRightHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = LowerRight;
    } else if (upperHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = Upper;
    } else if (lowerHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = Lower;
    } else if (leftHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = Left;
    } else if (rightHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = Right;
    } else if (borderRect.contains(currentViewPoint)) {
        handleType = Inside;
    }

    return handleType;
}

void KisToolCrop::setMoveResizeCursor(qint32 handle)
{
    QCursor cursor;

    switch (handle) {
    case(UpperLeft):
    case(LowerRight):
        cursor = KisCursor::sizeFDiagCursor();
        break;
    case(LowerLeft):
    case(UpperRight):
        cursor = KisCursor::sizeBDiagCursor();
        break;
    case(Upper):
    case(Lower):
        cursor = KisCursor::sizeVerCursor();
        break;
    case(Left):
    case(Right):
        cursor = KisCursor::sizeHorCursor();
        break;
    case(Inside):
        cursor = KisCursor::sizeAllCursor();
        break;
    default:
        cursor = KisCursor::arrowCursor();
        break;
    }
    useCursor(cursor);
}

QRectF KisToolCrop::boundingRect()
{
    QRectF rect = handlesPath().boundingRect();
    rect.adjust(-HANDLE_BORDER_LINE_WIDTH, -HANDLE_BORDER_LINE_WIDTH, HANDLE_BORDER_LINE_WIDTH, HANDLE_BORDER_LINE_WIDTH);
    return rect;
}

void KisToolCrop::drawDecorationLine(QPainter *p, DecorationLine *decorLine, const QRectF rect)
{
    QPointF start = rect.topLeft();
    QPointF end = rect.topLeft();
    qreal small = qMin(rect.width(), rect.height());
    qreal large = qMax(rect.width(), rect.height());

    switch (decorLine->startXRelation) {
    case DecorationLine::Width:
        start.setX(start.x() + decorLine->start.x() * rect.width());
        break;
    case DecorationLine::Height:
        start.setX(start.x() + decorLine->start.x() * rect.height());
        break;
    case DecorationLine::Smallest:
        start.setX(start.x() + decorLine->start.x() * small);
        break;
    case DecorationLine::Largest:
        start.setX(start.x() + decorLine->start.x() * large);
        break;
    }

    switch (decorLine->startYRelation) {
    case DecorationLine::Width:
        start.setY(start.y() + decorLine->start.y() * rect.width());
        break;
    case DecorationLine::Height:
        start.setY(start.y() + decorLine->start.y() * rect.height());
        break;
    case DecorationLine::Smallest:
        start.setY(start.y() + decorLine->start.y() * small);
        break;
    case DecorationLine::Largest:
        start.setY(start.y() + decorLine->start.y() * large);
        break;
    }

    switch (decorLine->endXRelation) {
    case DecorationLine::Width:
        end.setX(end.x() + decorLine->end.x() * rect.width());
        break;
    case DecorationLine::Height:
        end.setX(end.x() + decorLine->end.x() * rect.height());
        break;
    case DecorationLine::Smallest:
        end.setX(end.x() + decorLine->end.x() * small);
        break;
    case DecorationLine::Largest:
        end.setX(end.x() + decorLine->end.x() * large);
        break;
    }

    switch (decorLine->endYRelation) {
    case DecorationLine::Width:
        end.setY(end.y() + decorLine->end.y() * rect.width());
        break;
    case DecorationLine::Height:
        end.setY(end.y() + decorLine->end.y() * rect.height());
        break;
    case DecorationLine::Smallest:
        end.setY(end.y() + decorLine->end.y() * small);
        break;
    case DecorationLine::Largest:
        end.setY(end.y() + decorLine->end.y() * large);
        break;
    }

    p->drawLine(start, end);
}
#include "kis_tool_crop.moc"
