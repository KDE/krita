/*
 * modify_selection.cc -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Michael Thaler (michael.thaler@physik.tu-muenchen.de)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
