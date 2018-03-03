/*
 *  Copyright (c) 2017 Eugene Ingerman
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

#include "ToolReferenceImagesWidget.h"

#include "ui_WdgToolOptions.h"

#include <KoSelection.h>
#include <KoShapeTransparencyCommand.h>
#include <KoShapeKeepAspectRatioCommand.h>
#include <kis_config.h>
#include <kis_signals_blocker.h>
#include <KisReferenceImage.h>

#include "ToolReferenceImages.h"

struct ToolReferenceImagesWidget::Private {
    Private(ToolReferenceImages *tool)
    : tool(tool)
    {
    }

    Ui_WdgToolOptions *ui;
    ToolReferenceImages *tool;
};

ToolReferenceImagesWidget::ToolReferenceImagesWidget(ToolReferenceImages *tool, KisCanvasResourceProvider *provider, QWidget *parent)
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

    connect(d->ui->bnAddReferenceImage, SIGNAL(clicked()), tool, SLOT(addReferenceImage()));
    connect(d->ui->bnDelete, SIGNAL(clicked()), tool, SLOT(removeAllReferenceImages()));
    connect(d->ui->bnSave, SIGNAL(clicked()), tool, SLOT(saveReferenceImages()));
    connect(d->ui->bnLoad, SIGNAL(clicked()), tool, SLOT(loadReferenceImages()));

    connect(d->ui->chkKeepAspectRatio, SIGNAL(stateChanged(int)), this, SLOT(slotKeepAspectChanged()));
    connect(d->ui->opacitySlider, SIGNAL(valueChanged(qreal)), this, SLOT(slotOpacitySliderChanged(qreal)));
    connect(d->ui->saturationSlider, SIGNAL(valueChanged(qreal)), this, SLOT(slotSaturationSliderChanged(qreal)));

    connect(d->ui->radioEmbed, SIGNAL(toggled(bool)), this, SLOT(slotEmbeddingChanged()));
    connect(d->ui->radioLinkExternal, SIGNAL(toggled(bool)), this, SLOT(slotEmbeddingChanged()));
}

ToolReferenceImagesWidget::~ToolReferenceImagesWidget()
{
}

void ToolReferenceImagesWidget::selectionChanged(KoSelection *selection)
{
    QList<KoShape*> shapes = selection->selectedEditableShapes();

    d->ui->opacitySlider->setSelection(shapes);
    d->ui->saturationSlider->setSelection(shapes);

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
        d->ui->radioEmbed,
        d->ui->radioLinkExternal
    );

    d->ui->chkKeepAspectRatio->setCheckState(
        (anyKeepingAspectRatio && anyNotKeepingAspectRatio) ? Qt::PartiallyChecked :
         anyKeepingAspectRatio ? Qt::Checked : Qt::Unchecked);

    d->ui->radioEmbed->setChecked(anyEmbedded && !anyLinked);
    d->ui->radioLinkExternal->setChecked(anyLinked && !anyEmbedded);
    d->ui->radioEmbed->setEnabled(!anyNonLinkable && anySelected);
    d->ui->radioLinkExternal->setEnabled(!anyNonLinkable && anySelected);
    d->ui->chkKeepAspectRatio->setEnabled(anySelected);
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

void ToolReferenceImagesWidget::slotEmbeddingChanged()
{
    KoSelection *selection = d->tool->koSelection();
    QList<KoShape*> shapes = selection->selectedEditableShapes();

    Q_FOREACH(KoShape *shape, shapes) {
        KisReferenceImage *reference = dynamic_cast<KisReferenceImage*>(shape);
        KIS_SAFE_ASSERT_RECOVER_RETURN(reference);

        if (d->ui->radioEmbed->isChecked()) {
            reference->setEmbed(true);
        } else if (d->ui->radioLinkExternal->isChecked()) {
            reference->setEmbed(false);
        }
    }
}
