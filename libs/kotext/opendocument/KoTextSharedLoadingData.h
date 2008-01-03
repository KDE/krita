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
#include "kotext_export.h"

class QString;
class KoOasisLoadingContext;
class KoParagraphStyle;
class KoCharacterStyle;
class KoListStyle;
class KoStyleManager;

/**
 * This class is used to cache the loaded styles so that they have to be loaded only once
 * and can be used by all text shapes.
 * When a text shape is loaded it checks if the KoTextSharedLoadingData is already there. 
 * If not it is created.
 */
class KOTEXT_EXPORT KoTextSharedLoadingData : public KoSharedLoadingData
{
public:
    KoTextSharedLoadingData();
    virtual ~KoTextSharedLoadingData();

    /**
     * Load the styles
     *
     * If your application uses a style manager call this function from you application with insertOfficeStyles = true 
     * to load the custom styles into the style manager before the rest of the loading is started.
     *
     * @param context The shape loading context.
     * @param styleManager The style manager too use or 0 if you don't have a style manager.
     * @param insertOfficeStyles If true the office:styles are inserted into the style manager.
     */
    void loadOdfStyles( KoOasisLoadingContext & context, KoStyleManager * styleManager, bool insertOfficeStyles = false );

    /**
     * Get the paragraph style for the given name
     *
     * The name is the style:name given in the file
     *
     * @param name The name of the style to get
     * @return The paragraph style for the given name or 0 if not found
     */
    KoParagraphStyle * paragraphStyle( const QString &name );

    /**
     * Get the character style for the given name
     *
     * The name is the style:name given in the file
     *
     * @param name The name of the style to get
     * @return The character style for the given name or 0 if not found
     */
    KoCharacterStyle * characterStyle( const QString &name );

    KoListStyle * listStyle( const QString &name );

    KoListLevelProperties outlineLevel( int level, const KoListLevelProperties& defaultprops = KoListLevelProperties() );

private:
    // helper functions for loading of paragraph styles
    void addParagraphStyles( KoOasisLoadingContext & context, QList<KoXmlElement*> styleElements,
                             KoStyleManager *styleManager, bool insertOfficeStyles = false );
    QList<QPair<QString, KoParagraphStyle *> > loadParagraphStyles( KoOasisLoadingContext & context, QList<KoXmlElement*> styleElements );

    // helper functions for loading of character styles
    void addCharacterStyles( KoOasisLoadingContext & context, QList<KoXmlElement*> styleElements,
                             KoStyleManager *styleManager, bool insertOfficeStyles = false );
    QList<QPair<QString, KoCharacterStyle *> > loadCharacterStyles( KoOasisLoadingContext & context, QList<KoXmlElement*> styleElements );

    // helper functions for loading of list styles
    void addListStyles( KoOasisLoadingContext & context );
    void addOutlineStyles( KoOasisLoadingContext & context );

    class Private;
    Private * const d;
};

#endif /* KOTEXTSHAREDLOADINGDATA_H */
