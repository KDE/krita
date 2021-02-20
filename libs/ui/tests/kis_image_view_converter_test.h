/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_IMAGEVIEWCONVERTER_TEST_H
#define KIS_IMAGEVIEWCONVERTER_TEST_H

#include <QtTest>

class KisImageViewConverterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testDocumentToView();
    void testViewToDocument();
    void testZoom();
};

#endif

