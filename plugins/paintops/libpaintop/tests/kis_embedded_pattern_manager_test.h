/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_EMBEDDED_PATTERN_MANAGER_TEST_H
#define __KIS_EMBEDDED_PATTERN_MANAGER_TEST_H

#include <QtTest>

#include <kis_properties_configuration.h>

#include <KoPattern.h>

class KisEmbeddedPatternManagerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRoundTrip();

    void init();

    void testLoadingNotOnServerValidName();
    void testLoadingNotOnServerEmptyName();
    void testLoadingNotOnServerPathName();

    void testLoadingOnServerValidName();
    void testLoadingOnServerEmptyName();
    void testLoadingOnServerPathName();

    void testLoadingOnServerValidNameMd5();
    void testLoadingOnServerEmptyNameMd5();
    void testLoadingOnServerPathNameMd5();

private:

    enum NameStatus {
        VALID,
        PATH,
        EMPTY
    };
    void checkOneConfig(NameStatus nameStatus, bool hasMd5, QString expectedName, bool isOnServer);
    KisPropertiesConfigurationSP createXML(NameStatus nameStatus, bool hasMd5);
    KoPatternSP createPattern();
};

#endif /* __KIS_EMBEDDED_PATTERN_MANAGER_TEST_H */
