/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>
   Copyright (C) 2004 Zack Rusin <zack@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KOSPELL_H
#define KOSPELL_H

#include <QObject>
#include <QStringList>
#include <QString>


class Loader;
#include <sonnet/backgroundchecker.h>
#include <koffice_export.h>
class KoTextIterator;
class KoTextParag;
class KoTextObject;
class KoTextDocument;
namespace KSpell2
{
    class Settings;
}

/**
 * KOffice spell checking object
 * Used for both spell-checking-with-a-dialog (directly)
 * and background spell-checking (via KoBgSpellCheck).
 *
 * @author Zack Rusin <zack@kde.org>, David Sweet <dsweet@kde.org>
 * @see KSpell2::Loader
 */
class KOTEXT_EXPORT KoSpell : public KSpell2::BackgroundChecker
{
    Q_OBJECT

public:
    KoSpell( const KSpell2::Loader::Ptr& loader, QObject *parent =0,
             const char *name =0 );
    /**
     * The destructor instructs ISpell/ASpell to write out the personal
     *  dictionary and then terminates ISpell/ASpell.
     */
    virtual ~KoSpell();

    /**
     * Returns whether the speller is already checking something.
     */
    bool checking() const;

    /**
     * Spellchecks a buffer of many words in plain text
     * format.
     *
     * The buffer is not modified.  The signal done() will be
     * sent when @ref check() is finished.
     */
    virtual bool check( KoTextIterator *itr, bool dialog = false );
    virtual bool check( KoTextParag *parag );
    virtual bool checkWordInParagraph( KoTextParag *parag, int pos,
                                       QString& word, int& start );

    KoTextParag  *currentParag() const;
    KoTextObject *currentTextObject() const;
    int currentStartIndex() const;

    KoTextDocument *textDocument() const;

    /**
     * Returns the Settings object used by the loader.
     */
    KSpell2::Settings *settings() const;

public slots:
    void slotCurrentParagraphDeleted();

signals:
    /**
     * Emitted after a paragraph has been checked.
     */
    void paragraphChecked( KoTextParag* );

    void aboutToFeedText();

protected:
    virtual QString getMoreText();
    virtual void finishedCurrentFeed();

private:
    class Private;
    Private *d;
};
#endif
