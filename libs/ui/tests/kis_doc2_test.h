/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DOC2_TEST_H
#define KIS_DOC2_TEST_H

#include <simpletest.h>

class KisDocumentTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void testOpenImageTwiceInSameDoc();
};

#endif /* KIS_DOC2_TEST_H */

