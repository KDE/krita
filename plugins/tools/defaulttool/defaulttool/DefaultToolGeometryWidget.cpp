/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Martin Pfeiffer <hubipete@gmx.net>
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2008 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2010 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "DefaultToolGeometryWidget.h"
#include "DefaultTool.h"

#include <KoInteractionTool.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoSelectedShapesProxy.h>
#include <KoSelection.h>
#include <KoUnit.h>
#include <commands/KoShapeResizeCommand.h>
#include <commands/KoShapeMoveCommand.h>
#include <commands/KoShapeSizeCommand.h>
#include <commands/KoShapeTransformCommand.h>
#include <commands/KoShapeKeepAspectRatioCommand.h>
#include <commands/KoShapeTransparencyCommand.h>
#include <commands/KoShapePaintOrderCommand.h>
#include "SelectionDecorator.h"
#include <KoShapeGroup.h>

#include "KoAnchorSelectionWidget.h"

#include <QAction>
#include <QSize>
#include <QRadioButton>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QList>
#include <kis_algebra_2d.h>

#include "kis_aspect_ratio_locker.h"
#include "kis_debug.h"
#include "kis_acyclic_signal_connector.h"
#include "kis_signal_compressor.h"
#include "kis_signals_blocker.h"
#include "kis_icon.h"
#include <kis_canvas2.h>


DefaultToolGeometryWidget::DefaultToolGeometryWidget(KoInteractionTool *tool, QWidget *parent)
    : QWidget(parent)
    , m_tool(tool)
    , m_sizeAspectLocker(new KisAspectRatioLocker())
    , m_savedUniformScaling(false)
{
    setupUi(this);

    setUnit(KoUnit(KoUnit::Point));

    // Connect and initialize automated aspect locker
    m_sizeAspectLocker->connectSpinBoxes(widthSpinBox, heightSpinBox, aspectButton);
    aspectButton->setKeepAspectRatio(false);


    // TODO: use valueChanged() instead!
    connect(positionXSpinBox, SIGNAL(valueChangedPt(qreal)), this, SLOT(slotRepositionShapes()));
    connect(positionYSpinBox, SIGNAL(valueChangedPt(qreal)), this, SLOT(slotRepositionShapes()));

    KoSelectedShapesProxy *selectedShapesProxy = m_tool->canvas()->selectedShapesProxy();

    connect(selectedShapesProxy, SIGNAL(selectionChanged()), this, SLOT(slotUpdateCheckboxes()));
    connect(selectedShapesProxy, SIGNAL(selectionChanged()), this, SLOT(slotUpdatePositionBoxes()));
    connect(selectedShapesProxy, SIGNAL(selectionChanged()), this, SLOT(slotUpdateOpacitySlider()));

    connect(selectedShapesProxy, SIGNAL(selectionContentChanged()), this, SLOT(slotUpdatePositionBoxes()));
    connect(selectedShapesProxy, SIGNAL(selectionContentChanged()), this, SLOT(slotUpdateOpacitySlider()));

    connect(chkGlobalCoordinates, SIGNAL(toggled(bool)), SLOT(slotUpdateSizeBoxes()));
    connect(chkGlobalCoordinates, SIGNAL(toggled(bool)), SLOT(slotUpdateAspectButton()));


    /**
     * A huge block of self-blocking acyclic connections
     */
    KisAcyclicSignalConnector *acyclicConnector = new KisAcyclicSignalConnector(this);
    acyclicConnector->connectForwardVoid(m_sizeAspectLocker.data(), SIGNAL(aspectButtonChanged()), this, SLOT(slotAspectButtonToggled()));
    acyclicConnector->connectBackwardVoid(selectedShapesProxy, SIGNAL(selectionChanged()), this, SLOT(slotUpdateAspectButton()));
    acyclicConnector->connectBackwardVoid(selectedShapesProxy, SIGNAL(selectionContentChanged()), this, SLOT(slotUpdateAspectButton()));

    KisAcyclicSignalConnector *sizeConnector = acyclicConnector->createCoordinatedConnector();
    sizeConnector->connectForwardVoid(m_sizeAspectLocker.data(), SIGNAL(sliderValueChanged()), this, SLOT(slotResizeShapes()));
    sizeConnector->connectBackwardVoid(selectedShapesProxy, SIGNAL(selectionChanged()), this, SLOT(slotUpdateSizeBoxes()));

    KisAcyclicSignalConnector *contentSizeConnector = acyclicConnector->createCoordinatedConnector();
    contentSizeConnector->connectBackwardVoid(selectedShapesProxy, SIGNAL(selectionContentChanged()), this, SLOT(slotUpdateSizeBoxesNoAspectChange()));


    // Connect and initialize anchor point resource
    KoCanvasResourceProvider *resourceManager = m_tool->canvas()->resourceManager();
    connect(resourceManager,
            SIGNAL(canvasResourceChanged(int,QVariant)),
            SLOT(resourceChanged(int,QVariant)));
    resourceManager->setResource(DefaultTool::HotPosition, int(KoFlake::AnchorPosition::Center));
    positionSelector->setValue(KoFlake::AnchorPosition(resourceManager->resource(DefaultTool::HotPosition).toInt()));

    // Connect anchor point selector
    connect(positionSelector, SIGNAL(valueChanged(KoFlake::AnchorPosition)), SLOT(slotAnchorPointChanged()));

    cmbPaintOrder->setIconSize(QSize(22, 22));
    cmbPaintOrder->addItem(KisIconUtils::loadIcon("paint-order-fill-stroke-marker"), i18n("Fill, Stroke, Markers"));
    cmbPaintOrder->addItem(KisIconUtils::loadIcon("paint-order-fill-marker-stroke"), i18n("Fill, Markers, Stroke"));
    cmbPaintOrder->addItem(KisIconUtils::loadIcon("paint-order-stroke-fill-marker"), i18n("Stroke, Fill, Markers"));
    cmbPaintOrder->addItem(KisIconUtils::loadIcon("paint-order-stroke-marker-fill"), i18n("Stroke, Markers, Fill"));
    cmbPaintOrder->addItem(KisIconUtils::loadIcon("paint-order-marker-fill-stroke"), i18n("Markers, Fill, Stroke"));
    cmbPaintOrder->addItem(KisIconUtils::loadIcon("paint-order-marker-stroke-fill"), i18n("Markers, Stroke, Fill"));
    connect(cmbPaintOrder, SIGNAL(currentIndexChanged(int)), SLOT(slotPaintOrderChanged()));

    connect(btnInternalShapes, SIGNAL(toggled(bool)), this, SLOT(slotUpdateInternalShapes()));


    dblOpacity->setRange(0.0, 1.0, 2);
    dblOpacity->setSingleStep(0.01);
    dblOpacity->setFastSliderStep(0.1);
    dblOpacity->setTextTemplates(i18nc("{n} is the number value, % is the percent sign", "Opacity: {n}"),
                                 i18nc("{n} is the number value, % is the percent sign", "Opacity [*varies*]: {n}"));

    dblOpacity->setValueGetter(
        [](KoShape *s) { return 1.0 - s->transparency(); }
    );

    connect(dblOpacity, SIGNAL(valueChanged(qreal)), SLOT(slotOpacitySliderChanged(qreal)));

    // cold init
    slotUpdateOpacitySlider();
}

