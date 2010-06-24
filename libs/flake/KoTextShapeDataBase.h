/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOTEXTSHAPEDATABASE_H
#define KOTEXTSHAPEDATABASE_H

#include "flake_export.h"

#include "KoShapeUserData.h"
#include "KoInsets.h"
#include <QTextDocument>

class KoXmlElement;
class KoShapeLoadingContext;
class KoShapeSavingContext;

class KoTextShapeDataBasePrivate;

/**
 * \internal
 */
class FLAKE_EXPORT KoTextShapeDataBase : public KoShapeUserData
{
    Q_OBJECT
public:
    /// constructor
    KoTextShapeDataBase();
    virtual ~KoTextShapeDataBase();

    /// return the document
    QTextDocument *document() const;

    /**
     * Set the margins that will make the shapes text area smaller.
     * The shape that owns this textShapeData object will layout text in an area
     * confined by the shape size made smaller by the margins set here.
     * @param margins the margins that shrink the text area.
     */
    void setShapeMargins(const KoInsets &margins);
    /**
     * returns the currently set margins for the shape.
     */
    KoInsets shapeMargins() const;

    /** Sets the vertical alignment of all the text inside the shape. */
    void setVerticalAlignment(Qt::Alignment alignment);
    /** Returns the vertical alignment of all the text in the shape */
    Qt::Alignment verticalAlignment() const;

    /** Loads the text starting with the specified element inside the specified context */
    virtual bool loadOdf(const KoXmlElement & element, KoShapeLoadingContext & context) = 0;

    /** Saves the specified range of text using the specified context */
    virtual void saveOdf(KoShapeSavingContext & context, int from = 0, int to = -1) const = 0;

protected:
    /// constructor
    KoTextShapeDataBase(KoTextShapeDataBasePrivate &);

    KoTextShapeDataBasePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(KoTextShapeDataBase)
};

#endif

