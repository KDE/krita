/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisGlobalTest.h"

#include "simpletest.h"

#include "KisFileUtils.h"

#include <QTest>

void KisGlobalTest::testDeduplicateFileName_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("separator");
    QTest::addColumn<QStringList>("existingFiles");
    QTest::addColumn<QString>("expectedResult");

    QTest::newRow("no_conflict") << "foo.tar.gz" << "_" << QStringList() << "foo.tar.gz";

    QTest::newRow("one_conflict") << "foo.tar.gz" << "_" << QStringList("foo.tar.gz") << "foo_0.tar.gz";

    QTest::newRow("multiple_conflicts") << "foo.tar.gz"
                                        << "_"
                                        << QStringList{"foo.tar.gz", "foo_0.tar.gz"}
                                        << "foo_1.tar.gz";

    QTest::newRow("different_separator") << "image.png"
                                         << "-"
                                         << QStringList("image.png")
                                         << "image-0.png";

    QTest::newRow("no_extension") << "README"
                                  << "_"
                                  << QStringList("README")
                                  << "README_0";

    QTest::newRow("merge_existing_separator")
        << "foo_0.tar.gz"
        << "_"
        << QStringList{"foo.tar.gz", "foo_0.tar.gz"}
        << "foo_1.tar.gz";

    QTest::newRow("merge_empty_separator")
        << "foo0.tar.gz"
        << ""
        << QStringList{"foo.tar.gz", "foo0.tar.gz"}
        << "foo1.tar.gz";

    QTest::newRow("dont_merge_a_different_separator")
        << "foo_0.tar.gz"
        << "-"
        << QStringList{"foo0.tar.gz", "foo_0.tar.gz"}
        << "foo_0-0.tar.gz";

    QTest::newRow("dot_as_separator") << "foo.0.tar.gz"
                                      << "."
                                      << QStringList{"foo.tar.gz", "foo.0.tar.gz"}
                                      << "foo.1.tar.gz";

    QTest::newRow("merge_existing_separator_no_suffix")
        << "foo_0"
        << "_"
        << QStringList{"foo", "foo_0"}
        << "foo_1";

    QTest::newRow("merge_empty_separator_no_suffix")
        << "foo0"
        << ""
        << QStringList{"foo", "foo0"}
        << "foo1";

    QTest::newRow("dont_merge_a_different_separator_no_suffix")
        << "foo_0"
        << "-"
        << QStringList{"foo0", "foo_0"}
        << "foo_0-0";

    QTest::newRow("dot_as_separator_no_suffix") << "foo.0"
                                      << "."
                                      << QStringList{"foo", "foo.0"}
                                      << "foo.1";

    QTest::newRow("dot_as_separator_multiple_numbers") << "foo.0.2.tar.gz"
                                      << "."
                                      << QStringList{"foo.tar.gz", "foo.0.2.tar.gz"}
                                      << "foo.1.2.tar.gz";

    QTest::newRow("weird_separator_with_a_dot") << "foo_xx.xx_0.tar.gz"
                                      << "_xx.xx_"
                                      << QStringList{"foo_xx.xx_0.tar.gz"}
                                      << "foo_xx.xx_1.tar.gz";

    /// we look for a separator **only** around the leftmost dot,
    /// all other dots are discarded
    QTest::newRow("name_with_a_dot_separator_with_a_dot") << "foo.bar_xx.xx_0.tar.gz"
                                      << "_xx.xx_"
                                      << QStringList{"foo.bar_xx.xx_0.tar.gz"}
                                      << "foo_xx.xx_0.bar_xx.xx_0.tar.gz";

    /// we look for a separator **only** around the leftmost dot,
    /// all other dots are discarded
    QTest::newRow("name_with_a_dot_separator_without_a_dot") << "foo.bar_xxxx_0.tar.gz"
                                      << "_xxxx_"
                                      << QStringList{"foo.bar_xxxx_0.tar.gz"}
                                      << "foo_xxxx_0.bar_xxxx_0.tar.gz";


}

void KisGlobalTest::testDeduplicateFileName()
{
    QFETCH(QString, fileName);
    QFETCH(QString, separator);
    QFETCH(QStringList, existingFiles);
    QFETCH(QString, expectedResult);

    auto fileAllowedCallback = [&existingFiles](const QString &name) {
        return !existingFiles.contains(name);
    };

    const QString result = KritaUtils::deduplicateFileName(fileName, separator, fileAllowedCallback);

    QCOMPARE(result, expectedResult);
}

SIMPLE_TEST_MAIN(KisGlobalTest);
