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

#include "KoOdfNumberStyles.h"

#include "KoGenStyles.h"
#include "KoXmlNS.h"

#include <QBuffer>
#include <QDateTime>
#include <QTime>

#include <KGlobal>
#include <KLocale>
#include <KDebug>

#include <KoXmlReader.h>
#include <KoXmlWriter.h>

#include <math.h>

namespace KoOdfNumberStyles
{

    static bool saveOdfTimeFormat(KoXmlWriter &elementWriter, QString &format, QString &text, bool &antislash);
    static void parseOdfDateKlocale(KoXmlWriter &elementWriter, QString &format, QString &text);
    static bool saveOdfKlocaleTimeFormat(KoXmlWriter &elementWriter, QString &format, QString &text);
    static void parseOdfTimeKlocale(KoXmlWriter &elementWriter, QString &format, QString &text);
    static void addCalligraNumericStyleExtension(KoXmlWriter &elementWriter, const QString &_suffix, const QString &_prefix);

QString format(const QString &value, const NumericStyleFormat &format)
{
    switch (format.type) {
        case Number: {
            bool ok;
            qreal v = value.toDouble(&ok);
            return ok ? formatNumber(v, format.formatStr, format.precision) : value;
        } break;
        case Boolean: {
            return formatBoolean(value, format.formatStr);
        } break;
        case Date: {
            bool ok;
            int v = value.toInt(&ok);
            return ok ? formatDate(v, format.formatStr) : value;
        } break;
        case Time: {
            bool ok;
            qreal v = value.toDouble(&ok);
            return ok ? formatTime(v, format.formatStr) : value;
        } break;
        case Percentage: {
            return formatPercent(value, format.formatStr, format.precision);
        } break;
        case Currency: {
            bool ok;
            qreal v = value.toDouble(&ok);
            return ok ? formatCurrency(v, format.formatStr, format.currencySymbol, format.precision) : value;
        } break;
        case Scientific: {
            bool ok;
            qreal v = value.toDouble(&ok);
            return ok ? formatScientific(v, format.formatStr, format.precision) : value;
        } break;
        case Fraction: {
            bool ok;
            qreal v = value.toDouble(&ok);
            return ok ? formatFraction(v, format.formatStr) : value;
        } break;
        case Text: {
            return value;
        } break;
    }
    return value;
}

QString formatNumber(qreal value, const QString &format, int precision)
{
    QString result;
    int start = 0;
    bool showNegative = format.startsWith('-');
    if (showNegative)
        start = 1;
    for (int i = start; i < format.length(); ++i) {
        QChar c = format[ i ];
        switch (c.unicode()) {
            case '.':
            case ',':
            case '#':
            case '0':
            case '?': {
//                bool grouping = false;
                bool gotDot = false;
                bool gotE = false;
                bool gotFraction = false;
                int decimalPlaces = 0;
                int integerDigits = 0;
                int optionalDecimalPlaces = 0;
                int optionalIntegerDigits = 0;
                int exponentDigits = 0;
                int numeratorDigits = 0;
                int denominatorDigits = 0;
                char ch = format[ i ].toLatin1();
                do {
                    if (ch == '.') {
                        gotDot = true;
                    } else if (ch == ',') {
                //        grouping = true; 
                    } else if (ch == 'E' || ch == 'e') {
                        //SET_TYPE_OR_RETURN(KoGenStyle::NumericScientificStyle);

                        if (i >= format.length() - 1) break;
                        const char chN = format[ i + 1 ].toLatin1();
                        if (chN == '-' || chN == '+') {
                            gotE = true;
                            ++i;
                        }
                    } else if (ch == '0' && gotE) {
                        ++exponentDigits;
                    } else if (ch == '0' && !gotDot && !gotFraction) {
                        ++integerDigits;
                    } else if (ch == '#' && !gotDot && !gotFraction) {
                        ++optionalIntegerDigits;
                    } else if (ch == '0' && gotDot && !gotFraction) {
                        ++decimalPlaces;
                    } else if (ch == '#' && gotDot && !gotFraction) {
                        ++optionalDecimalPlaces;
                    } else if (ch == '?' && !gotFraction) {
                        ++numeratorDigits;
                    } else if (ch == '?' && gotFraction) {
                        ++denominatorDigits;
                    } else if (ch == '/') {
                        //SET_TYPE_OR_RETURN(KoGenStyle::NumericFractionStyle);
                        if (gotDot) return QString(); // invalid
                        gotFraction = true;
                    }

                    if (i >= format.length() - 1) break;
                    ch = format[ ++i ].toLatin1();

                    if (ch == ' ') {
                        // spaces are not allowed - but there's an exception: if this is a fraction. Let's check for '?' or '/'
                        const char c = format[ i + 1 ].toLatin1();
                        if (c == '?' || c == '/')
                            ch = format[ ++i ].toLatin1();
                    }
                } while (i < format.length() && (ch == '.' || ch == ',' || ch == '#' || ch == '0' || ch == 'E' || ch == 'e' || ch == '?' || ch == '/'));
                if (!(ch == '.' || ch == ',' || ch == '#' || ch == '0' || ch == 'E' || ch == 'e' || ch == '?' || ch == '/')) {
                    --i;
                }

                QString v(QString::number(qAbs(value), 'f', precision >= 0 ? precision : (optionalDecimalPlaces + decimalPlaces)));
                int p = v.indexOf('.');
                QString integerValue = p >= 0 ? v.left(p) : v;
                if (integerValue.length() < integerDigits)
                    integerValue.prepend(QString().fill('0', integerDigits - integerValue.length()));
                QString decimalValue =  p >= 0 ? v.mid(p + 1) : QString();
                if (decimalValue.length() < decimalPlaces)
                    decimalValue.append(QString().fill('0', decimalPlaces - decimalValue.length()));

                if (showNegative && value < 0)
                    result.append('-');
                result.append(integerValue);
                if (!decimalValue.isEmpty())
                    result.append('.' + decimalValue);
            } break;
            case '\\': { // backslash escapes the next char
                if (i < format.length() - 1) {
                    result.append(format[ ++i ]);
                }
            } break;
            default:
                result.append(c);
                break;
        }
    }

    return result;
}

QString formatBoolean(const QString &value, const QString &format)
{
    Q_UNUSED(format);
    bool ok = false;
    int v = value.toInt(&ok);
    return ok && v != 0 ? "TRUE" : "FALSE";
}

QString formatDate(int value, const QString &format)
{
    QDateTime dt(QDate(1899, 12, 30)); // reference date
    dt = dt.addDays(value);
    return dt.toString(format);
}

QString formatTime(qreal value, const QString &format)
{
    QTime t(0,0,0);
    t = t.addSecs(qRound(value * 86400.0)); // 24 hours
    return t.toString(format);
}

QString formatCurrency(qreal value, const QString &format, const QString& currencySymbol, int precision)
{
    if (currencySymbol == "CCC") // undocumented hack, see doc attached to comment 6 at bug 282972
        return KGlobal::locale()->formatMoney(value, "USD", precision);
    if (format.isEmpty()) // no format means use locale format
        return KGlobal::locale()->formatMoney(value, currencySymbol.isEmpty() ? KGlobal::locale()->currencySymbol() : currencySymbol, precision);
    return formatNumber(value, format, precision);
}

QString formatScientific(qreal value, const QString &format, int precision)
{
    Q_UNUSED(format);
    QString v(QString::number(value, 'E', precision));
    int pos = v.indexOf('.');
    if (pos != -1) {
        v = v.replace(pos, 1, KGlobal::locale()->decimalSymbol());
    }
    return v;
}

QString formatFraction(qreal value, const QString &format)
{
    QString prefix = value < 0 ? "-" : "";
    value = fabs(value);
    qreal result = value - floor(value);

    if (result == 0) // return w/o fraction part if not necessary
        return prefix + QString::number(value);

    int index = 0;
    int limit = 0;
    if (format.endsWith("/2")) {
        index = 2;
    } else if (format.endsWith("/4")) {
        index = 4;
    } else if (format.endsWith("/8")) {
        index = 8;
    } else if (format.endsWith("/16")) {
        index = 16;
    } else if (format.endsWith("/10")) {
        index = 10;
    } else if (format.endsWith("/100")) {
        index = 100;
    } else if (format.endsWith("/?")) {
        index = 3;
        limit = 9;
    } else if (format.endsWith("/??")) {
        index = 4;
        limit = 99;
    } else if (format.endsWith("/???")) {
        index = 5;
        limit = 999;
    } else { // fallback
        return prefix + QString::number(value);
    }

    // handle halves, quarters, tenths, ...
    if (!format.endsWith("/?") && !format.endsWith("/??") && !format.endsWith("/???")) {
        qreal calc = 0;
        int index1 = 0;
        qreal diff = result;
        for (int i = 1; i <= index; i++) {
            calc = i * 1.0 / index;
            if (fabs(result - calc) < diff) {
                index1 = i;
                diff = fabs(result - calc);
            }
        }
        if (index1 == 0)
            return prefix + QString("%1").arg(floor(value));
        if (index1 == index)
            return prefix + QString("%1").arg(floor(value) + 1);
        if (floor(value) == 0)
            return prefix + QString("%1/%2").arg(index1).arg(index);
        return prefix + QString("%1 %2/%3").arg(floor(value)).arg(index1).arg(index);
    }

    // handle Format::fraction_one_digit, Format::fraction_two_digit and Format::fraction_three_digit style
    qreal target = result;
    qreal numerator = 1;
    qreal denominator = 1;
    qreal bestNumerator = 0;
    qreal bestDenominator = 1;
    qreal bestDist = target;

    // as soon as either numerator or denominator gets above the limit, we're done
    while (numerator <= limit && denominator <= limit) {
        qreal dist = fabs((numerator / denominator) - target);
        if (dist < bestDist) {
            bestDist = dist;
            bestNumerator = numerator;
            bestDenominator = denominator;
        }
        if (numerator / denominator > target) {
            ++denominator;
        } else {
            ++numerator;
        }
    }

    if (bestNumerator == 0)
        return prefix + QString().setNum(floor(value));
    if (bestDenominator == bestNumerator)
        return prefix + QString().setNum(floor(value + 1));
    if (floor(value) == 0)
        return prefix + QString("%1/%2").arg(bestNumerator).arg(bestDenominator);
    return prefix + QString("%1 %2/%3").arg(floor(value)).arg(bestNumerator).arg(bestDenominator);

}

QString formatPercent(const QString &value, const QString &/*format*/, int precision)
{
    if (value.contains('.')) {
        bool ok;
        qreal v = value.toDouble(&ok);
        if (ok)
            return QString::number(v * 100., 'f', precision) + QLatin1String("%");
    }
    return value;
}

// OO spec 2.5.4. p68. Conversion to Qt format: see qdate.html
// OpenCalcImport::loadFormat has similar code, but slower, intermixed with other stuff,
// lacking long-textual forms.
QPair<QString, NumericStyleFormat> loadOdfNumberStyle(const KoXmlElement &parent)
{
    NumericStyleFormat dataStyle;

    const QString localName = parent.localName();
    if (localName == "number-style")
        dataStyle.type = Number;
    else if (localName == "currency-style")
        dataStyle.type = Currency;
    else if (localName == "percentage-style")
        dataStyle.type = Percentage;
    else if (localName == "boolean-style")
        dataStyle.type = Boolean;
    else if (localName == "text-style")
        dataStyle.type = Text;
    else if (localName == "date-style")
        dataStyle.type = Date;
    else if (localName == "time-style")
        dataStyle.type = Time;

    QString format;
    int precision = -1;
    int leadingZ  = 1;

    bool thousandsSep = false;
    //todo negred
    //bool negRed = false;
    bool ok = false;
    int i = 0;
    KoXmlElement e;
    QString prefix;
    QString suffix;
    forEachElement(e, parent) {
        if (e.namespaceURI() != KoXmlNS::number)
            continue;
        QString localName = e.localName();
        const QString numberStyle = e.attributeNS(KoXmlNS::number, "style", QString());
        const bool shortForm = numberStyle == "short" || numberStyle.isEmpty();
        if (localName == "day") {
            format += shortForm ? "d" : "dd";
        } else if (localName == "day-of-week") {
            format += shortForm ? "ddd" : "dddd";
        } else if (localName == "month") {
            if (e.attributeNS(KoXmlNS::number, "possessive-form", QString()) == "true") {
                format += shortForm ? "PPP" : "PPPP";
            }
            // TODO the spec has a strange mention of number:format-source
            else if (e.attributeNS(KoXmlNS::number, "textual", QString()) == "true") {
                bool isExtraShort = false;      // find out if we have to use the extra-short month name (just 1st letter)
                if (e.attributeNS(KoXmlNS::calligra, "number-length", QString()) == "extra-short") {
                    isExtraShort = true;
                }

                if (!isExtraShort) {            // for normal month format (first 3 letters or complete name)
                    format += shortForm ? "MMM" : "MMMM";
                } else {                        // for the extra-short month name use 'X' as a special mark
                    format += "X";
                }
            } else { // month number
                format += shortForm ? "M" : "MM";
            }
        } else if (localName == "year") {
            format += shortForm ? "yy" : "yyyy";
        } else if (localName == "era") {
            //TODO I don't know what is it... (define into oo spec)
        } else if (localName == "week-of-year" || localName == "quarter") {
            // ### not supported in Qt
        } else if (localName == "hours") {
            format += shortForm ? "h" : "hh";
        } else if (localName == "minutes") {
            format += shortForm ? "m" : "mm";
        } else if (localName == "seconds") {
            format += shortForm ? "s" : "ss";
        } else if (localName == "am-pm") {
            format += "ap";
        } else if (localName == "text") {   // literal
            format += e.text();
        } else if (localName == "suffix") {
            suffix = e.text();
            kDebug(30003) << " suffix :" << suffix;
        } else if (localName == "prefix") {
            prefix = e.text();
            kDebug(30003) << " prefix :" << prefix;
        } else if (localName == "currency-symbol") {
            dataStyle.currencySymbol = e.text();
            kDebug(30003) << " currency-symbol:" << dataStyle.currencySymbol;
            format += e.text();
            //TODO
            // number:language="de" number:country="DE">€</number:currency-symbol>
            // Stefan: localization of the symbol?
        } else if (localName == "number") {
            if (e.hasAttributeNS(KoXmlNS::number, "grouping")) {
                thousandsSep = e.attributeNS(KoXmlNS::number, "grouping", QString()).toLower() == "true";
            }
            if (e.hasAttributeNS(KoXmlNS::number, "decimal-places")) {
                int d = e.attributeNS(KoXmlNS::number, "decimal-places", QString()).toInt(&ok);
                if (ok)
                    precision = d;
            }
            if (e.hasAttributeNS(KoXmlNS::number, "min-integer-digits")) {
                int d = e.attributeNS(KoXmlNS::number, "min-integer-digits", QString()).toInt(&ok);
                if (ok)
                    leadingZ = d;
            }
            if (thousandsSep && leadingZ <= 3) {
                format += "#,";
                for (i = leadingZ; i <= 3; ++i)
                    format += '#';
            }
            for (i = 1; i <= leadingZ; ++i) {
                format +=  '0';
                if ((i % 3 == 0) && thousandsSep)
                    format = + ',' ;
            }
            if (precision > -1) {
                format += '.';
                for (i = 0; i < precision; ++i)
                    format += '0';
            }
        } else if (localName == "scientific-number") {
            if (dataStyle.type == Number)
                dataStyle.type = Scientific;
            int exp = 2;

            if (e.hasAttributeNS(KoXmlNS::number, "decimal-places")) {
                int d = e.attributeNS(KoXmlNS::number, "decimal-places", QString()).toInt(&ok);
                if (ok)
                    precision = d;
            }

            if (e.hasAttributeNS(KoXmlNS::number, "min-integer-digits")) {
                int d = e.attributeNS(KoXmlNS::number, "min-integer-digits", QString()).toInt(&ok);
                if (ok)
                    leadingZ = d;
            }

            if (e.hasAttributeNS(KoXmlNS::number, "min-exponent-digits")) {
                int d = e.attributeNS(KoXmlNS::number, "min-exponent-digits", QString()).toInt(&ok);
                if (ok)
                    exp = d;
                if (exp <= 0)
                    exp = 1;
            }

            if (e.hasAttributeNS(KoXmlNS::number, "grouping")) {
                thousandsSep = e.attributeNS(KoXmlNS::number, "grouping", QString()).toLower() == "true";
            }

            if (thousandsSep && leadingZ <= 3) {
                format += "#,";
                for (i = leadingZ; i <= 3; ++i)
                    format += '#';
            }

            for (i = 1; i <= leadingZ; ++i) {
                format += '0';
                if ((i % 3 == 0) && thousandsSep)
                    format += ',';
            }

            if (precision > -1) {
                format += '.';
                for (i = 0; i < precision; ++i)
                    format += '0';
            }

            format += "E+";
            for (i = 0; i < exp; ++i)
                format += '0';
        } else if (localName == "fraction") {
            if (dataStyle.type == Number)
                dataStyle.type = Fraction;
            int integer = 0;
            int numerator = 1;
            int denominator = 1;
            int denominatorValue = 0;
            if (e.hasAttributeNS(KoXmlNS::number, "min-integer-digits")) {
                int d = e.attributeNS(KoXmlNS::number, "min-integer-digits", QString()).toInt(&ok);
                if (ok)
                    integer = d;
            }
            if (e.hasAttributeNS(KoXmlNS::number, "min-numerator-digits")) {
                int d = e.attributeNS(KoXmlNS::number, "min-numerator-digits", QString()).toInt(&ok);
                if (ok)
                    numerator = d;
            }
            if (e.hasAttributeNS(KoXmlNS::number, "min-denominator-digits")) {
                int d = e.attributeNS(KoXmlNS::number, "min-denominator-digits", QString()).toInt(&ok);
                if (ok)
                    denominator = d;
            }
            if (e.hasAttributeNS(KoXmlNS::number, "denominator-value")) {
                int d = e.attributeNS(KoXmlNS::number, "denominator-value", QString()).toInt(&ok);
                if (ok)
                    denominatorValue = d;
            }
            if (e.hasAttributeNS(KoXmlNS::number, "grouping")) {
                thousandsSep = e.attributeNS(KoXmlNS::number, "grouping", QString()).toLower() == "true";
            }

            for (i = 0; i < integer; ++i)
                format += '#';

            format += ' ';

            for (i = 0; i < numerator; ++i)
                format += '?';

            format += '/';

            if (denominatorValue != 0)
                format += QString::number(denominatorValue);
            else {
                for (i = 0; i < denominator; ++i)
                    format += '?';
            }
        }

        // stylesmap's are embedded into a style and are pointing to another style that
        // should be used insteat ot this style if the defined condition is true. E.g.;
        // <number:number-style style:name="N139P0" style:volatile="true"/>
        // <number:number-style style:name="N139P1" style:volatile="true"/>
        // <number:number-style style:name="N139P2" style:volatile="true"/>
        // <number:text-style style:name="N139">
        //   <style:map style:condition="value()&gt;0" style:apply-style-name="N139P0"/>
        //   <style:map style:condition="value()&lt;0" style:apply-style-name="N139P1"/>
        //   <style:map style:condition="value()=0" style:apply-style-name="N139P2"/>
        // </number:text-style>
        for (KoXmlNode node(e); !node.isNull(); node = node.nextSibling()) {
            KoXmlElement elem = node.toElement();
            if (elem.namespaceURI() == KoXmlNS::style && elem.localName() == "map") {
                QString condition, applyStyleName;
                if (elem.hasAttributeNS(KoXmlNS::style, "condition"))
                    condition = elem.attributeNS(KoXmlNS::style, "condition");
                if (elem.hasAttributeNS(KoXmlNS::style, "apply-style-name"))
                    applyStyleName = elem.attributeNS(KoXmlNS::style, "apply-style-name");
                dataStyle.styleMaps.append( QPair<QString,QString>(condition,applyStyleName) );
            }
        }
    }

    const QString styleName = parent.attributeNS(KoXmlNS::style, "name", QString());

kDebug()<<"99 *****************************************************************************";
//Q_ASSERT(false);
    kDebug(30003) << "data style:" << styleName << " qt format=" << format;
    if (!prefix.isEmpty()) {
        kDebug(30003) << " format.left( prefix.length() ) :" << format.left(prefix.length()) << " prefix :" << prefix;
        if (format.left(prefix.length()) == prefix) {
            format = format.right(format.length() - prefix.length());
        } else
            prefix.clear();
    }
    if (!suffix.isEmpty()) {
        kDebug(30003) << "format.right( suffix.length() ) :" << format.right(suffix.length()) << " suffix :" << suffix;
        if (format.right(suffix.length()) == suffix) {
            format = format.left(format.length() - suffix.length());
        } else
            suffix.clear();
    }

    dataStyle.formatStr = format;
    dataStyle.prefix = prefix;
    dataStyle.suffix = suffix;
    dataStyle.precision = precision;
    dataStyle.thousandsSep = thousandsSep;
    kDebug(30003) << " finish insert format :" << format << " prefix :" << prefix << " suffix :" << suffix;
    return QPair<QString, NumericStyleFormat>(styleName, dataStyle);
}

QString saveOdfNumberStyle(KoGenStyles &mainStyles, const NumericStyleFormat &format)
{
    QString styleName;
    switch (format.type) {
        case KoOdfNumberStyles::Number: {
            styleName = KoOdfNumberStyles::saveOdfNumberStyle(mainStyles, format.formatStr, format.prefix, format.suffix, format.thousandsSep);
        } break;
        case KoOdfNumberStyles::Boolean: {
            styleName = KoOdfNumberStyles::saveOdfBooleanStyle(mainStyles, format.formatStr, format.prefix, format.suffix);
        } break;
        case KoOdfNumberStyles::Date: {
            bool localeFormat = format.formatStr.isEmpty();
            styleName = KoOdfNumberStyles::saveOdfDateStyle(mainStyles, format.formatStr, localeFormat, format.prefix, format.suffix);
        } break;
        case KoOdfNumberStyles::Time: {
            bool localeFormat = format.formatStr.isEmpty();
            styleName = KoOdfNumberStyles::saveOdfTimeStyle(mainStyles, format.formatStr, localeFormat, format.prefix, format.suffix);
        } break;
        case KoOdfNumberStyles::Percentage: {
            styleName = KoOdfNumberStyles::saveOdfPercentageStyle(mainStyles, format.formatStr, format.prefix, format.suffix);
        } break;
        case KoOdfNumberStyles::Currency: {
            styleName = KoOdfNumberStyles::saveOdfCurrencyStyle(mainStyles, format.formatStr, format.currencySymbol, format.prefix, format.suffix);
        } break;
        case KoOdfNumberStyles::Scientific: {
            styleName = KoOdfNumberStyles::saveOdfScientificStyle(mainStyles, format.formatStr, format.prefix, format.suffix);
        } break;
        case KoOdfNumberStyles::Fraction: {
            styleName = KoOdfNumberStyles::saveOdfFractionStyle(mainStyles, format.formatStr, format.prefix, format.suffix);
        } break;
        case KoOdfNumberStyles::Text: {
            styleName = KoOdfNumberStyles::saveOdfTextStyle(mainStyles, format.formatStr, format.prefix, format.suffix);
        } break;
    }
    return styleName;
}

#define addTextNumber( text, elementWriter ) { \
        if ( !text.isEmpty() ) \
        { \
            elementWriter.startElement( "number:text" ); \
            elementWriter.addTextNode( text ); \
            elementWriter.endElement(); \
            text.clear(); \
        } \
    }

