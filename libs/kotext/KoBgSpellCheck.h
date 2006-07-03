/* This file is part of the KDE project
   Copyright (C) 2004 Zack Rusin <zack@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOBGSPELLCHECK_H
#define KOBGSPELLCHECK_H

#include <sonnet/loader.h>
#include <QObject>
#include <QStringList>
#include <koffice_export.h>
class KoTextObject;
class KoDocument;
class KoTextParag;
class KoTextIterator;

class KOTEXT_EXPORT KoBgSpellCheck : public QObject
{
    Q_OBJECT
public:
    KoBgSpellCheck( const KSpell2::Loader::Ptr& loader, QObject *parent =0,
                    const char *name =0 );
    virtual ~KoBgSpellCheck();

    void registerNewTextObject( KoTextObject *object );

    virtual KoTextIterator *createWholeDocIterator() const=0;

    bool enabled() const;

    /**
     * Returns the Settings object used by the loader.
     */
    KSpell2::Settings *settings() const;

    /**
     * Tell KoBgSpellCheck to avoid spell-checking the word around this position yet,
     * while the user is editing it
     */
    void setIntraWordEditing( KoTextParag* parag, int index );

public slots:
    void start();
    void stop();
    void setEnabled( bool b );

protected slots:
    void spellCheckerMisspelling(const QString &, int );
    void spellCheckerDone();
    void checkerContinue();

    void slotParagraphCreated( KoTextParag* parag );
    void slotParagraphModified( KoTextParag* parag, int /*ParagModifyType*/, int pos, int length );
    void slotParagraphDeleted( KoTextParag* parag );

    void slotClearPara();

protected:
    void markWord( KoTextParag* parag, int pos, int length, bool misspelled );
private:
    class Private;
    Private *d;
};
#endif
