/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _TEST_KO_CHANNEL_INFO_H_
#define _TEST_KO_CHANNEL_INFO_H_

#include <QObject>

class TestKoChannelInfo : public QObject
{
    Q_OBJECT
    
private Q_SLOTS:
    
    void testDisplayPositionToChannelIndex();
    void testdisplayOrderSorted();
};

#endif