void parseOdfTimeKlocale(KoXmlWriter &elementWriter, QString &format, QString &text)
{
    kDebug(30003) << "parseOdfTimeKlocale(KoXmlWriter &elementWriter, QString & format, QString & text ) :" << format;
    do {
        if (!saveOdfKlocaleTimeFormat(elementWriter, format, text)) {
            text += format[0];
            format = format.remove(0, 1);
        }
    } while (format.length() > 0);
    addTextNumber(text, elementWriter);
}

bool saveOdfKlocaleTimeFormat(KoXmlWriter &elementWriter, QString &format, QString &text)
{
    bool changed = false;
    if (format.startsWith("%H")) {   //hh
        //hour in 24h
        addTextNumber(text, elementWriter);

        elementWriter.startElement("number:hours");
        elementWriter.addAttribute("number:style", "long");
        elementWriter.endElement();
        format = format.remove(0, 2);
        changed = true;
    } else if (format.startsWith("%k")) { //h
        addTextNumber(text, elementWriter);

        elementWriter.startElement("number:hours");
        elementWriter.addAttribute("number:style", "short");
        elementWriter.endElement();
        format = format.remove(0, 2);
        changed = true;
    } else if (format.startsWith("%I")) { // ?????
        //TODO hour in 12h
        changed = true;
    } else if (format.startsWith("%l")) {
        //TODO hour in 12h with 1 digit
        changed = true;
    } else if (format.startsWith("%M")) { // mm
        addTextNumber(text, elementWriter);

        elementWriter.startElement("number:minutes");
        elementWriter.addAttribute("number:style", "long");
        elementWriter.endElement();
        format = format.remove(0, 2);
        changed = true;

    } else if (format.startsWith("%S")) {  //ss
        addTextNumber(text, elementWriter);

        elementWriter.startElement("number:seconds");
        elementWriter.addAttribute("number:style", "long");
        elementWriter.endElement();
        format = format.remove(0, 2);
        changed = true;
    } else if (format.startsWith("%p")) {
        //TODO am or pm
        addTextNumber(text, elementWriter);

        elementWriter.startElement("number:am-pm");
        elementWriter.endElement();
        format = format.remove(0, 2);
        changed = true;
    }
    return changed;
}


