/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Ducroquet <pinaraf@gmail.com>
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

#ifndef TOCVARIABLE_H
#define TOCVARIABLE_H

#include <KoVariable.h>
#include <QTextDocument>

class KoShapeSavingContext;
class KoParagraphStyle;

class TOCSourceTemplate
{
public:
    int outlineLevel() const { return m_outlineLevel; }
    bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext & context );
private:
    int m_outlineLevel;
    KoParagraphStyle *m_style;
};

/**
 * This class represent the table-of-content-source element in the OpenDocument file.
 * It builds the TOC from the document using the settings specified by the user.
 */
class TOCSource
{
public:
    bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext & context );
    const QList<TOCSourceTemplate> &sources() { return m_sources; }
    int outlineLevel() const { return m_outlineLevel; }
    void buildFromDocument (const QTextDocument *source, QTextCursor *target);
private:
    QString m_titleTemplate;
    KoParagraphStyle *m_titleStyle;
    int m_outlineLevel;
    QList<TOCSourceTemplate> m_sources;
};

/**
 * This is a KoVariable for tables of content.
 */
class TOCVariable : public KoVariable
{
public:

    /**
     * Constructor.
     */
    TOCVariable();

    void setProperties(const KoProperties *props);

    void propertyChanged(Property property, const QVariant &value);

    /// reimplmented
    void saveOdf( KoShapeSavingContext & context );

    ///reimplemented
    bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext & context );

    ///reimplemented
    void update( );
private:
    void variableMoved(const KoShape *shape, const QTextDocument *document, int posInDocument);

    /// reimplemented
    void resize(const QTextDocument *document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);
    /// reimplemented
    void paint (QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
            const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format);

    QTextDocument indexBody;
    const QTextDocument *currentDoc;
    TOCSource source;
};

#endif
