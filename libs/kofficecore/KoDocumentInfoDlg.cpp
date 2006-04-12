/* This file is part of the KDE project
   Copyright (c) 2000 Simon Hausmann <hausmann@kde.org>

   $Id$

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

#include "KoDocumentInfoDlg.h"
#include "KoDocumentInfo.h"
#include "koDocumentInfoAboutWidget.h"
#include "koDocumentInfoAuthorWidget.h"
#include "koDocumentInfoUserMetadataWidget.h"
#include "KoDocument.h"

#include <KoGlobal.h>
#include <KoStore.h>

#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include <QLabel>
#include <QBuffer>
#include <QDom>
#include <QDir>
#include <kvbox.h>
#include <QDateTime>
//Added by qt3to4:
#include <QTextStream>

#include <kabc/addressee.h>
#include <kabc/stdaddressbook.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktar.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <kmimetype.h>
#include <QLayout>
#include <k3listview.h>
#include <q3grid.h>
#include <QMap>
#include <kfilterdev.h>
#include <klineedit.h>
#include <ktextedit.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <klocale.h>

class KoDocumentInfoDlg::KoDocumentInfoDlgPrivate
{
public:
  KoDocumentInfoDlgPrivate()
  {
  }
  ~KoDocumentInfoDlgPrivate()
  {
  }

  KoDocumentInfo *m_info;
  KoDocumentInfoAboutWidget *m_aboutWidget;
  KoDocumentInfoAuthorWidget *m_authorWidget;
  KoDocumentInfoUserMetadataWidget *m_metaWidget;

  bool m_bDeleteDialog;
  KDialogBase *m_dialog;
};

KoDocumentInfoDlg::KoDocumentInfoDlg( KoDocumentInfo *docInfo, QWidget *parent, const char *name,
                                      KDialogBase *dialog )
: QObject( parent, "docinfodlg" )
{
  d = new KoDocumentInfoDlgPrivate;
  d->m_info = docInfo;

  d->m_dialog = dialog;
  d->m_bDeleteDialog = false;

  if ( !dialog )
  {
    d->m_dialog = new KDialogBase( KDialogBase::Tabbed,
                                   i18n( "Document Information" ),
                                   KDialogBase::Ok | KDialogBase::Cancel,
                                   KDialogBase::Ok, parent, name, true, false );
    d->m_dialog->setInitialSize( QSize( 500, 500 ) );
    d->m_bDeleteDialog = true;
  }

  QStringList pages = docInfo->pages();
  QStringList::ConstIterator it = pages.begin();
  QStringList::ConstIterator end = pages.end();
  for (; it != end; ++it )
  {
    KoDocumentInfoPage *pg = docInfo->page( *it );
    if ( pg->inherits( "KoDocumentInfoAuthor" ) )
      addAuthorPage( static_cast<KoDocumentInfoAuthor *>( pg ) );
    else if ( pg->inherits( "KoDocumentInfoAbout" ) )
      addAboutPage( static_cast<KoDocumentInfoAbout *>( pg ) );
/*    else if ( pg->inherits( "KoDocumentInfoUserMetadata" ) )
      addUserMetadataPage( static_cast<KoDocumentInfoUserMetadata *>( pg ) );*/
  }
}

KoDocumentInfoDlg::~KoDocumentInfoDlg()
{
  if ( d->m_bDeleteDialog )
    delete d->m_dialog;

  delete d;
}

int KoDocumentInfoDlg::exec()
{
  return d->m_dialog->exec();
}

KDialogBase *KoDocumentInfoDlg::dialog() const
{
  return d->m_dialog;
}

