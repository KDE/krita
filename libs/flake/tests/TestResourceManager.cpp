/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "TestResourceManager.h"

#include "KoCanvasResourceProvider.h"
#include "KoResourceManager_p.h"
#include "KoPathShape.h"
#include "KoUnit.h"
#include <QSignalSpy>
#include <QTest>

#include "kis_debug.h"

void TestResourceManager::koShapeResource()
{
    KoPathShape * shape = new KoPathShape();
    int key = 9001;

    KoCanvasResourceProvider rp( 0 );
    rp.setResource( key, shape );
    QVERIFY( shape == rp.koShapeResource( key ) );
}

void TestResourceManager::testUnitChanged()
{
    KoCanvasResourceProvider rm(0);
    QSignalSpy spy(&rm, SIGNAL(canvasResourceChanged(int,QVariant)));

    KoUnit a;
    rm.setResource(KoCanvasResourceProvider::Unit, a);
    QCOMPARE(spy.count(), 1);

    KoUnit b(KoUnit::Millimeter);
    rm.setResource(KoCanvasResourceProvider::Unit, b);
    QCOMPARE(spy.count(), 2);
}

#include "kis_pointer_utils.h"

struct DerivedResource : public KoDerivedResourceConverter
{
    DerivedResource(int key, int sourceKey) : KoDerivedResourceConverter(key, sourceKey) {}

    QVariant fromSource(const QVariant &value) override {
        return value.toInt() + 10;
    }

    QVariant toSource(const QVariant &value, const QVariant &sourceValue) override {
        Q_UNUSED(sourceValue);
        return value.toInt() - 10;
    }
};

void TestResourceManager::testConverters()
{
    KoResourceManager m;

    const int key1 = 1;
    const int key2 = 2;
    const int derivedKey = 3;

    m.setResource(key1, 1);
    m.setResource(key2, 2);

    QCOMPARE(m.resource(key1).toInt(), 1);
    QCOMPARE(m.resource(key2).toInt(), 2);
    QVERIFY(!m.hasResource(derivedKey));

    m.addDerivedResourceConverter(toQShared(new DerivedResource(derivedKey, key2)));

    QCOMPARE(m.resource(derivedKey).toInt(), 12);
    QVERIFY(m.hasResource(derivedKey));

    m.setResource(derivedKey, 15);

    QCOMPARE(m.resource(key2).toInt(), 5);
    QCOMPARE(m.resource(derivedKey).toInt(), 15);

    QVERIFY(m.hasResource(derivedKey));
    m.clearResource(derivedKey);
    QVERIFY(m.hasResource(derivedKey));
    m.clearResource(key2);
    QVERIFY(!m.hasResource(derivedKey));
}

void TestResourceManager::testDerivedChanged()
{
    // const int key1 = 1;
    const int key2 = 2;
    const int derivedKey = 3;
    const int otherDerivedKey = 4;

    KoCanvasResourceProvider m(0);
    m.addDerivedResourceConverter(toQShared(new DerivedResource(derivedKey, key2)));
    m.addDerivedResourceConverter(toQShared(new DerivedResource(otherDerivedKey, key2)));

    m.setResource(derivedKey, 15);

    QCOMPARE(m.resource(key2).toInt(), 5);
    QCOMPARE(m.resource(derivedKey).toInt(), 15);

    QSignalSpy spy(&m, SIGNAL(canvasResourceChanged(int,QVariant)));

    m.setResource(derivedKey, 16);

    QCOMPARE(spy.count(), 3);
    QList<QVariant> args;

    args = spy[0];
    QCOMPARE(args[0].toInt(), derivedKey);
    QCOMPARE(args[1].toInt(), 16);

    args = spy[1];
    QCOMPARE(args[0].toInt(), key2);
    QCOMPARE(args[1].toInt(), 6);

    args = spy[2];
    QCOMPARE(args[0].toInt(), otherDerivedKey);
    QCOMPARE(args[1].toInt(), 16);

    spy.clear();

    m.setResource(key2, 7);
    QCOMPARE(spy.count(), 3);

    args = spy[0];
    QCOMPARE(args[0].toInt(), key2);
    QCOMPARE(args[1].toInt(), 7);

    args = spy[1];
    QCOMPARE(args[0].toInt(), otherDerivedKey);
    QCOMPARE(args[1].toInt(), 17);

    args = spy[2];
    QCOMPARE(args[0].toInt(), derivedKey);
    QCOMPARE(args[1].toInt(), 17);
}

struct ComplexResource {
    QHash<int, QVariant> m_resources;
};

typedef QSharedPointer<ComplexResource> ComplexResourceSP;
Q_DECLARE_METATYPE(ComplexResourceSP);

struct ComplexConverter : public KoDerivedResourceConverter
{
    ComplexConverter(int key, int sourceKey) : KoDerivedResourceConverter(key, sourceKey) {}

