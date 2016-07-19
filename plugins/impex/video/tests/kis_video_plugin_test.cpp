/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_video_plugin_test.h"


#include <QTest>
#include <QCoreApplication>

#include <QTest>

#include "filestest.h"

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing the importing of files in krita"
#endif

#include "../video_saver.h"


void KisVideoPluginTest::testFiles()
{
    QString fname = QString(FILES_DATA_DIR) + QDir::separator() + "test_animation_small.kra";

    KisDocument *doc = KisPart::instance()->createDocument();
    doc->loadNativeFormat(fname);


    VideoSaver saver(doc, false);

    KisImageBuilder_Result result =
        //saver.encode("testfile.gif");
        saver.encode("testfile.ogg");

    QCOMPARE(result, KisImageBuilder_RESULT_OK);
}
QTEST_MAIN(KisVideoPluginTest)