bool saveOdfTimeFormat(KoXmlWriter &elementWriter, QString &format, QString &text, bool &antislash)
{
    bool changed = false;
    //we can also add time to date.
    if (antislash) {
        text += format[0];
        format = format.remove(0, 1);
        antislash = false;
        changed = true;
    } else if (format.startsWith("hh")) {
        addTextNumber(text, elementWriter);

        elementWriter.startElement("number:hours");
        elementWriter.addAttribute("number:style", "long");
        elementWriter.endElement();
        format = format.remove(0, 2);
        changed = true;
    } else if (format.startsWith('h')) {
        addTextNumber(text, elementWriter);

        elementWriter.startElement("number:hours");
        elementWriter.addAttribute("number:style", "short");
        elementWriter.endElement();
        format = format.remove(0, 1);
        changed = true;
    } else if (format.startsWith("mm")) {
        addTextNumber(text, elementWriter);

        elementWriter.startElement("number:minutes");
        elementWriter.addAttribute("number:style", "long");
        elementWriter.endElement();
        format = format.remove(0, 2);
        changed = true;
    } else if (format.startsWith('m')) {
        addTextNumber(text, elementWriter);

        elementWriter.startElement("number:minutes");
        elementWriter.addAttribute("number:style", "short");
        elementWriter.endElement();
        format = format.remove(0, 1);
        changed = true;
    } else if (format.startsWith("ss")) {
        addTextNumber(text, elementWriter);

        elementWriter.startElement("number:seconds");
        elementWriter.addAttribute("number:style", "long");
        elementWriter.endElement();
        format = format.remove(0, 2);
        changed = true;
    } else if (format.startsWith('s')) {
        addTextNumber(text, elementWriter);

        elementWriter.startElement("number:seconds");
        elementWriter.addAttribute("number:style", "short");
        elementWriter.endElement();
        format = format.remove(0, 1);
        changed = true;
    } else if (format.startsWith("ap")) {
        addTextNumber(text, elementWriter);

        elementWriter.startElement("number:am-pm");
        elementWriter.endElement();
        format = format.remove(0, 2);
        changed = true;
    }
    return changed;
}