    QVariant fromSource(const QVariant &value) override {
        KIS_ASSERT(value.canConvert<ComplexResourceSP>());
        ComplexResourceSP res = value.value<ComplexResourceSP>();

        return res->m_resources[key()];
    }

    QVariant toSource(const QVariant &value, const QVariant &sourceValue) override {
        KIS_ASSERT(sourceValue.canConvert<ComplexResourceSP>());
        ComplexResourceSP res = sourceValue.value<ComplexResourceSP>();

        res->m_resources[key()] = value;
        return QVariant::fromValue(res);
    }
};

struct ComplexMediator : public KoResourceUpdateMediator
{
    ComplexMediator(int key) : KoResourceUpdateMediator(key) {}

    void connectResource(QVariant sourceResource) override {
        m_res = sourceResource;
    }

    void forceNotify() {
        emit sigResourceChanged(key());
    }

    QVariant m_res;
};
typedef QSharedPointer<ComplexMediator> ComplexMediatorSP;

void TestResourceManager::testComplexResource()
{
    const int key = 2;
    const int complex1 = 3;
    const int complex2 = 4;

    KoCanvasResourceProvider m(0);
    m.addDerivedResourceConverter(toQShared(new ComplexConverter(complex1, key)));
    m.addDerivedResourceConverter(toQShared(new ComplexConverter(complex2, key)));

    ComplexMediatorSP mediator(new ComplexMediator(key));
    m.addResourceUpdateMediator(mediator);

    QSignalSpy spy(&m, SIGNAL(canvasResourceChanged(int,QVariant)));

    ComplexResourceSP r1(new ComplexResource());
    r1->m_resources[complex1] = 10;
    r1->m_resources[complex2] = 20;

    ComplexResourceSP r2(new ComplexResource());
    r2->m_resources[complex1] = 15;
    r2->m_resources[complex2] = 25;


    // ####################################################
    // Initial assignment
    // ####################################################
    m.setResource(key, QVariant::fromValue(r1));

    QCOMPARE(mediator->m_res.value<ComplexResourceSP>(), r1);
    QCOMPARE(m.resource(key).value<ComplexResourceSP>(), r1);
    QCOMPARE(m.resource(complex1).toInt(), 10);
    QCOMPARE(m.resource(complex2).toInt(), 20);

    QCOMPARE(spy[0][0].toInt(), key);
    QCOMPARE(spy[0][1].value<ComplexResourceSP>(), r1);
    QCOMPARE(spy[1][0].toInt(), complex2);
    QCOMPARE(spy[1][1].toInt(), 20);
    QCOMPARE(spy[2][0].toInt(), complex1);
    QCOMPARE(spy[2][1].toInt(), 10);
    spy.clear();

    // ####################################################
    // Change the whole resource
    // ####################################################
    m.setResource(key, QVariant::fromValue(r2));

    QCOMPARE(mediator->m_res.value<ComplexResourceSP>(), r2);
    QCOMPARE(m.resource(key).value<ComplexResourceSP>(), r2);
    QCOMPARE(m.resource(complex1).toInt(), 15);
    QCOMPARE(m.resource(complex2).toInt(), 25);

    QCOMPARE(spy[0][0].toInt(), key);
    QCOMPARE(spy[0][1].value<ComplexResourceSP>(), r2);
    QCOMPARE(spy[1][0].toInt(), complex2);
    QCOMPARE(spy[1][1].toInt(), 25);
    QCOMPARE(spy[2][0].toInt(), complex1);
    QCOMPARE(spy[2][1].toInt(), 15);
    spy.clear();

    // ####################################################
    // Change a derived resource
    // ####################################################
    m.setResource(complex1, 16);

    QCOMPARE(mediator->m_res.value<ComplexResourceSP>(), r2);
    QCOMPARE(m.resource(key).value<ComplexResourceSP>(), r2);
    QCOMPARE(m.resource(complex1).toInt(), 16);
    QCOMPARE(m.resource(complex2).toInt(), 25);

    QCOMPARE(spy[0][0].toInt(), complex1);
    QCOMPARE(spy[0][1].toInt(), 16);
    spy.clear();

    // ####################################################
    // Change another derived resource
    // ####################################################
    m.setResource(complex2, 26);

    QCOMPARE(mediator->m_res.value<ComplexResourceSP>(), r2);
    QCOMPARE(m.resource(key).value<ComplexResourceSP>(), r2);
    QCOMPARE(m.resource(complex1).toInt(), 16);
    QCOMPARE(m.resource(complex2).toInt(), 26);

    QCOMPARE(spy[0][0].toInt(), complex2);
    QCOMPARE(spy[0][1].toInt(), 26);
    spy.clear();

    // ####################################################
    // Switch back the whole source resource
    // ####################################################
    m.setResource(key, QVariant::fromValue(r1));

    QCOMPARE(mediator->m_res.value<ComplexResourceSP>(), r1);
    QCOMPARE(m.resource(key).value<ComplexResourceSP>(), r1);
    QCOMPARE(m.resource(complex1).toInt(), 10);
    QCOMPARE(m.resource(complex2).toInt(), 20);

    QCOMPARE(spy[0][0].toInt(), key);
    QCOMPARE(spy[0][1].value<ComplexResourceSP>(), r1);
    QCOMPARE(spy[1][0].toInt(), complex2);
    QCOMPARE(spy[1][1].toInt(), 20);
    QCOMPARE(spy[2][0].toInt(), complex1);
    QCOMPARE(spy[2][1].toInt(), 10);
    spy.clear();

    // ####################################################
    // The value keep unchanged case!
    // ####################################################
    m.setResource(complex1, 10);

    QCOMPARE(mediator->m_res.value<ComplexResourceSP>(), r1);
    QCOMPARE(m.resource(key).value<ComplexResourceSP>(), r1);
    QCOMPARE(m.resource(complex1).toInt(), 10);
    QCOMPARE(m.resource(complex2).toInt(), 20);

    QCOMPARE(spy.size(), 0);
    spy.clear();

    // ####################################################
    // While switching a complex resource one derived value
    // is kept unchanged
    // ####################################################
    r2->m_resources[complex1] = 10;
    m.setResource(key, QVariant::fromValue(r2));

    QCOMPARE(mediator->m_res.value<ComplexResourceSP>(), r2);
    QCOMPARE(m.resource(key).value<ComplexResourceSP>(), r2);
    QCOMPARE(m.resource(complex1).toInt(), 10);
    QCOMPARE(m.resource(complex2).toInt(), 26);

    QCOMPARE(spy[0][0].toInt(), key);
    QCOMPARE(spy[0][1].value<ComplexResourceSP>(), r2);
    QCOMPARE(spy[1][0].toInt(), complex2);
    QCOMPARE(spy[1][1].toInt(), 26);
    spy.clear();

    // ####################################################
    // No devived values are changed!
    // ####################################################
    *r1 = *r2;
    m.setResource(key, QVariant::fromValue(r1));

    QCOMPARE(mediator->m_res.value<ComplexResourceSP>(), r1);
    QCOMPARE(m.resource(key).value<ComplexResourceSP>(), r1);
    QCOMPARE(m.resource(complex1).toInt(), 10);
    QCOMPARE(m.resource(complex2).toInt(), 26);

    QCOMPARE(spy[0][0].toInt(), key);
    QCOMPARE(spy[0][1].value<ComplexResourceSP>(), r1);
    spy.clear();

    // ####################################################
    // Try to set the same pointer. No signals emitted!
    // ####################################################
    m.setResource(key, QVariant::fromValue(r1));

    QCOMPARE(mediator->m_res.value<ComplexResourceSP>(), r1);
    QCOMPARE(m.resource(key).value<ComplexResourceSP>(), r1);
    QCOMPARE(m.resource(complex1).toInt(), 10);
    QCOMPARE(m.resource(complex2).toInt(), 26);

    QCOMPARE(spy.size(), 0);
    spy.clear();


    // ####################################################
    // Internals 'officially' changed, but the values not
    // ####################################################
    mediator->forceNotify();

    QCOMPARE(spy.size(), 0);
    spy.clear();

    // ####################################################
    // We changed the values, but didn't notify anyone :)
    // ####################################################
    r1->m_resources[complex1] = 11;
    r1->m_resources[complex2] = 21;

    mediator->forceNotify();

    QCOMPARE(spy[0][0].toInt(), complex2);
    QCOMPARE(spy[0][1].toInt(), 21);
    QCOMPARE(spy[1][0].toInt(), complex1);
    QCOMPARE(spy[1][1].toInt(), 11);
    spy.clear();
}

