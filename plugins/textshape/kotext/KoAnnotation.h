/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
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

#ifndef KOANNOTATION_H
#define KOANNOTATION_H

#include "KoTextRange.h"
#include "kritatext_export.h"

class KoShape;
class KoAnnotationManager;

/**
 * An annotation is a note made by the user regarding a part of the
 * text. The annotation refers to either a position or a range of
 * text. The annotation location will be automatically updated if user
 * alters the text in the document.

 * An annotation is identified by it's name, and all annotations are
 * managed by KoAnnotationManager. An annotation can be retrieved from
 * the annotation manager by using name as identifier.
 *
 * @see KoAnnotationManager
 */
class KRITATEXT_EXPORT KoAnnotation : public KoTextRange
{
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * By default an annotation has the SinglePosition type and an empty name.
     * The name is set when the annotation is inserted into the annotation manager.
     *
     * @param document the text document where this annotation is located
     */
    explicit KoAnnotation(const QTextCursor &);

    virtual ~KoAnnotation();

    /// reimplemented from super
    virtual void saveOdf(KoShapeSavingContext &context, int position, TagType tagType) const;

    /**
     * Set the new name for this annotation
     * @param name the new name of the annotation
     */
    void setName(const QString &name);

    /// @return the name of this annotation
    QString name() const;


    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);

    /**
     * This is called to allow Cut and Paste of annotations. This
     * method gives a correct, unique, name
     */
    static QString createUniqueAnnotationName(const KoAnnotationManager* kam,
                                              const QString &annotationName, bool isEndMarker);

    void setAnnotationShape(KoShape *shape);

    KoShape *annotationShape() const;

private:

    class Private;
    Private *const d;
};

#endif