QString saveOdfTimeStyle(KoGenStyles &mainStyles, const QString &_format, bool klocaleFormat,
        const QString &_prefix, const QString &_suffix)
{
    Q_UNUSED(_prefix);
    Q_UNUSED(_suffix);
    //kDebug(30003) << "QString KoOdfNumberStyles::saveOdfTimeStyle( KoGenStyles &mainStyles, const QString & _format ) :" << _format;
    QString format(_format);
    KoGenStyle currentStyle(KoGenStyle::NumericTimeStyle);
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    QString text;
    if (klocaleFormat) {
        parseOdfTimeKlocale(elementWriter, format, text);
    } else {
        bool antislash = false;
        do {
            if (!saveOdfTimeFormat(elementWriter, format, text, antislash)) {
                QString elem(format[0]);
                format = format.remove(0, 1);
                if (elem == "\\") {
                    antislash = true;
                } else {
                    text += elem;
                    antislash = false;
                }
            }
        } while (format.length() > 0);
        addTextNumber(text, elementWriter);
    }
    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    currentStyle.addChildElement("number", elementContents);
    return mainStyles.insert(currentStyle, "N");
}

//convert klocale string to good format
void parseOdfDateKlocale(KoXmlWriter &elementWriter, QString &format, QString &text)
{
    kDebug(30003) << format;
    do {
        if (format.startsWith("%Y")) {
            addTextNumber(text, elementWriter);
            elementWriter.startElement("number:year");
            elementWriter.addAttribute("number:style", "long");
            elementWriter.endElement();
            format = format.remove(0, 2);
        } else if (format.startsWith("%y")) {

            addTextNumber(text, elementWriter);

            elementWriter.startElement("number:year");
            elementWriter.addAttribute("number:style", "short");
            elementWriter.endElement();
            format = format.remove(0, 2);
        } else if (format.startsWith("%n")) {
            addTextNumber(text, elementWriter);
            elementWriter.startElement("number:month");
            elementWriter.addAttribute("number:style", "short");
            elementWriter.addAttribute("number:textual", "false");
            elementWriter.endElement();
            format = format.remove(0, 2);
        } else if (format.startsWith("%m")) {
            addTextNumber(text, elementWriter);
            elementWriter.startElement("number:month");
            elementWriter.addAttribute("number:style", "long");
            elementWriter.addAttribute("number:textual", "false");  //not necessary remove it
            elementWriter.endElement();
            format = format.remove(0, 2);
        } else if (format.startsWith("%e")) {
            addTextNumber(text, elementWriter);

            elementWriter.startElement("number:day");
            elementWriter.addAttribute("number:style", "short");
            elementWriter.endElement();
            format = format.remove(0, 2);
        } else if (format.startsWith("%d")) {
            addTextNumber(text, elementWriter);

            elementWriter.startElement("number:day");
            elementWriter.addAttribute("number:style", "long");
            elementWriter.endElement();
            format = format.remove(0, 2);
        } else if (format.startsWith("%b")) {
            addTextNumber(text, elementWriter);
            elementWriter.startElement("number:month");
            elementWriter.addAttribute("number:style", "short");
            elementWriter.addAttribute("number:textual", "true");
            elementWriter.endElement();
            format = format.remove(0, 2);
        } else if (format.startsWith("%B")) {
            addTextNumber(text, elementWriter);
            elementWriter.startElement("number:month");
            elementWriter.addAttribute("number:style", "long");
            elementWriter.addAttribute("number:textual", "true");
            elementWriter.endElement();
            format = format.remove(0, 2);
        } else if (format.startsWith("%a")) {
            addTextNumber(text, elementWriter);
            elementWriter.startElement("number:day-of-week");
            elementWriter.addAttribute("number:style", "short");
            elementWriter.endElement();

            format = format.remove(0, 2);
        } else if (format.startsWith("%A")) {
            addTextNumber(text, elementWriter);
            elementWriter.startElement("number:day-of-week");
            elementWriter.addAttribute("number:style", "long");
            elementWriter.endElement();
            format = format.remove(0, 2);
        } else {
            if (!saveOdfKlocaleTimeFormat(elementWriter, format, text)) {
                text += format[0];
                format = format.remove(0, 1);
            }
        }
    } while (format.length() > 0);
    addTextNumber(text, elementWriter);
}

