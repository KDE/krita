/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt (boud@valdyas.org)
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "TestKoShapeFactory.h"

#include <simpletest.h>
#include <QBuffer>

#include <KoShapeLoadingContext.h>
#include <KoPathShapeFactory.h>
#include <KoShape.h>
#include <KoShapeFactoryBase.h>
#include <KoXmlNS.h>
#include <FlakeDebug.h>

void TestKoShapeFactory::testCreateFactory()
{
    KoShapeFactoryBase * factory = new KoPathShapeFactory(QStringList());
    QVERIFY(factory != 0);
    delete factory;
}

void TestKoShapeFactory::testSupportsKoXmlElement()
{
}

void TestKoShapeFactory::testPriority()
{
    KoShapeFactoryBase * factory = new KoPathShapeFactory(QStringList());
    QVERIFY(factory->loadingPriority() == 0);
    delete factory;
}

void TestKoShapeFactory::testCreateDefaultShape()
{
    KoShapeFactoryBase * factory = new KoPathShapeFactory(QStringList());
    KoShape *shape = factory->createDefaultShape();
    QVERIFY(shape != 0);
    delete shape;
    delete factory;
}

void TestKoShapeFactory::testCreateShape()
{
    KoShapeFactoryBase * factory = new KoPathShapeFactory(QStringList());
    KoShape *shape = factory->createShape(0);
    QVERIFY(shape != 0);
    delete shape;
    delete factory;
}

QTEST_GUILESS_MAIN(TestKoShapeFactory)
