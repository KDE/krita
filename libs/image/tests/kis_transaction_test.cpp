/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transaction_test.h"
#include <simpletest.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kundo2qstack.h>

#include "kis_types.h"
#include "kis_transform_worker.h"
#include "kis_paint_device.h"
#include "kis_transaction.h"
#include "kis_surrogate_undo_adapter.h"
#include "kis_image.h"
#include "kis_paint_device_debug_utils.h"
#include "kistest.h"

void KisTransactionTest::testUndo()
{
    KisSurrogateUndoAdapter undoAdapter;
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    quint8* pixel = new quint8[cs->pixelSize()];
    cs->fromQColor(Qt::white, pixel);
    dev->fill(0, 0, 512, 512, pixel);

    cs->fromQColor(Qt::black, pixel);
    dev->fill(512, 0, 512, 512, pixel);

    QColor c1, c2;
    dev->pixel(5, 5, &c1);
    dev->pixel(517, 5, &c2);

    QVERIFY(c1 == Qt::white);
    QVERIFY(c2 == Qt::black);

    KisTransaction transaction(kundo2_noi18n("mirror"), dev, 0);
    KisTransformWorker::mirrorX(dev);
    transaction.commit(&undoAdapter);

    dev->pixel(5, 5, &c1);
    dev->pixel(517, 5, &c2);

    QVERIFY(c1 == Qt::black);
    QVERIFY(c2 == Qt::white);

    undoAdapter.undo();

    dev->pixel(5, 5, &c1);
    dev->pixel(517, 5, &c2);

    QVERIFY(c1 == Qt::white);
    QVERIFY(c2 == Qt::black);

}

void KisTransactionTest::testRedo()
{
    KisSurrogateUndoAdapter undoAdapter;

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    quint8* pixel = new quint8[cs->pixelSize()];
    cs->fromQColor(Qt::white, pixel);
    dev->fill(0, 0, 512, 512, pixel);

    cs->fromQColor(Qt::black, pixel);
    dev->fill(512, 0, 512, 512, pixel);

    QColor c1, c2;
    dev->pixel(5, 5, &c1);
    dev->pixel(517, 5, &c2);

    QVERIFY(c1 == Qt::white);
    QVERIFY(c2 == Qt::black);

    KisTransaction transaction(kundo2_noi18n("mirror"), dev, 0);
    KisTransformWorker::mirrorX(dev);
    transaction.commit(&undoAdapter);

    dev->pixel(5, 5, &c1);
    dev->pixel(517, 5, &c2);

    QVERIFY(c1 == Qt::black);
    QVERIFY(c2 == Qt::white);


    undoAdapter.undo();

    dev->pixel(5, 5, &c1);
    dev->pixel(517, 5, &c2);

    QVERIFY(c1 == Qt::white);
    QVERIFY(c2 == Qt::black);

    undoAdapter.redo();

    dev->pixel(5, 5, &c1);
    dev->pixel(517, 5, &c2);

    QVERIFY(c1 == Qt::black);
    QVERIFY(c2 == Qt::white);
}

void KisTransactionTest::testDeviceMove()
{
    KisSurrogateUndoAdapter undoAdapter;

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    QCOMPARE(dev->x(), 0);
    QCOMPARE(dev->y(), 0);

    KisTransaction t1(kundo2_noi18n("move1"), dev, 0);
    dev->moveTo(10,20);
    t1.commit(&undoAdapter);

    QCOMPARE(dev->x(), 10);
    QCOMPARE(dev->y(), 20);

    KisTransaction t2(kundo2_noi18n("move2"), dev, 0);
    dev->moveTo(7,11);
    t2.commit(&undoAdapter);

    QCOMPARE(dev->x(),  7);
    QCOMPARE(dev->y(), 11);

    undoAdapter.undo();

    QCOMPARE(dev->x(), 10);
    QCOMPARE(dev->y(), 20);

    undoAdapter.undo();

    QCOMPARE(dev->x(), 0);
    QCOMPARE(dev->y(), 0);

    undoAdapter.redo();

    QCOMPARE(dev->x(), 10);
    QCOMPARE(dev->y(), 20);

    undoAdapter.redo();

    QCOMPARE(dev->x(),  7);
    QCOMPARE(dev->y(), 11);
}

