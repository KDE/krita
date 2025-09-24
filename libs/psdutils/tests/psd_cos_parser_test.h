/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PSD_COS_PARSER_TEST_H
#define PSD_COS_PARSER_TEST_H

#include <simpletest.h>

class psd_cos_parser_test : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void test_roundtrip_cos_struct_data();
    void test_roundtrip_cos_struct();

    void test_parse_cos_struct_data();
    void test_parse_cos_struct();
};

#endif // PSD_COS_PARSER_TEST_H
