/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DOCUMENT_REPLACE_TEST_H_
#define KIS_DOCUMENT_REPLACE_TEST_H_

#include <simpletest.h>

class KisDocument;
class KisImage;

class KisDocumentReplaceTest : public QObject
{
    Q_OBJECT
    void init();
    void finalize();

private Q_SLOTS:
    void testCopyFromDocument();

private:
    KisDocument *m_doc;
};

#endif // KIS_DOCUMENT_REPLACE_TEST_H_
