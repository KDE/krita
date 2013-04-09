/*
 *  dlg_feather_selection.cc - part of Krita
 *
 *  Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
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

#include "dlg_feather_selection.h"

#include <klocale.h>
#include <kis_debug.h>
#include <operations/kis_operation_configuration.h>

WdgFeatherSelection::WdgFeatherSelection(QWidget* parent) : KisOperationUIWidget(i18n("Feather Selection"), parent)
{
    setupUi(this);
}

void WdgFeatherSelection::getConfiguration(KisOperationConfiguration* config)
{
    config->setProperty("radius", radiusSpinBox->value());
}

#include "dlg_feather_selection.moc"
