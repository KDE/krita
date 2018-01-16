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

#include <KoColorSpaceRegistry.h>
#include "KisPaletteModel.h"

#include "kis_config.h"
#include <resources/KoColorSet.h>
#include "kis_canvas_resource_provider.h"

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

    connect(d->ui->bnAddReferenceImage, SIGNAL(clicked()), tool, SLOT(addReferenceImage()));
    connect(d->ui->bnDelete, SIGNAL(clicked()), tool, SLOT(removeAllReferenceImages()));
    connect(d->ui->bnSave, SIGNAL(clicked()), tool, SLOT(saveReferenceImages()));
    connect(d->ui->bnLoad, SIGNAL(clicked()), tool, SLOT(loadReferenceImages()));


}

ToolReferenceImagesWidget::~ToolReferenceImagesWidget()
{
}
