/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Jarosław Staniek <staniek@kde.org>

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

#ifndef KOODFEXPORTER_H
#define KOODFEXPORTER_H

#include "KoFilter.h"

class KoXmlWriter;
class KoStore;
class KoGenStyles;

/**
 * @brief Convenience structure encapsulating XML writers used when writing ODF document.
 */
struct KoOdfWriters
{
    /**
    * Creates structure encapsulating XML writers. All members are set initially to 0.
    */
    KoOdfWriters();
    KoXmlWriter *content;
    KoXmlWriter *body;
    KoXmlWriter *meta;
    KoXmlWriter *manifest;
    KoGenStyles *mainStyles;
};

/**
 * @brief The base class for filters exporting to ODF.
 *
 * Derive your filter class from this base class and implement
 * the pure virtual methods. Don't forget to specify the Q_OBJECT
 * macro in your class even if you don't use signals or slots.
 * This is needed as filters are created on the fly.
 * The m_chain member allows access to the @ref KoFilterChain
 * which invokes the filter to query for input/output.
 *
 * @note Take care: The m_chain pointer is invalid while the constructor
 * runs due to the implementation -- @em don't use it in the constructor.
 * After the constructor, when running the @ref convert() method it's
 * guaranteed to be valid, so no need to check against 0.
 *
 * @author Jarosław Staniek <staniek@kde.org>
 */
class KoOdfExporter : public KoFilter
{
    Q_OBJECT
public:
    virtual ~KoOdfExporter();

    virtual KoFilter::ConversionStatus convert( const QByteArray& from, const QByteArray& to );

protected:
    /**
     * This is the constructor your filter has to call, obviously.
     * @param bodyContentElement element name for the content:
     *                           "text" for ODT format, "presentation" for ODP,
     *                           "spreadsheet" for ODS, "drawing" for ODG.
     *                           office:text element will be created within office:body, etc.
     * @param parent parent object.
     */
    KoOdfExporter(const QString& bodyContentElement, QObject* parent = 0);

    /**
     * @return true if @a mime is accepted source mime type.
     * Implement it for your filter.
     */
    virtual bool acceptsSourceMimeType(const QByteArray& mime) const = 0;

    /**
     * @return true if @a mime is accepted destination mime type.
     * Implement it for your filter.
     */
    virtual bool acceptsDestinationMimeType(const QByteArray& mime) const = 0;

    /**
     * This method is called in convert() after creating @a outputStore, @a writers and @a mainStyles.
     * Implement it for your filter with code that fills the ODF structures with converted data.
     */
    virtual KoFilter::ConversionStatus createDocument(KoStore *outputStore,
                                                      KoOdfWriters *writers) = 0;

private:
    class Private;
    Private* d;
};

#endif /* KOODFEXPORTER_H */
