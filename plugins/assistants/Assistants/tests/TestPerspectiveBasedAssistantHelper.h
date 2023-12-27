/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef __TEST_PERSPECTIVE_BASED_ASSISTANT_HELPER_H
#define __TEST_PERSPECTIVE_BASED_ASSISTANT_HELPER_H

#include <simpletest.h>
#include "empty_nodes_test.h"

#include "kis_painting_assistant.h"


class TestPerspectiveBasedAssistantHelper : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testDistanceInGrid();

private:
    QList<KisPaintingAssistantHandleSP> getHandles(QList<QPointF> points);
};

#endif /* __TEST_PERSPECTIVE_BASED_ASSISTANT_HELPER_H */
