/*
 *  Copyright (c) 2017 Eugene Ingerman
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

#include "kis_tool_smart_patch.h"

#include "QApplication"
#include "QPainterPath"

#include <klocalizedstring.h>
#include <KoCanvasBase.h>
#include <KoColor.h>
#include <KisViewManager.h>
#include "kis_canvas2.h"
#include "kis_cursor.h"
#include "kis_config.h"
#include "kis_painter.h"

#include "kundo2magicstring.h"
#include "kis_processing_applicator.h"
#include "kis_datamanager.h"

#include "KoProperties.h"
#include "KoColorSpaceRegistry.h"
#include "KoShapeController.h"
#include "KoDocumentResourceManager.h"
//#include "kis_node_manager.h"
#include "kis_cursor.h"

#include "kis_tool_smart_patch_options_widget.h"
#include "libs/image/kis_paint_device_debug_utils.h"

#include "kis_resources_snapshot.h"
#include "kis_layer.h"
#include "kis_transaction.h"
#include "kis_paint_layer.h"
#include "kis_paint_information.h"
#include "kis_distance_information.h"

QRect patchImage(KisPaintDeviceSP imageDev, KisPaintDeviceSP maskDev, int radius, int accuracy,
                 KisPaintDeviceSP originalImageDev, KisPaintDeviceSP patchedImageDev);

class KisToolSmartPatch::InpaintCommand : public KUndo2Command {
public:
    InpaintCommand( KisPaintDeviceSP maskDev, KisPaintDeviceSP imageDev, int accuracy, int patchRadius ) :
        m_maskDev(maskDev), m_imageDev(imageDev), m_patchedImageDev(nullptr), m_accuracy(accuracy), m_patchRadius(patchRadius) {}

    void redo() override {
        if( m_patchedImageDev.isNull() ){
            m_originalImageDev = new KisPaintDevice(m_imageDev->colorSpace());
            m_patchedImageDev  = new KisPaintDevice(m_imageDev->colorSpace());
            m_modifiedRect = patchImage(m_imageDev, m_maskDev, m_patchRadius, m_accuracy, m_originalImageDev, m_patchedImageDev);
        }
        m_imageDev->dataManager()->bitBlt( m_patchedImageDev->dataManager(), m_modifiedRect);
    }

    void undo() override {
        Q_ASSERT(!m_originalImageDev.isNull());
        m_imageDev->dataManager()->bitBlt( m_originalImageDev->dataManager(), m_modifiedRect);
    }

private:
    KisPaintDeviceSP m_maskDev, m_imageDev;
    KisPaintDeviceSP m_originalImageDev;
    KisPaintDeviceSP m_patchedImageDev;
    QRect m_modifiedRect;
    int m_accuracy, m_patchRadius;
};

struct KisToolSmartPatch::Private {
    KisPaintDeviceSP imageDev = nullptr;
    KisPaintDeviceSP maskDev = nullptr;
    KisPainter maskDevPainter;
    KisResourcesSnapshotSP resources = nullptr;
    KoColor currentFgColor;
    float brushRadius = 22.;
    KisToolSmartPatchOptionsWidget *optionsWidget = nullptr;
    QRectF brushRect;
    QRectF oldOutlineRect;
    QPainterPath brushOutline;
};


KisToolSmartPatch::KisToolSmartPatch(KoCanvasBase * canvas)
    : KisToolPaint(canvas, KisCursor::blankCursor()),
      m_d(new Private)
{
    setSupportOutline(true);
    setObjectName("tool_SmartPatch");
    m_d->maskDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    m_d->maskDevPainter.begin( m_d->maskDev );

    m_d->maskDevPainter.setPaintColor(KoColor(Qt::magenta, m_d->maskDev->colorSpace()));
    m_d->maskDevPainter.setBackgroundColor(KoColor(Qt::white, m_d->maskDev->colorSpace()));
    m_d->maskDevPainter.setFillStyle( KisPainter::FillStyleForegroundColor );
}

KisToolSmartPatch::~KisToolSmartPatch()
{
    m_d->optionsWidget = nullptr;
    m_d->maskDevPainter.end();
}

void KisToolSmartPatch::activate(ToolActivation activation, const QSet<KoShape*> &shapes)
{
    KisToolPaint::activate(activation, shapes);
}

void KisToolSmartPatch::deactivate()
{
    KisToolPaint::deactivate();
}

void KisToolSmartPatch::resetCursorStyle()
{
    KisToolPaint::resetCursorStyle();
}

//QRect KisToolSmartPatch::inpaintImage(KisPaintDeviceSP maskDev, KisPaintDeviceSP imageDev)
//{
//    int accuracy = 50; //default accuracy - middle value
//    int patchRadius = 4; //default radius, which works well for most cases tested

//    if (!m_d.isNull() && m_d->optionsWidget) {
//        accuracy = m_d->optionsWidget->getAccuracy();
//        patchRadius = m_d->optionsWidget->getPatchRadius();
//    }
//    return patchImage(imageDev, maskDev, patchRadius, accuracy);
//}

void KisToolSmartPatch::activatePrimaryAction()
{
    setOutlineEnabled(true);
    KisToolPaint::activatePrimaryAction();
}

void KisToolSmartPatch::deactivatePrimaryAction()
{
    setOutlineEnabled(false);
    KisToolPaint::deactivatePrimaryAction();
}

void KisToolSmartPatch::addMaskPath( KoPointerEvent *event )
{
    QPointF imagePos = currentImage()->documentToPixel(event->point);
    QPainterPath currentBrushOutline = brushOutline().translated(imagePos);
    m_d->maskDevPainter.fillPainterPath(currentBrushOutline);

    canvas()->updateCanvas(currentImage()->pixelToDocument(m_d->maskDev->exactBounds()));
}

void KisToolSmartPatch::beginPrimaryAction(KoPointerEvent *event)
{
    //we can only apply inpaint operation to paint layer
    if ( currentNode().isNull() || !currentNode()->inherits("KisPaintLayer")) {
        KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
        kiscanvas->viewManager()->
                showFloatingMessage(
                    i18n("Select a paint layer to use this tool"),
                    QIcon(), 2000, KisFloatingMessage::Medium, Qt::AlignCenter);
        event->ignore();
        return;
    }

    addMaskPath(event);
    setMode(KisTool::PAINT_MODE);
    KisToolPaint::beginPrimaryAction(event);
}

void KisToolSmartPatch::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    addMaskPath(event);
    KisToolPaint::continuePrimaryAction(event);
}


void KisToolSmartPatch::endPrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    addMaskPath(event);
    KisToolPaint::endPrimaryAction(event);
    setMode(KisTool::HOVER_MODE);

    QApplication::setOverrideCursor(KisCursor::waitCursor());

    m_d->imageDev = currentNode()->paintDevice();

    //KIS_DUMP_DEVICE_2(m_d->maskDev, m_d->imageDev->exactBounds(), "maskDev", "/home/eugening/Projects/Mask");

    int accuracy = 50; //default accuracy - middle value
    int patchRadius = 4; //default radius, which works well for most cases tested

    if (!m_d.isNull() && m_d->optionsWidget) {
        accuracy = m_d->optionsWidget->getAccuracy();
        patchRadius = m_d->optionsWidget->getPatchRadius();
    }

    KisProcessingApplicator applicator( image(), currentNode(), KisProcessingApplicator::NONE, KisImageSignalVector() << ModifiedSignal,
                                        kundo2_i18n("Smart Patch"));

    //actual inpaint operation. filling in areas masked by user
    applicator.applyCommand( new InpaintCommand( KisPainter::convertToAlphaAsAlpha(m_d->maskDev), m_d->imageDev, accuracy, patchRadius ),
                             KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE );

    applicator.end();
    image()->waitForDone();

    QApplication::restoreOverrideCursor();
    m_d->maskDev->clear();
}

QPainterPath KisToolSmartPatch::brushOutline( void )
{
    const qreal diameter = m_d->brushRadius;
    QPainterPath outline;
    outline.addEllipse(QPointF(0,0), -0.5 * diameter, -0.5 * diameter );
    return outline;
}

QPainterPath KisToolSmartPatch::getBrushOutlinePath(const QPointF &documentPos,
                                          const KoPointerEvent *event)
{
    Q_UNUSED(event);

    QPointF imagePos = currentImage()->documentToPixel(documentPos);
    QPainterPath path = brushOutline();

    return path.translated( imagePos.rx(), imagePos.ry() );
}

void KisToolSmartPatch::requestUpdateOutline(const QPointF &outlineDocPoint, const KoPointerEvent *event)
{
    if( !event )
        return;

    m_d->brushOutline = getBrushOutlinePath(outlineDocPoint, event);

    QRectF outlinePixelRect = m_d->brushOutline.boundingRect();
    QRectF outlineDocRect = currentImage()->pixelToDocument(outlinePixelRect);

    // This adjusted call is needed as we paint with a 3 pixel wide brush and the pen is outside the bounds of the path
    // Pen uses view coordinates so we have to zoom the document value to match 2 pixel in view coordiates
    // See BUG 275829
    qreal zoomX;
    qreal zoomY;
    canvas()->viewConverter()->zoom(&zoomX, &zoomY);
    qreal xoffset = 2.0/zoomX;
    qreal yoffset = 2.0/zoomY;

    if (!outlineDocRect.isEmpty()) {
        outlineDocRect.adjust(-xoffset,-yoffset,xoffset,yoffset);
    }

    if (!m_d->oldOutlineRect.isEmpty()) {
        canvas()->updateCanvas(m_d->oldOutlineRect);
    }

    if (!outlineDocRect.isEmpty()) {
        canvas()->updateCanvas(outlineDocRect);
    }

    m_d->oldOutlineRect = outlineDocRect;
}

void KisToolSmartPatch::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    painter.save();
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.setPen(QColor(128, 255, 128));
    painter.drawPath(pixelToView(m_d->brushOutline));
    painter.restore();

    painter.save();
    painter.setBrush(Qt::magenta);
    QImage img = m_d->maskDev->convertToQImage(0);
    if( !img.size().isEmpty() ){
        painter.drawImage(pixelToView(m_d->maskDev->exactBounds()), img);
    }
    painter.restore();
}

QWidget * KisToolSmartPatch::createOptionWidget()
{
    KisCanvas2 * kiscanvas = dynamic_cast<KisCanvas2*>(canvas());

    m_d->optionsWidget = new KisToolSmartPatchOptionsWidget(kiscanvas->viewManager()->resourceProvider(), 0);
    m_d->optionsWidget->setObjectName(toolId() + "option widget");

    return m_d->optionsWidget;
}