QString saveOdfDateStyle(KoGenStyles &mainStyles, const QString &_format, bool klocaleFormat,
        const QString &_prefix, const QString &_suffix)
{
    Q_UNUSED(_prefix);
    Q_UNUSED(_suffix);
    //kDebug(30003) << _format;
    QString format(_format);

    // Not supported into Qt: "era" "week-of-year" "quarter"

    KoGenStyle currentStyle(KoGenStyle::NumericDateStyle);
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    QString text;
    if (klocaleFormat) {
        parseOdfDateKlocale(elementWriter, format, text);
    } else {
        bool antislash = false;
        do {
            if (antislash) {
                text += format[0];
                format = format.remove(0, 1);
            }
            //TODO implement loading ! What is it ?
            else if (format.startsWith("MMMMM")) {        // MMMMM is extra-short month name (only 1st character)
                addTextNumber(text, elementWriter);
                elementWriter.startElement("number:month");
                elementWriter.addAttribute("number:textual", "true");
                elementWriter.addAttribute("calligra:number-length", "extra-short");
                elementWriter.endElement();
                format = format.remove(0, 5);
            } else if (format.startsWith("MMMM")) {
                addTextNumber(text, elementWriter);
                elementWriter.startElement("number:month");
                elementWriter.addAttribute("number:style", "long");
                elementWriter.addAttribute("number:textual", "true");
                elementWriter.endElement();
                format = format.remove(0, 4);
            } else if (format.startsWith("MMM")) {
                addTextNumber(text, elementWriter);
                elementWriter.startElement("number:month");
                elementWriter.addAttribute("number:style", "short");
                elementWriter.addAttribute("number:textual", "true");
                elementWriter.endElement();
                format = format.remove(0, 3);
            } else if (format.startsWith("MM")) {
                addTextNumber(text, elementWriter);
                elementWriter.startElement("number:month");
                elementWriter.addAttribute("number:style", "long");
                elementWriter.addAttribute("number:textual", "false");  //not necessary remove it
                elementWriter.endElement();
                format = format.remove(0, 2);
            } else if (format.startsWith('M')) {
                addTextNumber(text, elementWriter);
                elementWriter.startElement("number:month");
                elementWriter.addAttribute("number:style", "short");
                elementWriter.addAttribute("number:textual", "false");
                elementWriter.endElement();
                format = format.remove(0, 1);
            } else if (format.startsWith("PPPP")) {
                addTextNumber(text, elementWriter);
                //<number:month number:possessive-form="true" number:textual="true" number:style="long"/>
                elementWriter.startElement("number:month");
                elementWriter.addAttribute("number:style", "short");
                elementWriter.addAttribute("number:textual", "false");
                elementWriter.addAttribute("number:possessive-form", "true");
                elementWriter.endElement();
                format = format.remove(0, 4);
            } else if (format.startsWith("PPP")) {
                addTextNumber(text, elementWriter);
                //<number:month number:possessive-form="true" number:textual="true" number:style="short"/>
                elementWriter.startElement("number:month");
                elementWriter.addAttribute("number:possessive-form", "true");

                elementWriter.addAttribute("number:style", "short");
                elementWriter.addAttribute("number:textual", "false");
                elementWriter.endElement();
                format = format.remove(0, 3);
            } else if (format.startsWith("dddd")) {
                addTextNumber(text, elementWriter);

                elementWriter.startElement("number:day-of-week");
                elementWriter.addAttribute("number:style", "long");
                elementWriter.endElement();
                format = format.remove(0, 4);
            } else if (format.startsWith("ddd")) {
                addTextNumber(text, elementWriter);

                elementWriter.startElement("number:day-of-week");
                elementWriter.addAttribute("number:style", "short");
                elementWriter.endElement();
                format = format.remove(0, 3);
            } else if (format.startsWith("dd")) {
                addTextNumber(text, elementWriter);

                elementWriter.startElement("number:day");
                elementWriter.addAttribute("number:style", "long");
                elementWriter.endElement();
                format = format.remove(0, 2);
            } else if (format.startsWith('d')) {
                addTextNumber(text, elementWriter);

                elementWriter.startElement("number:day");
                elementWriter.addAttribute("number:style", "short");
                elementWriter.endElement();
                format = format.remove(0, 1);
            } else if (format.startsWith("yyyy")) {
                addTextNumber(text, elementWriter);

                elementWriter.startElement("number:year");
                elementWriter.addAttribute("number:style", "long");
                elementWriter.endElement();
                format = format.remove(0, 4);
            } else if (format.startsWith("yy")) {
                addTextNumber(text, elementWriter);

                elementWriter.startElement("number:year");
                elementWriter.addAttribute("number:style", "short");
                elementWriter.endElement();
                format = format.remove(0, 2);
            } else {
                if (!saveOdfTimeFormat(elementWriter, format, text, antislash)) {
                    QString elem(format[0]);
                    format = format.remove(0, 1);
                    if (elem == "\\") {
                        antislash = true;
                    } else {
                        text += elem;
                        antislash = false;
                    }
                }
            }
        } while (format.length() > 0);
        addTextNumber(text, elementWriter);
    }

    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    currentStyle.addChildElement("number", elementContents);
    return mainStyles.insert(currentStyle, "N");
}


