/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_LINKED_PATTERN_MANAGER_TEST_H
#define __KIS_LINKED_PATTERN_MANAGER_TEST_H

#include <kis_properties_configuration.h>

#include <KoPattern.h>
#include <QBuffer>

class KisLinkedPatternManagerTest : public QObject
{
    Q_OBJECT
public:
    enum SaveDataFlag {
        None = 0x0,
        SaveName = 0x1,
        SaveFileName = 0x2,
        SaveFileNameWithPath = 0x4,
        SaveOldMd5Base64 = 0x8,
        SaveEmbeddedData = 0x10,
    };
    Q_DECLARE_FLAGS(SaveDataFlags, SaveDataFlag)


private Q_SLOTS:
    void testRoundTrip_data();
    void testRoundTrip();

    void init();

    void testLoadingLegacyXML_data();
    void testLoadingLegacyXML();

private:
    KisPropertiesConfigurationSP createXML(SaveDataFlags flags, KoPatternSP pattern);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisLinkedPatternManagerTest::SaveDataFlags)
Q_DECLARE_METATYPE(KisLinkedPatternManagerTest::SaveDataFlags)


#endif /* __KIS_LINKED_PATTERN_MANAGER_TEST_H */
