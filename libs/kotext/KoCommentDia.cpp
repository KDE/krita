/* This file is part of the KDE project
   Copyright (C)  2002 Montel Laurent <lmontel@mandrakesoft.com>

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

#include <klocale.h>
#include <q3vbox.h>
#include <q3multilineedit.h>
#include "KoCommentDia.h"
#include <QLayout>
#include <QPushButton>
#include <kglobal.h>
#include <QDateTime>
#include <QLabel>
#include <kdebug.h>

KoCommentDia::KoCommentDia( QWidget *parent, const QString &_note, const QString & _authorName, const QString &_createNote, const char *name )
    : KDialogBase( parent, name , true, "", Ok|Cancel, Ok, true )
{
    setCaption( i18n("Edit Comment") );
    authorName = _authorName;
//    KVBox *page = makeVBoxMainWidget();
    kDebug()<<"_createNote :"<<_createNote<<endl;
    if ( !_createNote.isEmpty() )
        new QLabel( _createNote, this );

    m_multiLine = new Q3MultiLineEdit( this );
    m_multiLine->setText( _note );
    m_multiLine->setFocus();
    pbAddAuthorName = new QPushButton(i18n("Add Author Name"),this);
    if ( authorName.isEmpty() )
        pbAddAuthorName->setEnabled( false );
    else
        connect (pbAddAuthorName, SIGNAL(clicked ()), this , SLOT(slotAddAuthorName()));
    connect ( m_multiLine , SIGNAL( textChanged()), this, SLOT( slotTextChanged(  )));
    slotTextChanged( );

    resize( 300,100 );
}

void KoCommentDia::slotTextChanged( )
{
    enableButtonOK( !m_multiLine->text().isEmpty() );
}

QString KoCommentDia::commentText()
{
    return m_multiLine->text();
}

void KoCommentDia::slotAddAuthorName()
{
    QString date = KGlobal::locale()->formatDate( QDate::currentDate() );
    QString time = KGlobal::locale()->formatTime( QTime::currentTime() );
    QString result = QString("--------%1 ,%2, %3------\n").arg(authorName).arg(date).arg(time);
    m_multiLine->insertLine( result, m_multiLine->numLines() );
}

#include "KoCommentDia.moc"
