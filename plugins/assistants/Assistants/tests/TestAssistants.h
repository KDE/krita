/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef __TEST_ASSISTANTS_H
#define __TEST_ASSISTANTS_H

#include <simpletest.h>
#include "empty_nodes_test.h"

#include "kis_painting_assistant.h"


class TestAssistants : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testConcentricEllipseAdjustLine();
    void testMirroringPoints();
    void testProjection();

};

#endif /* __TEST_ASSISTANTS_H */
