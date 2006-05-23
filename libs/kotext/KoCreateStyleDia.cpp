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

#include <kvbox.h>
#include <QLabel>
#include <QLineEdit>
#include "KoCreateStyleDia.h"
#include <kmessagebox.h>

KoCreateStyleDia::KoCreateStyleDia( const QStringList & _list, QWidget *parent, const char *name )
    : KDialogBase( parent, name , true, "", Ok|Cancel, Ok, true )
{
    styleList=_list;
    setCaption( i18n("Create New Style") );
    KVBox *page = makeVBoxMainWidget();
    new QLabel(i18n("Please specify a new style name:"), page);
    m_styleName = new QLineEdit( page );
    m_styleName->setMinimumWidth( m_styleName->sizeHint().width() * 3 );

    connect( m_styleName, SIGNAL(textChanged ( const QString & )), this, SLOT(nameChanged( const QString &)));
    m_styleName->setFocus();
    enableButtonOK( false );
}

void KoCreateStyleDia::slotOk()
{
    if ( styleList.findIndex(m_styleName->text() ) != -1 )
    {
        KMessageBox::error(this, i18n("Name already exists! Please choose another name"));
        m_styleName->clear();
    }
    else
        KDialogBase::slotOk();
}

QString KoCreateStyleDia::nameOfNewStyle()const
{
    return m_styleName->text();
}

void KoCreateStyleDia::nameChanged( const QString &text)
{
    enableButtonOK( !text.isEmpty() );
}
#include "KoCreateStyleDia.moc"
