/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISFRAMESERIALIZERTEST_H
#define KISFRAMESERIALIZERTEST_H

#include <QObject>

class KisFrameSerializerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testFrameDataSerialization();
    void testFrameUniquenessEstimation();
    void testFrameArithmetics();

};

#endif // KISFRAMESERIALIZERTEST_H
