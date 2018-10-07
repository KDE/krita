/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_guides_decoration.h"

#include <KisDocument.h>
#include "kis_config.h"
#include "kis_guides_config.h"
#include "kis_coordinates_converter.h"

struct KisGuidesDecoration::Private
{
    KisGuidesConfig guidesConfig;
};

KisGuidesDecoration::KisGuidesDecoration(QPointer<KisView> view)
    : KisCanvasDecoration(GUIDES_DECORATION_ID, view),
      m_d(new Private)
{
    setPriority(90);
}

KisGuidesDecoration::~KisGuidesDecoration()
{
}

void KisGuidesDecoration::setGuidesConfig(const KisGuidesConfig &value)
{
    m_d->guidesConfig = value;
}

const KisGuidesConfig& KisGuidesDecoration::guidesConfig() const
{
    return m_d->guidesConfig;
}


void KisGuidesDecoration::drawDecoration(QPainter &painter, const QRectF& updateArea, const KisCoordinatesConverter *converter, KisCanvas2 *canvas)
{
    Q_UNUSED(canvas);

    const qreal borderDelta = 2.0;
    const QPen guidesPen(m_d->guidesConfig.guidesPen());

    painter.save();
    painter.setPen(guidesPen);
    painter.setTransform(QTransform());
    painter.setRenderHints(QPainter::Antialiasing, false);
    painter.setRenderHints(QPainter::HighQualityAntialiasing, false);

    Q_FOREACH (qreal guide, m_d->guidesConfig.horizontalGuideLines()) {
        if (guide < updateArea.top() - borderDelta ||
            guide > updateArea.bottom() + borderDelta) {

            continue;
        }

        const QPoint p0 = converter->documentToWidget(QPointF(updateArea.left() - borderDelta, guide)).toPoint();
        const QPoint p1 = converter->documentToWidget(QPointF(updateArea.right() + borderDelta, guide)).toPoint();
        painter.drawLine(p0, p1);
    }

    Q_FOREACH (qreal guide, m_d->guidesConfig.verticalGuideLines()) {
        if (guide < updateArea.left() - borderDelta ||
            guide > updateArea.right() + borderDelta) {

            continue;
        }

        const QPoint p0 = converter->documentToWidget(QPointF(guide, updateArea.top() - borderDelta)).toPoint();
        const QPoint p1 = converter->documentToWidget(QPointF(guide, updateArea.bottom() + borderDelta)).toPoint();
        painter.drawLine(p0, p1);
    }

    painter.restore();
}
