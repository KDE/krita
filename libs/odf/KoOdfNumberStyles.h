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
namespace KoOdfNumberStyles
{
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

    KOODF_EXPORT QPair<QString, NumericStyleFormat> loadOdfNumberStyle(const KoXmlElement &parent);

    KOODF_EXPORT QString saveOdfDateStyle(KoGenStyles &mainStyles, const QString &format, bool klocaleFormat, const QString &prefix = QString(), const QString &suffix = QString());
    KOODF_EXPORT QString saveOdfTimeStyle(KoGenStyles &mainStyles, const QString &format, bool klocaleFormat, const QString &prefix = QString(), const QString &suffix = QString());
    KOODF_EXPORT QString saveOdfFractionStyle(KoGenStyles &mainStyles, const QString &format, const QString &prefix = QString(), const QString &suffix = QString());
    KOODF_EXPORT QString saveOdfScientificStyle(KoGenStyles &mainStyles, const QString &format, const QString &prefix = QString(), const QString &suffix = QString());
    KOODF_EXPORT QString saveOdfNumberStyle(KoGenStyles &mainStyles, const QString &format, const QString &prefix = QString(), const QString &suffix = QString());
    KOODF_EXPORT QString saveOdfPercentageStyle(KoGenStyles &mainStyles, const QString &format, const QString &prefix = QString(), const QString &suffix = QString());
    KOODF_EXPORT QString saveOdfCurrencyStyle(KoGenStyles &mainStyles, const QString &format, const QString &symbol, const QString &prefix = QString(), const QString &suffix = QString());
    KOODF_EXPORT QString saveOdfTextStyle(KoGenStyles &mainStyles, const QString &format, const QString &prefix = QString(), const QString &suffix = QString());
}

#endif // KOODFNUMBERSTYLES_H
