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

#include <klocale.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include "kis_view2.h"
#include "kis_selection_manager.h"
#include "kis_action.h"
#include <kis_action_manager.h>
#include <operations/kis_operation_ui_widget_factory.h>

#include "dlg_grow_selection.h"
#include "dlg_shrink_selection.h"
#include "dlg_border_selection.h"
#include "dlg_feather_selection.h"

K_PLUGIN_FACTORY(ModifySelectionFactory, registerPlugin<ModifySelection>();)
K_EXPORT_PLUGIN(ModifySelectionFactory("krita"))

ModifySelection::ModifySelection(QObject *parent, const QVariantList &)
        : KisViewPlugin(parent, "kritaplugins/modify_selection.rc")
{
    KisAction* action  = new KisAction(i18n("Grow Selection..."), this);
    action->setActivationFlags(KisAction::PIXEL_SELECTION_WITH_PIXELS);
    action->setActivationConditions(KisAction::SELECTION_EDITABLE);
    action->setOperationID("growselection");
    addAction("growselection", action);

    addUIFactory(new KisOperationUIWidgetFactory<WdgGrowSelection>("growselection"));

    action = new KisAction(i18n("Shrink Selection..."), this);
    action->setActivationFlags(KisAction::PIXEL_SELECTION_WITH_PIXELS);
    action->setActivationConditions(KisAction::SELECTION_EDITABLE);
    addAction("shrinkselection", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotShrinkSelection()));

    action  = new KisAction(i18n("Border Selection..."), this);
    action->setActivationFlags(KisAction::PIXEL_SELECTION_WITH_PIXELS);
    action->setActivationConditions(KisAction::SELECTION_EDITABLE);
    addAction("borderselection", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotBorderSelection()));

    action  = new KisAction(i18n("Feather Selection..."), this);
    action->setActivationFlags(KisAction::PIXEL_SELECTION_WITH_PIXELS);
    action->setActivationConditions(KisAction::SELECTION_EDITABLE);
    addAction("featherselection", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotFeatherSelection()));

    action = new KisAction(i18n("Smooth..."), this);
    action->setActivationFlags(KisAction::PIXEL_SELECTION_WITH_PIXELS);
    action->setActivationConditions(KisAction::SELECTION_EDITABLE);
    addAction("smoothselection", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotSmoothSelection()));
}

ModifySelection::~ModifySelection()
{
}

void ModifySelection::slotShrinkSelection()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    DlgShrinkSelection * dlgShrinkSelection = new DlgShrinkSelection(m_view, "ShrinkSelection");
    Q_CHECK_PTR(dlgShrinkSelection);

    dlgShrinkSelection->setCaption(i18n("Shrink Selection"));

    if (dlgShrinkSelection->exec() == QDialog::Accepted) {
        qint32 xradius = dlgShrinkSelection->xradius();
        qint32 yradius = dlgShrinkSelection->yradius();
        bool shrinkFromImageBorder = dlgShrinkSelection->shrinkFromImageBorder();

        //third parameter is edge_lock so shrinkFromImageBorder needs to be inverted
        m_view->selectionManager()->shrink(xradius, yradius, !shrinkFromImageBorder);
    }

    delete dlgShrinkSelection;
}

void ModifySelection::slotBorderSelection()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    DlgBorderSelection * dlgBorderSelection = new DlgBorderSelection(m_view, "BorderSelection");
    Q_CHECK_PTR(dlgBorderSelection);

    dlgBorderSelection->setCaption(i18n("Border Selection"));

    if (dlgBorderSelection->exec() == QDialog::Accepted) {
        qint32 xradius = dlgBorderSelection->xradius();
        qint32 yradius = dlgBorderSelection->yradius();

        m_view->selectionManager()->border(xradius, yradius);
    }

    delete dlgBorderSelection;
}

void ModifySelection::slotFeatherSelection()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    DlgFeatherSelection * dlgFeatherSelection = new DlgFeatherSelection(m_view, "FeatherSelection");
    Q_CHECK_PTR(dlgFeatherSelection);

    dlgFeatherSelection->setCaption(i18n("Feather Selection"));

    if (dlgFeatherSelection->exec() == QDialog::Accepted) {
        qint32 radius = dlgFeatherSelection->radius();

        m_view->selectionManager()->feather(radius);
    }

    delete dlgFeatherSelection;
}

void ModifySelection::slotSmoothSelection()
{
    m_view->selectionManager()->smooth();
}


#include "modify_selection.moc"
