/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_painting_information_builder.h"

#include <KoPointerEvent.h>

#include "kis_config.h"
#include "kis_config_notifier.h"

#include "kis_cubic_curve.h"
#include "kis_speed_smoother.h"

#include <KoCanvasResourceManager.h>
#include "kis_canvas_resource_provider.h"


/***********************************************************************/
/*           KisPaintingInformationBuilder                             */
/***********************************************************************/


const int KisPaintingInformationBuilder::LEVEL_OF_PRESSURE_RESOLUTION = 1024;


KisPaintingInformationBuilder::KisPaintingInformationBuilder()
    : m_speedSmoother(new KisSpeedSmoother()),
      m_pressureDisabled(false)
{
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()),
            SLOT(updateSettings()));

    updateSettings();
}

KisPaintingInformationBuilder::~KisPaintingInformationBuilder()
{

}

void KisPaintingInformationBuilder::updateSettings()
{
    KisConfig cfg;
    KisCubicCurve curve;
    curve.fromString(cfg.pressureTabletCurve());
    m_pressureSamples = curve.floatTransfer(LEVEL_OF_PRESSURE_RESOLUTION + 1);
}

KisPaintInformation KisPaintingInformationBuilder::startStroke(KoPointerEvent *event,
                                                               int timeElapsed,
                                                               const KoCanvasResourceManager *manager)
{
    if (manager) {
        m_pressureDisabled = manager->resource(KisCanvasResourceProvider::DisablePressure).toBool();
    }

    m_startPoint = event->point;
    return createPaintingInformation(event, timeElapsed);

}

KisPaintInformation KisPaintingInformationBuilder::continueStroke(KoPointerEvent *event,
                                                                  int timeElapsed)
{
    return createPaintingInformation(event, timeElapsed);
}

QPointF KisPaintingInformationBuilder::adjustDocumentPoint(const QPointF &point, const QPointF &/*startPoint*/)
{
    return point;
}

QPointF KisPaintingInformationBuilder::documentToImage(const QPointF &point)
{
    return point;
}

QPointF KisPaintingInformationBuilder::imageToView(const QPointF &point)
{
    return point;
}

qreal KisPaintingInformationBuilder::calculatePerspective(const QPointF &documentPoint)
{
    Q_UNUSED(documentPoint);
    return 1.0;
}


KisPaintInformation KisPaintingInformationBuilder::createPaintingInformation(KoPointerEvent *event,
                                                                             int timeElapsed)
{

    QPointF adjusted = adjustDocumentPoint(event->point, m_startPoint);
    QPointF imagePoint = documentToImage(adjusted);
    qreal perspective = calculatePerspective(adjusted);
    qreal speed = m_speedSmoother->getNextSpeed(imageToView(imagePoint));

    return KisPaintInformation(imagePoint,
                               !m_pressureDisabled ? 1.0 : pressureToCurve(event->pressure()),
                               event->xTilt(), event->yTilt(),
                               event->rotation(),
                               event->tangentialPressure(),
                               perspective,
                               timeElapsed,
                               speed);
}

KisPaintInformation KisPaintingInformationBuilder::hover(const QPointF &imagePoint,
                                                         const KoPointerEvent *event)
{
    qreal perspective = calculatePerspective(imagePoint);
    qreal speed = m_speedSmoother->getNextSpeed(imageToView(imagePoint));

    if (event) {
        return KisPaintInformation::createHoveringModeInfo(imagePoint,
                                                           PRESSURE_DEFAULT,
                                                           event->xTilt(), event->yTilt(),
                                                           event->rotation(),
                                                           event->tangentialPressure(),
                                                           perspective,
                                                           speed);
    } else {
        return KisPaintInformation::createHoveringModeInfo(imagePoint);
    }
}

qreal KisPaintingInformationBuilder::pressureToCurve(qreal pressure)
{
    return m_pressureSamples.at(qRound(pressure * LEVEL_OF_PRESSURE_RESOLUTION));
}

/***********************************************************************/
/*           KisConverterPaintingInformationBuilder                        */
/***********************************************************************/

#include "kis_coordinates_converter.h"

KisConverterPaintingInformationBuilder::KisConverterPaintingInformationBuilder(const KisCoordinatesConverter *converter)
    : m_converter(converter)
{
}

QPointF KisConverterPaintingInformationBuilder::documentToImage(const QPointF &point)
{
    return m_converter->documentToImage(point);
}

QPointF KisConverterPaintingInformationBuilder::imageToView(const QPointF &point)
{
    return m_converter->documentToWidget(point);
}

/***********************************************************************/
/*           KisToolFreehandPaintingInformationBuilder                        */
/***********************************************************************/

#include "kis_tool_freehand.h"

KisToolFreehandPaintingInformationBuilder::KisToolFreehandPaintingInformationBuilder(KisToolFreehand *tool)
    : m_tool(tool)
{
}

QPointF KisToolFreehandPaintingInformationBuilder::documentToImage(const QPointF &point)
{
    return m_tool->convertToPixelCoord(point);
}

QPointF KisToolFreehandPaintingInformationBuilder::imageToView(const QPointF &point)
{
    return m_tool->pixelToView(point);
}

QPointF KisToolFreehandPaintingInformationBuilder::adjustDocumentPoint(const QPointF &point, const QPointF &startPoint)
{
    return m_tool->adjustPosition(point, startPoint);
}

qreal KisToolFreehandPaintingInformationBuilder::calculatePerspective(const QPointF &documentPoint)
{
    return m_tool->calculatePerspective(documentPoint);
}