DefaultToolGeometryWidget::~DefaultToolGeometryWidget()
{
}

namespace {

void tryAnchorPosition(KoFlake::AnchorPosition anchor,
                       const QRectF &rect,
                       QPointF *position)
{
    bool valid = false;
    QPointF anchoredPosition = KoFlake::anchorToPoint(anchor, rect, &valid);

    if (valid) {
        *position = anchoredPosition;
    }
}

QRectF calculateSelectionBounds(KoSelection *selection,
                                KoFlake::AnchorPosition anchor,
                                bool useGlobalSize,
                                QList<KoShape*> *outShapes = 0)
{
    QList<KoShape*> shapes = selection->selectedEditableShapes();

    KoShape *shape = shapes.size() == 1 ? shapes.first() : selection;

    QRectF resultRect = shape->outlineRect();

    QPointF resultPoint = resultRect.topLeft();
    tryAnchorPosition(anchor, resultRect, &resultPoint);

    if (useGlobalSize) {
        resultRect = shape->absoluteTransformation().mapRect(resultRect);
    } else {
        /**
         * Some shapes, e.g. KoSelection and KoShapeGroup don't have real size() and
         * do all the resizing with transformation(), just try to cover this case and
         * fetch their scale using the transform.
         */

        KisAlgebra2D::DecomposedMatrix matrix(shape->transformation());
        resultRect = matrix.scaleTransform().mapRect(resultRect);
    }

    resultPoint = shape->absoluteTransformation().map(resultPoint);

    if (outShapes) {
        *outShapes = shapes;
    }

    return QRectF(resultPoint, resultRect.size());
}

}

