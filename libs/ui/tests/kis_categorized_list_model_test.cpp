/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_categorized_list_model_test.h"

#include <QTest>
#include "testing_categories_mapper.h"
#include "kis_categorized_list_model.h"

#include "modeltest.h"

void KisCategorizedListModelTest::test()
{
    KisCategorizedListModel<QString, QStringConverter> model;
    ModelTest modelTest(&model);
}

QTEST_MAIN(KisCategorizedListModelTest)
