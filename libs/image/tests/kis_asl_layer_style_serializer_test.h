/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASL_LAYER_STYLE_SERIALIZER_TEST_H
#define __KIS_ASL_LAYER_STYLE_SERIALIZER_TEST_H

#include <simpletest.h>

class KisAslLayerStyleSerializerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testReading();
    void testWriting();
    void testWritingGlobalPatterns();
    void testReadMultipleStyles();

    void testWritingGradients();
};

#endif /* __KIS_ASL_LAYER_STYLE_SERIALIZER_TEST_H */
