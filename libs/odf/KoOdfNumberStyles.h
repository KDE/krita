/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOODFNUMBERSTYLES_H
#define KOODFNUMBERSTYLES_H

#include "koodf_export.h"
#include <KoXmlReader.h>

#include <QtCore/QPair>

class KoGenStyles;
class KoXmlWriter;
class KoGenStyle;

/**
 * Loading and saving of number styles
 */
class KOODF_EXPORT KoOdfNumberStyles
{
public:
    enum Format {
        Number,
        Scientific,
        Fraction,
        Currency,
        Percentage,
        Date,
        Time,
        Boolean,
        Text
    };
    /// Prefix and suffix are always included into formatStr. Having them as separate fields simply
    /// allows to extract them from formatStr, to display them in separate widgets.
    struct NumericStyleFormat {
        QString formatStr;
        QString prefix;
        QString suffix;
        Format type;
        int precision;
        QString currencySymbol;
        QList<QPair<QString,QString> > styleMaps; // conditional formatting, first=condition, second=applyStyleName
    };

    static QPair<QString, NumericStyleFormat> loadOdfNumberStyle(const KoXmlElement& parent);

    static QString saveOdfDateStyle(KoGenStyles &mainStyles, const QString & _format, bool klocaleFormat,
                                    const QString &_prefix = QString() , const QString &_suffix = QString());
    static QString saveOdfTimeStyle(KoGenStyles &mainStyles, const QString & _format, bool klocaleFormat,
                                    const QString &_prefix = QString() , const QString &_suffix = QString());
    static QString saveOdfFractionStyle(KoGenStyles &mainStyles, const QString & _format,
                                        const QString &_prefix = QString() , const QString &_suffix = QString());
    static QString saveOdfScientificStyle(KoGenStyles &mainStyles, const QString & _format,
                                          const QString &_prefix = QString() , const QString &_suffix = QString());
    static QString saveOdfNumberStyle(KoGenStyles &mainStyles, const QString & _format,
                                      const QString &_prefix = QString() , const QString &_suffix = QString());
    static QString saveOdfPercentageStyle(KoGenStyles &mainStyles, const QString & _format,
                                          const QString &_prefix = QString() , const QString &_suffix = QString());
    static QString saveOdfCurrencyStyle(KoGenStyles &mainStyles, const QString & _format, const QString &symbol,
                                        const QString &_prefix = QString() , const QString &_suffix = QString());
    static QString saveOdfTextStyle(KoGenStyles &mainStyles, const QString & _format,
                                    const QString &_prefix = QString() , const QString &_suffix = QString());

private:
    static bool saveOdfTimeFormat(KoXmlWriter &elementWriter, QString & format, QString & text, bool &antislash);
    static void parseOdfDateKlocale(KoXmlWriter &elementWriter, QString & format, QString & text);
    static bool saveOdfKlocaleTimeFormat(KoXmlWriter &elementWriter, QString & format, QString & text);
    static void parseOdfTimeKlocale(KoXmlWriter &elementWriter, QString & format, QString & text);
    static void addKofficeNumericStyleExtension(KoXmlWriter & elementWriter, const QString &_suffix, const QString &_prefix);
};

#endif // KOODFNUMBERSTYLES_H