QString saveOdfFractionStyle(KoGenStyles &mainStyles, const QString &_format,
        const QString &_prefix, const QString &_suffix)
{
    //kDebug(30003) << "QString saveOdfFractionStyle( KoGenStyles &mainStyles, const QString & _format ) :" << _format;
    QString format(_format);

    KoGenStyle currentStyle(KoGenStyle::NumericFractionStyle);
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    QString text;
    int integer = 0;
    int numerator = 0;
    int denominator = 0;
    int denominatorValue = 0;
    bool beforeSlash = true;
    do {
        if (format[0] == '#')
            integer++;
        else if (format[0] == '/')
            beforeSlash = false;
        else if (format[0] == '?') {
            if (beforeSlash)
                numerator++;
            else
                denominator++;
        } else {
            bool ok;
            int value = format.toInt(&ok);
            if (ok) {
                denominatorValue = value;
                break;
            }
        }
        format.remove(0, 1);
    } while (format.length() > 0);

    text = _prefix;
    addTextNumber(text, elementWriter);

    elementWriter.startElement("number:fraction");
    elementWriter.addAttribute("number:min-integer-digits", integer);
    elementWriter.addAttribute("number:min-numerator-digits", numerator);
    elementWriter.addAttribute("number:min-denominator-digits", denominator);
    if (denominatorValue != 0)
        elementWriter.addAttribute("number:denominator-value", denominatorValue);
    elementWriter.endElement();

    addCalligraNumericStyleExtension(elementWriter, _suffix, _prefix);

    text = _suffix;
    addTextNumber(text, elementWriter);

    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    currentStyle.addChildElement("number", elementContents);
    return mainStyles.insert(currentStyle, "N");
}