void KoDocumentInfoDlg::loadFromKABC()
{
  KABC::StdAddressBook *ab = static_cast<KABC::StdAddressBook*>
                             ( KABC::StdAddressBook::self() );

  if ( !ab )
    return;

  KABC::Addressee addr = ab->whoAmI();
  if ( addr.isEmpty() )
  {
    KMessageBox::sorry( 0L, i18n( "No personal contact data set, please use the option \
                                  \"Set as Personal Contact Data\" from the \"Edit\" menu in KAddressbook to set one." ) );
    return;
  }

  d->m_authorWidget->leFullName->setText( addr.formattedName() );
  d->m_authorWidget->leInitial->setText( addr.givenName()[ 0 ] + ". " +
                           addr.familyName()[ 0 ] + "." );
  d->m_authorWidget->leAuthorTitle->setText( addr.title() );
  d->m_authorWidget->leCompany->setText( addr.organization() );
  d->m_authorWidget->leEmail->setText( addr.preferredEmail() );

  KABC::PhoneNumber phone = addr.phoneNumber( KABC::PhoneNumber::Home );
  d->m_authorWidget->leTelephoneHome->setText( phone.number() );
  phone = addr.phoneNumber( KABC::PhoneNumber::Work );
  d->m_authorWidget->leTelephoneWork->setText( phone.number() );

  phone = addr.phoneNumber( KABC::PhoneNumber::Fax );
  d->m_authorWidget->leFax->setText( phone.number() );

  KABC::Address a = addr.address( KABC::Address::Home );
  d->m_authorWidget->leCountry->setText( a.country() );
  d->m_authorWidget->lePostalCode->setText( a.postalCode() );
  d->m_authorWidget->leCity->setText( a.locality() );
  d->m_authorWidget->leStreet->setText( a.street() );

  emit changed();
}

void KoDocumentInfoDlg::deleteInfo()
{
  d->m_authorWidget->leFullName->setText( QString::null );
  d->m_authorWidget->leInitial->setText( QString::null );
  d->m_authorWidget->leAuthorTitle->setText( QString::null );
  d->m_authorWidget->leCompany->setText( QString::null );
  d->m_authorWidget->leEmail->setText( QString::null );
  d->m_authorWidget->leTelephoneHome->setText( QString::null );
  d->m_authorWidget->leTelephoneWork->setText( QString::null );
  d->m_authorWidget->leFax->setText( QString::null );
  d->m_authorWidget->leCountry->setText( QString::null );
  d->m_authorWidget->lePostalCode->setText( QString::null );
  d->m_authorWidget->leCity->setText( QString::null );
  d->m_authorWidget->leStreet->setText( QString::null );
  emit changed();
}

void KoDocumentInfoDlg::resetMetaData()
{
  QString s = KGlobal::locale()->formatDateTime( QDateTime::currentDateTime() );
  d->m_aboutWidget->labelCreated->setText( s + ", " + d->m_info->creator() );
  d->m_aboutWidget->labelModified->setText( "" );
  d->m_aboutWidget->labelRevision->setText( "0" );
  emit changed();
}

void KoDocumentInfoDlg::addAuthorPage( KoDocumentInfoAuthor *authorInfo )
{
  KVBox *page = d->m_dialog->addVBoxPage( i18n( "Author" ) );
  d->m_authorWidget = new KoDocumentInfoAuthorWidget( page );
  d->m_authorWidget->labelAuthor->setPixmap( KGlobal::iconLoader()->loadIcon( "personal", K3Icon::Desktop, 48 ) );
  d->m_authorWidget->pbLoadKABC->setIconSet( QIcon( KGlobal::iconLoader()->loadIcon( "kaddressbook", K3Icon::Small ) ) );
  d->m_authorWidget->pbDelete->setIconSet( QIcon( KGlobal::iconLoader()->loadIcon( "eraser", K3Icon::Small ) ) );

  d->m_authorWidget->leFullName->setText( authorInfo->fullName() );
  d->m_authorWidget->leInitial->setText( authorInfo->initial() );
  d->m_authorWidget->leAuthorTitle->setText( authorInfo->title() );
  d->m_authorWidget->leCompany->setText( authorInfo->company() );
  d->m_authorWidget->leEmail->setText( authorInfo->email() );
  d->m_authorWidget->leTelephoneWork->setText( authorInfo->telephoneWork() );
  d->m_authorWidget->leTelephoneHome->setText( authorInfo->telephoneHome() );
  d->m_authorWidget->leFax->setText( authorInfo->fax() );
  d->m_authorWidget->leCountry->setText( authorInfo->country() );
  d->m_authorWidget->lePostalCode->setText( authorInfo->postalCode() );
  d->m_authorWidget->leCity->setText( authorInfo->city() );
  d->m_authorWidget->leStreet->setText( authorInfo->street() );
  d->m_authorWidget->leAuthorPosition->setText( authorInfo->position() );

  connect( d->m_authorWidget->leFullName, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_authorWidget->leInitial, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_authorWidget->leAuthorTitle, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_authorWidget->leCompany, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_authorWidget->leEmail, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_authorWidget->leTelephoneWork, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_authorWidget->leTelephoneHome, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_authorWidget->leFax, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_authorWidget->leCountry, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_authorWidget->lePostalCode, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_authorWidget->leCity, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_authorWidget->leStreet, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_authorWidget->leAuthorPosition, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_authorWidget->pbLoadKABC, SIGNAL( clicked() ),
           this, SLOT( loadFromKABC() ) );
  connect( d->m_authorWidget->pbDelete, SIGNAL( clicked() ),
           this, SLOT( deleteInfo() ) );
}

