/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Frederic Coiffier <fcoiffie@gmail.com>
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
#include <string.h>

#include <qslider.h>
#include <qpoint.h>
#include <qcolor.h>

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
#include <kis_iterators_pixel.h>
#include <kis_colorspace.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include "levelfilter.h"
#include "kis_level_filter.h"

typedef KGenericFactory<LevelFilter> LevelFilterFactory;
K_EXPORT_COMPONENT_FACTORY( kritalevelfilter, LevelFilterFactory( "krita" ) )

LevelFilter::LevelFilter(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    setInstance(LevelFilterFactory::instance());

    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(new KisLevelFilter());
    }
}

LevelFilter::~LevelFilter()
{
}
