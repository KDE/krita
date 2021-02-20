/*
 *  SPDX-FileCopyrightText: 2015 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_ONION_SKIN_COMPOSITOR_TEST_H
#define KIS_ONION_SKIN_COMPOSITOR_TEST_H

#include <QtTest>

class KisOnionSkinCompositorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testComposite();
    void testSettings();
};

#endif

