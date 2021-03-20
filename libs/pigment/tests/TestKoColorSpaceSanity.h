/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _TESTKOCOLORSPACESANITY_H_
#define _TESTKOCOLORSPACESANITY_H_

#include <QObject>

class TestKoColorSpaceSanity : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testChannelsInfo();
};

#endif
