/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTNOTIFIER_H
#define TESTNOTIFIER_H

#include <QObject>
class Document;
class TestNotifier : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testNotifier();
    void documentAdded(Document *image);
private:
    Document *m_document {0};
};

#endif // TESTNOTIFIER_H
