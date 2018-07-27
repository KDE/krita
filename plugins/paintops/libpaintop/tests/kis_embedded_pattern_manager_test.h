/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_EMBEDDED_PATTERN_MANAGER_TEST_H
#define __KIS_EMBEDDED_PATTERN_MANAGER_TEST_H

#include <QtTest>

#include <kis_properties_configuration.h>

class KoPattern;

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
    KoPattern *createPattern();
};

#endif /* __KIS_EMBEDDED_PATTERN_MANAGER_TEST_H */
