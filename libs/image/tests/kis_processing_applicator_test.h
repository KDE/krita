/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PROCESSING_APPLICATOR_TEST_H
#define __KIS_PROCESSING_APPLICATOR_TEST_H

#include <simpletest.h>

class KisProcessingApplicatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testNonRecursiveProcessing();
    void testRecursiveProcessing();
    void testNoUIUpdates();
};

#endif /* __KIS_PROCESSING_APPLICATOR_TEST_H */
