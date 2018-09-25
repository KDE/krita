/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_doc2_test.h"

#include <KisMainWindow.h>

#include <QTest>

#include "KisDocument.h"
#include "kis_image.h"
#include "kis_undo_store.h"
#include "KisPart.h"
#include <KisViewManager.h>
#include "util.h"
#include <KisView.h>
#include <kis_config.h>
#include "sdk/tests/kistest.h"

void silenceReignsSupreme(QtMsgType /*type*/, const QMessageLogContext &/*context*/, const QString &/*msg*/)
{

}

void KisDocumentTest::init()
{
    qInstallMessageHandler(silenceReignsSupreme);
}

void KisDocumentTest::testOpenImageTwiceInSameDoc()
{
    QString fname2 = QString(FILES_DATA_DIR) + QDir::separator() + "load_test.kra";
    QString fname = QString(FILES_DATA_DIR) + QDir::separator() + "load_test2.kra";


    Q_ASSERT(!fname.isEmpty());
    Q_ASSERT(!fname2.isEmpty());

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

    doc->loadNativeFormat(fname);
    doc->loadNativeFormat(fname2);
}


KISTEST_MAIN(KisDocumentTest)

