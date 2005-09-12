/*
* testcs_plugin.cc -- Part of Krita
*
* Copyright (c) 2004 oudewijn Rempt (boud@valdyas.org)
* Copyright (c) 2005 Casper Boemann (cbr@boemann.dk)
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
#include <stdlib.h>
#include <vector>

#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_colorspace_registry.h>
#include <kis_factory.h>

#include "testcs_plugin.h"

#include "kis_test_colorspace.h"

typedef KGenericFactory<TestCSPlugin> TestCSPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritatestcsplugin, TestCSPluginFactory( "krita" ) )


TestCSPlugin::TestCSPlugin(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
        setInstance(TestCSPluginFactory::instance());

    kdDebug(DBG_AREA_PLUGINS) << "TestCS Color model plugin. Class: "
        << className()
        << ", Parent: "
        << parent -> className()
        << "\n";

    if ( parent->inherits("KisFactory") )
    {
        KisColorSpace * colorSpaceTestCS = new KisTestColorSpace();
        Q_CHECK_PTR(colorSpaceTestCS);
        KisColorSpaceRegistry::instance() -> add(colorSpaceTestCS);
    }

}

TestCSPlugin::~TestCSPlugin()
{
}

#include "testcs_plugin.moc"
