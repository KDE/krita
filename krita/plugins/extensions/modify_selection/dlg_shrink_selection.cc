/*
 *  dlg_shrink_selection.cc - part of Krita
 *
 *  Copyright (c) 2006 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include "dlg_shrink_selection.h"

#include <klocale.h>
#include <kis_debug.h>

WdgShrinkSelection::WdgShrinkSelection(QWidget* parent): KisOperationUIWidget(i18n("Shrink Selection"), parent) {
        setupUi(this);
}

void WdgShrinkSelection::getConfiguration(KisOperationConfiguration* config)
{
    config->setProperty("x-radius", radiusSpinBox->value());
    config->setProperty("y-radius", radiusSpinBox->value());
    config->setProperty("edgeLock", !shrinkFromImageBorderCheckBox->isChecked());
}

#include "dlg_shrink_selection.moc"