#include "kis_keyframe_channel.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_paint_device_frames_interface.h"
#include "testing_timed_default_bounds.h"

#include <KoColor.h>


void KisTransactionTest::testUndoWithUnswitchedFrames()
{
    KisSurrogateUndoAdapter undoAdapter;
    const QRect imageRect(0,0,100,100);


    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();
    dev->setDefaultBounds(bounds);

    KisRasterKeyframeChannel *channel = dev->createKeyframeChannel(KisKeyframeChannel::Raster);
    QVERIFY(channel);

    KisPaintDeviceFramesInterface *i = dev->framesInterface();
    QVERIFY(i);

    QCOMPARE(i->frames().size(), 1);


    dev->fill(QRect(10,10,20,20), KoColor(Qt::white, cs));

    KIS_DUMP_DEVICE_2(dev, imageRect, "00_f0_w20", "dd");
    QCOMPARE(dev->exactBounds(), QRect(10,10,20,20));


    // add keyframe at position 10
    channel->addKeyframe(10);

    // add keyframe at position 11
    channel->addKeyframe(11);

    // add keyframe at position 12
    channel->addKeyframe(12);

    KIS_DUMP_DEVICE_2(dev, imageRect, "01_f0_b20", "dd");
    QCOMPARE(dev->exactBounds(), QRect(10,10,20,20));

    {
        KisTransaction transaction(kundo2_noi18n("first_stroke"), dev, 0);

        dev->clear();
        dev->fill(QRect(40,40,21,21), KoColor(Qt::red, cs));

        transaction.commit(&undoAdapter);

        KIS_DUMP_DEVICE_2(dev, imageRect, "02_f0_b21_stroke", "dd");
        QCOMPARE(dev->exactBounds(), QRect(40,40,21,21));
    }

    // switch to frame 10
    bounds->testingSetTime(10);

    KIS_DUMP_DEVICE_2(dev, imageRect, "03_f10_b0_switched", "dd");
    QVERIFY(dev->exactBounds().isEmpty());

    {
        KisTransaction transaction(kundo2_noi18n("second_stroke"), dev, 0);

        dev->fill(QRect(60,60,22,22), KoColor(Qt::green, cs));

        transaction.commit(&undoAdapter);

        KIS_DUMP_DEVICE_2(dev, imageRect, "04_f10_b22_stroke", "dd");
        QCOMPARE(dev->exactBounds(), QRect(60,60,22,22));
    }

    undoAdapter.undo();

    KIS_DUMP_DEVICE_2(dev, imageRect, "05_f10_b0_undone", "dd");
    QVERIFY(dev->exactBounds().isEmpty());

    bounds->testingSetTime(0);
    KIS_DUMP_DEVICE_2(dev, imageRect, "06_f0_b21_undone", "dd");
    QCOMPARE(dev->exactBounds(), QRect(40,40,21,21));

    bounds->testingSetTime(10);
    QVERIFY(dev->exactBounds().isEmpty());

    undoAdapter.undo();

    KIS_DUMP_DEVICE_2(dev, imageRect, "07_f10_b0_undone_x2", "dd");
    QVERIFY(dev->exactBounds().isEmpty());

    bounds->testingSetTime(0);
    KIS_DUMP_DEVICE_2(dev, imageRect, "08_f0_b20_undone_x2", "dd");
    QCOMPARE(dev->exactBounds(), QRect(10,10,20,20));

    {
        KisTransaction transaction(kundo2_noi18n("third_move"), dev, 0);

        dev->moveTo(17,17);

        transaction.commit(&undoAdapter);

        KIS_DUMP_DEVICE_2(dev, imageRect, "09_f0_o27_move", "dd");
        QCOMPARE(dev->exactBounds(), QRect(27,27,20,20));
    }

    bounds->testingSetTime(10);
    QVERIFY(dev->exactBounds().isEmpty());

    undoAdapter.undo();

    KIS_DUMP_DEVICE_2(dev, imageRect, "10_f10_b0_undone_x3", "dd");
    QVERIFY(dev->exactBounds().isEmpty());

    bounds->testingSetTime(0);
    KIS_DUMP_DEVICE_2(dev, imageRect, "11_f0_b20_undone_x3", "dd");
    QCOMPARE(dev->exactBounds(), QRect(10,10,20,20));
}

