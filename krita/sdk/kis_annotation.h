/* 
 * This file is part of the KDE project
 * 
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_ANNOTATION_H_
#define _KIS_ANNOTATION_H_

#include <ksharedptr.h>
#include "kis_shared_ptr_vector.h"

#include <QString>
#include <q3cstring.h>
#include <q3memarray.h>

/**
 * An data extension mechanism for Krita.
 * 
 * An annotation can be of something like a QByteArray or a QString op a more specific
 * datatype that can be attached to an image (or maybe later, if needed, to a layer)
 * and contains data that must be associated with an image for purposes of import/export.
 * 
 * Annotations will be saved to krita images and may be exported in filetypes that support
 * them.
 *
 * Examples of annotations are EXIF data and ICC profiles.
 */
class KisAnnotation : public KShared {


public:

    /**
     * Creates a new annotation object. The annotation object cannot be changed later
     *
     * @param type a non-localized string identifying the type of the annotation
     * @param description a localized string describing the annotation
     * @param data a binary blob containing the annotation data
     */
    KisAnnotation(const QString & type, const QString & description, const QByteArray & data)
        : m_type(type),
          m_description(description),
          m_annotation(data) {};

    /**
     * @return a non-localized string identifiying the type of the annotation
     */
    QString & type() {return m_type;};

    /**
     * @return a localized string describing the type of the annotations
     *         for user interface purposes.
     */
    QString & description() {return m_description;};

    /**
     * @return a binary blob representation of this annotation
     */
    QByteArray & annotation() { return m_annotation;};

private:

    QString m_type;
    QString m_description;
    QByteArray m_annotation;

};

typedef KSharedPtr<KisAnnotation> KisAnnotationSP;
typedef KisSharedPtrVector<KisAnnotation> vKisAnnotationSP;
typedef vKisAnnotationSP::iterator vKisAnnotationSP_it;
typedef vKisAnnotationSP::const_iterator vKisAnnotationSP_cit;

#endif // _KIS_ANNOTATION_H_
