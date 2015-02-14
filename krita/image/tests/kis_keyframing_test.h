
#ifndef KIS_KEYFRAMING_TEST_H
#define KIS_KEYFRAMING_TEST_H

#include <QtTest>

class KisKeyframingTest : public QObject
{
    Q_OBJECT

private slots:

    void testChannels();
    void testKeyframes();

};

#endif

