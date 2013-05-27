/*
 * This file is part of the KDE project
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "randompickfilterplugin.h"
#include "randompickfilter.h"
#include <stdlib.h>
#include <vector>
#include <math.h>

#include <QPoint>

#include <kis_debug.h>
#include <kcomponentdata.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_random_accessor_ng.h>
#include <kis_random_generator.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "kis_wdg_random_pick.h"
#include "ui_wdgrandompickoptions.h"
#include <kis_iterator_ng.h>
#include <kis_random_accessor_ng.h>

K_PLUGIN_FACTORY(KritaRandomPickFilterFactory, registerPlugin<KritaRandomPickFilter>();)
K_EXPORT_PLUGIN(KritaRandomPickFilterFactory("krita"))

KritaRandomPickFilter::KritaRandomPickFilter(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisFilterRandomPick());
}

KritaRandomPickFilter::~KritaRandomPickFilter()
{
}
