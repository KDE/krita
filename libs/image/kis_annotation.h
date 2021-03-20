/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * @file kis_annotation.h
 * @brief This file is part of the Krita application in calligra
 * @author Boudewijn Rempt
 * @author comments by hscott
 * @since 1.4 or 2005
 */

#ifndef _KIS_ANNOTATION_H_
#define _KIS_ANNOTATION_H_

#include <kis_shared.h>

#include <QByteArray>
#include <QString>

/**
 * @class KisAnnotation
 * @brief A data extension mechanism for Krita.
 *
 * An annotation can be of something like a QByteArray or a QString or
 * a more specific datatype that can be attached to an image (or maybe
 * later, if needed, to a layer) and contains data that must be
 * associated with an image for purposes of import/export.
 *
 * Annotations will be saved to krita images and may be exported in
 * filetypes that support them.
 *
 * Examples of annotations are EXIF data and ICC profiles.
 */
class KisAnnotation : public KisShared
{

public:

    /**
     * creates a new annotation object. The annotation object cannot
     * be changed later.
     *
     * @param type a non-localized string identifying the type of the
     * annotation. There can only be one annotation of a given type attached
     * to an image.
     * @param description a localized string describing the annotation
     * @param data a binary blob containing the annotation data
     */
    KisAnnotation(const QString & type, const QString & description, const QByteArray & data)
        : m_type(type)
        , m_description(description)
        , m_annotation(data) {}

    virtual ~KisAnnotation() {}

    virtual KisAnnotation* clone() const {
        return new KisAnnotation(*this);
    }

    /**
     * gets a non-localized string identifying the type of the
     * annotation.
     * @return a non-localized string identifiying the type of the
     * annotation
     */
    const QString & type() const {
        return m_type;
    }

    /**
     * gets a localized string describing the type of annotations for
     * used interface purposes.
     * @return a localized string describing the type of the
     * annotations for user interface purposes.
     */
    const QString & description() const {
        return m_description;
    }

    /**
     * gets a binary blob representation of this annotation
     * @return a binary blob representation of this annotation
     */
    const QByteArray & annotation() const {
        return m_annotation;
    }

    void setAnnotation(const QByteArray ba) {
        m_annotation = ba;
    }

    /**
     * @brief displayText: override this to return an interpreted version of the annotation
     */
    virtual QString displayText() const {
        return QString::fromUtf8(m_annotation);
    }

protected:
    KisAnnotation(const KisAnnotation &rhs)
     : KisShared(),
       m_type(rhs.m_type),
       m_description(rhs.m_description),
       m_annotation(rhs.m_annotation)
    {
    }

protected:

    QString m_type;
    QString m_description;
    QByteArray m_annotation;

};

#endif // _KIS_ANNOTATION_H_
