/*
 *  Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
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
#include "SketchApplication.h"

#include <KoPluginLoader.h>
#include <KoShapeRegistry.h>

#include <KoDpi.h>
#include "KoGlobal.h"

#include <kcrash.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kdesktopfile.h>
#include <QMessageBox>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <krecentdirs.h>

#include <QFile>
#include <QWidget>
#include <QSysInfo>
#include <QStringList>
#include <QDesktopServices>
#include <QProcessEnvironment>
#include <QDir>
#include <QDesktopWidget>

#include <stdlib.h>

#include "KisPrintJob.h"
#include "KisDocumentEntry.h"
#include "KisDocument.h"
#include "KisMainWindow.h"
#include "kis_factory2.h"
#include "KisAutoSaveRecoveryDialog.h"
#include "KisPart.h"

#include "flake/kis_shape_selection.h"
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <generator/kis_generator_registry.h>
#include <generator/kis_generator.h>
#include <kis_paintop_registry.h>
#include <metadata/kis_meta_data_io_backend.h>
#include "kisexiv2/kis_exiv2.h"


SketchApplication::SketchApplication()
    : KApplication()
{
}

void SketchApplication::start()
{
    // Load various global plugins
    KoShapeRegistry* r = KoShapeRegistry::instance();
    r->add(new KisShapeSelectionFactory());

    KisFilterRegistry::instance();
    KisGeneratorRegistry::instance();
    KisPaintOpRegistry::instance();

    // Load the krita-specific tools
    KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Tool"),
                                     QString::fromLatin1("[X-Krita-Version] == 28"));

    // Load dockers
    KoPluginLoader::instance()->load(QString::fromLatin1("Krita/Dock"),
                                     QString::fromLatin1("[X-Krita-Version] == 28"));


    // XXX_EXIV: make the exiv io backends real plugins
    KisExiv2::initialize();

}
