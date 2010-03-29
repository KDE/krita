/* This file is part of the KDE project

   Copyright (C) 2010 Boudewijn Rempt

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
   Boston, MA 02110-1301, USA.
 */
#ifndef KOODFNUMBERDEFINITION_H
#define KOODFNUMBERDEFINITION_H

#include <QString>

#include "KoXmlReader.h"
#include "KoXmlWriter.h"
#include "koodf_export.h"

/**
 * Load and save the ODF numbering scheme according to section 12.
 *
 * The OpenDocument number format consists of three parts:
 * • Prefix – the text that is displayed before the number
 * • Display format specification, for example, A, B, C, or 1, 2, 3
 * • Suffix – the text that is displayed after the number
 */
class KOODF_EXPORT KoOdfNumberDefinition
{
public:
    explicit KoOdfNumberDefinition();
    ~KoOdfNumberDefinition();
    KoOdfNumberDefinition(const KoOdfNumberDefinition &other);
    KoOdfNumberDefinition &operator=(const KoOdfNumberDefinition &other);

    /**
     * load the number definition element
     */
    void loadOdf(const KoXmlElement &element);

    /**
     * save the number definition element
     */
    void saveOdf(KoXmlWriter *writer) const;

    /**
     * create a string representation of the specified number.
     */
    QString formattedNumber(int number) const;

    /**
     * The style:num-prefix and style:num-suffix attributes specify what to display before and
     * after the number.
     *
     * If the prefix and suffix do not contain alphanumeric characters, an [XSLT] format attribute can
     * be created from the OpenDocument attributes by concatenating the values of the style:num-
     * prefix, style:num-format, and style:num-suffix attributes.
     */
    QString prefix() const;
    void setPrefix(const QString &prefix);

    QString suffix() const;
    void setSuffix(const QString &suffix);


    /**
     * The style:num-format attribute specifies the format of the number in the same way as the
     * [XSLT] format attribute. The number styles supported are as follows:
     * • Numeric: 1, 2, 3, ...
     * • Alphabetic: a, b, c, ... or A, B, C, ...
     * • Roman: i, ii, iii, iv, ... or I, II, III, IV,...
     *
     * The value of this attribute can be "1", "a", "A", "i", or "I". For some
     * elements, the attribute value also can be empty. In this case, no number
     * is displayed.
     */
    enum FormatSpecification {
        Numeric,
        AlphabeticLowerCase,
        AlphabeticUpperCase,
        RomanLowerCase,
        RomanUpperCase,
        Empty
    };

    FormatSpecification formatSpecification() const;
    void setFormatSpecification(FormatSpecification formatSpecification);

    /**
     * Letter synchronization
     *
     * If letters are used in alphabetical order for numbering, there are two ways to process overflows
     * within a digit, as follows:
     *  •  A new digit is inserted. Its start value is A, and it is incremented every time an overflow occurs
     * in the following digit. The numbering sequence in this case is something like a,b,c, ..., z, aa,
     * ab, ac, ...,az, ba, ..., and so on.
     *  • A new digit is inserted that always has the same value as the following digit. The numbering
     * sequence in this case is something like a, b, c, ..., z, aa, bb, cc, ..., zz, aaa, ..., and so on. This
     * is called letter synchronization.
     *
     * The style:num-letter-sync specifies whether letter synchronization shall take place.
     */
    bool letterSynchronization() const;
    void setLetterSynchronization(bool letterSynchronization);

private:

    class Private;
    Private * const d;


};

#endif // KOODFNUMBERDEFINITION_H
