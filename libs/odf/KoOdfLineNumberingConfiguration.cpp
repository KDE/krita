/* This file is part of the KDE project

   Copyright (C) 2010 KO GmbH <boud@kogmbh.com>

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
#include "KoOdfLineNumberingConfiguration.h"
#include <kdebug.h>
#include <KoXmlNS.h>
#include "KoUnit.h"

class KoOdfLineNumberingConfiguration::Private
{
public:
    bool lineNumberingEnabled;
    KoOdfNumberDefinition numberFormat;
    QString textStyle;
    int increment;
    Position position;
    int offset;
    bool countEmptyLines;
    bool countLinesInTextBoxes;
    bool restartNumberingOnEveryPage;
    QString separator;
    int separatorIncrement;
};

KoOdfLineNumberingConfiguration::KoOdfLineNumberingConfiguration()
    : d(new Private())
{
    d->lineNumberingEnabled = false;
    d->increment = 1;
    d->position = Left;
    d->offset = 10;
    d->countEmptyLines = false;
    d->countLinesInTextBoxes = false;
    d->separatorIncrement = 5;
}

KoOdfLineNumberingConfiguration::~KoOdfLineNumberingConfiguration()
{
    delete d;
}

KoOdfLineNumberingConfiguration::KoOdfLineNumberingConfiguration(const KoOdfLineNumberingConfiguration &other)
    : d(new Private())
{
    d->lineNumberingEnabled = other.d->lineNumberingEnabled;
    d->numberFormat = other.d->numberFormat;
    d->textStyle = other.d->textStyle;
    d->increment = other.d->increment;
    d->position = other.d->position;
    d->offset = other.d->offset;
    d->countEmptyLines = other.d->countEmptyLines;
    d->countLinesInTextBoxes = other.d->countLinesInTextBoxes;
    d->restartNumberingOnEveryPage = other.d->restartNumberingOnEveryPage;
    d->separator = other.d->separator;
    d->separatorIncrement = other.d->separatorIncrement;
}

KoOdfLineNumberingConfiguration &KoOdfLineNumberingConfiguration::operator=(const KoOdfLineNumberingConfiguration &other)
{
    d->lineNumberingEnabled = other.d->lineNumberingEnabled;
    d->numberFormat = other.d->numberFormat;
    d->textStyle = other.d->textStyle;
    d->increment = other.d->increment;
    d->position = other.d->position;
    d->offset = other.d->offset;
    d->countEmptyLines = other.d->countEmptyLines;
    d->countLinesInTextBoxes = other.d->countLinesInTextBoxes;
    d->restartNumberingOnEveryPage = other.d->restartNumberingOnEveryPage;
    d->separator = other.d->separator;
    d->separatorIncrement = other.d->separatorIncrement;

    return *this;
}


void KoOdfLineNumberingConfiguration::loadOdf(const KoXmlElement &element)
{

    d->lineNumberingEnabled = element.attributeNS(KoXmlNS::text, "number-lines", "false") == "true";
    d->numberFormat.loadOdf(element);
    d->textStyle = element.attributeNS(KoXmlNS::text, "style-name", QString::null);
    d->increment = KoUnit::parseValue(element.attributeNS(KoXmlNS::text, "increment", "1"));

    QString position = element.attributeNS(KoXmlNS::text, "position", "left");
    if (position == "left") {
        d->position = Left;
    }
    else if (position == "right") {
        d->position = Right;
    }
    else if (position == "inner") {
        d->position = Inner;
    }
    else if (position == "outer") {
        d->position = Outer;
    }

    d->offset = KoUnit::parseValue(element.attributeNS(KoXmlNS::text, "offset", "10"));
    d->countEmptyLines = element.attributeNS(KoXmlNS::text, "count-empty-lines", "false") == "true";
    d->countLinesInTextBoxes = element.attributeNS(KoXmlNS::text, "count-in-text-boxes", "false") == "true";
    d->restartNumberingOnEveryPage = element.attributeNS(KoXmlNS::text, "restart-on-page", "false") == "true";
    //d->separator;
    //d->separatorIncrement;
}

void KoOdfLineNumberingConfiguration::saveOdf(KoXmlWriter *writer) const
{
    Q_UNUSED(writer);
#ifdef __GNUC__
#warning Implement saving of text:notes-configuration
#endif

}

bool KoOdfLineNumberingConfiguration::enabled() const
{
    return d->lineNumberingEnabled;
}

void KoOdfLineNumberingConfiguration::setEnabled(bool enabled)
{
    d->lineNumberingEnabled = enabled;
}

KoOdfNumberDefinition KoOdfLineNumberingConfiguration::numberFormat() const
{
    return d->numberFormat;
}

void KoOdfLineNumberingConfiguration::setNumberFormat(const KoOdfNumberDefinition &numberFormat)
{
    d->numberFormat = numberFormat;
}

QString KoOdfLineNumberingConfiguration::textStyle() const
{
    return d->textStyle;
}

void KoOdfLineNumberingConfiguration::setTextStyle(const QString &textStyle)
{
    d->textStyle = textStyle;
}

int KoOdfLineNumberingConfiguration::increment() const
{
    return d->increment;
}

void KoOdfLineNumberingConfiguration::setIncrement(int increment)
{
    d->increment = increment;
}

KoOdfLineNumberingConfiguration::Position KoOdfLineNumberingConfiguration::position() const
{
    return d->position;
}

void KoOdfLineNumberingConfiguration::setPosition(KoOdfLineNumberingConfiguration::Position position)
{
    d->position = position;
}

int KoOdfLineNumberingConfiguration::offset() const
{
    return d->offset;
}

void KoOdfLineNumberingConfiguration::setOffset(int offset)
{
    d->offset = offset;
}

bool KoOdfLineNumberingConfiguration::countEmptyLines() const
{
    return d->countEmptyLines;
}

void KoOdfLineNumberingConfiguration::setCountEmptyLines(bool countEmptyLines)
{
    d->countEmptyLines = countEmptyLines;
}

bool KoOdfLineNumberingConfiguration::countLinesInTextBoxes() const
{
    return d->countLinesInTextBoxes;
}

void KoOdfLineNumberingConfiguration::setCountLinesInTextBoxes(bool countLinesInTextBoxes)
{
    d->countLinesInTextBoxes = countLinesInTextBoxes;
}

bool KoOdfLineNumberingConfiguration::restartNumberingOnEveryPage() const
{
    return d->restartNumberingOnEveryPage;
}

void KoOdfLineNumberingConfiguration::setRestartNumberingOnEveryPage(bool restartNumberingOnEveryPage)
{
    d->restartNumberingOnEveryPage = restartNumberingOnEveryPage;
}

QString KoOdfLineNumberingConfiguration::separator() const
{
    return d->separator;
}

void KoOdfLineNumberingConfiguration::setSeparator(const QString &separator)
{
    d->separator = separator;
}

int KoOdfLineNumberingConfiguration::separatorIncrement() const
{
    return d->separatorIncrement;
}

void KoOdfLineNumberingConfiguration::setSeparatorIncrement(int separatorIncrement)
{
    d->separatorIncrement = separatorIncrement;
}

