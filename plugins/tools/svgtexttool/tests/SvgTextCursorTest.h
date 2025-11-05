/*
 * SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGTEXTCURSORTEST_H
#define SVGTEXTCURSORTEST_H

#include <QTest>

class SvgTextCursorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void test_ltr_data();
    void test_ltr();

    void test_rtl_data();
    void test_rtl();

    void test_ttb_rl_data();
    void test_ttb_rl();

    void test_ttb_lr_data();
    void test_ttb_lr();

    void test_text_insert_command();
    void test_text_remove_command();

    void test_text_remove_dedicated_data();
    void test_text_remove_dedicated();

    void test_set_transforms_on_text_command_data();
    void test_set_transforms_on_text_command();
};

#endif // SVGTEXTCURSORTEST_H
