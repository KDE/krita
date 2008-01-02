/* This file is part of the KDE project
 * Copyright (C) 2004-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
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

#ifndef KOTEXTLOADINGCONTEXT_H
#define KOTEXTLOADINGCONTEXT_H

#include "kotext_export.h"

#include <KoOasisLoadingContext.h>

class KoTextLoader;

/**
 * Used during loading of Oasis format (and discarded at the end of the loading).
 *
 * This class extends the \a KoOasisLoadingContext class with KoText specific
 * functionality like for example handling of lists.
 *
 * The context is used within the \a KoTextLoader class to provide additional
 * state informations like the \a KoDocument and the \a KoStore we operate on.
 *
 * In the \a KoTextShapeData::loadOdf() method the \a KoTextLoader is used
 * with a KoTextLoadingContext instance like this;
 *
 * \code
 * KoStyleManager *stylemanager = new KoStyleManager();
 * KoTextLoader *loader = new KoTextLoader(stylemanager);
 * KoDocument* doc = oasisContext.koDocument();
 * KoOdfStylesReader& styles = oasisContext.stylesReader();
 * KoStore *store = oasisContext.store()
 * KoTextLoadingContext *loaderContext = new KoTextLoadingContext(loader, doc, styles, store);
 * QTextCursor cursor( document() );
 * loader->loadBody(*loaderContext, element, cursor); // load the body from the ODF KoXmlElement.
 * \endcode
 */
class KOTEXT_EXPORT KoTextLoadingContext : public KoOasisLoadingContext
{
public:

    /**
    * Constructor.
    *
    * \param loader The KoTextLoader instance the context belongs to.
    * We need to pass on the \a TextLoader instance here since the loader need to be passed
    * around between the different loadOdf(const KoXmlElement&, KoShapeLoadingContext&) calls.
    * For that case the \a TextLoaderContext that inherits the \a KoOasisLoadingContext class
    * got introduced and holds a pointer to the \a KoTextLoader and to the needed
    * \a KoStyleManager instance.
    * \param doc The document we are loading the content into.
    * \param styles The styles used for loading.
    * \param store The storage backend we are reading from.
    */
    explicit KoTextLoadingContext( KoTextLoader* loader, KoDocument* doc, KoOdfStylesReader& stylesReader, KoStore* store );

    /**
    * Destructor.
    */
    virtual ~KoTextLoadingContext();

    /**
    * \return the KoTextLoader instance this context belongs to.
    */
    KoTextLoader* loader() const;

#if 0 //1.6:
    KoVariableCollection& variableCollection() { return m_varColl; }

    ///// List handling

    KoListStyleStack& listStyleStack() { return m_listStyleStack; }
    QString currentListStyleName() const { return m_currentListStyleName; }
    void setCurrentListStyleName( const QString& s ) { m_currentListStyleName = s; }

    /// Used for lists (numbered paragraphs)
    /// @return true on success (a list style was found and pushed)
    bool pushListLevelStyle( const QString& listStyleName, int level );
    /// Used for outline levels
    bool pushOutlineListLevelStyle( int level );

    /// Set cursor position (set by KoTextParag upon finding the processing instruction)
    void setCursorPosition( KoTextParag* cursorTextParagraph,
                            int cursorTextIndex );

    KoTextParag* cursorTextParagraph() const { return m_cursorTextParagraph; }
    int cursorTextIndex() const { return m_cursorTextIndex; }

private:
    /// @return true on success (a list style was found and pushed)
    bool pushListLevelStyle( const QString& listStyleName, const KoXmlElement& fullListStyle, int level );

    KoListStyleStack m_listStyleStack;
    QString m_currentListStyleName;
    KoVariableCollection& m_varColl;

    KoTextParag* m_cursorTextParagraph;
    int m_cursorTextIndex;
#endif

private:
    /// \internal d-pointer class.
    class Private;
    /// \internal d-pointer instance.
    Private* const d;
};

#endif
