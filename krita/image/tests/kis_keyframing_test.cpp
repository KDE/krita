/*
 *  Copyright (c) 2015 Jouni Pentikäinen <mctyyppi42@gmail.com>
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
