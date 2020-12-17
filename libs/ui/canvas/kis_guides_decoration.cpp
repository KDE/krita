/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
