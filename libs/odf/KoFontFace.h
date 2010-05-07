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

#ifndef KOFONTFACE_H
#define KOFONTFACE_H

#include <QtCore/QString>
#include <QtCore/QSharedData>
#include "koodf_export.h"

class KoXmlWriter;
class KoFontFacePrivate;

/**
 * @brief Represents font style.
 * Font style is defined by the style:font-face element.
 * @todo add more parameters.
 * @todo add class KoFontFaceDeclarations instead of adding methods to KoGenStyle?
 */
class KOODF_EXPORT KoFontFace
{
public:
    /**
     * Constructor. Creates font face definition with empty parameters.
     *
     * @param name the font name.
     *
     * The other are empty. If you don't pass the name, the font face will be considered null.
     * @see isEmpty()
     */
    explicit KoFontFace(const QString &name = QString());

    /**
     * Copy constructor.
     */
    KoFontFace(const KoFontFace &other);

    /**
     * Destructor.
     */
    ~KoFontFace();

    /**
     * @return true if the font face object is null, i.e. has no name assigned.
     */
    bool isNull() const;

    KoFontFace& operator=(const KoFontFace &other);

    bool operator==(const KoFontFace &other) const;

    enum Pitch {
        FixedPitch,
        VariablePitch
    };
    //! @todo add enum FamilyGeneric?

    QString name() const;
    void setName(const QString &name);
    QString family() const;
    void setFamily(const QString &family);
    QString familyGeneric() const;
    void setFamilyGeneric(const QString &familyGeneric);
    QString style() const;
    void setStyle(const QString &style);
    KoFontFace::Pitch pitch() const;
    void setPitch(KoFontFace::Pitch pitch);

    /** Saves font face definition into @a xmlWriter as a style:font-face element.
     */
    void saveOdf(KoXmlWriter *xmlWriter) const;

private:
    QSharedDataPointer<KoFontFacePrivate> d;
};

#endif /* KOFONTFACE_H */
