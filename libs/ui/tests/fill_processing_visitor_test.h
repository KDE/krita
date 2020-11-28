/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __FILL_PROCESSING_VISITOR_TEST_H
#define __FILL_PROCESSING_VISITOR_TEST_H

#include <QtTest>

class FillProcessingVisitorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFillColorNoSelection();
    void testFillPatternNoSelection();
    void testFillColorHaveSelection();
    void testFillPatternHaveSelection();

    void testFillColorNoSelectionSelectionOnly();
    void testFillPatternNoSelectionSelectionOnly();
    void testFillColorHaveSelectionSelectionOnly();
    void testFillPatternHaveSelectionSelectionOnly();
};

#endif /* __FILL_PROCESSING_VISITOR_TEST_H */