void DefaultToolGeometryWidget::slotAnchorPointChanged()
{
    if (!isVisible()) return;

    QVariant newValue(positionSelector->value());
    m_tool->canvas()->resourceManager()->setResource(DefaultTool::HotPosition, newValue);
    slotUpdatePositionBoxes();
}

void DefaultToolGeometryWidget::slotUpdateCheckboxes()
{
    if (!isVisible()) return;

    KoSelection *selection = m_tool->canvas()->selectedShapesProxy()->selection();
    QList<KoShape*> shapes = selection->selectedEditableShapes();

    KoShapeGroup *onlyGroupShape = 0;

    if (shapes.size() == 1) {
        onlyGroupShape = dynamic_cast<KoShapeGroup*>(shapes.first());
    }

    const bool uniformScalingAvailable = shapes.size() <= 1 && !onlyGroupShape;

    if (uniformScalingAvailable && !chkUniformScaling->isEnabled()) {
        chkUniformScaling->setChecked(m_savedUniformScaling);
        chkUniformScaling->setEnabled(uniformScalingAvailable);
    } else if (!uniformScalingAvailable && chkUniformScaling->isEnabled()) {
        m_savedUniformScaling = chkUniformScaling->isChecked();
        chkUniformScaling->setChecked(true);
        chkUniformScaling->setEnabled(uniformScalingAvailable);
    }

    // TODO: not implemented yet!
    chkAnchorLock->setEnabled(false);
}

void DefaultToolGeometryWidget::slotAspectButtonToggled()
{
    KoSelection *selection = m_tool->canvas()->selectedShapesProxy()->selection();
    QList<KoShape*> shapes = selection->selectedEditableShapes();

    KUndo2Command *cmd =
        new KoShapeKeepAspectRatioCommand(shapes, aspectButton->keepAspectRatio());

    m_tool->canvas()->addCommand(cmd);
}

void DefaultToolGeometryWidget::slotUpdateAspectButton()
{
    if (!isVisible()) return;

    KoSelection *selection = m_tool->canvas()->selectedShapesProxy()->selection();
    QList<KoShape*> shapes = selection->selectedEditableShapes();

    bool hasKeepAspectRatio = false;
    bool hasNotKeepAspectRatio = false;

    Q_FOREACH (KoShape *shape, shapes) {
        if (shape->keepAspectRatio()) {
            hasKeepAspectRatio = true;
        } else {
            hasNotKeepAspectRatio = true;
        }

        if (hasKeepAspectRatio && hasNotKeepAspectRatio) break;
    }

    Q_UNUSED(hasNotKeepAspectRatio); // TODO: use for tristated mode of the checkbox

    const bool useGlobalSize = chkGlobalCoordinates->isChecked();
    const KoFlake::AnchorPosition anchor = positionSelector->value();
    const QRectF bounds = calculateSelectionBounds(selection, anchor, useGlobalSize);
    const bool hasNullDimensions = bounds.isEmpty();

    aspectButton->setKeepAspectRatio(hasKeepAspectRatio && !hasNullDimensions);
    aspectButton->setEnabled(!hasNullDimensions);
}

//namespace {
//qreal calculateCommonShapeTransparency(const QList<KoShape*> &shapes)
//{
//    qreal commonTransparency = -1.0;

//    Q_FOREACH (KoShape *shape, shapes) {
//        if (commonTransparency < 0) {
//            commonTransparency = shape->transparency();
//        } else if (!qFuzzyCompare(commonTransparency, shape->transparency())) {
//            commonTransparency = -1.0;
//            break;
//        }
//    }

//    return commonTransparency;
//}
//}

void DefaultToolGeometryWidget::slotOpacitySliderChanged(qreal newOpacity)
{
    KoSelection *selection = m_tool->canvas()->selectedShapesProxy()->selection();
    QList<KoShape*> shapes = selection->selectedEditableShapes();
    if (shapes.isEmpty()) return;

    KUndo2Command *cmd =
        new KoShapeTransparencyCommand(shapes, 1.0 - newOpacity);

    m_tool->canvas()->addCommand(cmd);
}

void DefaultToolGeometryWidget::slotUpdateOpacitySlider()
{
    if (!isVisible()) return;

    KoSelection *selection = m_tool->canvas()->selectedShapesProxy()->selection();
    QList<KoShape*> shapes = selection->selectedEditableShapes();

    dblOpacity->setSelection(shapes);
}

