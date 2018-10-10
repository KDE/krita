/*
 *
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_tool_measure.h"

#include <math.h>

#include <QPainter>
#include <QLayout>
#include <QWidget>
#include <QLabel>
#include <kcombobox.h>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include "kis_image.h"
#include "kis_cursor.h"
#include "KoPointerEvent.h"
#include "KoCanvasBase.h"
#include <KoViewConverter.h>
#include "krita_utils.h"

#define INNER_RADIUS 50

KisToolMeasureOptionsWidget::KisToolMeasureOptionsWidget(QWidget* parent, double resolution)
        : QWidget(parent),
        m_resolution(resolution),
        m_unit(KoUnit::Pixel)
{
    m_distance = 0.0;

    QGridLayout* optionLayout = new QGridLayout(this);
    Q_CHECK_PTR(optionLayout);
    optionLayout->setMargin(0);

    optionLayout->addWidget(new QLabel(i18n("Distance:"), this), 0, 0);
    optionLayout->addWidget(new QLabel(i18n("Angle:"), this), 1, 0);

    m_distanceLabel = new QLabel(this);
    m_distanceLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    optionLayout->addWidget(m_distanceLabel, 0, 1);

    m_angleLabel = new QLabel(this);
    m_angleLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    optionLayout->addWidget(m_angleLabel, 1, 1);

    KComboBox* unitBox = new KComboBox(this);
    unitBox->addItems(KoUnit::listOfUnitNameForUi(KoUnit::ListAll));
    connect(unitBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUnitChanged(int)));
    unitBox->setCurrentIndex(m_unit.indexInListForUi(KoUnit::ListAll));

    optionLayout->addWidget(unitBox, 0, 2);
    optionLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding), 2, 0, 1, 2);
}

void KisToolMeasureOptionsWidget::slotSetDistance(double distance)
{
    m_distance = distance / m_resolution;
    updateDistance();
}

void KisToolMeasureOptionsWidget::slotSetAngle(double angle)
{
    m_angleLabel->setText(i18nc("angle value in degrees", "%1°", KritaUtils::prettyFormatReal(angle)));
}

void KisToolMeasureOptionsWidget::slotUnitChanged(int index)
{
    m_unit = KoUnit::fromListForUi(index, KoUnit::ListAll, m_resolution);
    updateDistance();
}

void KisToolMeasureOptionsWidget::updateDistance()
{
    m_distanceLabel->setText(QString("%1").arg(m_unit.toUserValue(m_distance), 5, 'f', 1));
}


KisToolMeasure::KisToolMeasure(KoCanvasBase * canvas)
    : KisTool(canvas, KisCursor::crossCursor())
{
}

KisToolMeasure::~KisToolMeasure()
{
}

void KisToolMeasure::paint(QPainter& gc, const KoViewConverter &converter)
{
    qreal sx, sy;
    converter.zoom(&sx, &sy);

    gc.scale(sx / currentImage()->xRes(), sy / currentImage()->yRes());

    QPen old = gc.pen();
    QPen pen(Qt::SolidLine);
    gc.setPen(pen);

    gc.drawLine(m_startPos, m_endPos);

    if (deltaX() >= 0)
        gc.drawLine(QPointF(m_startPos.x(), m_startPos.y()), QPointF(m_startPos.x() + INNER_RADIUS, m_startPos.y()));
    else
        gc.drawLine(QPointF(m_startPos.x(), m_startPos.y()), QPointF(m_startPos.x() - INNER_RADIUS, m_startPos.y()));

    if (distance() >= INNER_RADIUS) {
        QRectF rectangle(m_startPos.x() - INNER_RADIUS, m_startPos.y() - INNER_RADIUS, 2*INNER_RADIUS, 2*INNER_RADIUS);
        int startAngle = (deltaX() >= 0) ? 0 : 180 * 16;

        int spanAngle;
        if ((deltaY() >= 0 && deltaX() >= 0) || (deltaY() < 0 && deltaX() < 0))
            spanAngle = static_cast<int>(angle() * 16);
        else
            spanAngle = static_cast<int>(-angle() * 16);
        gc.drawArc(rectangle, startAngle, spanAngle);
    }

    gc.setPen(old);
}

void KisToolMeasure::beginPrimaryAction(KoPointerEvent *event)
{
    setMode(KisTool::PAINT_MODE);

    // Erase old temporary lines
    canvas()->updateCanvas(convertToPt(boundingRect()));

    m_startPos = convertToPixelCoord(event);
    m_endPos = m_startPos;

    emit sigDistanceChanged(0.0);
    emit sigAngleChanged(0.0);
}

void KisToolMeasure::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    // Erase old temporary lines
    canvas()->updateCanvas(convertToPt(boundingRect()));

    QPointF pos = convertToPixelCoord(event);

    if (event->modifiers() == Qt::AltModifier) {
        QPointF trans = pos - m_endPos;
        m_startPos += trans;
        m_endPos += trans;
    } else {
        m_endPos = pos;
    }

    canvas()->updateCanvas(convertToPt(boundingRect()));
    emit sigDistanceChanged(distance());
    emit sigAngleChanged(angle());
}

void KisToolMeasure::endPrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    Q_UNUSED(event);
    setMode(KisTool::HOVER_MODE);
}

QWidget* KisToolMeasure::createOptionWidget()
{
    if (!currentImage())
        return 0;
    m_optionsWidget = new KisToolMeasureOptionsWidget(0, currentImage()->xRes());

    // See https://bugs.kde.org/show_bug.cgi?id=316896
    QWidget *specialSpacer = new QWidget(m_optionsWidget);
    specialSpacer->setObjectName("SpecialSpacer");
    specialSpacer->setFixedSize(0, 0);
    m_optionsWidget->layout()->addWidget(specialSpacer);

    m_optionsWidget->setObjectName(toolId() + " option widget");
    connect(this, SIGNAL(sigDistanceChanged(double)), m_optionsWidget, SLOT(slotSetDistance(double)));
    connect(this, SIGNAL(sigAngleChanged(double)), m_optionsWidget, SLOT(slotSetAngle(double)));
    m_optionsWidget->setFixedHeight(m_optionsWidget->sizeHint().height());
    return m_optionsWidget;
}

double KisToolMeasure::angle()
{
    return atan(qAbs(deltaY()) / qAbs(deltaX())) / (2*M_PI)*360;
}

double KisToolMeasure::distance()
{
    return sqrt(deltaX()*deltaX() + deltaY()*deltaY());
}

QRectF KisToolMeasure::boundingRect()
{
    QRectF bound;
    bound.setTopLeft(m_startPos);
    bound.setBottomRight(m_endPos);
    bound = bound.united(QRectF(m_startPos.x() - INNER_RADIUS, m_startPos.y() - INNER_RADIUS, 2 * INNER_RADIUS, 2 * INNER_RADIUS));
    return bound.normalized();
}

