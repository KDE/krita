/*
 *  SPDX-FileCopyrightText: 2017 Eugene Ingerman
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_smart_patch_options_widget.h"

#include "ui_kis_tool_smart_patch_options_widget.h"

#include <KoColorSpaceRegistry.h>
#include "KisPaletteModel.h"

#include "kis_config.h"
#include <resources/KoColorSet.h>
#include "kis_canvas_resource_provider.h"


struct KisToolSmartPatchOptionsWidget::Private {
    Private()
    {
    }

    Ui_KisToolSmartPatchOptionsWidget *ui;

    int getPatchRadius(void)
    {
        return ui->patchRadius->value();
    }
    int getAccuracy(void)
    {
        return ui->accuracySlider->value();
    }
};

KisToolSmartPatchOptionsWidget::KisToolSmartPatchOptionsWidget(KisCanvasResourceProvider */*provider*/, QWidget *parent)
    : QWidget(parent),
      m_d(new Private)
{
    m_d->ui = new Ui_KisToolSmartPatchOptionsWidget();
    m_d->ui->setupUi(this);

}

KisToolSmartPatchOptionsWidget::~KisToolSmartPatchOptionsWidget()
{
}

int KisToolSmartPatchOptionsWidget::getPatchRadius()
{
    return m_d->getPatchRadius();
}

int KisToolSmartPatchOptionsWidget::getAccuracy()
{
    return m_d->getAccuracy();
}

