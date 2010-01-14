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
#include <kiconloader.h>
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


#include "dlg_grow_selection.h"
#include "dlg_shrink_selection.h"
#include "dlg_border_selection.h"

K_PLUGIN_FACTORY(ModifySelectionFactory, registerPlugin<ModifySelection>();)
K_EXPORT_PLUGIN(ModifySelectionFactory("krita"))

ModifySelection::ModifySelection(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent)
{
    if (parent->inherits("KisView2")) {
        setComponentData(ModifySelectionFactory::componentData());

        setXMLFile(KStandardDirs::locate("data", "kritaplugins/modify_selection.rc"),
                   true);

        m_view = (KisView2*) parent;

        // Selection manager takes ownership?
        KAction* a  = new KAction(i18n("Grow selection..."), this);
        actionCollection()->addAction("growselection", a);
        KAction* b  = new KAction(i18n("Shrink selection..."), this);
        actionCollection()->addAction("shrinkselection", b);
        KAction* c  = new KAction(i18n("Border selection..."), this);
        actionCollection()->addAction("borderselection", c);

        Q_CHECK_PTR(a);
        Q_CHECK_PTR(b);
        Q_CHECK_PTR(c);

        connect(a, SIGNAL(triggered()), this, SLOT(slotGrowSelection()));
        connect(b, SIGNAL(triggered()), this, SLOT(slotShrinkSelection()));
        connect(c, SIGNAL(triggered()), this, SLOT(slotBorderSelection()));

        m_view->selectionManager()->addSelectionAction(a);
        m_view->selectionManager()->addSelectionAction(b);
        m_view->selectionManager()->addSelectionAction(c);
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

#include "modify_selection.moc"