void KoDocumentInfoDlg::addAboutPage( KoDocumentInfoAbout *aboutInfo )
{
  KVBox *page = d->m_dialog->addVBoxPage( i18n( "General" ) );
  d->m_aboutWidget = new KoDocumentInfoAboutWidget( page );
  d->m_aboutWidget->pbReset->setIconSet( QIcon( KGlobal::iconLoader()->loadIcon( "reload", K3Icon::Small ) ) );
  KoDocument* doc = dynamic_cast< KoDocument* >( d->m_info->parent() );
  if ( doc )
  {
    d->m_aboutWidget->leDocFile->setText( doc->file() );
    d->m_aboutWidget->labelType->setText( KMimeType::mimeType( doc->mimeType() )->comment() );
    d->m_aboutWidget->pixmapLabel->setPixmap( KMimeType::mimeType( doc->mimeType() )->pixmap( K3Icon::Desktop, 48 ) );
  }
  if ( aboutInfo->creationDate() != QString::null )
    d->m_aboutWidget->labelCreated->setText( aboutInfo->creationDate() + ", " + aboutInfo->initialCreator() );
  if ( aboutInfo->modificationDate() != QString::null )
    d->m_aboutWidget->labelModified->setText( aboutInfo->modificationDate() + ", " + d->m_info->creator() );
  d->m_aboutWidget->labelRevision->setText( aboutInfo->editingCycles() );
  d->m_aboutWidget->leDocTitle->setText( aboutInfo->title() );
  d->m_aboutWidget->leDocSubject->setText( aboutInfo->subject() );
  d->m_aboutWidget->leDocKeywords->setText( aboutInfo->keywords() );
  d->m_aboutWidget->meDocAbstract->setText( aboutInfo->abstract() );

  connect( d->m_aboutWidget->leDocTitle, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_aboutWidget->meDocAbstract, SIGNAL( textChanged() ),
           this, SIGNAL( changed() ) );
  connect( d->m_aboutWidget->leDocSubject, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_aboutWidget->leDocKeywords, SIGNAL( textChanged( const QString & ) ),
           this, SIGNAL( changed() ) );
  connect( d->m_aboutWidget->pbReset, SIGNAL( clicked() ),
           aboutInfo, SLOT( resetMetaData() ) );
  connect( d->m_aboutWidget->pbReset, SIGNAL( clicked() ),
           this, SLOT( resetMetaData() ) );
}

void KoDocumentInfoDlg::addUserMetadataPage( KoDocumentInfoUserMetadata *userMetadataInfo )
{
  KVBox *page = d->m_dialog->addVBoxPage( i18n( "User-Defined Metadata" ) );
  d->m_metaWidget = new KoDocumentInfoUserMetadataWidget( page );

  d->m_metaWidget->metaListView->addColumn( "Name" );
  d->m_metaWidget->metaListView->setFullWidth( true );

  QMap<QString, QString>::iterator it;
    for ( it = userMetadataInfo->metadataList()->begin(); it != userMetadataInfo->metadataList()->end(); ++it )
    {
        QString name = it.key();
        QString value = it.data();
        K3ListViewItem* it = new K3ListViewItem( d->m_metaWidget->metaListView, name, value );
        it->setPixmap( 0, KGlobal::iconLoader()->loadIcon( "text", K3Icon::Small ) );
    }
}

