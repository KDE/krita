/* This file is part of the KDE project
   Copyright (C)  2001 Montel Laurent <lmontel@mandrakesoft.com>

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

#include <kapplication.h>
#include <klocale.h>

#include <QVBoxLayout>
#include <kdebug.h>
#include <QLabel>
#include <QComboBox>
#include <QDesktopWidget>

#include <klineedit.h>
#include <kurlrequester.h>
#include <kseparator.h>
#include <kiconloader.h>
#include "KoInsertLink.h"
#include <kdesktopfile.h>
#include <krecentdocument.h>

using namespace KOfficePrivate;

KoInsertLinkDia::KoInsertLinkDia( QWidget *parent, const char *name, bool displayBookmarkLink )
    : KPageDialog( parent )
{
  setFaceType( KPageDialog::List );
  setButtons( KDialog::Ok|KDialog::Cancel );
  setDefaultButton( KDialog::Ok );
  setCaption(i18n("Insert Link") );
  setObjectName( name );

  bookmarkLink = 0L;

  KVBox *page = new KVBox();
  p1=addPage(page, i18n("Internet") );
  p1->setIcon( BarIcon("html",K3Icon::SizeMedium) );
  internetLink = new  internetLinkPage(page );
  connect(internetLink,SIGNAL(textChanged()),this,SLOT(slotTextChanged (  )));

  page = new KVBox(); 
  p2=addPage(page, i18n("Mail & News") );
  p2->setIcon( BarIcon("mail_generic",K3Icon::SizeMedium) );
  mailLink = new  mailLinkPage(page );
  connect(mailLink,SIGNAL(textChanged()),this,SLOT(slotTextChanged ()));

  page = new KVBox();
  p3=addPage(page, i18n("File"));
  p3->setIcon( BarIcon("filenew",K3Icon::SizeMedium) );
  fileLink = new  fileLinkPage(page );
  connect(fileLink,SIGNAL(textChanged()),this,SLOT(slotTextChanged ()));

  if ( displayBookmarkLink)
  {
      page = new KVBox();
      p4=addPage(page, i18n("Bookmark"));
      p4->setIcon( BarIcon("bookmark",K3Icon::SizeMedium) );
      bookmarkLink = new  bookmarkLinkPage(page );
      connect(bookmarkLink,SIGNAL(textChanged()),this,SLOT(slotTextChanged ()));
  }

  connect( this, SIGNAL( aboutToShowPage(QWidget *) ), this, SLOT( tabChanged(QWidget *) ) );

  slotTextChanged ( );
  resize(400,300);
}

void KoInsertLinkDia::tabChanged(QWidget *)
{
    if ( currentPage() == p1 )
        internetLink->setLinkName( currentText );
    else if ( currentPage() == p2 )
        mailLink->setLinkName( currentText );
    else if ( currentPage() == p3 )
        fileLink->setLinkName( currentText );
    else if ( currentPage() == p4 )
    {
        if ( bookmarkLink)
            bookmarkLink->setLinkName( currentText );
    }
    enableButtonOK( !(linkName().isEmpty()  || hrefName().isEmpty()) );
}

void KoInsertLinkDia::slotTextChanged ( )
{
    enableButtonOK( !(linkName().isEmpty()  || hrefName().isEmpty()));
    currentText = linkName();
}

bool KoInsertLinkDia::createLinkDia(QString & _linkName, QString & _hrefName, const QStringList& bkmlist, bool displayBookmarkLink, QWidget* parent, const char* name)
{
    bool res = false;

    KoInsertLinkDia *dlg = new KoInsertLinkDia( parent, name, displayBookmarkLink );
    dlg->setHrefLinkName(_hrefName,_linkName, bkmlist);
    if ( dlg->exec() == Accepted )
    {
        _linkName = dlg->linkName();
        _hrefName = dlg->hrefName();
        res = true;
    }
    delete dlg;

    return res;
}

void KoInsertLinkDia::setHrefLinkName(const QString &_href, const QString &_link, const QStringList & bkmlist)
{
    if ( bookmarkLink)
        bookmarkLink->setBookmarkList(bkmlist);
    if ( _href.isEmpty())
    {
        if ( !_link.isEmpty() )
        {
            internetLink->setLinkName(_link);
            setCurrentPage(p1);
            slotTextChanged ( );
        }
        return;
    } 
    if( _href.contains("http://") || _href.contains("https://") || _href.contains("ftp://") )
    {
        internetLink->setHrefName(_href);
        internetLink->setLinkName(_link);
        setCurrentPage(p1);
    }
    else if( _href.contains("file:/") )
    {
        fileLink->setHrefName(_href);
        fileLink->setLinkName(_link);
        setCurrentPage(p3);
    }
    else if( _href.contains("mailto:") || _href.contains("news:") )
    {
        mailLink->setHrefName(_href);
        mailLink->setLinkName(_link);
        setCurrentPage(p2);
    }
    else if( _href.contains("bkm://") )
    {
        if ( bookmarkLink )
        {
            bookmarkLink->setHrefName(_href.mid(6));
            bookmarkLink->setLinkName(_link);
            setCurrentPage(p4);
        }
    }
    slotTextChanged ( );
}

QString KoInsertLinkDia::linkName() const
{
    QString result;
    if ( currentPage() == p1 )
        result=internetLink->linkName();
    else if ( currentPage() == p2 )
        result=mailLink->linkName();
    else if ( currentPage() == p3 )
        result=fileLink->linkName();
    else if ( currentPage() == p4 )
    {
         if ( bookmarkLink)
             result=bookmarkLink->linkName();
    }
    return result;
}

QString KoInsertLinkDia::hrefName() const
{
    QString result;
    if ( currentPage() == p1 )
        result=internetLink->hrefName();
    else if ( currentPage() == p2 )
        result=mailLink->hrefName();
    else if ( currentPage() == p3 )
        result=fileLink->hrefName();
    else if ( currentPage() == p4 )
    {
         if ( bookmarkLink )
             result=bookmarkLink->hrefName();
    }
  return result;
}

void KoInsertLinkDia::slotOk()
{
    slotButtonClicked( KDialog::Ok );
}


internetLinkPage::internetLinkPage( QWidget *parent , char* /*name*/  )
  : QWidget(parent)
{
  QVBoxLayout *lay1 = new QVBoxLayout( this );
  lay1->setSpacing( KDialog::spacingHint() );
  QVBoxLayout *lay2 = new QVBoxLayout();
  lay2->setSpacing( KDialog::spacingHint() );
  lay1->addLayout( lay2 );

  QLabel* tmpQLabel = new QLabel( this);

  lay2->addWidget(tmpQLabel);
  tmpQLabel->setText(i18n("Text to display:"));

  m_linkName = new QLineEdit( this );
  lay2->addWidget(m_linkName);

  tmpQLabel = new QLabel( this);
  lay2->addWidget(tmpQLabel);

  tmpQLabel->setText(i18n("Internet address:"));
  m_hrefName = new QLineEdit( this );

  lay2->addWidget(m_hrefName);

  lay2->addStretch( 1 );
  
  m_linkName->setFocus();

  connect(m_linkName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  connect(m_hrefName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  KSeparator* bar1 = new KSeparator( Qt::Horizontal, this);
  bar1->setFixedHeight( 10 );
  lay2->addWidget( bar1 );
}

QString internetLinkPage::createInternetLink()
{
    QString result=m_hrefName->text();

    if(result.isEmpty())
        return result;

    if( !result.contains("http://") && !result.contains("https://") && !result.contains("ftp://") )
        result = "http://"+result;
    return result;
}


void internetLinkPage::setLinkName(const QString & _name)
{
    m_linkName->setText(_name);
}

void internetLinkPage::setHrefName(const QString &_name)
{
    m_hrefName->setText(_name);
}

QString internetLinkPage::linkName()const
{
  return m_linkName->text();
}

QString internetLinkPage::hrefName()
{
  return createInternetLink();
}

void internetLinkPage::textChanged ( const QString & )
{
    emit textChanged();
}

bookmarkLinkPage::bookmarkLinkPage( QWidget *parent , char* /*name*/  )
  : QWidget(parent)
{
  QVBoxLayout *lay1 = new QVBoxLayout( this );
  lay1->setSpacing( KDialog::spacingHint() );
  QVBoxLayout *lay2 = new QVBoxLayout();
  lay2->setSpacing( KDialog::spacingHint() );
  lay1->addLayout( lay2 );

  QLabel* tmpQLabel = new QLabel( this);

  lay2->addWidget(tmpQLabel);
  tmpQLabel->setText(i18n("Text to display:"));

  m_linkName = new QLineEdit( this );
  lay2->addWidget(m_linkName);

  tmpQLabel = new QLabel( this);
  lay2->addWidget(tmpQLabel);

  tmpQLabel->setText(i18n("Bookmark name:"));
  m_hrefName = new QComboBox( this );

  lay2->addWidget(m_hrefName);

  lay2->addStretch( 1 );
  
  m_linkName->setFocus();

  connect(m_linkName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  connect(m_hrefName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  KSeparator* bar1 = new KSeparator( Qt::Horizontal, this);
  bar1->setFixedHeight( 10 );
  lay2->addWidget( bar1 );
}

QString bookmarkLinkPage::createBookmarkLink()
{
    QString result=m_hrefName->currentText();

    if(result.isEmpty())
        return result;

    if( !result.contains("bkm://") )
        result = "bkm://"+result;
    return result;
}


void bookmarkLinkPage::setLinkName(const QString & _name)
{
    m_linkName->setText(_name);
}

void bookmarkLinkPage::setHrefName(const QString &_name)
{
    m_hrefName->setItemText( m_hrefName->currentIndex(), _name );
}

void bookmarkLinkPage::setBookmarkList(const QStringList & bkmlist)
{
    m_hrefName->clear();
    m_hrefName->insertItems( 0, bkmlist );
    if ( bkmlist.isEmpty())
        m_linkName->setEnabled( false);
    //m_hrefName->setEditable(true);
}

QString bookmarkLinkPage::linkName()const
{
  return m_linkName->text();
}

QString bookmarkLinkPage::hrefName()
{
  return createBookmarkLink();
}

void bookmarkLinkPage::textChanged ( const QString & )
{
    emit textChanged();
}

mailLinkPage::mailLinkPage( QWidget *parent , char* /*name*/  )
  : QWidget(parent)
{
  QVBoxLayout *lay1 = new QVBoxLayout( this );
  lay1->setSpacing( KDialog::spacingHint() );
  QVBoxLayout *lay2 = new QVBoxLayout();
  lay2->setSpacing( KDialog::spacingHint() );
  lay1->addLayout( lay2 );

  QLabel* tmpQLabel = new QLabel( this);

  lay2->addWidget(tmpQLabel);
  tmpQLabel->setText(i18n("Text to display:"));

  m_linkName = new QLineEdit( this );
  lay2->addWidget(m_linkName);

  tmpQLabel = new QLabel( this);
  lay2->addWidget(tmpQLabel);

  tmpQLabel->setText(i18n("Target:"));
  m_hrefName = new QLineEdit( this );

  lay2->addWidget(m_hrefName);
  lay2->addStretch( 1 );
  
  connect(m_linkName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  connect(m_hrefName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  KSeparator* bar1 = new KSeparator( Qt::Horizontal, this);
  bar1->setFixedHeight( 10 );
  lay2->addWidget( bar1 );
}

QString mailLinkPage::createMailLink()
{
    QString result=m_hrefName->text();

    if(result.isEmpty())
        return result;

    if( !result.contains("mailto:") && !result.contains("news:") )
        result = "mailto:"+result;
    return result;
}


void mailLinkPage::setLinkName(const QString & _name)
{
    m_linkName->setText(_name);
}

void mailLinkPage::setHrefName(const QString &_name)
{
    m_hrefName->setText(_name);
}

QString mailLinkPage::linkName()const
{
  return m_linkName->text();
}

QString mailLinkPage::hrefName()
{
  return createMailLink();
}

void mailLinkPage::textChanged ( const QString & )
{
    emit textChanged();
}

fileLinkPage::fileLinkPage( QWidget *parent , char* /*name*/  )
  : QWidget(parent)
{
  QVBoxLayout *lay1 = new QVBoxLayout( this );
  lay1->setSpacing( KDialog::spacingHint() );
  QVBoxLayout *lay2 = new QVBoxLayout();
  lay2->setSpacing( KDialog::spacingHint() );
  lay1->addLayout( lay2 );

  QLabel* tmpQLabel = new QLabel( this);

  lay2->addWidget(tmpQLabel);
  tmpQLabel->setText(i18n("Text to display:"));

  m_linkName = new QLineEdit( this );
  lay2->addWidget(m_linkName);

  tmpQLabel = new QLabel( this);
  lay2->addWidget(tmpQLabel);
  tmpQLabel->setText(i18n("Recent file:"));

  QComboBox * recentFile = new QComboBox( this );
  recentFile->setMaximumWidth( kapp->desktop()->width()*3/4 );
  lay2->addWidget(recentFile);

  QStringList fileList = KRecentDocument::recentDocuments();
  QStringList lst;
  lst <<"";
  for (QStringList::ConstIterator it = fileList.begin();it != fileList.end(); ++it)
  {
      KDesktopFile f(*it, true /* read only */);
      if ( !f.readURL().isEmpty())
          lst.append( f.readURL());
  }
  if ( lst.count()<= 1 )
  {
      recentFile->clear();
      recentFile->addItem( i18n("No Entries") );
      recentFile->setEnabled( false );
  }
  else
      recentFile->addItems( lst );
  
  recentFile->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  
  connect( recentFile , SIGNAL(highlighted ( const QString &)), this,  SLOT( slotSelectRecentFile( const QString & )));

  tmpQLabel = new QLabel( this);
  lay2->addWidget(tmpQLabel);

  tmpQLabel->setText(i18n("File location:"));
  m_hrefName = new KUrlRequester( this );

  lay2->addWidget(m_hrefName);
  lay2->addStretch( 1 );

  connect(m_linkName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  connect(m_hrefName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));

  KSeparator* bar1 = new KSeparator( Qt::Horizontal, this);
  bar1->setFixedHeight( 10 );
  lay2->addWidget( bar1 );
}

void fileLinkPage::slotSelectRecentFile( const QString &_file )
{
    m_hrefName->lineEdit()->setText(_file );
}

QString fileLinkPage::createFileLink()
{
    QString result=m_hrefName->lineEdit()->text();
    if(result.isEmpty())
        return result;

    if( !result.contains("file:/") )
        result = "file://"+result;
    return result;
}

void fileLinkPage::setLinkName(const QString & _name)
{
    m_linkName->setText(_name);
}

void fileLinkPage::setHrefName(const QString &_name)
{
    m_hrefName->lineEdit()->setText(_name);
}

QString fileLinkPage::linkName()const
{
  return m_linkName->text();
}

QString fileLinkPage::hrefName()
{
  return createFileLink();
}

void fileLinkPage::textChanged ( const QString & )
{
    emit textChanged();
}

#include "KoInsertLink.moc"
