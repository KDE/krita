/* This file is part of the KDE project
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

#ifndef KOTEXTSHAREDLOADINGDATA_H
#define KOTEXTSHAREDLOADINGDATA_H

#include <KoSharedLoadingData.h>

#include <QList>
#include "styles/KoListLevelProperties.h"

class QString;
class KoParagraphStyle;
class KoCharacterStyle;
class KoListStyle;
class KoShapeLoadingContext;

/**
 * This class is used to cache the loaded styles so that they have to be loaded only once
 * and can be used by all text shapes.
 * When a text shape is loaded it checks if the KoTextSharedLoadingData is already there. 
 * If not it is created.
 */
class KoTextSharedLoadingData : KoSharedLoadingData
{
public:
    KoTextSharedLoadingData();
    virtual ~KoTextSharedLoadingData();

    bool loadOdfStyles( KoShapeLoadingContext & context );

    KoParagraphStyle * paragraphStyle( const QString &name );

    KoCharacterStyle * characterStyle( const QString &name );

    KoListStyle * listStyle(const QString &name);

    KoListLevelProperties outlineLevel( int level, const KoListLevelProperties& defaultprops = KoListLevelProperties() );

    // Get the document styles which should be displayed in the style manager (office:styles)
    QList<KoParagraphStyle *> documentParagraphStyles();
    QList<KoCharacterStyle *> documentCharacterStyles();

private:
    class Private;
    Private * const d;
};

#endif /* KOTEXTSHAREDLOADINGDATA_H */
