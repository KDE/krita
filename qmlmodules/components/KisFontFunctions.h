/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISFONTFUNCTIONS_H
#define KISFONTFUNCTIONS_H

#include <QObject>
#include <QQmlEngine>
class QQmlEngine;
class QJSEngine;
/**
 * Number of functions related to handling FontFamilies inside QML.
 * For some reason, if you have a collection of static functions,
 * they can only be accessed via QML via an Object.
 */
class KisFontFunctions: public QObject {

    Q_OBJECT
    QML_NAMED_ELEMENT(FontFunctions)

public:

    /// Returns the wwsname when present, otherwise returns the family name;
    Q_INVOKABLE static QString wwsFontFamilyName(QString familyName);

    /// Returns the wws name as a variant which can be invalid when not found.
    Q_INVOKABLE static QVariant wwsFontFamilyNameVariant(QString familyName);

    /// Returns the sample tag for a given locale, calls KoWritingSystemUtils
    Q_INVOKABLE static QString sampleTagForQLocale(const QLocale &locale);

    /// QML can only really use QVariantMaps, so if you have a potential
    /// QVariantHash, you can use this function to convert it to a QVariantMap.
    Q_INVOKABLE static QVariantMap getMapFromQVariant(QVariant var);

};

#endif // KISFONTFUNCTIONS_H
