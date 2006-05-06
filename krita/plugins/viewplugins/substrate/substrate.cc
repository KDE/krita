/*
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
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

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>

#include "substrate.h"
#include "dlg_substrate.h"

typedef KGenericFactory<SubstratePlugin> SubstrateFactory;
K_EXPORT_COMPONENT_FACTORY( kritasubstrate, SubstrateFactory( "krita" ) )

SubstratePlugin::SubstratePlugin(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{

    if ( parent->inherits("KisView") )
    {
        setInstance(SubstrateFactory::instance());
        setXMLFile(locate("data","kritaplugins/substrate.rc"), true);

        (void) new KAction(i18n("&Substrate..."), 0, 0, this, SLOT(slotSubstrateActivated()), actionCollection(), "substrate");

         m_view = (KisView*) parent;
    }
}

SubstratePlugin::~SubstratePlugin()
{
}

void SubstratePlugin::slotSubstrateActivated()
{
    DlgSubstrate * dlgSubstrate = new DlgSubstrate(m_view, "Substrate");
    Q_CHECK_PTR(dlgSubstrate);
    if (dlgSubstrate -> exec() == QDialog::Accepted) {
        // Retrieve changes made by dialog
        // Apply changes to layer (selection)
    }
    delete dlgSubstrate;
}

#include "substrate.moc"

