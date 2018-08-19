/*
 *
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_dlg_blacklist_cleanup.h"
#include <KisResourceServerProvider.h>

#include <kis_icon.h>
#include <KoResourceServerProvider.h>

#include <brushengine/kis_paintop_preset.h>
#include <kis_workspace_resource.h>
#include <resources/KoColorSet.h>
#include <resources/KoAbstractGradient.h>
#include <resources/KoPattern.h>

KisDlgBlacklistCleanup::KisDlgBlacklistCleanup()
{
    setCaption(i18n("Cleanup resource files"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    QWidget* page = new QWidget(this);
    setupUi(page);
    setMainWidget(page);
    labelWarning->setPixmap(KisIconUtils::loadIcon("warning").pixmap(32, 32));
}

void KisDlgBlacklistCleanup::accept()
{
    QDialog::accept();
    if (cbRemovePresets->isChecked()) {
        KisResourceServerProvider::instance()->paintOpPresetServer()->removeBlackListedFiles();
    }
    if (cbRemoveBrushes->isChecked()) {
        KisResourceServerProvider::instance()->brushBlacklistCleanup();
    }
    if (cbRemoveWorkspaces->isChecked()) {
        KisResourceServerProvider::instance()->workspaceServer()->removeBlackListedFiles();
    }
    if (cbRemoveColorsets->isChecked()) {
        KoResourceServerProvider::instance()->paletteServer()->removeBlackListedFiles();
    }
    if (cbRemoveGradients->isChecked()) {
        KoResourceServerProvider::instance()->gradientServer()->removeBlackListedFiles();
    }
    if (cbRemovePattern->isChecked()) {
        KoResourceServerProvider::instance()->patternServer()->removeBlackListedFiles();
    }
}

