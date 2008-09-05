/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#ifndef KOTEXTDOCUMENT_H
#define KOTEXTDOCUMENT_H

#include <QTextDocument>
#include <QUrl>

class KoStyleManager;

/**
 * KoTextDocument provides an easy mechanism to set and access to the
 * meta data of a QTextDocument. The meta data are stored as resources
 * in the QTextDocument using QTextDocument::addResource() and fetched
 * using QTextDocument::resource().
 *
 */
class KOTEXT_EXPORT KoTextDocument
{
public:
    /// Constructor
    KoTextDocument(QTextDocument *document);
    /// Constructor
    KoTextDocument(const QTextDocument *document);

    /// Destructor
    ~KoTextDocument();

    /// Returns the document that was passed in the constructor
    QTextDocument *document() const;

    /// Sets the style manager that defines the named styles in the document
    void setStyleManager(KoStyleManager *styleManager);

    /// Returns the style manager
    KoStyleManager *styleManager() const;

    /**
     * Clears the text in the document. Unlike QTextDocument::clear(), this
     * function does not clear the resources of the QTextDocument.
     */
    void clearText();

    /// Enum (type) used to add resources using QTextDocument::addResource()
    enum ResourceType {
        StyleManager = QTextDocument::UserResource
    };
    static const QUrl StyleManagerURL;

private:
    QTextDocument *m_document;
};

#endif // KOTEXTDOCUMENT_H