#include "KisTransactionWrapperFactory.h"

void KisTransactionTest::testTransactionWrapperFactory()
{
    KisSurrogateUndoAdapter undoAdapter;
    const QRect imageRect(0,0,100,100);


    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    enum CommandState {
        Inexistent,
        Created,
        Redone,
        Undone,
        Destroyed
    };
    Q_ENUMS(CommandState)

    struct StateTrackingUndoCommand : public KUndo2Command
    {
        StateTrackingUndoCommand(CommandState &state)
            : m_state(state)
        {
            KIS_ASSERT(m_state == Inexistent);
            m_state = Created;
        }

        ~StateTrackingUndoCommand() override {
            m_state = Destroyed;
        }

        void redo() override {
            m_state = Redone;
        }

        void undo() override {
            m_state = Undone;
        }

        CommandState &m_state;
    };

    struct Factory : public KisTransactionWrapperFactory {
        Factory(CommandState &beginState, CommandState &endState)
            : m_beginState(beginState), m_endState(endState)
        {
        }

        KUndo2Command* createBeginTransactionCommand(KisPaintDeviceSP device) override {
            Q_UNUSED(device);
            return new StateTrackingUndoCommand(m_beginState);
        }

        KUndo2Command* createEndTransactionCommand() override {
            return new StateTrackingUndoCommand(m_endState);
        }

        CommandState &m_beginState;
        CommandState &m_endState;
    };

    CommandState beginState = Inexistent;
    CommandState endState = Inexistent;

    KisTransaction transaction(dev, AUTOKEY_DISABLED, 0, -1, new Factory(beginState, endState));

    QCOMPARE(beginState, Redone);
    QCOMPARE(endState, Inexistent);

    transaction.commit(&undoAdapter);

    QCOMPARE(beginState, Redone);
    QCOMPARE(endState, Redone);

    undoAdapter.undo();

    QCOMPARE(beginState, Undone);
    QCOMPARE(endState, Undone);

    undoAdapter.redo();

    QCOMPARE(beginState, Redone);
    QCOMPARE(endState, Redone);
}

#include "KisInterstrokeDataTransactionWrapperFactory.h"
#include "KisInterstrokeDataFactory.h"
#include "KisInterstrokeData.h"

struct TestInterstrokeData : public KisInterstrokeData
{
    TestInterstrokeData(KisPaintDeviceSP device,
                        int _typeId, int _value)
        : KisInterstrokeData(device),
          typeId(_typeId), value(_value)
    {
    }

    void beginTransaction() override {
        ENTER_FUNCTION() << ppVar(value);
        m_savedValue = value;
    }

    KUndo2Command* endTransaction() override {

        ENTER_FUNCTION() << ppVar(value) << ppVar(m_savedValue);
        struct SimpleIntCommand : public KUndo2Command
        {
            SimpleIntCommand(int *pointer, int newValue, int oldValue)
                : m_pointer(pointer),
                  m_newValue(newValue),
                  m_oldValue(oldValue)
            {
            }

            void redo() override {
                if (m_firstRedo) {
                    m_firstRedo = false;
                    return;
                }

                *m_pointer = m_newValue;
            }

            void undo() override {
                *m_pointer = m_oldValue;
            }

        private:
            bool m_firstRedo {false};
            int *m_pointer {0};
            int m_newValue {0};
            int m_oldValue {0};
        };

        return new SimpleIntCommand(&value, value, m_savedValue);
    }

