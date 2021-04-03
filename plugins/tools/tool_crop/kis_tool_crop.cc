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


#include <QCheckBox>
#include <QComboBox>
#include <QObject>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QRect>
#include <QVector>
#include <QMenu>

#include <kis_debug.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

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
#include <KisViewManager.h>
#include <kis_floating_message.h>
#include <kis_group_layer.h>
#include <kis_resources_snapshot.h>

#include <kundo2command.h>
#include <kis_crop_saved_extra_data.h>


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
    m_handleSize = 13;
    m_haveCropSelection = false;
    m_cropTypeSelectable = false;
    m_cropType = ImageCropType;
    m_decoration = 1;

    connect(&m_finalRect, SIGNAL(sigValuesChanged()), SLOT(slotRectChanged()));
    connect(&m_finalRect, SIGNAL(sigLockValuesChanged()), SLOT(slotRectChanged()));

    // context menu options (mirrors tool options)
    m_contextMenu.reset(new QMenu());
    applyCrop = new KisAction(i18n("Crop"));

    centerToggleOption = new KisAction(i18n("Center"));
    centerToggleOption->setCheckable(true);
    
    growToggleOption = new KisAction(i18n("Grow"));
    growToggleOption->setCheckable(true);
}

KisToolCrop::~KisToolCrop()
{
}

void KisToolCrop::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{

    KisTool::activate(toolActivation, shapes);
    configGroup =  KSharedConfig::openConfig()->group(toolId()); // save settings to kritarc

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image(), currentNode(), this->canvas()->resourceManager());


    // load settings from configuration
    setGrowCenter(configGroup.readEntry("growCenter", false));
    setAllowGrow(configGroup.readEntry("allowGrow", false));

    // Default: thirds decoration
    setDecoration(configGroup.readEntry("decoration", 1));

    // Default: crop the entire image
    setCropType(configGroup.readEntry("cropType", 1) == 0 ? LayerCropType : ImageCropType);

    m_finalRect.setCropRect(image()->bounds());

    KisSelectionSP sel = resources->activeSelection();
    if (sel) {
        m_haveCropSelection = true;
        m_finalRect.setRectInitial(sel->selectedExactRect());
    }
    useCursor(cursor());

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
    doCanvasUpdate(image()->bounds());
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

void KisToolCrop::requestUndoDuringStroke()
{
    cancelStroke();
}

void KisToolCrop::requestRedoDuringStroke()
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

QMenu *KisToolCrop::popupActionsMenu()
{
    if (m_contextMenu) {
        // Sync state of context menu toggles with state of Tool Options toggles
        centerToggleOption->setChecked(growCenter());
        growToggleOption->setChecked(allowGrow());
        m_contextMenu->clear();

        m_contextMenu->addSection(i18n("Crop Tool Actions"));
        m_contextMenu->addSeparator();

        if (m_haveCropSelection) {         // can't crop if there is no selection
            m_contextMenu->addAction(applyCrop);
            m_contextMenu->addSeparator();
        }

        m_contextMenu->addAction(centerToggleOption);
        m_contextMenu->addAction(growToggleOption);
    }

    return m_contextMenu.data();
}

void KisToolCrop::beginPrimaryAction(KoPointerEvent *event)
{
    m_finalRect.setCropRect(image()->bounds());
    setMode(KisTool::PAINT_MODE);

    const QPointF imagePoint = convertToPixelCoord(event);
    m_mouseOnHandleType = mouseOnHandle(pixelToView(imagePoint));

    if (m_mouseOnHandleType != KisConstrainedRect::None) {
        QPointF snapPoint = m_finalRect.handleSnapPoint(KisConstrainedRect::HandleType(m_mouseOnHandleType), imagePoint);
        QPointF snapDocPoint = image()->pixelToDocument(snapPoint);
        m_dragOffsetDoc = snapDocPoint - event->point;
    } else {
        m_dragOffsetDoc = QPointF();
    }

    QPointF snappedPoint = convertToPixelCoordAndSnap(event, m_dragOffsetDoc);

    m_dragStart = snappedPoint.toPoint();
    m_resettingStroke = false;

    if (!m_haveCropSelection || m_mouseOnHandleType == None) {
        m_lastCanvasUpdateRect = image()->bounds();
        const int initialWidth = m_finalRect.widthLocked() ? m_finalRect.rect().width() : 1;
        const int initialHeight = m_finalRect.heightLocked() ? m_finalRect.rect().height() : 1;
        const QRect initialRect = QRect(m_dragStart, QSize(initialWidth, initialHeight));
        m_finalRect.setRectInitial(initialRect);
        m_initialDragRect = initialRect;
        m_mouseOnHandleType = KisConstrainedRect::Creation;
        m_resettingStroke = true;
    } else {
        m_initialDragRect = m_finalRect.rect();
    }
}

