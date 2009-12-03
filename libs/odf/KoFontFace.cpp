/* This file is part of the KDE project
   Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).

   Contact: Suresh Chande suresh.chande@nokia.com

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoFontFace.h"
#include <KoXmlWriter.h>
#include <KDebug>

class KoFontFacePrivate : public QSharedData
{
public:
    KoFontFacePrivate(const QString &_name)
    : name(_name), pitch(KoFontFace::VariablePitch)
    {
    }

    ~KoFontFacePrivate()
    {
    }

    void saveOdf(KoXmlWriter* xmlWriter) const
    {
        xmlWriter->startElement("style:font-face");
        xmlWriter->addAttribute("style:name", name);
        xmlWriter->addAttribute("svg:font-family", family.isEmpty() ? name : family);
        if (!familyGeneric.isEmpty())
            xmlWriter->addAttribute("style:font-family-generic", familyGeneric);
        if (!style.isEmpty())
            xmlWriter->addAttribute("svg:font-style", style);
        xmlWriter->addAttribute("style:font-pitch", pitch == KoFontFace::FixedPitch ? "fixed" : "variable");
        xmlWriter->endElement(); // style:font-face
    }

    QString name;            //!< for style:name attribute
    QString family;          //!< for svg:font-family attribute
    QString familyGeneric;   //!< for style:font-family-generic attribute
    QString style;           //!< for svg:font-style attribute
    KoFontFace::Pitch pitch; //!< for style:font-pitch attribute
};


KoFontFace::KoFontFace(const QString &_name)
 : d(new KoFontFacePrivate(_name))
{
}

KoFontFace::KoFontFace(const KoFontFace &other)
 : d(other.d)
{
}

KoFontFace::~KoFontFace()
{
}

KoFontFace &KoFontFace::operator=(const KoFontFace &other)
{
    d = other.d;
    return *this;
}

bool KoFontFace::operator==(const KoFontFace &other) const
{
    if (isNull() && other.isNull())
        return true;
    return d.data() == other.d.data();
}

bool KoFontFace::isNull() const
{
    return d->name.isEmpty();
}

QString KoFontFace::name() const
{
    return d->name;
}

void KoFontFace::setName(const QString &name)
{
    d->name = name;
}

QString KoFontFace::family() const
{
    return d->family;
}

void KoFontFace::setFamily(const QString &family)
{
    d->family = family;
}

QString KoFontFace::familyGeneric() const
{
    return d->familyGeneric;
}

void KoFontFace::setFamilyGeneric(const QString &familyGeneric)
{
    d->familyGeneric = familyGeneric;
}

QString KoFontFace::style() const
{
    return d->style;
}

void KoFontFace::setStyle(const QString &style)
{
    d->style = style;
}

KoFontFace::Pitch KoFontFace::pitch() const
{
    return d->pitch;
}

void KoFontFace::setPitch(KoFontFace::Pitch pitch)
{
    d->pitch = pitch;
}

void KoFontFace::saveOdf(KoXmlWriter* xmlWriter) const
{
    Q_ASSERT(!isNull());
    if (isNull()) {
        kWarning() << "This font face is null and will not be saved: set at least the name";
        return;
    }
    d->saveOdf(xmlWriter);
}
