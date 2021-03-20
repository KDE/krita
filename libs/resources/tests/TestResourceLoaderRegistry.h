/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef TESTRESOURCELOADERREGISTRY_H
#define TESTRESOURCELOADERREGISTRY_H

#include <QObject>

class TestResourceLoaderRegistry : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRegistry();
private:
};

#endif
