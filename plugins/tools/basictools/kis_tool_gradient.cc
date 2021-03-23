/*
 *  kis_tool_gradient.cc - part of Krita
 *
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2003 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004-2007 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_gradient.h"

#include <cfloat>

#include <QApplication>
#include <QPainter>
#include <QLabel>
#include <QLayout>
#include <QCheckBox>

#include <kis_transaction.h>
#include <kis_debug.h>
#include <klocalizedstring.h>
#include <kcombobox.h>


#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoViewConverter.h>
#include <KoUpdater.h>
#include <KoProgressUpdater.h>

#include <kis_gradient_painter.h>
#include <kis_painter.h>
#include <kis_canvas_resource_provider.h>
#include <kis_layer.h>
#include <kis_selection.h>
#include <kis_paint_layer.h>

#include <canvas/kis_canvas2.h>
#include <KisViewManager.h>
#include <widgets/kis_cmb_composite.h>
#include <widgets/kis_double_widget.h>
#include <kis_slider_spin_box.h>
#include <kis_cursor.h>
#include <kis_config.h>
#include "kis_resources_snapshot.h"
#include "kis_command_utils.h"
#include "kis_processing_applicator.h"
#include "kis_processing_visitor.h"


KisToolGradient::KisToolGradient(KoCanvasBase * canvas)
        : KisToolPaint(canvas, KisCursor::load("tool_gradient_cursor.png", 6, 6))
{
    setObjectName("tool_gradient");

    m_startPos = QPointF(0, 0);
    m_endPos = QPointF(0, 0);

    m_dither = false;
    m_reverse = false;
    m_shape = KisGradientPainter::GradientShapeLinear;
    m_repeat = KisGradientPainter::GradientRepeatNone;
    m_antiAliasThreshold = 0.2;
}

KisToolGradient::~KisToolGradient()
{
}

void KisToolGradient::resetCursorStyle()
{
    KisToolPaint::resetCursorStyle();

    overrideCursorIfNotEditable();
}

void KisToolGradient::activate(const QSet<KoShape*> &shapes)
{
    KisToolPaint::activate(shapes);
    m_configGroup =  KSharedConfig::openConfig()->group(toolId());
}

void KisToolGradient::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (mode() == KisTool::PAINT_MODE && m_startPos != m_endPos) {
            qreal sx, sy;
            converter.zoom(&sx, &sy);
            painter.scale(sx / currentImage()->xRes(), sy / currentImage()->yRes());
            paintLine(painter);
    }
}

void KisToolGradient::beginPrimaryAction(KoPointerEvent *event)
{
    if (!nodeEditable()) {
        event->ignore();
        return;
    }

    setMode(KisTool::PAINT_MODE);

    m_startPos = convertToPixelCoordAndSnap(event, QPointF(), false);
    m_endPos = m_startPos;
}

void KisToolGradient::continuePrimaryAction(KoPointerEvent *event)
{
    /**
     * TODO: The gradient tool is still not in strokes, so the end of
     *       its action can call processEvent(), which would result in
     *       nested event hadler calls. Please uncomment this line
     *       when the tool is ported to strokes.
     */
    //CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    QPointF pos = convertToPixelCoordAndSnap(event, QPointF(), false);

    QRectF bound(m_startPos, m_endPos);
    canvas()->updateCanvas(convertToPt(bound.normalized()));

    if (event->modifiers() == Qt::ShiftModifier) {
        m_endPos = straightLine(pos);
    } else {
        m_endPos = pos;
    }

    bound.setTopLeft(m_startPos);
    bound.setBottomRight(m_endPos);
    canvas()->updateCanvas(convertToPt(bound.normalized()));
}

void KisToolGradient::endPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);
    setMode(KisTool::HOVER_MODE);

    if (!currentNode())
        return;

    if (m_startPos == m_endPos) {
        return;
    }

    KisImageSP image = this->image();

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image, currentNode(), this->canvas()->resourceManager());

    if (image && resources->currentNode()->paintDevice()) {
        // TODO: refactor out local variables when we switch to C++14
        QPointF startPos = m_startPos;
        QPointF endPos = m_endPos;
        KisGradientPainter::enumGradientShape shape = m_shape;
        KisGradientPainter::enumGradientRepeat repeat = m_repeat;
        bool reverse = m_reverse;
        double antiAliasThreshold = m_antiAliasThreshold;
        bool dither = m_dither;

        KUndo2MagicString actionName = kundo2_i18n("Gradient");
        KisProcessingApplicator applicator(image, resources->currentNode(),
                                           KisProcessingApplicator::NONE,
                                           KisImageSignalVector(),
                                           actionName);

        applicator.applyCommand(
            new KisCommandUtils::LambdaCommand(
                [resources, startPos, endPos,
                 shape, repeat, reverse, antiAliasThreshold, dither] () mutable {

                    KisNodeSP node = resources->currentNode();
                    KisPaintDeviceSP device = node->paintDevice();
                    KisProcessingVisitor::ProgressHelper helper(node);
                    const QRect bounds = device->defaultBounds()->bounds();

                    KisGradientPainter painter(device, resources->activeSelection());
                    resources->setupPainter(&painter);
                    painter.setProgress(helper.updater());

                    painter.beginTransaction();

                    painter.setGradientShape(shape);
                    painter.paintGradient(startPos, endPos,
                                          repeat, antiAliasThreshold,
                                          reverse, 0, 0,
                                          bounds.width(), bounds.height(),
                                          dither);

                    return painter.endAndTakeTransaction();
                }));
        applicator.end();
    }
    canvas()->updateCanvas(convertToPt(currentImage()->bounds()));
}