void KoDocumentInfoDlg::save()
{
  QStringList pages = d->m_info->pages();
  QStringList::ConstIterator it = pages.begin();
  QStringList::ConstIterator end = pages.end();
  bool saveInfo=false;
  for (; it != end; ++it )
  {
    KoDocumentInfoPage *pg = d->m_info->page( *it );
    if ( pg->inherits( "KoDocumentInfoAuthor" ) )
    {
        saveInfo=true;
        save( static_cast<KoDocumentInfoAuthor *>( pg ) );
    }
    else if ( pg->inherits( "KoDocumentInfoAbout" ) )
    {
        saveInfo=true;
        save( static_cast<KoDocumentInfoAbout *>( pg ) );
    }
  }
  if(saveInfo)
      d->m_info->documentInfochanged();
}

void KoDocumentInfoDlg::save( KoDocumentInfoAuthor *authorInfo )
{
  authorInfo->setFullName( d->m_authorWidget->leFullName->text() );
  authorInfo->setInitial( d->m_authorWidget->leInitial->text() );
  authorInfo->setTitle( d->m_authorWidget->leAuthorTitle->text() );
  authorInfo->setCompany( d->m_authorWidget->leCompany->text() );
  authorInfo->setEmail( d->m_authorWidget->leEmail->text() );
  authorInfo->setTelephoneWork( d->m_authorWidget->leTelephoneWork->text() );
  authorInfo->setTelephoneHome( d->m_authorWidget->leTelephoneHome->text() );
  authorInfo->setFax( d->m_authorWidget->leFax->text() );
  authorInfo->setCountry( d->m_authorWidget->leCountry->text() );
  authorInfo->setPostalCode( d->m_authorWidget->lePostalCode->text() );
  authorInfo->setCity( d->m_authorWidget->leCity->text() );
  authorInfo->setStreet( d->m_authorWidget->leStreet->text() );
  authorInfo->setPosition( d->m_authorWidget->leAuthorPosition->text() );

  KConfig* config = KoGlobal::kofficeConfig();
  KConfigGroup cgs( config, "Author" );
  config->writeEntry("telephone", d->m_authorWidget->leTelephoneHome->text());
  config->writeEntry("telephone-work", d->m_authorWidget->leTelephoneWork->text());
  config->writeEntry("fax", d->m_authorWidget->leFax->text());
  config->writeEntry("country",d->m_authorWidget->leCountry->text());
  config->writeEntry("postal-code",d->m_authorWidget->lePostalCode->text());
  config->writeEntry("city",  d->m_authorWidget->leCity->text());
  config->writeEntry("street", d->m_authorWidget->leStreet->text());
  config->sync();
}

void KoDocumentInfoDlg::save( KoDocumentInfoAbout *aboutInfo )
{
  aboutInfo->setTitle( d->m_aboutWidget->leDocTitle->text() );
  aboutInfo->setSubject( d->m_aboutWidget->leDocSubject->text() );
  aboutInfo->setKeywords( d->m_aboutWidget->leDocKeywords->text() );
  aboutInfo->setAbstract( d->m_aboutWidget->meDocAbstract->text() );
}

void KoDocumentInfoDlg::save( KoDocumentInfoUserMetadata* )
{
    // FIXME
}

class KoDocumentInfoPropsPage::KoDocumentInfoPropsPagePrivate
{
public:
  KoDocumentInfo *m_info;
  KoDocumentInfoDlg *m_dlg;
  KUrl m_url;
  KTar *m_src;
  KTar *m_dst;

  const KArchiveFile *m_docInfoFile;
};

KoDocumentInfoPropsPage::KoDocumentInfoPropsPage( KPropertiesDialog *props,
                                                  const char *,
                                                  const QStringList & )
: KPropsDlgPlugin( props )
{
  d = new KoDocumentInfoPropsPagePrivate;
  d->m_info = new KoDocumentInfo( this, "docinfo" );
  d->m_url = props->item()->url();
  d->m_dlg = 0;

  if ( !d->m_url.isLocalFile() )
    return;

  d->m_dst = 0;

#ifdef __GNUC__
#warning TODO port this to KoStore !!!
#endif
  d->m_src = new KTar( d->m_url.path(), "application/x-gzip" );

  if ( !d->m_src->open( QIODevice::ReadOnly ) )
    return;

  const KArchiveDirectory *root = d->m_src->directory();
  if ( !root )
    return;

  const KArchiveEntry *entry = root->entry( "documentinfo.xml" );

  if ( entry && entry->isFile() )
  {
    d->m_docInfoFile = static_cast<const KArchiveFile *>( entry );

    QBuffer buffer( &d->m_docInfoFile->data() );
    buffer.open( QIODevice::ReadOnly );

    QDomDocument doc;
    doc.setContent( &buffer );

    d->m_info->load( doc );
  }

  d->m_dlg = new KoDocumentInfoDlg( d->m_info, 0, 0, props );
  connect( d->m_dlg, SIGNAL( changed() ),
           this, SIGNAL( changed() ) );
}

