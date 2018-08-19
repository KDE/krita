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

