/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PERSPECTIVE_TRANSFORM_WORKER_TEST_H
#define __KIS_PERSPECTIVE_TRANSFORM_WORKER_TEST_H

#include <simpletest.h>

class KisPerspectiveTransformWorkerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSimpleTransform();
};

#endif /* __KIS_PERSPECTIVE_TRANSFORM_WORKER_TEST_H */