QPointF KisToolGradient::straightLine(QPointF point)
{
    QPointF comparison = point - m_startPos;
    QPointF result;

    if (fabs(comparison.x()) > fabs(comparison.y())) {
        result.setX(point.x());
        result.setY(m_startPos.y());
    } else {
        result.setX(m_startPos.x());
        result.setY(point.y());
    }

    return result;
}

void KisToolGradient::paintLine(QPainter& gc)
{
    if (canvas()) {
        QPen old = gc.pen();
        QPen pen(Qt::SolidLine);

        gc.setPen(pen);
        gc.drawLine(m_startPos, m_endPos);
        gc.setPen(old);
    }
}

QWidget* KisToolGradient::createOptionWidget()
{
    QWidget *widget = KisToolPaint::createOptionWidget();
    Q_CHECK_PTR(widget);
    widget->setObjectName(toolId() + " option widget");


    // Make sure to create the connections last after everything is set up. The initialized values
    // won't be loaded from the configuration file if you add the widget before the connection
    m_lbShape = new QLabel(i18n("Shape:"), widget);
    m_cmbShape = new KComboBox(widget);
    m_cmbShape->setObjectName("shape_combo");
    m_cmbShape->addItem(i18nc("the gradient will be drawn linearly", "Linear"));
    m_cmbShape->addItem(i18nc("the gradient will be drawn bilinearly", "Bi-Linear"));
    m_cmbShape->addItem(i18nc("the gradient will be drawn radially", "Radial"));
    m_cmbShape->addItem(i18nc("the gradient will be drawn in a square around a centre", "Square"));
    m_cmbShape->addItem(i18nc("the gradient will be drawn as an asymmetric cone", "Conical"));
    m_cmbShape->addItem(i18nc("the gradient will be drawn as a symmetric cone", "Conical Symmetric"));
    m_cmbShape->addItem(i18nc("the gradient will be drawn as a spiral", "Spiral"));
    m_cmbShape->addItem(i18nc("the gradient will be drawn as a reverse spiral", "Reverse Spiral"));
    m_cmbShape->addItem(i18nc("the gradient will be drawn in a selection outline", "Shaped"));
    addOptionWidgetOption(m_cmbShape, m_lbShape);
    connect(m_cmbShape, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetShape(int)));

    m_lbRepeat = new QLabel(i18n("Repeat:"), widget);
    m_cmbRepeat = new KComboBox(widget);
    m_cmbRepeat->setObjectName("repeat_combo");
    m_cmbRepeat->addItem(i18nc("The gradient will not repeat", "None"));
    m_cmbRepeat->addItem(i18nc("The gradient will repeat forwards", "Forwards"));
    m_cmbRepeat->addItem(i18nc("The gradient will repeat alternatingly", "Alternating"));
    addOptionWidgetOption(m_cmbRepeat, m_lbRepeat);
    connect(m_cmbRepeat, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetRepeat(int)));


    m_lbAntiAliasThreshold = new QLabel(i18n("Anti-alias threshold:"), widget);
    m_slAntiAliasThreshold = new KisDoubleSliderSpinBox(widget);
    m_slAntiAliasThreshold->setObjectName("threshold_slider");
    m_slAntiAliasThreshold->setRange(0, 1, 3);
    addOptionWidgetOption(m_slAntiAliasThreshold, m_lbAntiAliasThreshold);
    connect(m_slAntiAliasThreshold, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetAntiAliasThreshold(qreal)));

    m_ckReverse = new QCheckBox(i18nc("the gradient will be drawn with the color order reversed", "Reverse"), widget);
    m_ckReverse->setObjectName("reverse_check");
    connect(m_ckReverse, SIGNAL(toggled(bool)), this, SLOT(slotSetReverse(bool)));
    addOptionWidgetOption(m_ckReverse);

    m_ckDither = new QCheckBox(i18nc("the gradient will be dithered", "Dither"), widget);
    m_ckDither->setObjectName("dither_check");
    connect(m_ckDither, SIGNAL(toggled(bool)), this, SLOT(slotSetDither(bool)));
    addOptionWidgetOption(m_ckDither);

    widget->setFixedHeight(widget->sizeHint().height());


    // load configuration settings into widget (updating UI will update internal variables from signals/slots)
    m_ckDither->setChecked(m_configGroup.readEntry<bool>("dither", false));
    m_ckReverse->setChecked((bool)m_configGroup.readEntry("reverse", false));
    m_cmbShape->setCurrentIndex((int)m_configGroup.readEntry("shape", 0));
    m_cmbRepeat->setCurrentIndex((int)m_configGroup.readEntry("repeat", 0));
    m_slAntiAliasThreshold->setValue((qreal)m_configGroup.readEntry("antialiasThreshold", 0.0));

    return widget;
}

void KisToolGradient::slotSetShape(int shape)
{
    m_shape = static_cast<KisGradientPainter::enumGradientShape>(shape);
    m_configGroup.writeEntry("shape", shape);
}

void KisToolGradient::slotSetRepeat(int repeat)
{
    m_repeat = static_cast<KisGradientPainter::enumGradientRepeat>(repeat);
    m_configGroup.writeEntry("repeat", repeat);
}

void KisToolGradient::slotSetReverse(bool state)
{
    m_reverse = state;
    m_configGroup.writeEntry("reverse", state);
}

void KisToolGradient::slotSetDither(bool state)
{
    m_dither = state;
    m_configGroup.writeEntry("dither", state);
}

void KisToolGradient::slotSetAntiAliasThreshold(qreal value)
{
    m_antiAliasThreshold = value;
    m_configGroup.writeEntry("antialiasThreshold", value);
}

void KisToolGradient::setOpacity(qreal opacity)
{
    m_opacity = opacity;
}