QString saveOdfNumberStyle(KoGenStyles &mainStyles, const QString &_format,
        const QString &_prefix, const QString &_suffix, bool thousandsSep)
{
    //kDebug(30003) << "QString saveOdfNumberStyle( KoGenStyles &mainStyles, const QString & _format ) :" << _format;
    QString format(_format);

    KoGenStyle currentStyle(KoGenStyle::NumericNumberStyle);
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    QString text;
    int decimalplaces = 0;
    int integerdigits = 0;
    bool beforeSeparator = true;
    do {
        if (format[0] == '.' || format[0] == ',')
            beforeSeparator = false;
        else if (format[0] == '0' && beforeSeparator)
            integerdigits++;
        else if (format[0] == '0' && !beforeSeparator)
            decimalplaces++;
        else
            kDebug(30003) << " error format 0";
        format.remove(0, 1);
    } while (format.length() > 0);
    text = _prefix ;
    addTextNumber(text, elementWriter);
    elementWriter.startElement("number:number");
    //kDebug(30003) << " decimalplaces :" << decimalplaces << " integerdigits :" << integerdigits;
    if (!beforeSeparator)
        elementWriter.addAttribute("number:decimal-places", decimalplaces);
    elementWriter.addAttribute("number:min-integer-digits", integerdigits);
    if (thousandsSep)
        elementWriter.addAttribute("number:grouping", true);
    elementWriter.endElement();

    text = _suffix ;
    addTextNumber(text, elementWriter);
    addCalligraNumericStyleExtension(elementWriter, _suffix, _prefix);

    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    currentStyle.addChildElement("number", elementContents);
    return mainStyles.insert(currentStyle, "N");
}

QString saveOdfBooleanStyle(KoGenStyles &mainStyles, const QString &format, const QString &prefix, const QString &suffix)
{
    Q_UNUSED(format);

    KoGenStyle currentStyle(KoGenStyle::NumericBooleanStyle);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    QString text = prefix;
    addTextNumber(text, elementWriter);
    elementWriter.startElement("number:boolean");
    elementWriter.endElement();
    text = suffix;
    addTextNumber(text, elementWriter);

    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    currentStyle.addChildElement("number", elementContents);
    return mainStyles.insert(currentStyle, "N");
}

QString saveOdfPercentageStyle(KoGenStyles &mainStyles, const QString &_format,
        const QString &_prefix, const QString &_suffix)
{
    //<number:percentage-style style:name="N11">
    //<number:number number:decimal-places="2" number:min-integer-digits="1"/>
    //<number:text>%</number:text>
    //</number:percentage-style>

    //kDebug(30003) << "QString saveOdfPercentageStyle( KoGenStyles &mainStyles, const QString & _format ) :" << _format;
    QString format(_format);

    KoGenStyle currentStyle(KoGenStyle::NumericPercentageStyle);
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    QString text;
    int decimalplaces = 0;
    int integerdigits = 0;
    bool beforeSeparator = true;
    do {
        if (format[0] == '.' || format[0] == ',')
            beforeSeparator = false;
        else if (format[0] == '0' && beforeSeparator)
            integerdigits++;
        else if (format[0] == '0' && !beforeSeparator)
            decimalplaces++;
        else
            kDebug(30003) << " error format 0";
        format.remove(0, 1);
    } while (format.length() > 0);
    text = _prefix ;
    addTextNumber(text, elementWriter);
    elementWriter.startElement("number:number");
    if (!beforeSeparator)
        elementWriter.addAttribute("number:decimal-places", decimalplaces);
    elementWriter.addAttribute("number:min-integer-digits", integerdigits);
    elementWriter.endElement();

    addTextNumber(QString("%"), elementWriter);

    text = _suffix ;
    addTextNumber(text, elementWriter);
    addCalligraNumericStyleExtension(elementWriter, _suffix, _prefix);

    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    currentStyle.addChildElement("number", elementContents);
    return mainStyles.insert(currentStyle, "N");

}

