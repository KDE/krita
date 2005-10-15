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

#include <qstring.h>
#include <qcstring.h>
#include <qmemarray.h>

/**
 * An data extension mechanism for Krita.
 * 
 * An annotation can be of something like a QByteArray or a QString op a more specific
 * datatype that can be attached to an image (or maybe later, if needed, to a layer)
 * and contains data that must be associated with an image for purposes of import/export.
 */
class KisAnnotation : public KShared {


public:

    KisAnnotation(const QString & type, const QString & description, const QByteArray & data)
        : m_type(type),
          m_description(description),
          m_annotation(data) {};

    QString & type() {return m_type;};
    QString & description() {return m_description;};
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
