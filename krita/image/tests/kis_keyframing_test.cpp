
#include "kis_keyframing_test.h"
#include <qtest_kde.h>

#include "kis_keyframe_sequence.h"

void KisKeyframingTest::testChannels()
{
    KisKeyframeSequence *seq = new KisKeyframeSequence();

    KisKeyframeChannel *channelCreated = seq->createChannel("foobar", "FooBar");

    QVERIFY(channelCreated != 0);
    QCOMPARE(seq->getChannel("foobar"), channelCreated);

    QVERIFY(seq->getChannel("baz") == 0);

    delete seq;
}
void KisKeyframingTest::testKeyframes()
{
    KisKeyframeChannel *channel = new KisKeyframeChannel("", "");

    QVERIFY(channel->getValueAt(10).isNull());

    QVariant value1 = QVariant("foo");
    channel->setKeyframe(10, value1);

    QVariant value2 = QVariant("bar");
    channel->setKeyframe(13, value2);

    QVERIFY(channel->getValueAt(9).isNull());
    QCOMPARE(channel->getValueAt(10), value1);
    QCOMPARE(channel->getValueAt(12), value1);
    QCOMPARE(channel->getValueAt(13), value2);

    channel->deleteKeyframe(13);

    QCOMPARE(channel->getValueAt(12), value1);
    QCOMPARE(channel->getValueAt(13), value1);

    channel->deleteKeyframe(10);

    QVERIFY(channel->getValueAt(9).isNull());
    QVERIFY(channel->getValueAt(12).isNull());
    QVERIFY(channel->getValueAt(13).isNull());

    delete channel;
}

QTEST_KDEMAIN(KisKeyframingTest, NoGUI)
#include "kis_keyframing_test.moc"
