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


#include <math.h>

#include <stdlib.h>

#include <qslider.h>
#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kstdaction.h>

#include <kis_doc.h>
#include <kis_config.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_selection.h>
#include <kis_selection_manager.h>
#include <kis_transaction.h>

#include "modify_selection.h"
#include "dlg_grow_selection.h"
#include "dlg_shrink_selection.h"
#include "dlg_border_selection.h"

typedef KGenericFactory<ModifySelection> ModifySelectionFactory;
K_EXPORT_COMPONENT_FACTORY( kritamodifyselection, ModifySelectionFactory( "krita" ) )

ModifySelection::ModifySelection(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
    if ( parent->inherits("KisView") )
    {
        setInstance(ModifySelectionFactory::instance());
        setXMLFile(locate("data","kritaplugins/modify_selection.rc"), true);

        m_view = (KisView*) parent;

        // Selection manager takes ownership?
        KAction* a = new KAction(i18n("Grow Selection..."), 0, 0, this, SLOT(slotGrowSelection()), actionCollection(), "growselection");
        KAction* b = new KAction(i18n("Shrink Selection..."), 0, 0, this, SLOT(slotShrinkSelection()), actionCollection(), "shrinkselection");
        KAction* c = new KAction(i18n("Border Selection..."), 0, 0, this, SLOT(slotBorderSelection()), actionCollection(), "borderselection");

        Q_CHECK_PTR(a);
        Q_CHECK_PTR(b);
        Q_CHECK_PTR(c);

        m_view ->canvasSubject()-> selectionManager()->addSelectionAction(a);
        m_view ->canvasSubject()-> selectionManager()->addSelectionAction(b);
        m_view ->canvasSubject()-> selectionManager()->addSelectionAction(c);
    }
}

ModifySelection::~ModifySelection()
{
    m_view = 0;
}

void ModifySelection::slotGrowSelection()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;

    DlgGrowSelection * dlgGrowSelection = new DlgGrowSelection(m_view, "GrowSelection");
    Q_CHECK_PTR(dlgGrowSelection);

    dlgGrowSelection->setCaption(i18n("Grow Selection"));

    KisConfig cfg;

    if (dlgGrowSelection->exec() == QDialog::Accepted) {
        Q_INT32 xradius = dlgGrowSelection->xradius();
        Q_INT32 yradius = dlgGrowSelection->yradius();

        m_view ->canvasSubject()-> selectionManager()->grow(xradius, yradius);
    }

    delete dlgGrowSelection;
}

void ModifySelection::slotShrinkSelection()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;

    DlgShrinkSelection * dlgShrinkSelection = new DlgShrinkSelection(m_view, "ShrinkSelection");
    Q_CHECK_PTR(dlgShrinkSelection);

    dlgShrinkSelection->setCaption(i18n("Shrink Selection"));

    KisConfig cfg;

    if (dlgShrinkSelection->exec() == QDialog::Accepted) {
        Q_INT32 xradius = dlgShrinkSelection->xradius();
        Q_INT32 yradius = dlgShrinkSelection->yradius();
        bool shrinkFromImageBorder = dlgShrinkSelection->shrinkFromImageBorder();

        m_view ->canvasSubject()-> selectionManager()->shrink(xradius, yradius, shrinkFromImageBorder);
    }

    delete dlgShrinkSelection;
}

void ModifySelection::slotBorderSelection()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();

    if (!image) return;

    DlgBorderSelection * dlgBorderSelection = new DlgBorderSelection(m_view, "BorderSelection");
    Q_CHECK_PTR(dlgBorderSelection);

    dlgBorderSelection->setCaption(i18n("Border Selection"));

    KisConfig cfg;

    if (dlgBorderSelection->exec() == QDialog::Accepted) {
        Q_INT32 xradius = dlgBorderSelection->xradius();
        Q_INT32 yradius = dlgBorderSelection->yradius();

        m_view ->canvasSubject()-> selectionManager()->border(xradius, yradius);
    }

    delete dlgBorderSelection;
}

#include "modify_selection.moc"
