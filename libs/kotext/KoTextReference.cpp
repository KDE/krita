/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoTextReference.h"
#include "KoTextLocator.h"
#include "KoInlineTextObjectManager.h"

KoTextReference::KoTextReference( int indexId )
    : KoVariable(),
    m_indexId(indexId)
{
}

void KoTextReference::variableMoved(const KoShape *shape, const QTextDocument *document, int posInDocument) {
    Q_UNUSED(shape);
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
    Q_ASSERT(manager());
    KoTextLocator *locator = dynamic_cast<KoTextLocator*> (manager()->inlineTextObject(m_indexId));
    Q_ASSERT(locator); // if this fails the id was faulty
    setValue(QString::number(locator->pageNumber()));
}

void KoTextReference::setup() {
    variableMoved(0, 0, 0);
}

