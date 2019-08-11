/*
 *  Copyright (c) 2011-2012 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOELEMENTREFERENCE_H
#define KOELEMENTREFERENCE_H

#include <QSharedDataPointer>
#include <QSharedData>
#include <QUuid>

#include "KoXmlReaderForward.h"

#include "kritaodf_export.h"

class KoXmlWriter;

class KoElementReferenceData : public QSharedData
{
public:

    KoElementReferenceData()
    {
        xmlid = QUuid::createUuid().toString();
        xmlid.remove('{');
        xmlid.remove('}');
    }

    KoElementReferenceData(const KoElementReferenceData &other)
        : QSharedData(other)
        , xmlid(other.xmlid)
    {
    }

    ~KoElementReferenceData() {}

    QString xmlid;
};

/**
 * KoElementReference is used to store unique identifiers for elements in an odf document.
 * Element references are saved as xml:id and optionally for compatibility also as draw:id
 * and text:id.
 *
 * You can use element references wherever you would have used a QString to refer to the id
 * of an object.
 *
 * Element references are implicitly shared, so you can and should pass them along by value.
 */
class KRITAODF_EXPORT KoElementReference
{
public:

    enum GenerationOption {
        UUID = 0,
        Counter = 1
    };

    enum SaveOption {
        XmlId = 0x0,
        DrawId = 0x1,
        TextId = 0x2
    };
    Q_DECLARE_FLAGS(SaveOptions, SaveOption)

    KoElementReference();
    explicit KoElementReference(const QString &prefix);
    KoElementReference(const QString &prefix, int counter);
    KoElementReference(const KoElementReference &other);
    KoElementReference &operator=(const KoElementReference &rhs);
    bool operator==(const KoElementReference &other) const;
    bool operator!=(const KoElementReference &other) const;

    /**
     * @return true if the xmlid is valid, i.e., not null
     */
    bool isValid() const;

    /**
     * @brief loadOdf creates a new KoElementReference from the given element. If the element
     *   does not have an xml:id, draw:id or text:id attribute, and invalid element reference
     *   is returned.
     * @param element the element that may contain xml:id, text:id or draw:id. xml:id has
     *    priority.
     * @return a new element reference
     */
    KoElementReference loadOdf(const KoXmlElement &element);

    /**
     * @brief saveOdf saves this element reference into the currently open element in the xml writer.
     * @param writer the writer we save to
     * @param saveOption determines which attributes we save. We always save the xml:id.
     */
    void saveOdf(KoXmlWriter *writer, SaveOption saveOption = XmlId) const;

    /**
     * @brief toString creates a QString from the element reference
     * @return a string that represents the element. Can be used in maps etc.
     */
    QString toString() const;

    /**
     * Invalidate the reference
     */
    void invalidate();


private:

    QSharedDataPointer<KoElementReferenceData> d;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(KoElementReference::SaveOptions)

#endif // KOELEMENTREFERENCE_H
