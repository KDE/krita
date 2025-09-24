/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "psd_cos_parser_test.h"
#include "cos/kis_cos_parser.h"
#include "cos/kis_cos_writer.h"

void psd_cos_parser_test::test_roundtrip_cos_struct_data()
{
    QTest::addColumn<QVariantHash>("doc");

    QTest::addRow("empty") << QVariantHash ();
    QTest::addRow("map") << QVariantHash ({{"/Map", QVariantHash ({{"/A", "a"}, {"/B", "b"}})}});
    QTest::addRow("array") << QVariantHash ({{"/List", QVariantList ({1, 2, 3, 4})}});
    QTest::addRow("string") << QVariantHash ({{"/A", "the (quick) \r brown fox"}, {"/B", "jumps over \rthe la\\zy dog."}});
    QTest::addRow("int") << QVariantHash ({{"/A", 2}, {"/B", 12}});
    QTest::addRow("double") << QVariantHash ({{"/A", 0.5}, {"/B", 0.001}});
    QTest::addRow("bool") << QVariantHash ({{"/A", true}, {"/B", false}});
}

void psd_cos_parser_test::test_roundtrip_cos_struct()
{
    QFETCH(QVariantHash, doc);
    KisCosWriter w;
    QByteArray data = w.writeCosFromVariantHash(doc);

    KisCosParser p;
    QVariantHash newDoc = p.parseCosToJson(&data);
    if (doc != newDoc) {
        qDebug() << doc;
        qDebug() << newDoc;
    }
    QVERIFY2(doc == newDoc, "Roundtrip failed");
}

void psd_cos_parser_test::test_parse_cos_struct_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QVariantHash>("reference");

    QTest::addRow("empty") << QByteArray("<<>>") << QVariantHash ();
    QTest::addRow("bool") << QByteArray("<< /Ligatures true /DLigatures false >>")
                                      << QVariantHash ({{"/Ligatures", true}, {"/DLigatures", false}});
    QTest::addRow("int") << QByteArray("<< /FigureStyle 0 /PreHyphen 2 >>")
                                      << QVariantHash ({{"/FigureStyle", 0}, {"/PreHyphen", 2}});
    QTest::addRow("string") << QByteArray("\n\n<<\n\t/CloseDoubleQuote (\xFE\xFF \x1D)\n>>\n")
                         << QVariantHash({{"/CloseDoubleQuote", "”"}});
    QTest::addRow("array of floats") << QByteArray("<< /Zone 36.0 /WordSpacing [ .8 1.0 1.33 ] /After -.5 >>")
                                      << QVariantHash ({{"/Zone", 36.0}, {"/WordSpacing", QVariantList({0.8, 1.0, 1.33})}, {"/After", -0.5}});
}

void psd_cos_parser_test::test_parse_cos_struct()
{
    QFETCH(QByteArray, data);
    QFETCH(QVariantHash, reference);

    KisCosParser p;
    QVariantHash newDoc = p.parseCosToJson(&data);
    if (reference != newDoc) {
        qDebug() << reference;
        qDebug() << newDoc;
    }
    QVERIFY2(reference == newDoc, "Parsing failed");
}

SIMPLE_TEST_MAIN(psd_cos_parser_test)
