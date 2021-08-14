/*
 *  SPDX-FileCopyrightText: 2017 Eugene Ingerman
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ToolReferenceImagesWidget.h"

#include "ui_WdgToolOptions.h"

#include <KoSelection.h>
#include <KoShapeTransparencyCommand.h>
#include <KoShapeKeepAspectRatioCommand.h>
#include <kis_config.h>
#include <kis_signals_blocker.h>
#include <kis_signal_compressor.h>
#include <KisReferenceImage.h>
#include <KisDocument.h>
#include <KisReferenceImagesLayer.h>
#include <KisReferenceImage.h>
#include "KisViewManager.h"

#include <QClipboard>
#include <QApplication>
#include <QStandardItemModel>

#include "ToolReferenceImages.h"

struct ToolReferenceImagesWidget::Private {
    Private(ToolReferenceImages *tool)
    : tool(tool)
    {
    }

    Ui_WdgToolOptions *ui;
    ToolReferenceImages *tool;
};

ToolReferenceImagesWidget::ToolReferenceImagesWidget(ToolReferenceImages *tool, KisCanvasResourceProvider */*provider*/, QWidget *parent)
    : QWidget(parent),
      d(new Private(tool))
{
    d->ui = new Ui_WdgToolOptions();
    d->ui->setupUi(this);

    d->ui->opacitySlider->setRange(0, 100);
    d->ui->opacitySlider->setPrefixes(i18n("Opacity: "), i18n("Opacity [*varies*]: "));
    d->ui->opacitySlider->setSuffix(i18n(" %"));
    d->ui->opacitySlider->setValueGetter(
        [](KoShape *s){ return 100.0 * (1.0 - s->transparency()); }
    );

    d->ui->saturationSlider->setRange(0, 100);
    d->ui->saturationSlider->setPrefixes(i18n("Saturation: "), i18n("Saturation [*varies*]: "));
    d->ui->saturationSlider->setSuffix(i18n(" %"));
    d->ui->saturationSlider->setValueGetter(
        [](KoShape *s){
            auto *r = dynamic_cast<KisReferenceImage*>(s);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(r, 0.0);
            return 100.0 * r->saturation();
        }
    );

    d->ui->sldOffsetX->setPrefixes(i18n("X: "), i18n("X [*varies*]: "));
    d->ui->sldOffsetX->setValueGetter(
        [](KoShape *s){
            KisReferenceImage *r = dynamic_cast<KisReferenceImage*>(s);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(r, 0.0);
            return r->cropRect().x();
        }
    );

    d->ui->sldOffsetY->setPrefixes(i18n("Y: "), i18n("Y [*varies*]: "));
    d->ui->sldOffsetY->setValueGetter(
        [](KoShape *s){
            KisReferenceImage *r = dynamic_cast<KisReferenceImage*>(s);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(r, 0.0);
            return r->cropRect().y();
        }
    );

    d->ui->sldWidth->setPrefixes(i18n("W: "), i18n("W [*varies*]: "));
    d->ui->sldWidth->setValueGetter(
        [](KoShape *s){
            KisReferenceImage *r = dynamic_cast<KisReferenceImage*>(s);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(r, 0.0);
            return r->cropRect().width();
        }
    );

    d->ui->sldHeight->setPrefixes(i18n("H: "), i18n("H [*varies*]: "));
    d->ui->sldHeight->setValueGetter(
        [](KoShape *s){
            KisReferenceImage *r = dynamic_cast<KisReferenceImage*>(s);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(r, 0.0);
            return r->cropRect().height();
        }
    );

    d->ui->bnAddReferenceImage->setToolTip(i18n("Add Reference Image"));
    d->ui->bnAddReferenceImage->setIcon(KisIconUtils::loadIcon("list-add"));
    d->ui->bnAddReferenceImage->setIconSize(QSize(16, 16));


    d->ui->bnDelete->setToolTip(i18n("Delete all Reference Images"));
    d->ui->bnDelete->setIcon(KisIconUtils::loadIcon("edit-delete"));
    d->ui->bnDelete->setIconSize(QSize(16, 16));

    d->ui->bnLoad->setToolTip(i18n("Load Reference Images Set"));
    d->ui->bnLoad->setIcon(KisIconUtils::loadIcon("folder"));
    d->ui->bnLoad->setIconSize(QSize(16, 16));

    d->ui->bnSave->setToolTip(i18n("Export Reference Images Set"));
    d->ui->bnSave->setIcon(KisIconUtils::loadIcon("document-save-16"));
    d->ui->bnSave->setIconSize(QSize(16, 16));

    d->ui->bnPasteReferenceImage->setToolTip(i18n("Paste Reference Image From System Clipboard"));
    d->ui->bnPasteReferenceImage->setIcon(KisIconUtils::loadIcon("edit-paste-16"));
    d->ui->bnPasteReferenceImage->setIconSize(QSize(16, 16));

    d->ui->bnCrop->setVisible(false);
    d->ui->bnCrop->setCheckable(true);
    d->ui->bnCrop->setToolTip(i18n("Crop selected Reference Image"));
    d->ui->bnCrop->setIcon(KisIconUtils::loadIcon("tool_crop"));
    d->ui->bnCrop->setIconSize(QSize(16, 16));

    d->ui->grpCrop->setVisible(false);
    d->ui->bnCancel->setVisible(false);
    d->ui->lblPin->setVisible(false);

    connect(d->ui->bnAddReferenceImage, SIGNAL(clicked()), tool, SLOT(addReferenceImage()));
    connect(d->ui->bnPasteReferenceImage, SIGNAL(clicked()), tool, SLOT(pasteReferenceImage()));

    connect(d->ui->bnDelete, SIGNAL(clicked()), tool, SLOT(removeAllReferenceImages()));
    connect(d->ui->bnSave, SIGNAL(clicked()), tool, SLOT(saveReferenceImages()));
    connect(d->ui->bnLoad, SIGNAL(clicked()), tool, SLOT(loadReferenceImages()));
    connect(d->ui->bnCrop, SIGNAL(toggled(bool)), this, SLOT(slotUpdateCrop(bool)));
    connect(d->ui->bnCancel, SIGNAL(clicked()), this, SLOT(slotCancelCrop()));


    connect(d->ui->chkKeepAspectRatio, SIGNAL(stateChanged(int)), this, SLOT(slotKeepAspectChanged()));
    connect(d->ui->chkPinRotate, SIGNAL(stateChanged(int)), this, SLOT(slotRotateChanged()));
    connect(d->ui->chkPinMirror, SIGNAL(stateChanged(int)), this, SLOT(slotMirrorChanged()));
    connect(d->ui->chkPinPos, SIGNAL(stateChanged(int)), this, SLOT(slotPositionChanged()));
    connect(d->ui->chkPinZoom, SIGNAL(stateChanged(int)), this, SLOT(slotZoomChanged()));
    connect(d->ui->chkPinAll, SIGNAL(stateChanged(int)), this, SLOT(slotPinAllChanged()));;

    KisSignalCompressor *compressor = new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE, this);
    connect(compressor, SIGNAL(timeout()), this, SLOT(slotImageValuesChanged()));

    connect(d->ui->saturationSlider, SIGNAL(valueChanged(qreal)), compressor, SLOT(start()));
    connect(d->ui->opacitySlider, SIGNAL(valueChanged(qreal)), compressor, SLOT(start()));

    connect(d->ui->sldOffsetX, SIGNAL(valueChanged(qreal)), this, SLOT(slotCropValuesChanged()));
    connect(d->ui->sldOffsetY, SIGNAL(valueChanged(qreal)), this, SLOT(slotCropValuesChanged()));
    connect(d->ui->sldWidth, SIGNAL(valueChanged(qreal)), this, SLOT(slotCropValuesChanged()));
    connect(d->ui->sldHeight, SIGNAL(valueChanged(qreal)), this, SLOT(slotCropValuesChanged()));

    d->ui->referenceImageLocationCombobox->addItem(i18n("Embed to .KRA"));
    d->ui->referenceImageLocationCombobox->addItem(i18n("Link to Image"));
    connect(d->ui->referenceImageLocationCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSaveLocationChanged(int)));

    updateVisibility(false); // no selection when we start
}

