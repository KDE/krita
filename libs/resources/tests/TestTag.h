/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTTAG_H
#define TESTTAG_H

#include <QObject>

class TestTag: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLoadTag();
    void testSaveTag();
private:
    QStringList m_languages;
};

#endif