KoDocumentInfoPropsPage::~KoDocumentInfoPropsPage()
{
  delete d->m_info;
  delete d->m_src;
  delete d->m_dst;
  delete d->m_dlg;
  delete d;
}

void KoDocumentInfoPropsPage::applyChanges()
{
  const KArchiveDirectory *root = d->m_src->directory();
  if ( !root )
    return;

  struct stat statBuff;

  if ( stat( QFile::encodeName( d->m_url.path() ), &statBuff ) != 0 )
    return;

  KTempFile tempFile( d->m_url.path(), QString::null, statBuff.st_mode );

  tempFile.setAutoDelete( true );

  if ( tempFile.status() != 0 )
    return;

  if ( !tempFile.close() )
    return;

  d->m_dst = new KTar( tempFile.name(), "application/x-gzip" );

  if ( !d->m_dst->open( QIODevice::WriteOnly ) )
    return;

  KMimeType::Ptr mimeType = KMimeType::findByURL( d->m_url, 0, true );
  if ( mimeType && dynamic_cast<KFilterDev *>( d->m_dst->device() ) != 0 )
  {
      QByteArray appIdentification( "KOffice " ); // We are limited in the number of chars.
      appIdentification += mimeType->name().toLatin1();
      appIdentification += '\004'; // Two magic bytes to make the identification
      appIdentification += '\006'; // more reliable (DF)
      d->m_dst->setOrigFileName( appIdentification );
  }

  bool docInfoSaved = false;

  QStringList entries = root->entries();
  QStringList::ConstIterator it = entries.begin();
  QStringList::ConstIterator end = entries.end();
  for (; it != end; ++it )
  {
    const KArchiveEntry *entry = root->entry( *it );

    assert( entry );

    if ( entry->name() == "documentinfo.xml" ||
         ( !docInfoSaved && !entries.contains( "documentinfo.xml" ) ) )
    {
      d->m_dlg->save();

      QBuffer buffer;
      buffer.open( QIODevice::WriteOnly );
      QTextStream str( &buffer );
      str << d->m_info->save();
      buffer.close();

      kDebug( 30003 ) << "writing documentinfo.xml" << endl;
      d->m_dst->writeFile( "documentinfo.xml", entry->user(), entry->group(),
                           buffer.buffer().data(), buffer.buffer().size() );

      docInfoSaved = true;
    }
    else
      copy( QString::null, entry );
  }

  d->m_dst->close();

  QDir dir;
  dir.rename( tempFile.name(), d->m_url.path() );

  delete d->m_dst;
  d->m_dst = 0;
}

void KoDocumentInfoPropsPage::copy( const QString &path, const KArchiveEntry *entry )
{
  kDebug( 30003 ) << "copy " << entry->name() << endl;
  if ( entry->isFile() )
  {
    const KArchiveFile *file = static_cast<const KArchiveFile *>( entry );
    kDebug( 30003 ) << "file :" << entry->name() << endl;
    kDebug( 30003 ) << "full path is: " << path << entry->name() << endl;
    d->m_dst->writeFile( path + entry->name(), entry->user(), entry->group(),
                         file->data().data(), file->size() );
  }
  else
  {
    const KArchiveDirectory *dir = static_cast<const KArchiveDirectory*>( entry );
    kDebug( 30003 ) << "dir : " << entry->name() << endl;
    kDebug( 30003 ) << "full path is: " << path << entry->name() << endl;

    QString p = path + entry->name();
    if ( p != "/" )
    {
      d->m_dst->writeDir( p, entry->user(), entry->group() );
      p.append( "/" );
    }

    QStringList entries = dir->entries();
    QStringList::ConstIterator it = entries.begin();
    QStringList::ConstIterator end = entries.end();
    for (; it != end; ++it )
      copy( p, dir->entry( *it ) );
  }
}

/* vim: sw=2 et
 */

#include "KoDocumentInfoDlg.moc"