ToolReferenceImagesWidget::~ToolReferenceImagesWidget()
{
}

void ToolReferenceImagesWidget::selectionChanged(KoSelection *selection)
{
    QList<KoShape*> shapes = selection->selectedEditableShapes();

    d->ui->opacitySlider->setSelection(shapes);
    d->ui->saturationSlider->setSelection(shapes);

    if(d->tool->activeReferenceImage()) {
        updateCropSliders();
    }
    else {
        d->ui->grpCrop->setVisible(false);
        d->ui->bnCancel->setVisible(false);
    }


    bool anyKeepingAspectRatio = false;
    bool anyNotKeepingAspectRatio = false;
    bool anyEmbedded = false;
    bool anyLinked = false;
    bool anyNonLinkable = false;
    bool anySelected = selection->count() > 0;

    Q_FOREACH(KoShape *shape, shapes) {
        KisReferenceImage *reference = dynamic_cast<KisReferenceImage*>(shape);

        anyKeepingAspectRatio |= shape->keepAspectRatio();
        anyNotKeepingAspectRatio |= !shape->keepAspectRatio();

        if (reference) {
            anyEmbedded |= reference->embed();
            anyLinked |= !reference->embed();
            anyNonLinkable |= !reference->hasLocalFile();
        }
    }

    KisSignalsBlocker blocker(
        d->ui->chkKeepAspectRatio,
        d->ui->referenceImageLocationCombobox

    );

    d->ui->chkKeepAspectRatio->setCheckState(
        (anyKeepingAspectRatio && anyNotKeepingAspectRatio) ? Qt::PartiallyChecked :
         anyKeepingAspectRatio ? Qt::Checked : Qt::Unchecked);



    // set save location combobox
    bool imagesEmbedded = anyEmbedded && !anyLinked;
    int comboBoxIndex = imagesEmbedded ? 0 : 1; // maps state to combobox index
    d->ui->referenceImageLocationCombobox->setCurrentIndex(comboBoxIndex);


    updateVisibility(anySelected);
}

