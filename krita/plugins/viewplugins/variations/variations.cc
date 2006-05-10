/*
 * variation.h -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
#include <QPoint>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>

#include "variations.h"
#include "dlg_variations.h"

typedef KGenericFactory<Variations> VariationsFactory;
K_EXPORT_COMPONENT_FACTORY( kritavariations, VariationsFactory( "krita" ) )

Variations::Variations(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{

    if ( parent->inherits("KisView") )
    {
        setInstance(VariationsFactory::instance());
        setXMLFile(locate("data","kritaplugins/variations.rc"), true);

        (void) new KAction(i18n("&Variations..."), 0, 0, this, SLOT(slotVariationsActivated()), actionCollection(), "variations");

         m_view = (KisView*) parent;
    }
}

Variations::~Variations()
{
}

void Variations::slotVariationsActivated()
{
    DlgVariations * dlgVariations = new DlgVariations(m_view, "Variations");
    Q_CHECK_PTR(dlgVariations);
    // Render layer to a QIMage -- keep in mind possibility of selection

    // Scale QImage

    // Set original QImage in dialog

    if (dlgVariations -> exec() == QDialog::Accepted) {
        // Retrieve changes made by dialog
        // Apply changes to layer (selection)
    }
    delete dlgVariations;
}

#include "variations.moc"