void KisToolCrop::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    const QPointF pos = convertToPixelCoordAndSnap(event, m_dragOffsetDoc);
    const QPoint drag = pos.toPoint() - m_dragStart;

    m_finalRect.moveHandle(KisConstrainedRect::HandleType(m_mouseOnHandleType), drag, m_initialDragRect);
}

bool KisToolCrop::tryContinueLastCropAction()
{
    bool result = false;

    const KUndo2Command *lastCommand = image()->undoAdapter()->presentCommand();
    const KisCropSavedExtraData *data = 0;

    if ((lastCommand = image()->undoAdapter()->presentCommand()) &&
        (data = dynamic_cast<const KisCropSavedExtraData*>(lastCommand->extraData()))) {

        bool cropImageConsistent =
            m_cropType == ImageCropType &&
            (data->type() == KisCropSavedExtraData::CROP_IMAGE ||
             data->type() == KisCropSavedExtraData::RESIZE_IMAGE);

        bool cropLayerConsistent =
            m_cropType == LayerCropType &&
            data->type() == KisCropSavedExtraData::CROP_LAYER &&
            currentNode() == data->cropNode();


        if (cropImageConsistent || cropLayerConsistent) {
            image()->undoAdapter()->undoLastCommand();
            image()->waitForDone();

            m_finalRect.setRectInitial(data->cropRect());
            m_haveCropSelection = true;

            result = true;
        }
    }

    return result;
}

void KisToolCrop::endPrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    setMode(KisTool::HOVER_MODE);

    QRectF viewCropRect = pixelToView(m_finalRect.rect());
    const bool haveValidRect =
        viewCropRect.width() > m_handleSize &&
        viewCropRect.height() > m_handleSize;


    if (!m_haveCropSelection && !haveValidRect) {
        if (!tryContinueLastCropAction()) {
            m_finalRect.setRectInitial(image()->bounds());
            m_haveCropSelection = true;
        }
    } else if (m_resettingStroke && !haveValidRect) {
        m_lastCanvasUpdateRect = image()->bounds();
        m_haveCropSelection = false;
    } else {
        m_haveCropSelection = true;
    }

    m_finalRect.normalize();

    qint32 type = mouseOnHandle(pixelToView(convertToPixelCoordAndSnap(event, m_dragOffsetDoc)));
    setMoveResizeCursor(type);
}

void KisToolCrop::mouseMoveEvent(KoPointerEvent *event)
{
    QPointF pos = convertToPixelCoordAndSnap(event);

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


#define BORDER_LINE_WIDTH 0
#define HALF_BORDER_LINE_WIDTH 0
#define HANDLE_BORDER_LINE_WIDTH 1

QRectF KisToolCrop::borderLineRect()
{
    QRectF borderRect = pixelToView(m_finalRect.rect());

    // Draw the border line right next to the crop rectangle perimeter.
    borderRect.adjust(-HALF_BORDER_LINE_WIDTH, -HALF_BORDER_LINE_WIDTH, HALF_BORDER_LINE_WIDTH, HALF_BORDER_LINE_WIDTH);

    return borderRect;
}

#define OUTSIDE_CROP_ALPHA 200

void KisToolCrop::paintOutlineWithHandles(QPainter& gc)
{
    if (canvas() && (mode() == KisTool::PAINT_MODE || m_haveCropSelection)) {
        gc.save();

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
        gc.setBrush(QColor(200, 200, 200, OUTSIDE_CROP_ALPHA));
        gc.drawPath(handlesPath());

        gc.setClipRect(borderRect, Qt::IntersectClip);

        if (m_decoration > 0) {
            for (int i = decorsIndex[m_decoration-1]; i<decorsIndex[m_decoration]; i++) {
                drawDecorationLine(&gc, &(decors[i]), borderRect);
            }
        }
        gc.restore();
    }
}

void KisToolCrop::crop()
{
    KIS_ASSERT_RECOVER_RETURN(currentImage());
    if (m_finalRect.rect().isEmpty()) return;

    if (m_cropType == LayerCropType) {
        //Cropping layer
        if (!nodeEditable()) {
            return;
        }
    }

    m_haveCropSelection = false;
    useCursor(cursor());

    QRect cropRect = m_finalRect.rect();

    // The visitor adds the undo steps to the macro
    if (m_cropType == LayerCropType && currentNode()->paintDevice()) {
        currentImage()->cropNode(currentNode(), cropRect);
    } else {
        currentImage()->cropImage(cropRect);
    }
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

    emit cropTypeChanged(m_cropType);
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
    emit decorationChanged(decoration());
    updateCanvasViewRect(boundingRect());

    configGroup.writeEntry("decoration", i);
}

void KisToolCrop::doCanvasUpdate(const QRect &updateRect)
{
    updateCanvasViewRect(updateRect | m_lastCanvasUpdateRect);
    m_lastCanvasUpdateRect = updateRect;
}

void KisToolCrop::slotRectChanged()
{
    emit cropHeightChanged(cropHeight());
    emit cropWidthChanged(cropWidth());
    emit cropXChanged(cropX());
    emit cropYChanged(cropY());
    emit ratioChanged(ratio());
    emit lockHeightChanged(lockHeight());
    emit lockWidthChanged(lockWidth());
    emit lockRatioChanged(lockRatio());

    emit canGrowChanged(allowGrow());
    emit isCenteredChanged(growCenter());

    doCanvasUpdate(boundingRect().toAlignedRect());
}

void KisToolCrop::setCropX(int x)
{
    if(x == m_finalRect.rect().x())
        return;

    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
        m_finalRect.setRectInitial(image()->bounds());
    }

    QPoint offset = m_finalRect.rect().topLeft();
    offset.setX(x);
    m_finalRect.setOffset(offset);
}