void ToolReferenceImagesWidget::slotKeepAspectChanged()
{
    KoSelection *selection = d->tool->koSelection();
    QList<KoShape*> shapes = selection->selectedEditableShapes();

    KUndo2Command *cmd =
            new KoShapeKeepAspectRatioCommand(shapes, d->ui->chkKeepAspectRatio->isChecked());

    d->tool->canvas()->addCommand(cmd);
}

void ToolReferenceImagesWidget::slotOpacitySliderChanged(qreal newOpacity)
{
    QList<KoShape*> shapes = d->ui->opacitySlider->selection();
    if (shapes.isEmpty()) return;

    KUndo2Command *cmd =
        new KoShapeTransparencyCommand(shapes, 1.0 - newOpacity / 100.0);

    d->tool->canvas()->addCommand(cmd);
}

void ToolReferenceImagesWidget::slotSaturationSliderChanged(qreal newSaturation)
{
    QList<KoShape*> shapes = d->ui->saturationSlider->selection();
    if (shapes.isEmpty()) return;

    KUndo2Command *cmd =
            new KisReferenceImage::SetSaturationCommand(shapes, newSaturation / 100.0);

    d->tool->canvas()->addCommand(cmd);
}

void ToolReferenceImagesWidget::slotSaveLocationChanged(int index)
{
    KoSelection *selection = d->tool->koSelection();
    QList<KoShape*> shapes = selection->selectedEditableShapes();


    Q_FOREACH(KoShape *shape, shapes) {
        KisReferenceImage *reference = dynamic_cast<KisReferenceImage*>(shape);
        KIS_SAFE_ASSERT_RECOVER_RETURN(reference);

        if (index == 0) { // embed to KRA
            reference->setEmbed(true);
        } else { // link to file
            if (reference->hasLocalFile()) {
                reference->setEmbed(false);
            } else {
                //In the case no local file is found, switch back to embed file data.
                d->ui->referenceImageLocationCombobox->setCurrentIndex(0);
            }
        }
    }
}

void ToolReferenceImagesWidget::slotImageValuesChanged()
{
    slotSaturationSliderChanged(d->ui->saturationSlider->value());
    slotOpacitySliderChanged(d->ui->opacitySlider->value());
}

