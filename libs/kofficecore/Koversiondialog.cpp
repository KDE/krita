/* This file is part of the KDE project
   Copyright (C) 2005 Laurent Montel <montel@kde.org>

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


#include <QLabel>
#include <QLayout>
#include <q3multilineedit.h>
#include <QPushButton>
#include <QToolButton>
#include <QApplication>
#include <QHBoxLayout>
#include <QGridLayout>

#include <kiconloader.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <k3listview.h>
#include <kdebug.h>

#include <q3multilineedit.h>

#include "Koversiondialog.h"


KoVersionDialog::KoVersionDialog( QWidget* parent,  const char* name )
  : KDialog( parent )
{
  setCaption( i18n("Version") );
  setButtons( Ok | Cancel );

  QWidget* page = new QWidget( this );
  setMainWidget( page );

  QGridLayout* grid1 = new QGridLayout( page );
  grid1->setMargin(KDialog::marginHint());
  grid1->setSpacing(KDialog::spacingHint());

  list=new K3ListView(page);
  list->setObjectName(  "versionlist");
  list->addColumn(i18n("Date & Time"));
  list->addColumn(i18n("Saved By"));
  list->addColumn(i18n("Comment"));

  grid1->addWidget(list,0,0,9,1);

  m_pAdd=new QPushButton(i18n("&Add"),page);
  grid1->addWidget(m_pAdd,1,2);

  m_pRemove=new QPushButton(i18n("&Remove"),page);
  grid1->addWidget(m_pRemove,2,2);

  m_pModify=new QPushButton(i18n("&Modify"),page);
  grid1->addWidget(m_pModify,3,2);

  m_pOpen=new QPushButton(i18n("&Open"),page);
  grid1->addWidget(m_pOpen,4,2);


  connect( m_pRemove, SIGNAL( clicked() ), this, SLOT( slotRemove() ) );
  connect( m_pAdd, SIGNAL( clicked() ), this, SLOT( slotAdd() ) );
  connect( m_pOpen, SIGNAL( clicked() ), this, SLOT( slotOpen() ) );
  connect( m_pModify, SIGNAL( clicked() ), this, SLOT( slotModify() ) );

  updateButton();

  resize( 600, 250 );

}

KoVersionDialog::~KoVersionDialog()
{
}

void KoVersionDialog::updateButton()
{
#if 0
    bool state = ( list->currentItem() >= 0 );
    m_pRemove->setEnabled( state );
#endif
}

void KoVersionDialog::slotAdd()
{
    //TODO create entry
}

void KoVersionDialog::slotRemove()
{
    //TODO remove entry
}

void KoVersionDialog::slotModify()
{
    KoVersionModifyDialog * dlg = new KoVersionModifyDialog(  this /*, const QString &_comment*/ /*TODO add*/ );
    if ( dlg->exec() )
    {
        //TODO
        kDebug()<<" comment :"<<dlg->comment()<<endl;
    }
    delete dlg;

}

void KoVersionDialog::slotOpen()
{
    //TODO open file
}

void KoVersionDialog::slotOk()
{
    accept();
}

KoVersionModifyDialog::KoVersionModifyDialog(  QWidget* parent, const QString &/*comment*/, const char* name )
  : KDialog( parent )
{
    setCaption( i18n("Comment") );
    setButtons( Ok | Cancel );

    QWidget* page = new QWidget( this );
    setMainWidget( page );

    QHBoxLayout *grid1 = new QHBoxLayout( page );
    grid1->setMargin(KDialog::marginHint());
    grid1->setSpacing(KDialog::spacingHint());

    m_multiline=new Q3MultiLineEdit(page, "multiline");
    grid1->addWidget( m_multiline );

}

QString KoVersionModifyDialog::comment() const
{
    return m_multiline->text();
}


#include "Koversiondialog.moc"
