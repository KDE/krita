/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "filters_gallery.h"

#include <kdebug.h>

#include <kgenericfactory.h>
#include <kstandarddirs.h>

#include "kis_filters_listview.h"
#include "kis_view.h"
#include "kis_dlg_filtersgallery.h"

namespace Krita {
namespace Plugins {
namespace FiltersGallery {

typedef KGenericFactory<KritaFiltersGallery> KritaFiltersGalleryFactory;
K_EXPORT_COMPONENT_FACTORY( kritafiltersgallery, KritaFiltersGalleryFactory( "krita" ) )

KritaFiltersGallery::KritaFiltersGallery(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    
    kdDebug() << "FiltersGallery plugin. Class: "
          << className()
          << ", Parent: "
          << parent -> className()
          << "\n";

    if ( parent->inherits("KisView") )
    {
        setInstance(KritaFiltersGallery::instance());
        //setXMLFile(locate("data","kritaplugins/kritafiltersgallery.rc"), true);

        m_view = (KisView*) parent;

        (void) new KAction(i18n("&Filters gallery"), 0, 0, this, SLOT(showFiltersGalleryDialog()), actionCollection(), "krita_filters_gallery");
    }

    
}

KritaFiltersGallery::~KritaFiltersGallery()
{
}

void KritaFiltersGallery::showFiltersGalleryDialog()
{
    KisDlgFiltersGallery dlg(m_view,m_view);
    if(dlg.exec())
    {
        if(dlg.currentFilter() != 0 )
        {
        
        }
    }
}

}
}
}

#include "filters_gallery.moc"
