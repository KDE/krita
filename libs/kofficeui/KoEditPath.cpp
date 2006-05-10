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

#include <kdeversion.h>
#include <klocale.h>
#include <QLayout>
#include <QPushButton>
#include <q3listbox.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include "KoEditPath.h"
#include <keditlistbox.h>
#include <kfiledialog.h>
#include <kurlrequester.h>
#include <q3hbox.h>
#include <klineedit.h>
#include <q3vbox.h>
#include <QCheckBox>
#include <QLabel>

KoEditPathDia::KoEditPathDia( const QString & _path, QWidget *parent, const char *name )
    : KDialogBase( parent, name , true, "", Ok|Cancel, Ok, true )
{
    setCaption( i18n("Edit Path") );
    QWidget *page = new QWidget( this );
    setMainWidget(page);
    Q3GridLayout * grid = new Q3GridLayout(page, 5, 2, KDialog::marginHint(), KDialog::spacingHint());

    urlReq = new KUrlRequester();
    urlReq->fileDialog()->setMode(KFile::Directory | KFile::LocalOnly);

    KEditListBox::CustomEditor tmp(urlReq, urlReq->lineEdit());

    m_listpath =  new KEditListBox( i18n("Expression Path"),
                    tmp,page, "list_editor" , false, KEditListBox::Add|KEditListBox::Remove );

    grid->addWidget(m_listpath, 0, 0, 5, 1);
    m_listpath->setItems(QStringList::split(QString(";"), _path));
    setFocus();
    resize( 500, 300);
}

QString KoEditPathDia::newPath()const
{
    QString tmp;
    QStringList items = m_listpath->items();
    QStringList::iterator it = items.begin();
    QStringList::iterator endIt = items.end();

    for (; it != endIt; ++it)
    {
        if (!tmp.isEmpty())
            tmp +=";";
        tmp += *it;
    }
    return tmp;
}


KoChangePathDia::KoChangePathDia( const QString & _path, QWidget *parent, const char *name )
    : KDialogBase( parent, name , true, "", Ok|Cancel, Ok, true )
{
    setCaption( i18n("Edit Path") );

    KVBox *page =makeVBoxMainWidget();
    new QLabel( i18n("Location:"), page);
    m_urlReq = new KUrlRequester(page);
    m_urlReq->setMinimumWidth( m_urlReq->sizeHint().width() * 3 );

    m_urlReq->lineEdit()->setText( _path );
    m_urlReq->fileDialog()->setMode(KFile::Directory | KFile::LocalOnly);
    m_defaultPath = new QCheckBox( i18n("Default path"), page );
    connect( m_defaultPath, SIGNAL(toggled ( bool )), this, SLOT( slotChangeDefaultValue( bool )));
    slotChangeDefaultValue( _path.isEmpty() );
    m_defaultPath->setChecked( _path.isEmpty() );
}

QString KoChangePathDia::newPath() const
{
    return m_defaultPath->isChecked() ? QString::null : m_urlReq->lineEdit()->text();
}

void KoChangePathDia::slotChangeDefaultValue( bool _b)
{
    m_urlReq->setEnabled( !_b);
}

#include "KoEditPath.moc"
