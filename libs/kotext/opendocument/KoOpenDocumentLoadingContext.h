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

#ifndef KOOPENDOCUMENTLOADINGCONTEXT_H
#define KOOPENDOCUMENTLOADINGCONTEXT_H

#include "kotext_export.h"

#include <KoOasisLoadingContext.h>

/**
 * Used during loading of Oasis format (and discarded at the end of the loading).
 */
class KOTEXT_EXPORT KoOpenDocumentLoadingContext : public KoOasisLoadingContext
{
public:
    KoOpenDocumentLoadingContext( KoDocument* doc, KoOasisStyles& styles, KoStore* store );
    virtual ~KoOpenDocumentLoadingContext();

#if 0
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
    class Private;
    Private* const d;
};

#endif
