/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_annotation_test.h"

#include <simpletest.h>
#include "kis_annotation.h"

void KisAnnotationTest::testCreation()
{
    QString s("Test");
    QByteArray b("test");
    KisAnnotation test(s,s,b);
}


SIMPLE_TEST_MAIN(KisAnnotationTest)