void ToolReferenceImagesWidget::updateVisibility(bool hasSelection)
{
    // hide UI elements if nothing is selected.
    d->ui->referenceImageLocationCombobox->setVisible(hasSelection);
    d->ui->chkKeepAspectRatio->setVisible(hasSelection);
    d->ui->saveLocationLabel->setVisible(hasSelection);
    d->ui->opacitySlider->setVisible(hasSelection);
    d->ui->saturationSlider->setVisible(hasSelection);
    d->ui->chkPinRotate->setVisible(hasSelection);
    d->ui->chkPinPos->setVisible(hasSelection);
    d->ui->chkPinMirror->setVisible(hasSelection);
    d->ui->chkPinZoom->setVisible(hasSelection);
    d->ui->chkPinAll->setVisible(hasSelection);
    d->ui->lblPin->setVisible(hasSelection);

    KisReferenceImage *ref = d->tool->activeReferenceImage();
    d->ui->bnCrop->setVisible(hasSelection);
        if(ref) {
            d->ui->chkPinMirror->setChecked(ref->pinMirror());
            d->ui->chkPinPos->setChecked(ref->pinPosition());
            d->ui->chkPinRotate->setChecked(ref->pinRotate());
            d->ui->chkPinZoom->setChecked(ref->pinZoom());
            d->ui->chkPinAll->setChecked(ref->pinAll());
        }


    // show a label indicating that a selection is required to show options
    d->ui->referenceImageOptionsLabel->setVisible(!hasSelection);

    if (hasSelection) {
        KoSelection* selection = d->tool->koSelection();
        QList<KoShape*> shapes = selection->selectedEditableShapes();
        bool usesLocalFile = true;

        Q_FOREACH(KoShape *shape, shapes) {
            KisReferenceImage *reference = dynamic_cast<KisReferenceImage*>(shape);

            if (reference) {
                usesLocalFile &= reference->hasLocalFile();
            }
        }

        QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(d->ui->referenceImageLocationCombobox->model());

        if (model) {
            QStandardItem* item = model->item(1);
            item->setFlags(usesLocalFile ? item->flags() | Qt::ItemIsEnabled :
                                           item->flags() & ~Qt::ItemIsEnabled);
        }
    }
}

void ToolReferenceImagesWidget::slotUpdateCrop(bool value)
{
    KisReferenceImage* ref = d->tool->activeReferenceImage();
    if(!ref) return;

    d->ui->bnCrop->setChecked(value);
    bool enable = d->ui->bnCrop->isChecked();
    d->ui->grpCrop->setVisible(enable);
    d->ui->bnCancel->setVisible(enable);
    ref->setCrop(enable, QRectF());

    if(enable) {
        updateCropSliders();
        //ref->setCrop(enable, QRectF());
    }
    else {
        if(d->ui->sldOffsetX->value() > 0 || d->ui->sldOffsetY->value() > 0
                || !d->ui->sldWidth->isMaximized() || !d->ui->sldHeight->isMaximized()) {
            //ref->setCrop(enable, cropRect());
            KUndo2Command *cmd =
                     new KisReferenceImage::CropReferenceImage(ref, cropRect());
            d->tool->canvas()->addCommand(cmd);
        }
    }
}

void ToolReferenceImagesWidget::slotCropValuesChanged()
{
    KisReferenceImage *ref = d->tool->activeReferenceImage();
    if(ref) {

        KisCanvas2 *kiscanvas = dynamic_cast<KisCanvas2*>(d->tool->canvas());
        const KisCoordinatesConverter *converter = kiscanvas->coordinatesConverter();

        QRectF rect = converter->documentToImage(ref->outlineRect());

        qreal x = d->ui->sldOffsetX->value();
        qreal y = d->ui->sldOffsetY->value();
        qreal width = d->ui->sldWidth->value();
        qreal height = d->ui->sldHeight->value();


        if(x + width > rect.width()) {
            width = rect.width() - x;
            d->ui->sldWidth->blockSignals(true);
            d->ui->sldWidth->setValue(width);
            d->ui->sldWidth->blockSignals(false);
        }
        if(y + height > rect.height())
        {
            height = rect.height() - y;
            d->ui->sldHeight->blockSignals(true);
            d->ui->sldHeight->setValue(height);
            d->ui->sldHeight->blockSignals(false);
        }

        QRectF finalRect = converter->imageToDocument(QRectF(x, y, width, height));
        ref->setCropRect(finalRect);
    }
}

void ToolReferenceImagesWidget::updateCropSliders()
{
    KisReferenceImage* ref = d->tool->activeReferenceImage();
    if(!ref) return;

    QList<KoShape*> shape;
    shape.append(ref);

    KisCanvas2 *kiscanvas = dynamic_cast<KisCanvas2*>(d->tool->canvas());
    const KisCoordinatesConverter *converter = kiscanvas->coordinatesConverter();

    QRectF rect = converter->documentToImage(ref->cropRect());
    qreal width = rect.width() - d->ui->sldOffsetX->value();
    qreal height = rect.height() - d->ui->sldOffsetY->value();

    d->ui->sldOffsetX->setSelection(shape);
    d->ui->sldOffsetX->setRange(0,rect.width());
    d->ui->sldOffsetX->blockSignals(true);
    d->ui->sldOffsetX->setValue(0);
    d->ui->sldOffsetX->blockSignals(false);

    d->ui->sldOffsetY->setSelection(shape);
    d->ui->sldOffsetY->setRange(0,rect.height());
    d->ui->sldOffsetY->blockSignals(true);
    d->ui->sldOffsetY->setValue(0);
    d->ui->sldOffsetY->blockSignals(false);

    d->ui->sldWidth->setSelection(shape);
    d->ui->sldWidth->setRange(0,rect.width());
    d->ui->sldWidth->blockSignals(true);
    d->ui->sldWidth->setValue(width);
    d->ui->sldWidth->blockSignals(false);

    d->ui->sldHeight->setSelection(shape);
    d->ui->sldHeight->setRange(0,rect.height());
    d->ui->sldHeight->blockSignals(true);
    d->ui->sldHeight->setValue(height);
    d->ui->sldHeight->blockSignals(false);

    QRectF finalRect = converter->imageToDocument(QRectF(0, 0, width, height));
    ref->setCropRect(finalRect);
}