int KisToolCrop::cropX() const
{
    return m_finalRect.rect().x();
}

void KisToolCrop::setCropY(int y)
{
    if(y == m_finalRect.rect().y())
        return;

    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
        m_finalRect.setRectInitial(image()->bounds());
    }

    QPoint offset = m_finalRect.rect().topLeft();
    offset.setY(y);
    m_finalRect.setOffset(offset);
}

int KisToolCrop::cropY() const
{
    return m_finalRect.rect().y();
}

void KisToolCrop::setCropWidth(int w)
{
    if(w == m_finalRect.rect().width())
        return;

    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
        m_finalRect.setRectInitial(image()->bounds());
    }

    m_finalRect.setWidth(w);
}

int KisToolCrop::cropWidth() const
{
    return m_finalRect.rect().width();
}

void KisToolCrop::setLockWidth(bool lock)
{
    m_finalRect.setWidthLocked(lock);
}

bool KisToolCrop::lockWidth() const
{
    return m_finalRect.widthLocked();
}

void KisToolCrop::setCropHeight(int h)
{
    if(h == m_finalRect.rect().height())
        return;

    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
        m_finalRect.setRectInitial(image()->bounds());
    }

    m_finalRect.setHeight(h);
}

int KisToolCrop::cropHeight() const
{
    return m_finalRect.rect().height();
}

void KisToolCrop::setLockHeight(bool lock)
{
    m_finalRect.setHeightLocked(lock);
}

bool KisToolCrop::lockHeight() const
{
    return m_finalRect.heightLocked();
}

void KisToolCrop::setAllowGrow(bool g)
{
    m_finalRect.setCanGrow(g);
    m_finalRect.setCropRect(image()->bounds());
    configGroup.writeEntry("allowGrow", g);

    emit canGrowChanged(g);
}

bool KisToolCrop::allowGrow() const
{
    return m_finalRect.canGrow();
}

void KisToolCrop::setGrowCenter(bool value)
{
    m_finalRect.setCentered(value);


    configGroup.writeEntry("growCenter", value);

    emit isCenteredChanged(value);
}

bool KisToolCrop::growCenter() const
{
    return m_finalRect.centered();
}

void KisToolCrop::setRatio(double ratio)
{
    if(ratio == m_finalRect.ratio())
        return;

    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
        m_finalRect.setRectInitial(image()->bounds());
    }

    m_finalRect.setRatio(ratio);
}

double KisToolCrop::ratio() const
{
    return m_finalRect.ratio();
}

void KisToolCrop::setLockRatio(bool lock)
{
    m_finalRect.setRatioLocked(lock);
}

bool KisToolCrop::lockRatio() const
{
    return m_finalRect.ratioLocked();
}

QWidget* KisToolCrop::createOptionWidget()
{
    optionsWidget = new KisToolCropConfigWidget(0, this);
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
    connect(optionsWidget, SIGNAL(lockHeightChanged(bool)), this, SLOT(setLockHeight(bool)));
    connect(optionsWidget, SIGNAL(cropWidthChanged(int)), this, SLOT(setCropWidth(int)));
    connect(optionsWidget, SIGNAL(lockWidthChanged(bool)), this, SLOT(setLockWidth(bool)));
    connect(optionsWidget, SIGNAL(ratioChanged(double)), this, SLOT(setRatio(double)));
    connect(optionsWidget, SIGNAL(lockRatioChanged(bool)), this, SLOT(setLockRatio(bool)));
    connect(optionsWidget, SIGNAL(decorationChanged(int)), this, SLOT(setDecoration(int)));
    connect(optionsWidget, SIGNAL(allowGrowChanged(bool)), this, SLOT(setAllowGrow(bool)));
    connect(optionsWidget, SIGNAL(growCenterChanged(bool)), this, SLOT(setGrowCenter(bool)));

    optionsWidget->setFixedHeight(optionsWidget->sizeHint().height());

    connect(applyCrop, SIGNAL(triggered(bool)), this, SLOT(crop()));
    connect(centerToggleOption, SIGNAL(triggered(bool)), this, SLOT(setGrowCenter(bool)));
    connect(growToggleOption, SIGNAL(triggered(bool)), this, SLOT(setAllowGrow(bool)));

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

    if (!m_haveCropSelection) {
        return None;
    }

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
