/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_IMAGE_SIGNAL_ROUTER_TEST_H
#define __KIS_IMAGE_SIGNAL_ROUTER_TEST_H

#include <QtTest>

#include <QObject>
#include "kis_image_signal_router.h"
#include "empty_nodes_test.h"


class KisImageSignalRouterTest : public QObject, public TestUtil::EmptyNodesTest
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();

    void testSignalForwarding();

private:
    void checkNotification(KisImageSignalType notification, const char *signal);
};

#endif /* __KIS_IMAGE_SIGNAL_ROUTER_TEST_H */
