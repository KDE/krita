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


/***********************************************************************/
/*           KisPaintingInformationBuilder                             */
/***********************************************************************/


const int KisPaintingInformationBuilder::LEVEL_OF_PRESSURE_RESOLUTION = 1024;


KisPaintingInformationBuilder::KisPaintingInformationBuilder()
{
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()),
            SLOT(updateSettings()));

    updateSettings();
}

void KisPaintingInformationBuilder::updateSettings()
{
    KisConfig cfg;
    KisCubicCurve curve;
    curve.fromString(cfg.pressureTabletCurve());
    m_pressureSamples = curve.floatTransfer(LEVEL_OF_PRESSURE_RESOLUTION + 1);
}

KisPaintInformation KisPaintingInformationBuilder::startStroke(KoPointerEvent *event,
                                                               int timeElapsed)
{
    m_startPoint = event->point;
    return createPaintingInformation(event, timeElapsed);

}

KisPaintInformation KisPaintingInformationBuilder::continueStroke(KoPointerEvent *event,
                                                                  int timeElapsed)
{
    return createPaintingInformation(event, timeElapsed);
}

QPointF KisPaintingInformationBuilder::startPoint() const
{
    return m_startPoint;
}

QPointF KisPaintingInformationBuilder::adjustDocumentPoint(const QPointF &point)
{
    return point;
}

QPointF KisPaintingInformationBuilder::documentToImage(const QPointF &point)
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

    QPointF adjusted = adjustDocumentPoint(event->point);
    QPointF imagePoint = documentToImage(adjusted);
    qreal perspective = calculatePerspective(adjusted);

    return KisPaintInformation(imagePoint,
                               pressureToCurve(event->pressure()),
                               event->xTilt(), event->yTilt(),
                               event->rotation(),
                               event->tangentialPressure(),
                               perspective,
                               timeElapsed);
}

qreal KisPaintingInformationBuilder::pressureToCurve(qreal pressure)
{
    return m_pressureSamples.at(qRound(pressure * LEVEL_OF_PRESSURE_RESOLUTION));
}


/***********************************************************************/
/*           KisToolPaintingInformationBuilder                        */
/***********************************************************************/

#include "kis_tool_freehand.h"

KisToolPaintingInformationBuilder::KisToolPaintingInformationBuilder(KisToolFreehand *tool)
    : m_tool(tool)
{
}

QPointF KisToolPaintingInformationBuilder::adjustDocumentPoint(const QPointF &point)
{
    return m_tool->adjustPosition(point, startPoint());
}

QPointF KisToolPaintingInformationBuilder::documentToImage(const QPointF &point)
{
    return m_tool->convertToPixelCoord(point);
}

qreal KisToolPaintingInformationBuilder::calculatePerspective(const QPointF &documentPoint)
{
    return m_tool->calculatePerspective(documentPoint);
}
