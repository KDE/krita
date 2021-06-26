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

    d->ui->bnLock->setVisible(false);
    d->ui->bnLock->setCheckable(true);

    d->ui->bnCrop->setVisible(false);
    d->ui->bnCrop->setCheckable(true);
    d->ui->bnCrop->setToolTip(i18n("Crop selected Reference Image"));
    d->ui->bnCrop->setIcon(KisIconUtils::loadIcon("tool_crop"));
    d->ui->bnCrop->setIconSize(QSize(16, 16));

    d->ui->grpCrop->setVisible(false);

    connect(d->ui->bnAddReferenceImage, SIGNAL(clicked()), tool, SLOT(addReferenceImage()));
    connect(d->ui->bnPasteReferenceImage, SIGNAL(clicked()), tool, SLOT(pasteReferenceImage()));

    connect(d->ui->bnDelete, SIGNAL(clicked()), tool, SLOT(removeAllReferenceImages()));
    connect(d->ui->bnSave, SIGNAL(clicked()), tool, SLOT(saveReferenceImages()));
    connect(d->ui->bnLoad, SIGNAL(clicked()), tool, SLOT(loadReferenceImages()));
    connect(d->ui->bnLock, SIGNAL(toggled(bool)), this, SLOT(slotUpdateLock(bool)));
    connect(d->ui->bnCrop, SIGNAL(toggled(bool)), this, SLOT(slotUpdateCrop(bool)));

    connect(d->ui->chkKeepAspectRatio, SIGNAL(stateChanged(int)), this, SLOT(slotKeepAspectChanged()));

    KisSignalCompressor *compressor = new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE, this);
    connect(compressor, SIGNAL(timeout()), this, SLOT(slotImageValuesChanged()));

    connect(d->ui->saturationSlider, SIGNAL(valueChanged(qreal)), compressor, SLOT(start()));
    connect(d->ui->opacitySlider, SIGNAL(valueChanged(qreal)), compressor, SLOT(start()));

    KisSignalCompressor *cropCompressor = new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE, this);
    connect(cropCompressor, SIGNAL(timeout()), this, SLOT(slotCropValuesChanged()));

    connect(d->ui->sldOffsetX, SIGNAL(valueChanged(qreal)), cropCompressor, SLOT(start()));
    connect(d->ui->sldOffsetY, SIGNAL(valueChanged(qreal)), cropCompressor, SLOT(start()));
    connect(d->ui->sldWidth, SIGNAL(valueChanged(qreal)), cropCompressor, SLOT(start()));
    connect(d->ui->sldHeight, SIGNAL(valueChanged(qreal)), cropCompressor, SLOT(start()));

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

    KisReferenceImage *ref = d->tool->getActiveReferenceImage();
    if(ref) {
        updateCropSliders();
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

void ToolReferenceImagesWidget::slotUpdateLock(bool value)
{
    d->ui->bnLock->setChecked(value);
    bool locked = d->ui->bnLock->isChecked();
    if(locked) {
        d->ui->bnLock->setIcon(KisIconUtils::loadIcon("locked"));
    }
    else {
        d->ui->bnLock->setIcon(KisIconUtils::loadIcon("unlocked"));
    }
    d->tool->document()->referenceImagesLayer()->setLock(locked, d->tool->canvas());
}

void ToolReferenceImagesWidget::slotUpdateCrop(bool value)
{
    KisReferenceImage* ref = d->tool->getActiveReferenceImage();
    d->ui->bnCrop->setChecked(value);
    bool enable = d->ui->bnCrop->isChecked();
    d->ui->grpCrop->setVisible(enable);

    ref->setCrop(enable);
    if(enable && ref) {
        updateCropSliders();
    }
}

void ToolReferenceImagesWidget::slotImageValuesChanged()
{
    slotSaturationSliderChanged(d->ui->saturationSlider->value());
    slotOpacitySliderChanged(d->ui->opacitySlider->value());
}

void ToolReferenceImagesWidget::slotCropValuesChanged()
{
    KisReferenceImage *ref = d->tool->getActiveReferenceImage();
    if(ref) {

        qreal x = d->ui->sldOffsetX->value();
        qreal y = d->ui->sldOffsetY->value();
        qreal width = d->ui->sldWidth->value();
        qreal height = d->ui->sldHeight->value();
        ref->setCropRect(QRectF(x, y, width, height));
    }
}

void ToolReferenceImagesWidget::updateVisibility(bool hasSelection)
{
    // hide UI elements if nothing is selected.
    d->ui->referenceImageLocationCombobox->setVisible(hasSelection);
    d->ui->chkKeepAspectRatio->setVisible(hasSelection);
    d->ui->saveLocationLabel->setVisible(hasSelection);
    d->ui->opacitySlider->setVisible(hasSelection);
    d->ui->saturationSlider->setVisible(hasSelection);
    d->ui->bnLock->setVisible(hasSelection);
    d->ui->bnCrop->setVisible(hasSelection);

    // show a label indicating that a selection is required to show options
    d->ui->referenceImageOptionsLabel->setVisible(!hasSelection);

    KisSharedPtr<KisReferenceImagesLayer> layer = d->tool->document()->referenceImagesLayer();
    if(layer) {
        d->ui->bnLock->setChecked(layer->lock());
        d->ui->bnLock->setIcon(d->ui->bnLock->isChecked() ? KisIconUtils::loadIcon("locked") : KisIconUtils::loadIcon("unlocked"));
    }
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

void ToolReferenceImagesWidget::updateCropSliders()
{
    KisReferenceImage* ref = d->tool->getActiveReferenceImage();
    if(!ref) return;

    QList<KoShape*> shape;
    shape.append(ref);

    KisCanvas2 *kiscanvas = dynamic_cast<KisCanvas2*>(d->tool->canvas());
    const KisCoordinatesConverter *converter = kiscanvas->coordinatesConverter();

    QRectF rect = converter->documentToImage(ref->boundingRect());
    ref->setCropRect(rect);

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
    d->ui->sldWidth->setValue(ref->cropRect().width());
    d->ui->sldWidth->blockSignals(false);

    d->ui->sldHeight->setSelection(shape);
    d->ui->sldHeight->setRange(0,rect.height());
    d->ui->sldHeight->blockSignals(true);
    d->ui->sldHeight->setValue(ref->cropRect().height());
    d->ui->sldHeight->blockSignals(false);
}

