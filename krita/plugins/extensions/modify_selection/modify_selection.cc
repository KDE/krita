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


#include <math.h>

#include <stdlib.h>

#include <QSlider>
#include <QPoint>

#include <klocale.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kstandardaction.h>
#include <kactioncollection.h>

#include "kis_config.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view2.h"
#include "kis_selection.h"
#include "kis_selection_manager.h"
#include "kis_transaction.h"
#include "kis_action.h"
#include "kis_action_manager.h"

#include "dlg_grow_selection.h"
#include "dlg_shrink_selection.h"
#include "dlg_border_selection.h"
#include "dlg_feather_selection.h"

K_PLUGIN_FACTORY(ModifySelectionFactory, registerPlugin<ModifySelection>();)
K_EXPORT_PLUGIN(ModifySelectionFactory("krita"))

ModifySelection::ModifySelection(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent)
{
    if (parent->inherits("KisView2")) {
        setXMLFile(KStandardDirs::locate("data", "kritaplugins/modify_selection.rc"),
                   true);

        m_view = (KisView2*) parent;

        // Selection manager takes ownership
        m_growSelection  = new KisAction(i18n("Grow Selection..."), this);
        m_growSelection->setActivationFlags(KisAction::PIXEL_SELECTION_WITH_PIXELS);
        m_growSelection->setActivationConditions(KisAction::SELECTION_EDITABLE);
        m_view->actionManager()->addAction("growselection", m_growSelection, actionCollection());

        m_shrinkSelection = new KisAction(i18n("Shrink Selection..."), this);
        m_shrinkSelection->setActivationFlags(KisAction::PIXEL_SELECTION_WITH_PIXELS);
        m_shrinkSelection->setActivationConditions(KisAction::SELECTION_EDITABLE);
        m_view->actionManager()->addAction("shrinkselection", m_shrinkSelection, actionCollection());

        m_borderSelection  = new KisAction(i18n("Border Selection..."), this);
        m_borderSelection->setActivationFlags(KisAction::PIXEL_SELECTION_WITH_PIXELS);
        m_borderSelection->setActivationConditions(KisAction::SELECTION_EDITABLE);
        m_view->actionManager()->addAction("borderselection", m_borderSelection, actionCollection());

        m_featherSelection  = new KisAction(i18n("Feather Selection..."), this);
        m_featherSelection->setActivationFlags(KisAction::PIXEL_SELECTION_WITH_PIXELS);
        m_featherSelection->setActivationConditions(KisAction::SELECTION_EDITABLE);
        m_view->actionManager()->addAction("featherselection", m_featherSelection, actionCollection());

        m_smoothSelection = new KisAction(i18n("Smooth..."), this);
        m_smoothSelection->setActivationFlags(KisAction::PIXEL_SELECTION_WITH_PIXELS);
        m_smoothSelection->setActivationConditions(KisAction::SELECTION_EDITABLE);
        m_view->actionManager()->addAction("smoothselection", m_smoothSelection, actionCollection());

        Q_CHECK_PTR(m_growSelection);
        Q_CHECK_PTR(m_shrinkSelection);
        Q_CHECK_PTR(m_borderSelection);
        Q_CHECK_PTR(m_featherSelection);
        Q_CHECK_PTR(m_smoothSelection);

        connect(m_growSelection, SIGNAL(triggered()), this, SLOT(slotGrowSelection()));
        connect(m_shrinkSelection, SIGNAL(triggered()), this, SLOT(slotShrinkSelection()));
        connect(m_borderSelection, SIGNAL(triggered()), this, SLOT(slotBorderSelection()));
        connect(m_featherSelection, SIGNAL(triggered()), this, SLOT(slotFeatherSelection()));
        connect(m_smoothSelection, SIGNAL(triggered()), this, SLOT(slotSmoothSelection()));
    }
}

ModifySelection::~ModifySelection()
{
    m_view = 0;
}

void ModifySelection::slotGrowSelection()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    DlgGrowSelection * dlgGrowSelection = new DlgGrowSelection(m_view, "GrowSelection");
    Q_CHECK_PTR(dlgGrowSelection);

    dlgGrowSelection->setCaption(i18n("Grow Selection"));

    KisConfig cfg;

    if (dlgGrowSelection->exec() == QDialog::Accepted) {
        qint32 xradius = dlgGrowSelection->xradius();
        qint32 yradius = dlgGrowSelection->yradius();

        m_view->selectionManager()->grow(xradius, yradius);
    }

    delete dlgGrowSelection;
}

void ModifySelection::slotShrinkSelection()
{
    KisImageWSP image = m_view->image();

    if (!image) return;

    DlgShrinkSelection * dlgShrinkSelection = new DlgShrinkSelection(m_view, "ShrinkSelection");
    Q_CHECK_PTR(dlgShrinkSelection);

    dlgShrinkSelection->setCaption(i18n("Shrink Selection"));

    KisConfig cfg;

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

    KisConfig cfg;

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

    KisConfig cfg;

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
