/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SPATIAL_CONTAINER_TEST_H
#define __KIS_SPATIAL_CONTAINER_TEST_H

#include <QTest>

class KisSpatialContainerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testSpatialContainerAddMovePoints();
    void testSpatialContainerSearchPoints();

    void testSpatialContainerInitializeWithGridPoints();

    void testMemoryCleanup();
    void testDeepCopy();

};

#endif /* __KIS_SPATIAL_CONTAINER_TEST_H */