void DefaultToolGeometryWidget::slotPaintOrderChanged()
{
    KoSelection *selection = m_tool->canvas()->selectedShapesProxy()->selection();
    QList<KoShape*> shapes = selection->selectedEditableShapes();
    if (shapes.isEmpty()) return;

    KoShape::PaintOrder first = KoShape::Fill;
    KoShape::PaintOrder second = KoShape::Stroke;

    switch(cmbPaintOrder->currentIndex()) {
    case 1:
        first = KoShape::Fill;
        second = KoShape::Markers;
        break;
    case 2:
        first = KoShape::Stroke;
        second = KoShape::Fill;
        break;
    case 3:
        first = KoShape::Stroke;
        second = KoShape::Markers;
        break;
    case 4:
        first = KoShape::Markers;
        second = KoShape::Fill;
        break;
    case 5:
        first = KoShape::Markers;
        second = KoShape::Stroke;
        break;
    default:
        first = KoShape::Fill;
        second = KoShape::Stroke;
    }

    KUndo2Command *cmd =
        new KoShapePaintOrderCommand(shapes, first, second);

    m_tool->canvas()->addCommand(cmd);
}

void DefaultToolGeometryWidget::slotUpdatePaintOrder() {
    if (!isVisible()) return;

    KoSelection *selection = m_tool->canvas()->selectedShapesProxy()->selection();
    QList<KoShape*> shapes = selection->selectedEditableShapes();

    if (!shapes.isEmpty()) {
        KoShape *shape = shapes.first();
        QVector<KoShape::PaintOrder> paintOrder = shape->paintOrder();
        int index = 0;
        if (paintOrder.first() == KoShape::Fill) {
            index = paintOrder.at(1) == KoShape::Stroke? 0: 1;
        } else if (paintOrder.first() == KoShape::Stroke) {
            index = paintOrder.at(1) == KoShape::Fill? 2: 3;
        } else {
            index = paintOrder.at(1) == KoShape::Fill? 4: 5;
        }
        cmbPaintOrder->setCurrentIndex(index);
    }
}


void DefaultToolGeometryWidget::slotUpdateInternalShapes()
{
    KisCanvas2 *canvas = dynamic_cast<KisCanvas2*>(m_tool->canvas());
    if (canvas) {
        canvas->setTextShapeManagerEnabled(btnInternalShapes->isChecked());
    }
}

void DefaultToolGeometryWidget::slotUpdateSizeBoxes(bool updateAspect)
{
    if (!isVisible()) return;

    const bool useGlobalSize = chkGlobalCoordinates->isChecked();
    const KoFlake::AnchorPosition anchor = positionSelector->value();

    KoSelection *selection = m_tool->canvas()->selectedShapesProxy()->selection();
    const QRectF bounds = calculateSelectionBounds(selection, anchor, useGlobalSize);

    const bool hasSizeConfiguration = !bounds.isNull();

    widthSpinBox->setEnabled(hasSizeConfiguration && bounds.width() > 0);
    heightSpinBox->setEnabled(hasSizeConfiguration && bounds.height() > 0);

    if (hasSizeConfiguration) {
        KisSignalsBlocker b(widthSpinBox, heightSpinBox);
        widthSpinBox->changeValue(bounds.width());
        heightSpinBox->changeValue(bounds.height());

        if (updateAspect) {
            m_sizeAspectLocker->updateAspect();
        }
    }
}

void DefaultToolGeometryWidget::slotUpdateSizeBoxesNoAspectChange()
{
    slotUpdateSizeBoxes(false);
}

void DefaultToolGeometryWidget::slotUpdatePositionBoxes()
{
    if (!isVisible()) return;

    const bool useGlobalSize = chkGlobalCoordinates->isChecked();
    const KoFlake::AnchorPosition anchor = positionSelector->value();

    KoSelection *selection = m_tool->canvas()->selectedShapesProxy()->selection();
    QRectF bounds = calculateSelectionBounds(selection, anchor, useGlobalSize);

    const bool hasSizeConfiguration = !bounds.isNull();

    positionXSpinBox->setEnabled(hasSizeConfiguration);
    positionYSpinBox->setEnabled(hasSizeConfiguration);

    if (hasSizeConfiguration) {
        KisSignalsBlocker b(positionXSpinBox, positionYSpinBox);
        positionXSpinBox->changeValue(bounds.x());
        positionYSpinBox->changeValue(bounds.y());
    }
}

