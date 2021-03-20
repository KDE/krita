/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __FILTER_STROKE_TEST_H
#define __FILTER_STROKE_TEST_H

#include <simpletest.h>


class FilterStrokeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testBlurFilter();
};

#endif /* __FILTER_STROKE_TEST_H */
