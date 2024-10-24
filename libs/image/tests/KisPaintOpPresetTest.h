/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPAINTOPPRESETTEST_H
#define KISPAINTOPPRESETTEST_H

#include <QObject>

class KisPaintOpPresetTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLoadingEmbeddedResources_data();
    void testLoadingEmbeddedResources();

};

#endif // KISPAINTOPPRESETTEST_H
