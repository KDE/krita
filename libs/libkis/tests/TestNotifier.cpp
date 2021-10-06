/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "TestNotifier.h"
#include <simpletest.h>

#include <Notifier.h>
#include <KisPart.h>
#include <Document.h>

#include  <sdk/tests/testui.h>

void TestNotifier::testNotifier()
{
    KisPart *part = KisPart::instance();

    Notifier *notifier = new Notifier();
    connect(notifier, SIGNAL(imageCreated(Document*)), SLOT(documentAdded(Document*)), Qt::DirectConnection);

    QVERIFY(notifier->active());
    notifier->setActive(false);
    QVERIFY(!notifier->active());

    KisDocument *doc = part->createDocument();
    part->addDocument(doc);

    QVERIFY(m_document);

}

void TestNotifier::documentAdded(Document *image)
{
    m_document = image;
}

KISTEST_MAIN(TestNotifier)

