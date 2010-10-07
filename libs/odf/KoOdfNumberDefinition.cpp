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
#include "KoOdfNumberDefinition.h"
#include "KoXmlNS.h"

class KoOdfNumberDefinition::Private {
public:
    QString prefix;
    QString suffix;
    KoOdfNumberDefinition::FormatSpecification formatSpecification;
    bool letterSynchronization;
};

KoOdfNumberDefinition::KoOdfNumberDefinition()
    : d(new Private())
{
    d->formatSpecification = Numeric;
    d->letterSynchronization = false;
}

KoOdfNumberDefinition::KoOdfNumberDefinition(const KoOdfNumberDefinition &other)
    : d(new Private())
{
    d->prefix = other.d->prefix;
    d->suffix = other.d->suffix;
    d->formatSpecification = other.d->formatSpecification;
    d->letterSynchronization = other.d->letterSynchronization;
}

KoOdfNumberDefinition &KoOdfNumberDefinition::operator=(const KoOdfNumberDefinition &other)
{
    d->prefix = other.d->prefix;
    d->suffix = other.d->suffix;
    d->formatSpecification = other.d->formatSpecification;
    d->letterSynchronization = other.d->letterSynchronization;

    return *this;
}

KoOdfNumberDefinition::~KoOdfNumberDefinition()
{
    delete d;
}

void KoOdfNumberDefinition::loadOdf(const KoXmlElement &element)
{
    const QString format = element.attributeNS(KoXmlNS::style, "num-format", QString());
    if (format.isEmpty()) {
        d->formatSpecification = Empty;
    } else {
        if (format[0] == '1') {
            d->formatSpecification = Numeric;
        }
        else if (format[0] == 'a') {
            d->formatSpecification = AlphabeticLowerCase;
        }
        else if (format[0] == 'A') {
            d->formatSpecification = AlphabeticUpperCase;
        }
        else if (format[0] == 'i') {
            d->formatSpecification = RomanLowerCase;
        }
        else if (format[0] == 'I') {
            d->formatSpecification = RomanUpperCase;
        }
    }

    //The style:num-prefix and style:num-suffix attributes specify what to display before and after the number.
    d->prefix = element.attributeNS(KoXmlNS::style, "num-prefix", QString::null);
    d->suffix = element.attributeNS(KoXmlNS::style, "num-suffix", QString::null);

    d->letterSynchronization = (element.attributeNS(KoXmlNS::style, "num-letter-sync", "false") == "true");
}

void KoOdfNumberDefinition::saveOdf(KoXmlWriter *writer) const
{
    if (!d->prefix.isNull()) {
        writer->addAttribute("style:num-prefix", d->prefix);
    }

    if (!d->suffix.isNull()) {
        writer->addAttribute("style:num-suffix", d->suffix);
    }
    QChar format;
    switch(d->formatSpecification) {
    case Numeric:
        format = '1';
        break;
    case AlphabeticLowerCase:
        format = 'a';
        break;
    case AlphabeticUpperCase:
        format = 'A';
        break;
    case RomanLowerCase:
        format = 'i';
        break;
    case RomanUpperCase:
        format = 'I';
        break;
    case Empty:
    default:
        ;
    };
    if (!format.isNull()) {
        writer->addAttribute("style:num-format", format);
    }

    if (d->letterSynchronization) {
        writer->addAttribute("style:num-letter-sync", "true");
    }
}

QString KoOdfNumberDefinition::formattedNumber(int number) const
{
    Q_UNUSED(number);
#ifdef __GNUC__
#warning Implement generating a formatted number
#endif

    return QString::number(number);
}


QString KoOdfNumberDefinition::prefix() const
{
    return d->prefix;
}

void KoOdfNumberDefinition::setPrefix(const QString &prefix)
{
    d->prefix = prefix;
}

QString KoOdfNumberDefinition::suffix() const
{
    return d->suffix;
}

void KoOdfNumberDefinition::setSuffix(const QString &suffix)
{
    d->suffix = suffix;
}

KoOdfNumberDefinition::FormatSpecification KoOdfNumberDefinition::formatSpecification() const
{
    return d->formatSpecification;
}

void KoOdfNumberDefinition::setFormatSpecification(FormatSpecification formatSpecification)
{
    d->formatSpecification = formatSpecification;
}

bool KoOdfNumberDefinition::letterSynchronization() const
{
    return d->letterSynchronization;
}

void KoOdfNumberDefinition::setLetterSynchronization(bool letterSynchronization)
{
    d->letterSynchronization = letterSynchronization;
}
