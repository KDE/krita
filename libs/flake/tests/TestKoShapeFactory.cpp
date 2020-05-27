/* This file is part of the KDE project
 * Copyright (c) 2007 Boudewijn Rempt (boud@valdyas.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "TestKoShapeFactory.h"

#include <QTest>
#include <QBuffer>

#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
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