    int typeId {0};
    int value {0};

    int m_savedValue {0};
};

struct TestInterstrokeDataFactory : public KisInterstrokeDataFactory
{
    TestInterstrokeDataFactory(int typeId)
        : m_typeId(typeId)
    {
    }

    bool isCompatible(KisInterstrokeData *_data) override {
        TestInterstrokeData *data = dynamic_cast<TestInterstrokeData*>(_data);
        return data && data->typeId == m_typeId;
    }

    KisInterstrokeData * create(KisPaintDeviceSP device) override {
        Q_UNUSED(device);
        return new TestInterstrokeData(device, m_typeId, 0);
    }

    int m_typeId {0};
};

TestInterstrokeData* resolveType(KisInterstrokeDataSP data) {
    TestInterstrokeData *result = dynamic_cast<TestInterstrokeData*>(data.data());
    KIS_ASSERT(result);
    return result;
}

void KisTransactionTest::testInterstrokeData()
{
    KisSurrogateUndoAdapter undoAdapter;
    const QRect imageRect(0,0,100,100);


    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    KisTransaction transaction1(dev, AUTOKEY_DISABLED, 0, -1,
                                new KisInterstrokeDataTransactionWrapperFactory(
                                    new TestInterstrokeDataFactory(13)));

    QVERIFY(dev->interstrokeData());
    QCOMPARE(resolveType(dev->interstrokeData())->typeId, 13);
    QCOMPARE(resolveType(dev->interstrokeData())->value, 0);

    resolveType(dev->interstrokeData())->value = 10;

    transaction1.commit(&undoAdapter);

    QVERIFY(dev->interstrokeData());
    QCOMPARE(resolveType(dev->interstrokeData())->typeId, 13);
    QCOMPARE(resolveType(dev->interstrokeData())->value, 10);

    KisInterstrokeDataSP firstData = dev->interstrokeData();

    KisTransaction transaction2(dev, AUTOKEY_DISABLED, 0, -1,
                                new KisInterstrokeDataTransactionWrapperFactory(
                                    new TestInterstrokeDataFactory(13)));

    QVERIFY(dev->interstrokeData());
    QCOMPARE(dev->interstrokeData(), firstData);
    QCOMPARE(resolveType(dev->interstrokeData())->typeId, 13);
    QCOMPARE(resolveType(dev->interstrokeData())->value, 10);

    resolveType(dev->interstrokeData())->value = 20;

    transaction2.commit(&undoAdapter);

    QVERIFY(dev->interstrokeData());
    QCOMPARE(dev->interstrokeData(), firstData);
    QCOMPARE(resolveType(dev->interstrokeData())->typeId, 13);
    QCOMPARE(resolveType(dev->interstrokeData())->value, 20);

    KisTransaction transaction3(dev, AUTOKEY_DISABLED, 0, -1,
                                new KisInterstrokeDataTransactionWrapperFactory(
                                    new TestInterstrokeDataFactory(17)));

    QVERIFY(dev->interstrokeData());
    QVERIFY(dev->interstrokeData() != firstData);
    QCOMPARE(resolveType(dev->interstrokeData())->typeId, 17);
    QCOMPARE(resolveType(dev->interstrokeData())->value, 0);

    resolveType(dev->interstrokeData())->value = 30;

    transaction3.commit(&undoAdapter);

    QVERIFY(dev->interstrokeData());
    QVERIFY(dev->interstrokeData() != firstData);
    QCOMPARE(resolveType(dev->interstrokeData())->typeId, 17);
    QCOMPARE(resolveType(dev->interstrokeData())->value, 30);


    KisTransaction transaction4(dev);
    QVERIFY(!dev->interstrokeData());
    transaction4.commit(&undoAdapter);
    QVERIFY(!dev->interstrokeData());

    undoAdapter.undo();

    QVERIFY(dev->interstrokeData());
    QVERIFY(dev->interstrokeData() != firstData);
    QCOMPARE(resolveType(dev->interstrokeData())->typeId, 17);
    QCOMPARE(resolveType(dev->interstrokeData())->value, 30);

    undoAdapter.undo();

    QVERIFY(dev->interstrokeData());
    QCOMPARE(dev->interstrokeData(), firstData);
    QCOMPARE(resolveType(dev->interstrokeData())->typeId, 13);
    QCOMPARE(resolveType(dev->interstrokeData())->value, 20);

    undoAdapter.undo();

    QVERIFY(dev->interstrokeData());
    QCOMPARE(dev->interstrokeData(), firstData);
    QCOMPARE(resolveType(dev->interstrokeData())->typeId, 13);
    QCOMPARE(resolveType(dev->interstrokeData())->value, 10);

    undoAdapter.undo();

    QVERIFY(!dev->interstrokeData());
}