void DefaultToolGeometryWidget::slotRepositionShapes()
{
    static const qreal eps = 1e-6;

    const bool useGlobalSize = chkGlobalCoordinates->isChecked();
    const KoFlake::AnchorPosition anchor = positionSelector->value();

    QList<KoShape*> shapes;
    KoSelection *selection = m_tool->canvas()->selectedShapesProxy()->selection();
    QRectF bounds = calculateSelectionBounds(selection, anchor, useGlobalSize, &shapes);

    if (bounds.isNull()) return;

    const QPointF oldPosition = bounds.topLeft();
    const QPointF newPosition(positionXSpinBox->value(), positionYSpinBox->value());
    const QPointF diff = newPosition - oldPosition;

    if (diff.manhattanLength() < eps) return;

    QList<QPointF> oldPositions;
    QList<QPointF> newPositions;

    Q_FOREACH (KoShape *shape, shapes) {
        const QPointF oldShapePosition = shape->absolutePosition(anchor);

        oldPositions << shape->absolutePosition(anchor);
        newPositions << oldShapePosition + diff;
    }

    KUndo2Command *cmd = new KoShapeMoveCommand(shapes, oldPositions, newPositions, anchor);
    m_tool->canvas()->addCommand(cmd);
}

void DefaultToolGeometryWidget::slotResizeShapes()
{
    static const qreal eps = 1e-4;

    const bool useGlobalSize = chkGlobalCoordinates->isChecked();
    const KoFlake::AnchorPosition anchor = positionSelector->value();

    QList<KoShape*> shapes;
    KoSelection *selection = m_tool->canvas()->selectedShapesProxy()->selection();
    QRectF bounds = calculateSelectionBounds(selection, anchor, useGlobalSize, &shapes);

    if (bounds.isNull()) return;

    const QSizeF oldSize(bounds.size());

    QSizeF newSize(widthSpinBox->value(), heightSpinBox->value());
    newSize = KisAlgebra2D::ensureSizeNotSmaller(newSize, QSizeF(eps, eps));

    const qreal scaleX = oldSize.width() > 0 ? newSize.width() / oldSize.width() : 1.0;
    const qreal scaleY = oldSize.height() > 0 ? newSize.height() / oldSize.height() : 1.0;

    if (qAbs(scaleX - 1.0) < eps && qAbs(scaleY - 1.0) < eps) return;

    const bool usePostScaling =
        shapes.size() > 1 || chkUniformScaling->isChecked();

    KUndo2Command *cmd = new KoShapeResizeCommand(shapes,
                                                  scaleX, scaleY,
                                                  bounds.topLeft(),
                                                  useGlobalSize,
                                                  usePostScaling,
                                                  selection->transformation());
    m_tool->canvas()->addCommand(cmd);
}

void DefaultToolGeometryWidget::setUnit(const KoUnit &unit)
{
    positionXSpinBox->setUnit(unit);
    positionYSpinBox->setUnit(unit);
    widthSpinBox->setUnit(unit);
    heightSpinBox->setUnit(unit);

    positionXSpinBox->setDecimals(2);
    positionYSpinBox->setDecimals(2);
    widthSpinBox->setDecimals(2);
    heightSpinBox->setDecimals(2);

    // here we need to ensure that even for pixels unit, we can use decimals
    positionXSpinBox->preventDecimalsChangeFromUnitManager(true);
    positionYSpinBox->preventDecimalsChangeFromUnitManager(true);
    widthSpinBox->preventDecimalsChangeFromUnitManager(true);
    heightSpinBox->preventDecimalsChangeFromUnitManager(true);

    positionXSpinBox->setLineStep(1.0);
    positionYSpinBox->setLineStep(1.0);
    widthSpinBox->setLineStep(1.0);
    heightSpinBox->setLineStep(1.0);

    slotUpdatePositionBoxes();
    slotUpdateSizeBoxes();
}

bool DefaultToolGeometryWidget::useUniformScaling() const
{
    return chkUniformScaling->isChecked();
}

void DefaultToolGeometryWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    slotUpdatePositionBoxes();
    slotUpdateSizeBoxes();
    slotUpdateOpacitySlider();
    slotUpdateAspectButton();
    slotUpdateCheckboxes();
    slotAnchorPointChanged();
    slotUpdatePaintOrder();
}

void DefaultToolGeometryWidget::resourceChanged(int key, const QVariant &res)
{
    if (key == KoCanvasResource::Unit) {
        setUnit(res.value<KoUnit>());
    } else if (key == DefaultTool::HotPosition) {
        positionSelector->setValue(KoFlake::AnchorPosition(res.toInt()));
    }
}
