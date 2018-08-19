/*
 * modify_selection.cc -- Part of Krita
 *
 * Copyright (c) 2006 Michael Thaler (michael.thaler@physik.tu-muenchen.de)
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

#include "modify_selection.h"

#include <klocalizedstring.h>
#include <kis_debug.h>

#include "kis_action.h"
#include <kpluginfactory.h>
#include <operations/kis_operation_ui_widget_factory.h>

#include "dlg_grow_selection.h"
#include "dlg_shrink_selection.h"
#include "dlg_border_selection.h"
#include "dlg_feather_selection.h"
#include "modify_selection_operations.h"

K_PLUGIN_FACTORY_WITH_JSON(ModifySelectionFactory, "kritamodifyselection.json", registerPlugin<ModifySelection>();)

ModifySelection::ModifySelection(QObject *parent, const QVariantList &)
        : KisActionPlugin(parent)
{
    KisAction* action = createAction("growselection");
    action->setOperationID("growselection");

    addUIFactory(new KisOperationUIWidgetFactory<WdgGrowSelection>("growselection"));
    addOperation(new GrowSelectionOperation);

    action = createAction("shrinkselection");
    action->setOperationID("shrinkselection");

    addUIFactory(new KisOperationUIWidgetFactory<WdgShrinkSelection>("shrinkselection"));
    addOperation(new ShrinkSelectionOperation);

    action = createAction("borderselection");
    action->setOperationID("borderselection");

    addUIFactory(new KisOperationUIWidgetFactory<WdgBorderSelection>("borderselection"));
    addOperation(new BorderSelectionOperation);

    action = createAction("featherselection");
    action->setOperationID("featherselection");

    addUIFactory(new KisOperationUIWidgetFactory<WdgFeatherSelection>("featherselection"));
    addOperation(new FeatherSelectionOperation);

    action = createAction("smoothselection");
    action->setOperationID("smoothselection");

    addOperation(new SmoothSelectionOperation);
}

ModifySelection::~ModifySelection()
{
}

#include "modify_selection.moc"
