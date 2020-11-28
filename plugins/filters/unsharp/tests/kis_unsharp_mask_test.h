/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_UNSHARP_MASK_TEST_H
#define __KIS_UNSHARP_MASK_TEST_H

#include <QtTest>

class KisUnsharpMaskTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testUnsharpWithTransparency();
};

#endif /* __KIS_UNSHARP_MASK_TEST_H */