void ToolReferenceImagesWidget::slotCropRectChanged()
{
    KisReferenceImage *ref = d->tool->activeReferenceImage();
    if(!ref) return;

    KisCanvas2 *kiscanvas = dynamic_cast<KisCanvas2*>(d->tool->canvas());
    const KisCoordinatesConverter *converter = kiscanvas->coordinatesConverter();
    QRectF rect = converter->documentToImage(ref->cropRect());

    d->ui->sldOffsetX->blockSignals(true);
    d->ui->sldOffsetX->setValue(rect.topLeft().x());
    d->ui->sldOffsetX->blockSignals(false);

    d->ui->sldOffsetY->blockSignals(true);
    d->ui->sldOffsetY->setValue(rect.topLeft().y());
    d->ui->sldOffsetY->blockSignals(false);

    d->ui->sldWidth->blockSignals(true);
    d->ui->sldWidth->setValue(rect.width());
    d->ui->sldWidth->blockSignals(false);

    d->ui->sldHeight->blockSignals(true);
    d->ui->sldHeight->setValue(rect.height());
    d->ui->sldHeight->blockSignals(false);
}

QRectF ToolReferenceImagesWidget::cropRect()
{
    KisCanvas2 *kiscanvas = dynamic_cast<KisCanvas2*>(d->tool->canvas());
    const KisCoordinatesConverter *converter = kiscanvas->coordinatesConverter();

    qreal x = d->ui->sldOffsetX->value();
    qreal y = d->ui->sldOffsetY->value();
    qreal width = d->ui->sldWidth->value();
    qreal height = d->ui->sldHeight->value();

    QRectF finalRect = converter->imageToDocument(QRectF(x, y, width, height));
    return finalRect;
}

void ToolReferenceImagesWidget::slotMirrorChanged()
{
    KisReferenceImage *ref= d->tool->activeReferenceImage();
        if(ref) {
            ref->setPinMirror(d->ui->chkPinMirror->isChecked());
     }
}

void ToolReferenceImagesWidget::slotPositionChanged()
{
    KisReferenceImage *ref= d->tool->activeReferenceImage();
        if(ref) {
            ref->setPinPosition(d->ui->chkPinPos->isChecked());
    }
}

void ToolReferenceImagesWidget::slotRotateChanged()
{
    KisReferenceImage *ref= d->tool->activeReferenceImage();
        if(ref) {
            ref->setPinRotate(d->ui->chkPinRotate->isChecked());
        }
}

void ToolReferenceImagesWidget::slotZoomChanged()
{
    KisReferenceImage *ref= d->tool->activeReferenceImage();
        if(ref) {
            ref->setPinZoom(d->ui->chkPinZoom->isChecked());
        }
}

void ToolReferenceImagesWidget::slotPinAllChanged()
{
    KisReferenceImage *ref= d->tool->activeReferenceImage();
        if(ref) {
            bool pinAll = d->ui->chkPinAll->isChecked();
            ref->setPinAll(pinAll);

            d->ui->chkPinMirror->setChecked(pinAll);
            d->ui->chkPinZoom->setChecked(pinAll);
            d->ui->chkPinPos->setChecked(pinAll);
            d->ui->chkPinRotate->setChecked(pinAll);
        }
}

void ToolReferenceImagesWidget::slotCancelCrop()
{
    KisReferenceImage* ref = d->tool->activeReferenceImage();
    if(!ref) return;

    d->ui->bnCrop->blockSignals(true);
    d->ui->bnCrop->setChecked(false);
    d->ui->bnCrop->blockSignals(false);
    d->ui->grpCrop->setVisible(false);
    d->ui->bnCancel->setVisible(false);
    ref->setCrop(false, QRectF());
}
