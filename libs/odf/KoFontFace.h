/* This file is part of the KDE project
   Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).

   Contact: Suresh Chande suresh.chande@nokia.com

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOFONTFACE_H
#define KOFONTFACE_H

#include <QString>
#include <QSharedData>
#include "kritaodf_export.h"

class KoXmlWriter;
class KoFontFacePrivate;

/**
 * @brief Represents font style.
 * Font style is defined by the style:font-face element.
 */
class KRITAODF_EXPORT KoFontFace
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

private:
    QSharedDataPointer<KoFontFacePrivate> d;
};

#endif /* KOFONTFACE_H */
