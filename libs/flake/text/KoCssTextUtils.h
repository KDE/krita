/*
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOCSSTEXTUTILS_H
#define KOCSSTEXTUTILS_H

#include <QString>
#include <QLocale>
#include <KoSvgText.h>
/**
 * @brief The KoCssTextUtils class
 *
 * This class keeps a number of utility functions related to CSS Text,
 * in particular CSS-Text-3 and CSS-Text-4.
 */
class KoCssTextUtils
{
public:

    static QString transformTextToUpperCase(const QString &text, const QString langCode) {
        QLocale locale(langCode);
        return locale.toUpper(text);
    };

    static QString transformTextToLowerCase(const QString &text, const QString langCode) {
        QLocale locale(langCode);
        return locale.toLower(text);
    };

    static QString transformTextCapitalize(const QString &text, const QString langCode) {
        QLocale locale(langCode);
        //TODO
        return text;
    };
    static QString transformTextFullWidth(const QString &text) {
        //TODO
        return text;
    };
    static QString transformTextFullSizeKana(const QString &text) {
        //TODO
        return text;
    };

    static QVector<bool> collapseSpaces(QString &text, KoSvgText::TextSpaceCollapse collapseMethod);

    static bool collapseLastSpace(const QChar c, KoSvgText::TextSpaceCollapse collapseMethod);

    static bool characterCanHang(const QChar c, KoSvgText::HangingPunctuations hangType);
};

#endif // KOCSSTEXTUTILS_H
