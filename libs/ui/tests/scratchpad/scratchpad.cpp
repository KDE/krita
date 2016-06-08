/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QFile>
#include <QString>
#include <QTextStream>
#include <QProcess>
#include <QTemporaryFile>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kis_debug.h>

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <kis_scratch_pad.h>

int main(int argc, char** argv)
{
    KAboutData aboutData("scratchpad",
                         0,
                         ki18n("scratchpad"),
                         "1.0",
                         ki18n("Test application for the single paint device scratchpad canvas"),
                         KAboutData::License_LGPL,
                         ki18n("(c) 2010 Boudewijn Rempt"),
                         KLocalizedString(),
                         "www.krita.org",
                         "submit@bugs.kde.org");
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;

    options.add("+preset", ki18n("preset to load"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KisScratchPad *scratchpad = new KisScratchPad();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count() > 0 ) {
        QString fileName = args->arg(0);
        if (QFile::exists(fileName)) {
            KisPaintOpPresetSP preset = new KisPaintOpPreset(fileName);
            preset->load();
            if (preset->valid()) {
//                scratchpad->setPreset(preset);
            }
        }
    }

//    const KoColorProfile* profile = KoColorSpaceRegistry::instance()->rgb8()->profile();
//    scratchpad->setColorSpace(KoColorSpaceRegistry::instance()->rgb16());
//    scratchpad->setDisplayProfile(profile);
//    scratchpad->setCanvasColor(Qt::white);
    scratchpad->show();
    return app.exec();
}
