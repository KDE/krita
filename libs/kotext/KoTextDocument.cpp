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

#include <QTextDocument>
#include <QTextCursor>
#include <QUrl>
#include <QVariant>

#include <kdebug.h>

#include "styles/KoStyleManager.h"
#include "KoTextDocument.h"

const QUrl KoTextDocument::StyleManagerURL = QUrl("kotext://stylemanager");

KoTextDocument::KoTextDocument(QTextDocument *document)
: m_document(document)
{
    Q_ASSERT(m_document);
}

KoTextDocument::KoTextDocument(const QTextDocument *document)
: m_document(const_cast<QTextDocument *>(document))
{
    Q_ASSERT(m_document);
}

KoTextDocument::~KoTextDocument()
{
}

QTextDocument *KoTextDocument::document() const
{
    return m_document;
}

void KoTextDocument::setStyleManager(KoStyleManager *sm)
{
    QVariant v;
    v.setValue(sm);
    m_document->addResource(KoTextDocument::StyleManager, StyleManagerURL, v);
}

KoStyleManager *KoTextDocument::styleManager() const
{
    QVariant resource = m_document->resource(KoTextDocument::StyleManager, StyleManagerURL);
    return resource.value<KoStyleManager *>();
}

void KoTextDocument::clearText()
{
    QTextCursor cursor(m_document);
    cursor.select(QTextCursor::Document);
    cursor.removeSelectedText();
}
