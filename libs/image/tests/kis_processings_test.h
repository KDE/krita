/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PROCESSINGS_TEST_H
#define __KIS_PROCESSINGS_TEST_H

#include <QtTest>

class KisProcessingsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCropVisitor();
    void testTransformVisitorScale();
    void testTransformVisitorScaleRotate();
};

#endif /* __KIS_PROCESSINGS_TEST_H */
