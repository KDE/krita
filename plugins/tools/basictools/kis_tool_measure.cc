/*
 *
 *  SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_measure.h"

#include <math.h>

#include <QPainter>
#include <QLayout>
#include <QWidget>
#include <QLabel>
#include <QPainterPath>
#include <kcombobox.h>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include "kis_algebra_2d.h"
#include "kis_image.h"
#include "kis_cursor.h"
#include "KoPointerEvent.h"
#include "KoCanvasBase.h"
#include <KoViewConverter.h>
#include "krita_utils.h"
#include "kis_floating_message.h"
#include "kis_canvas2.h"
#include "KisViewManager.h"
#include <KisOptimizedBrushOutline.h>

#define INNER_RADIUS 50

KisToolMeasureOptionsWidget::KisToolMeasureOptionsWidget(QWidget* parent, KisImageWSP image)
        : QWidget(parent),
        m_resolution(image->xRes()),
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

    connect(image, SIGNAL(sigResolutionChanged(double, double)), this, SLOT(slotResolutionChanged(double, double)));
}

void KisToolMeasureOptionsWidget::slotSetDistance(double distance)
{
    m_distance = distance;
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

void KisToolMeasureOptionsWidget::slotResolutionChanged(double xRes, double /*yRes*/)
{
    m_resolution = xRes;
    updateDistance();
}

void KisToolMeasureOptionsWidget::updateDistance()
{
    double distance = m_distance / m_resolution;
    m_distanceLabel->setText(KritaUtils::prettyFormatReal(m_unit.toUserValue(distance)));
}


KisToolMeasure::KisToolMeasure(KoCanvasBase * canvas)
    : KisTool(canvas, KisCursor::crossCursor())
{
}

KisToolMeasure::~KisToolMeasure()
{
}
QPointF KisToolMeasure::lockedAngle(QPointF pos)
{
    const QPointF lineVector = pos - m_startPos;
    qreal lineAngle = normalizeAngle(std::atan2(lineVector.y(), lineVector.x()));

    const qreal ANGLE_BETWEEN_CONSTRAINED_LINES = (2 * M_PI) / 24;

    const quint32 constrainedLineIndex = static_cast<quint32>((lineAngle / ANGLE_BETWEEN_CONSTRAINED_LINES) + 0.5);
    const qreal constrainedLineAngle = constrainedLineIndex * ANGLE_BETWEEN_CONSTRAINED_LINES;

    const qreal lineLength = KisAlgebra2D::norm(lineVector);

    const QPointF constrainedLineVector(lineLength * std::cos(constrainedLineAngle), lineLength * std::sin(constrainedLineAngle));

    const QPointF result = m_startPos + constrainedLineVector;

    return result;
}

void KisToolMeasure::paint(QPainter& gc, const KoViewConverter &converter)
{
    QPen old = gc.pen();
    QPen pen(Qt::SolidLine);
    gc.setPen(pen);

    QPainterPath elbowPath;
    elbowPath.moveTo(m_endPos);
    elbowPath.lineTo(m_startPos);

    QPointF offset = (m_baseLineVec * INNER_RADIUS).toPoint();
    QPointF diff = m_endPos-m_startPos;

    bool switch_elbow = QPointF::dotProduct(diff, offset) > 0.0;
    if(switch_elbow) {
        elbowPath.lineTo(m_startPos + offset);
    } else {
        elbowPath.lineTo(m_startPos - offset);
    }

    if (distance() >= INNER_RADIUS) {
        QRectF rectangle(m_startPos.x() - INNER_RADIUS, m_startPos.y() - INNER_RADIUS, 2*INNER_RADIUS, 2*INNER_RADIUS);
        
        double det = diff.x() * m_baseLineVec.y() - diff.y() * m_baseLineVec.x();
        int startAngle = -atan2(m_baseLineVec.y(), m_baseLineVec.x()) / (2*M_PI) * 360;
        int spanAngle = switch_elbow ? -angle() : angle();

        if(!switch_elbow) {
            startAngle+=180;
            startAngle%=360;
        }

        if(det > 0) {
            spanAngle = -spanAngle;
        }

        elbowPath.arcTo(rectangle, startAngle, spanAngle);
    }

    // The opengl renderer doesn't take the QPainter's transform, so the path is scaled here
    qreal sx, sy;
    converter.zoom(&sx, &sy);
    QTransform transf;
    transf.scale(sx / currentImage()->xRes(), sy / currentImage()->yRes());
    paintToolOutline(&gc, transf.map(elbowPath));

    gc.setPen(old);
}
void KisToolMeasure::showDistanceAngleOnCanvas()
{
    KisCanvas2 *kisCanvas = qobject_cast<KisCanvas2*>(canvas());
    QString message = i18nc("%1=distance %2=unit of distance %3=angle in degrees", "%1 %2\n%3°",
                            m_optionsWidget->m_distanceLabel->text(),
                            m_optionsWidget->m_unit.symbol(),
                            QString::number(angle(),'f',1));
    kisCanvas->viewManager()->showFloatingMessage(message, QIcon(), 2000, KisFloatingMessage::High);
}
void KisToolMeasure::beginPrimaryAction(KoPointerEvent *event)
{
    setMode(KisTool::PAINT_MODE);

    // Erase old temporary lines
    canvas()->updateCanvas(convertToPt(boundingRect()));

    m_startPos = convertToPixelCoord(event);
    m_endPos = m_startPos;
    m_baseLineVec = QVector2D(1.0f, 0.0f);

    Q_EMIT sigDistanceChanged(0.0);
    Q_EMIT sigAngleChanged(0.0);
}

void KisToolMeasure::continuePrimaryAction(KoPointerEvent *event)
{
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    // Erase old temporary lines
    canvas()->updateCanvas(convertToPt(boundingRect()));

    QPointF pos = convertToPixelCoord(event);

    if (event->modifiers() & Qt::AltModifier) {
        QPointF trans = pos - m_endPos;
        m_startPos += trans;
        m_endPos += trans;
    } else if(event->modifiers() & Qt::ShiftModifier){
        m_endPos = lockedAngle(pos);
    } else {
        m_endPos = pos;
    }

    if(!(event->modifiers() & Qt::ControlModifier)) {
        m_chooseBaseLineVec = false;
    } else if(!m_chooseBaseLineVec) {
        m_chooseBaseLineVec = true;
        m_baseLineVec = QVector2D(m_endPos-m_startPos).normalized();
    }

    canvas()->updateCanvas(convertToPt(boundingRect()));
    Q_EMIT sigDistanceChanged(distance());
    Q_EMIT sigAngleChanged(angle());
    showDistanceAngleOnCanvas();
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
        return nullptr;
    m_optionsWidget = new KisToolMeasureOptionsWidget(nullptr, currentImage());

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
    double dot = QVector2D::dotProduct(QVector2D(m_endPos-m_startPos).normalized(), m_baseLineVec);
    return acos(qAbs(dot)) / (2*M_PI)*360;
}

double KisToolMeasure::distance()
{
    return QVector2D(m_endPos - m_startPos).length();
}

QRectF KisToolMeasure::boundingRect()
{
    QRectF bound;
    bound.setTopLeft(m_startPos);
    bound.setBottomRight(m_endPos);
    bound = bound.united(QRectF(m_startPos.x() - INNER_RADIUS, m_startPos.y() - INNER_RADIUS, 2 * INNER_RADIUS, 2 * INNER_RADIUS));
    return bound.normalized();
}