void KisTransactionTest::testInterstrokeDataWithUnswitchedFrames()
{
    KisSurrogateUndoAdapter undoAdapter;
    const QRect imageRect(0,0,100,100);


    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    TestUtil::TestingTimedDefaultBounds *bounds = new TestUtil::TestingTimedDefaultBounds();
    dev->setDefaultBounds(bounds);

    KisRasterKeyframeChannel *channel = dev->createKeyframeChannel(KisKeyframeChannel::Raster);
    QVERIFY(channel);

    KisPaintDeviceFramesInterface *i = dev->framesInterface();
    QVERIFY(i);

    QCOMPARE(i->frames().size(), 1);


    dev->fill(QRect(10,10,20,20), KoColor(Qt::white, cs));

    KIS_DUMP_DEVICE_2(dev, imageRect, "00_f0_w20", "dd");
    QCOMPARE(dev->exactBounds(), QRect(10,10,20,20));


    // add keyframe at position 10
    channel->addKeyframe(10);

    // add keyframe at position 11
    channel->addKeyframe(11);

    // add keyframe at position 12
    channel->addKeyframe(12);

    QVERIFY(!dev->interstrokeData());

    {
        KisTransaction transaction(dev, AUTOKEY_DISABLED, 0, -1,
                new KisInterstrokeDataTransactionWrapperFactory(
                    new TestInterstrokeDataFactory(17)));

        QVERIFY(dev->interstrokeData());
        QCOMPARE(resolveType(dev->interstrokeData())->typeId, 17);
        QCOMPARE(resolveType(dev->interstrokeData())->value, 0);

        resolveType(dev->interstrokeData())->value = 30;
        transaction.commit(&undoAdapter);
    }

    // switch to frame 10
    bounds->testingSetTime(10);
    QVERIFY(!dev->interstrokeData());

    {
        KisTransaction transaction(dev, AUTOKEY_DISABLED, 0, -1,
                new KisInterstrokeDataTransactionWrapperFactory(
                    new TestInterstrokeDataFactory(18)));

        QVERIFY(dev->interstrokeData());
        QCOMPARE(resolveType(dev->interstrokeData())->typeId, 18);
        QCOMPARE(resolveType(dev->interstrokeData())->value, 0);

        resolveType(dev->interstrokeData())->value = 40;
        transaction.commit(&undoAdapter);
    }

    QVERIFY(dev->interstrokeData());

    undoAdapter.undo();

    QVERIFY(!dev->interstrokeData());

    // switch to frame 0
    bounds->testingSetTime(0);

    QVERIFY(dev->interstrokeData());
    QCOMPARE(resolveType(dev->interstrokeData())->typeId, 17);
    QCOMPARE(resolveType(dev->interstrokeData())->value, 30);

    // switch back to frame 10
    bounds->testingSetTime(10);

    QVERIFY(!dev->interstrokeData());

    undoAdapter.undo();

    QVERIFY(!dev->interstrokeData());

    // switch to frame 0
    bounds->testingSetTime(0);

    QVERIFY(!dev->interstrokeData());
}

SIMPLE_TEST_MAIN(KisTransactionTest)
