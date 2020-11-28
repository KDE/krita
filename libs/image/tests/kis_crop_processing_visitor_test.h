/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CROP_PROCESSING_VISITOR_TEST_H
#define __KIS_CROP_PROCESSING_VISITOR_TEST_H

#include <QtTest>

class KisCropProcessingVisitorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testUndo();
    void testCropTransparencyMask();
    void testWrappedInCommand();
};

#endif /* __KIS_CROP_PROCESSING_VISITOR_TEST_H */