struct NeverChangingResource : public KoDerivedResourceConverter
{
    NeverChangingResource(int key, int sourceKey) : KoDerivedResourceConverter(key, sourceKey) {}

    QVariant fromSource(const QVariant &value) override {
        Q_UNUSED(value);
        return 10;
    }

    QVariant toSource(const QVariant &value, const QVariant &sourceValue) override {
        Q_UNUSED(value);
        return sourceValue;
    }
};

void TestResourceManager::testNeverChangingConverters()
{
    KoResourceManager m;

    const int key1 = 1;
    const int key2 = 2;
    const int derivedKey = 3;

    m.setResource(key1, 1);
    m.setResource(key2, 2);

    QCOMPARE(m.resource(key1).toInt(), 1);
    QCOMPARE(m.resource(key2).toInt(), 2);
    QVERIFY(!m.hasResource(derivedKey));

    m.addDerivedResourceConverter(toQShared(new NeverChangingResource(derivedKey, key2)));

    QVERIFY(m.hasResource(derivedKey));
    QCOMPARE(m.resource(derivedKey).toInt(), 10);

    m.setResource(derivedKey, 150);

    QCOMPARE(m.resource(key2).toInt(), 2);
    QCOMPARE(m.resource(derivedKey).toInt(), 10);
}


QTEST_MAIN(TestResourceManager)
