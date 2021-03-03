/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TEST_ANIMATED_TRANSFORM_PARAMETERS_H
#define TEST_ANIMATED_TRANSFORM_PARAMETERS_H

#include <simpletest.h>

class KisAnimatedTransformParametersTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testTransformKeyframing();
};

#endif

