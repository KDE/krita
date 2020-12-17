/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LIQUIFY_TRANSFORM_WORKER_TEST_H
#define __KIS_LIQUIFY_TRANSFORM_WORKER_TEST_H

#include <QtTest>

class KisLiquifyTransformWorkerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPoints();
    void testPointsQImage();
    void testIdentityTransform();
};

#endif /* __KIS_LIQUIFY_TRANSFORM_WORKER_TEST_H */
