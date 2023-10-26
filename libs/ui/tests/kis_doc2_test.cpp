/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_doc2_test.h"

#include <KisMainWindow.h>

#include <simpletest.h>

#include "KisDocument.h"
#include "KisPart.h"
#include "kis_image.h"
#include "kis_undo_store.h"
#include "util.h"
#include <KisView.h>
#include <KisViewManager.h>
#include <kis_config.h>
#include <testui.h>
#include "KritaTransformMaskStubs.h"


void silenceReignsSupreme(QtMsgType /*type*/, const QMessageLogContext &/*context*/, const QString &/*msg*/)
{
}

void KisDocumentTest::initTestCase()
{
    TestUtil::registerTransformMaskStubs();
    qInstallMessageHandler(silenceReignsSupreme);
}

void KisDocumentTest::testOpenImageTwiceInSameDoc()
{
    QString fname2 = QString(FILES_DATA_DIR) + '/' + "load_test.kra";
    QString fname = QString(FILES_DATA_DIR) + '/' + "load_test2.kra";


    Q_ASSERT(!fname.isEmpty());
    Q_ASSERT(!fname2.isEmpty());

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

    doc->loadNativeFormat(fname);
    doc->loadNativeFormat(fname2);
}


KISTEST_MAIN(KisDocumentTest)

