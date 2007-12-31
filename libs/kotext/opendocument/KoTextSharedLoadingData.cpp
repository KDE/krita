/* This file is part of the KDE project
 * Copyright (C) 2004-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoTextSharedLoadingData.h"

#include <QString>
#include <QHash>
#include <KoShapeLoadingContext.h>
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoListLevelProperties.h"

class KoTextSharedLoadingData::Private
{
public:
    QHash<QString, KoParagraphStyle *> paragraphStyles;
    QHash<QString, KoCharacterStyle *> characterStyles;

    QHash<QString, KoListStyle *> listStyles;
    KoListStyle outlineStyles;

    QList<KoParagraphStyle *> documentParagraphStyles;
    QList<KoCharacterStyle *> documentCharacterStyles;
};

KoTextSharedLoadingData::KoTextSharedLoadingData()
: d( new Private() )
{
}

KoTextSharedLoadingData::~KoTextSharedLoadingData()
{
    delete d;
}

bool KoTextSharedLoadingData::loadOdfStyles( KoShapeLoadingContext & context )
{
}

KoParagraphStyle * KoTextSharedLoadingData::paragraphStyle( const QString &name )
{
}

KoCharacterStyle * KoTextSharedLoadingData::characterStyle( const QString &name )
{
}

KoListStyle * KoTextSharedLoadingData::listStyle(const QString &name)
{
}

KoListLevelProperties KoTextSharedLoadingData::outlineLevel( int level, const KoListLevelProperties& defaultprops )
{
}

QList<KoParagraphStyle *> KoTextSharedLoadingData::documentParagraphStyles()
{
}

QList<KoCharacterStyle *> KoTextSharedLoadingData::documentCharacterStyles()
{
}
