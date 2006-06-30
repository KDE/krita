/* This file is part of the KDE project
   Copyright (C) 2005 Laurent Montel <montel@kde.org>
   Copyright (C) 2006 Fredrik Edemar <f_edemar@linux.se>

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


#include <QFile>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QPushButton>
#include <QToolButton>
#include <QTreeWidget>

#include <kbuttonbox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>

#include <q3multilineedit.h>

#include "KoMainWindow.h"

#include "KoVersionDialog.h"


KoVersionDialog::KoVersionDialog( QWidget* parent, KoDocument *doc  )
    : KDialog( parent )
{
  setCaption( i18n("Version") );
  setButtons( Close );
  setDefaultButton( Close );
  m_doc = doc;

  QWidget* page = new QWidget( this );
  setMainWidget( page );
  setModal( true );

  QGridLayout* grid1 = new QGridLayout( page );
  grid1->setMargin(KDialog::marginHint());
  grid1->setSpacing(KDialog::spacingHint());

  list=new QTreeWidget(page);
  list->setColumnCount( 3 );
  QStringList h;
  h.append( i18n("Date & Time") );
  h.append( i18n("Saved By") );
  h.append( i18n("Comment") );
  list->setHeaderLabels( h );

  updateVersionList();

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
  connect( list, SIGNAL( itemActivated( QTreeWidgetItem *, int ) ), this, SLOT( slotOpen() ) );

  updateButton();

  resize( 600, 250 );

}

KoVersionDialog::~KoVersionDialog()
{
}

void KoVersionDialog::updateVersionList()
{
  list->clear();
  // add all versions to the tree widget
  QList<KoVersionInfo> versions = m_doc->versionList();
  QList<QTreeWidgetItem *> items;
  for (int i = 0; i < versions.size(); ++i)
  {
    QStringList l;
    l.append( versions.at(i).date.toString() );
    l.append( versions.at(i).saved_by );
    l.append( versions.at(i).comment );
    items.append( new QTreeWidgetItem( l ) );
  }
  list->insertTopLevelItems(0, items );
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
  KoVersionModifyDialog * dlg = new KoVersionModifyDialog( this, 0 );
  if ( !dlg->exec() )
  {
    delete dlg;
    return;
  }

  if ( !m_doc->addVersion( dlg->comment() ) )
    KMessageBox::error( this, i18n("A new version could not be added") );

  delete dlg;

  updateVersionList();
}

void KoVersionDialog::slotRemove()
{
  if ( !list->currentItem() )
    return;

  for (int i = 0; i < m_doc->versionList().size(); ++i) {
    if ( m_doc->versionList().at(i).date.toString() == list->currentItem()->text(0) )
    {
      m_doc->versionList().takeAt(i);
      delete list->currentItem();
      return;
    }
  }
}

void KoVersionDialog::slotModify()
{
    if ( !list->currentItem() )
        return;

    KoVersionInfo *version = 0;
    for (int i = 0; i < m_doc->versionList().size(); ++i) {
      if ( m_doc->versionList().at(i).date.toString() == list->currentItem()->text(0) )
      {
        version = &m_doc->versionList()[i];
        break;
      }
    }
    if ( !version )
      return;

    KoVersionModifyDialog * dlg = new KoVersionModifyDialog( this, version );
    if ( dlg->exec() )
    {
      version->comment = dlg->comment();
      list->currentItem()->setText(2, version->comment );
    }
    delete dlg;

}

void KoVersionDialog::slotOpen()
{
  if ( !list->currentItem() )
    return;

  KoVersionInfo *version = 0;
  for (int i = 0; i < m_doc->versionList().size(); ++i) {
    if ( m_doc->versionList().at(i).date.toString() == list->currentItem()->text(0) )
    {
      version = &m_doc->versionList()[i];
      break;
    }
  }
  if ( !version )
    return;

    KTempFile tmp;
    QFile *file = tmp.file();
    file->write( version->data );
    file->close();
    tmp.sync();

    if ( !m_doc->shells().isEmpty() ) //open the version in a new window if possible
    {
        KoMainWindow *window = m_doc->shells().current();
        window->openDocument( tmp.name() );
    }
    else
        m_doc->openURL( tmp.name() );

    tmp.unlink();
    slotButtonClicked( Close );
}

KoVersionModifyDialog::KoVersionModifyDialog(  QWidget* parent, KoVersionInfo *info )
    : KDialog( parent  )
{
    setCaption( i18n("Comment") );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
    setModal( true );

    QWidget* page = new QWidget( this );
    setMainWidget( page );

    QVBoxLayout *grid1 = new QVBoxLayout( page );
    grid1->setMargin(KDialog::marginHint());
    grid1->setSpacing(KDialog::spacingHint());

    QLabel *l = new QLabel( page );
    if ( info )
        l->setText( i18n("Date: %1", info->date.toString() ) );
    else
        l->setText( i18n("Date: %1", QDateTime::currentDateTime().toString( Qt::ISODate ) ) );
    grid1->addWidget( l );

    m_multiline = new Q3MultiLineEdit( page );
    if ( info )
        m_multiline->setText( info->comment );
    grid1->addWidget( m_multiline );

}

QString KoVersionModifyDialog::comment() const
{
    return m_multiline->text();
}


#include "KoVersionDialog.moc"