QString saveOdfScientificStyle(KoGenStyles &mainStyles, const QString &_format,
        const QString &_prefix, const QString &_suffix, bool thousandsSep)
{
    //<number:number-style style:name="N60">
    //<number:scientific-number number:decimal-places="2" number:min-integer-digits="1" number:min-exponent-digits="3"/>
    //</number:number-style>

    //example 000,000e+0000
    //kDebug(30003) << "QString saveOdfScientificStyle( KoGenStyles &mainStyles, const QString & _format ) :" << _format;
    QString format(_format);

    KoGenStyle currentStyle(KoGenStyle::NumericScientificStyle);
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    int decimalplace = 0;
    int integerdigits = 0;
    int exponentdigits = 0;
    KoXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    QString text;
    bool beforeSeparator = true;
    bool exponential = false;
    bool positive = true;
    do {
        if (!exponential) {
            if (format[0] == '0' && beforeSeparator)
                integerdigits++;
            else if (format[0] == ',' || format[0] == '.')
                beforeSeparator = false;
            else if (format[0] == '0' && !beforeSeparator)
                decimalplace++;
            else if (format[0].toLower() == 'e') {
                format.remove(0, 1);
                if (format[0] == '+')
                    positive = true;
                else if (format[0] == '-')
                    positive = false;
                else
                    kDebug(30003) << "Error into scientific number";
                exponential = true;
            }
        } else {
            if (format[0] == '0' && positive)
                exponentdigits++;
            else if (format[0] == '0' && !positive)
                exponentdigits--;
            else
                kDebug(30003) << " error into scientific number exponential value";
        }
        format.remove(0, 1);
    } while (format.length() > 0);
    text =  _prefix ;
    addTextNumber(text, elementWriter);

    elementWriter.startElement("number:scientific-number");
    //kDebug(30003) << " decimalplace :" << decimalplace << " integerdigits :" << integerdigits << " exponentdigits :" << exponentdigits;
    if (!beforeSeparator)
        elementWriter.addAttribute("number:decimal-places", decimalplace);
    elementWriter.addAttribute("number:min-integer-digits", integerdigits);
    elementWriter.addAttribute("number:min-exponent-digits", exponentdigits);
    if (thousandsSep)
        elementWriter.addAttribute("number:grouping", true);
    elementWriter.endElement();

    text = _suffix;
    addTextNumber(text, elementWriter);
    addCalligraNumericStyleExtension(elementWriter, _suffix, _prefix);

    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    currentStyle.addChildElement("number", elementContents);
    return mainStyles.insert(currentStyle, "N");
}

QString saveOdfCurrencyStyle(KoGenStyles &mainStyles,
        const QString &_format, const QString &symbol,
        const QString &_prefix, const QString &_suffix)
{

    //<number:currency-style style:name="N107P0" style:volatile="true">
    //<number:number number:decimal-places="2" number:min-integer-digits="1" number:grouping="true"/>
    //<number:text> </number:text>
    //<number:currency-symbol>VEB</number:currency-symbol>
    //</number:currency-style>

    //kDebug(30003) << "QString saveOdfCurrencyStyle( KoGenStyles &mainStyles, const QString & _format ) :" << _format;
    QString format(_format);

    KoGenStyle currentStyle(KoGenStyle::NumericCurrencyStyle);
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    QString text;
    int decimalplaces = 0;
    int integerdigits = 0;
    bool beforeSeparator = true;
    do {
        if (format[0] == '.' || format[0] == ',')
            beforeSeparator = false;
        else if (format[0] == '0' && beforeSeparator)
            integerdigits++;
        else if (format[0] == '0' && !beforeSeparator)
            decimalplaces++;
        else
            kDebug(30003) << " error format 0";
        format.remove(0, 1);
    } while (format.length() > 0);

    text =  _prefix ;
    addTextNumber(text, elementWriter);

    elementWriter.startElement("number:number");
    //kDebug(30003) << " decimalplaces :" << decimalplaces << " integerdigits :" << integerdigits;
    if (!beforeSeparator)
        elementWriter.addAttribute("number:decimal-places", decimalplaces);
    elementWriter.addAttribute("number:min-integer-digits", integerdigits);
    elementWriter.endElement();

    text =  _suffix ;
    addTextNumber(text, elementWriter);
    addCalligraNumericStyleExtension(elementWriter, _suffix, _prefix);

    elementWriter.startElement("number:currency-symbol");
    //kDebug(30003) << " currency-symbol:" << symbol;
    elementWriter.addTextNode(symbol.toUtf8());
    elementWriter.endElement();

    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    currentStyle.addChildElement("number", elementContents);
    return mainStyles.insert(currentStyle, "N");
}

QString saveOdfTextStyle(KoGenStyles &mainStyles, const QString &_format, const QString &_prefix, const QString &_suffix)
{
    Q_UNUSED(_format);

    //<number:text-style style:name="N100">
    //<number:text-content/>
    ///</number:text-style>

    //kDebug(30003) << "QString saveOdfTextStyle( KoGenStyles &mainStyles, const QString & _format ) :" << _format;

    KoGenStyle currentStyle(KoGenStyle::NumericTextStyle);
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    QString text =  _prefix ;
    addTextNumber(text, elementWriter);

    elementWriter.startElement("number:text-content");
    elementWriter.endElement();

    text =  _suffix ;
    addTextNumber(text, elementWriter);
    addCalligraNumericStyleExtension(elementWriter, _suffix, _prefix);

    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    currentStyle.addChildElement("number", elementContents);
    return mainStyles.insert(currentStyle, "N");
}

//This is an extension of numeric style. For the moment we used namespace of
//oasis format for specific calligra extension. Change it for the future.
void addCalligraNumericStyleExtension(KoXmlWriter &elementWriter, const QString &_suffix, const QString &_prefix)
{
    if (!_suffix.isEmpty()) {
        elementWriter.startElement("number:suffix");
        elementWriter.addTextNode(_suffix);
        elementWriter.endElement();
    }
    if (!_prefix.isEmpty()) {
        elementWriter.startElement("number:prefix");
        elementWriter.addTextNode(_prefix);
        elementWriter.endElement();
    }
}
}
