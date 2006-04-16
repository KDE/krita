/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000-2006 David Faure <faure@kde.org>

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

#include "KoMainWindow.h"
#include "KoDocument.h"
#include "KoView.h"
#include "KoFilterManager.h"
#include "KoDocumentInfo.h"
#include "KoDocumentInfoDlg.h"
#include "KoQueryTrader.h"
#include "KoMainWindowIface.h"
#include "KoFrame.h"
#include "KoFileDialog.h"
#include "Koversiondialog.h"
#include "kkbdaccessextensions.h"

#include <kprinter.h>
#include <kdeversion.h>
#include <kstdaction.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kprogressbar.h>
#include <kpushbutton.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <krecentdocument.h>
#include <kparts/partmanager.h>
#include <kparts/plugin.h>
#include <kparts/event.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kglobalsettings.h>
#include <ktoolinvocation.h>
#include <kxmlguifactory.h>

#include <QObject>
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3PtrList>
#include <QEvent>
#include <QLabel>
#include <QResizeEvent>
#include <QSplitter>
#include <kxmlguifactory.h>
#include <ktoolbar.h>
#include <unistd.h>
#include <stdlib.h>
#include <ktoolbar.h>

class KoPartManager : public KParts::PartManager
{
public:
  KoPartManager( QWidget * parent )
    : KParts::PartManager( parent )
  {
      setSelectionPolicy( KParts::PartManager::TriState );
      setAllowNestedParts( true );
      setIgnoreScrollBars( true );
      // Allow right-click on embedded objects (without activating them)
      // But beware: this means right-click on parent, from embedded object,
      // doesn't make the parent active first...
      setActivationButtonMask( Qt::LeftButton | Qt::MidButton );
  }
  virtual bool eventFilter( QObject *obj, QEvent *ev )
  {
    if ( !obj->isWidgetType() || ::qobject_cast<KoFrame *>( obj ) )
      return false;
    return KParts::PartManager::eventFilter( obj, ev );
  }
};

class KoMainWindowPrivate
{
public:
  KoMainWindowPrivate()
  {
    m_rootDoc = 0L;
    m_docToOpen = 0L;
    m_manager = 0L;
    bMainWindowGUIBuilt = false;
    m_forQuit=false;
    m_splitted=false;
    m_activePart = 0L;
    m_activeView = 0L;
    m_splitter=0L;
    m_orientation=0L;
    m_removeView=0L;
    m_firstTime=true;
    m_progress=0L;
    m_paDocInfo = 0;
    m_paSave = 0;
    m_paSaveAs = 0;
    m_paPrint = 0;
    m_paPrintPreview = 0;
    statusBarLabel = 0L;
    m_dcopObject = 0;
    m_sendfile = 0;
    m_paCloseFile = 0L;
    m_reloadfile = 0L;
    m_versionsfile = 0L;
    m_importFile = 0;
    m_exportFile = 0;
    m_isImporting = false;
    m_isExporting = false;
    m_windowSizeDirty = false;
    m_lastExportSpecialOutputFlag = 0;

  }
  ~KoMainWindowPrivate()
  {
    delete m_dcopObject;
    qDeleteAll( m_toolbarList );
  }

  KoDocument *m_rootDoc;
  KoDocument *m_docToOpen;
  Q3PtrList<KoView> m_rootViews;
  KParts::PartManager *m_manager;

  KParts::Part *m_activePart;
  KoView *m_activeView;

  QLabel * statusBarLabel;
  KProgressBar *m_progress;

  QList<KAction *> m_splitViewActionList;
  // This additional list is needed, because we don't plug
  // the first list, when an embedded view gets activated (Werner)
  QList<KAction *> m_veryHackyActionList;
  QSplitter *m_splitter;
  KSelectAction *m_orientation;
  KAction *m_removeView;
  KoMainWindowIface *m_dcopObject;

  QList<KAction *> m_toolbarList;

  bool bMainWindowGUIBuilt;
  bool m_splitted;
  bool m_forQuit;
  bool m_firstTime;
  bool m_windowSizeDirty;

  KAction *m_paDocInfo;
  KAction *m_paSave;
  KAction *m_paSaveAs;
  KAction *m_paPrint;
  KAction *m_paPrintPreview;
  KAction *m_sendfile;
  KAction *m_paCloseFile;
  KAction *m_reloadfile;
  KAction *m_versionsfile;
  KAction *m_importFile;
  KAction *m_exportFile;

  bool m_isImporting;
  bool m_isExporting;

  KUrl m_lastExportURL;
  QByteArray m_lastExportFormat;
  int m_lastExportSpecialOutputFlag;
};

KoMainWindow::KoMainWindow( KInstance *instance, const char* name )
    : KParts::MainWindow( name )
{
    setStandardToolBarMenuEnabled(true); // should there be a check for >= 3.1 ?
    Q_ASSERT(instance);
    d = new KoMainWindowPrivate;

    d->m_manager = new KoPartManager( this );

    connect( d->m_manager, SIGNAL( activePartChanged( KParts::Part * ) ),
             this, SLOT( slotActivePartChanged( KParts::Part * ) ) );

    if ( instance )
        setInstance( instance, false ); // don't load plugins! we don't want
    // the part's plugins with this shell, even though we are using the
    // part's instance! (Simon)

    QString doc;
    QStringList allFiles = KGlobal::dirs()->findAllResources( "data", "koffice/koffice_shell.rc" );
    setXMLFile( findMostRecentXMLFile( allFiles, doc ) );
    setLocalXMLFile( locateLocal( "data", "koffice/koffice_shell.rc" ) );

    KStdAction::openNew( this, SLOT( slotFileNew() ), actionCollection(), "file_new" );
    KStdAction::open( this, SLOT( slotFileOpen() ), actionCollection(), "file_open" );
    m_recent = KStdAction::openRecent( this, SLOT(slotFileOpenRecent(const KUrl&)), actionCollection() );
    d->m_paSave = KStdAction::save( this, SLOT( slotFileSave() ), actionCollection(), "file_save" );
    d->m_paSaveAs = KStdAction::saveAs( this, SLOT( slotFileSaveAs() ), actionCollection(), "file_save_as" );
    d->m_paPrint = KStdAction::print( this, SLOT( slotFilePrint() ), actionCollection(), "file_print" );
    d->m_paPrintPreview = KStdAction::printPreview( this, SLOT( slotFilePrintPreview() ), actionCollection(), "file_print_preview" );
    d->m_sendfile = KStdAction::mail( this, SLOT( slotEmailFile() ), actionCollection(), "file_send_file");

    d->m_paCloseFile = KStdAction::close( this, SLOT( slotFileClose() ), actionCollection(), "file_close" );
    KStdAction::quit( this, SLOT( slotFileQuit() ), actionCollection(), "file_quit" );

    d->m_reloadfile = new KAction( i18n( "Reload"),
                                   actionCollection(), "file_reload_file");
    connect( d->m_reloadfile, SIGNAL( triggered(bool) ), this, SLOT( slotReloadFile() ) );

    d->m_versionsfile = new KAction( i18n( "Versions..."),
                                     actionCollection(), "file_versions_file");
    connect( d->m_versionsfile, SIGNAL( triggered(bool) ), this, SLOT( slotVersionsFile() ) );

    d->m_importFile = new KAction( i18n( "I&mport..." ),
                                   actionCollection(), "file_import_file");
    connect( d->m_importFile, SIGNAL( triggered(bool) ), this, SLOT( slotImportFile() ) );

    d->m_exportFile = new KAction( i18n( "E&xport..." ),
                                   actionCollection(), "file_export_file");
    connect( d->m_exportFile, SIGNAL( triggered(bool) ), this, SLOT( slotExportFile() ) );

    /* The following entry opens the document information dialog.  Since the action is named so it
        intends to show data this entry should not have a trailing ellipses (...).  */
    d->m_paDocInfo = new KAction( KIcon("documentinfo"), i18n( "&Document Information" ),
                                  actionCollection(), "file_documentinfo" );
    connect( d->m_paDocInfo, SIGNAL( triggered(bool) ), this, SLOT( slotDocumentInfo() ) );

    KStdAction::keyBindings( this, SLOT( slotConfigureKeys() ), actionCollection() );
    KStdAction::configureToolbars( this, SLOT( slotConfigureToolbars() ), actionCollection() );

    d->m_paDocInfo->setEnabled( false );
    d->m_paSaveAs->setEnabled( false );
    d->m_reloadfile->setEnabled( false );
    d->m_versionsfile->setEnabled( false );
    d->m_importFile->setEnabled( true );  // always enabled like File --> Open
    d->m_exportFile->setEnabled( false );
    d->m_paSave->setEnabled( false );
    d->m_paPrint->setEnabled( false );
    d->m_paPrintPreview->setEnabled( false );
    d->m_sendfile->setEnabled( false);
    d->m_paCloseFile->setEnabled( false);

    d->m_splitter = new QSplitter( Qt::Vertical, this );
    d->m_splitter->setObjectName( "mw-splitter" );
    setCentralWidget( d->m_splitter );
    // Keyboard accessibility enhancements.
    new KKbdAccessExtensions(this, "mw-panelSizer");

    // set up the action "list" for "Close all Views" (hacky :) (Werner)
    KAction* closeAllViews = new KAction( KIcon("fileclose"), i18n("&Close All Views"),
                                          actionCollection(), "view_closeallviews" );
    closeAllViews->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_W);
    connect( closeAllViews, SIGNAL(triggered(bool)), this, SLOT(slotCloseAllViews()) );
    d->m_veryHackyActionList.append(closeAllViews);

    // set up the action list for the splitter stuff
    KAction* splitView = new KAction( KIcon("view_split"), i18n("&Split View"),
                                     actionCollection(), "view_split");
    connect( splitView, SIGNAL(triggered(bool)), this, SLOT(slotSplitView()) );
    d->m_splitViewActionList.append(splitView);

    d->m_removeView = new KAction( KIcon("view_remove"), i18n("&Remove View"),
                                   actionCollection(), "view_rm_splitter");
    connect( d->m_removeView, SIGNAL(triggered(bool)), this, SLOT(slotRemoveView()) );
    d->m_splitViewActionList.append(d->m_removeView);
    d->m_removeView->setEnabled(false);

    d->m_orientation = new KSelectAction( KIcon("view_orientation"), i18n("Splitter &Orientation"),
                                          actionCollection(), "view_splitter_orientation");
    connect( d->m_orientation, SIGNAL(triggered(bool)), this, SLOT(slotSetOrientation()) );
    QStringList items;
    items << i18n("&Vertical")
          << i18n("&Horizontal");
    d->m_orientation->setItems(items);
    d->m_orientation->setCurrentItem(static_cast<int>(d->m_splitter->orientation()));
    d->m_splitViewActionList.append(d->m_orientation);
    d->m_splitViewActionList.append(new KActionSeparator(actionCollection()));

    // Load list of recent files
    KConfig * config = instance ? instance->config() : KGlobal::config();
    m_recent->loadEntries( config );

    createShellGUI();
    d->bMainWindowGUIBuilt = true;

    if ( !initialGeometrySet() )
    {
        // Default size
        const int deskWidth = KGlobalSettings::desktopGeometry(this).width();
        if (deskWidth > 1100) // very big desktop ?
            resize( 1000, 800 );
        if (deskWidth > 850) // big desktop ?
            resize( 800, 600 );
        else // small (800x600, 640x480) desktop
            resize( 600, 400 );
    }

    // Saved size
    config->setGroup( "MainWindow" );
    //kDebug(30003) << "KoMainWindow::restoreWindowSize" << endl;
    restoreWindowSize( config );
}

KoMainWindow::~KoMainWindow()
{
    // The doc and view might still exist (this is the case when closing the window)
    if (d->m_rootDoc)
        d->m_rootDoc->removeShell(this);

    if (d->m_docToOpen) {
      d->m_docToOpen->removeShell(this);
      delete d->m_docToOpen;
    }

    // safety first ;)
    d->m_manager->setActivePart(0);

    if(d->m_rootViews.findRef(d->m_activeView)==-1) {
        delete d->m_activeView;
        d->m_activeView=0L;
    }
    d->m_rootViews.setAutoDelete( true );
    d->m_rootViews.clear();

    // We have to check if this was a root document.
    // -> We aren't allowed to delete the (embedded) document!
    // This has to be checked from queryClose, too :)
    if ( d->m_rootDoc && d->m_rootDoc->viewCount() == 0 &&
         !d->m_rootDoc->isEmbedded())
    {
        //kDebug(30003) << "Destructor. No more views, deleting old doc " << d->m_rootDoc << endl;
        delete d->m_rootDoc;
    }

    delete d->m_manager;
    delete d;
}

void KoMainWindow::setRootDocument( KoDocument *doc )
{
  if ( d->m_rootDoc == doc )
    return;

  if (d->m_docToOpen && d->m_docToOpen != doc) {
    d->m_docToOpen->removeShell(this);
    delete d->m_docToOpen;
    d->m_docToOpen = 0;
  } else {
    d->m_docToOpen = 0;
  }

  //kDebug(30003) <<  "KoMainWindow::setRootDocument this = " << this << " doc = " << doc << endl;
  Q3PtrList<KoView> oldRootViews = d->m_rootViews;
  d->m_rootViews.clear();
  KoDocument *oldRootDoc = d->m_rootDoc;

  if ( oldRootDoc )
    oldRootDoc->removeShell( this );

  d->m_rootDoc = doc;

  if ( doc )
  {
    doc->setSelectable( false );
    //d->m_manager->addPart( doc, false ); // done by KoView::setPartManager
    d->m_rootViews.append( doc->createView( d->m_splitter, "view" /*not unique, but better than unnamed*/ ) );
    d->m_rootViews.current()->setPartManager( d->m_manager );

    d->m_rootViews.current()->show();
    // The addShell has been done already if using openURL
    if ( !d->m_rootDoc->shells().contains( this ) )
        d->m_rootDoc->addShell( this );
    d->m_removeView->setEnabled(false);
    d->m_orientation->setEnabled(false);
  }

  bool enable = d->m_rootDoc != 0 ? true : false;
  d->m_paDocInfo->setEnabled( enable );
  d->m_paSave->setEnabled( enable );
  d->m_paSaveAs->setEnabled( enable );
  d->m_importFile->setEnabled( enable );
  d->m_exportFile->setEnabled( enable );
  d->m_paPrint->setEnabled( enable );
  d->m_paPrintPreview->setEnabled( enable );
  d->m_sendfile->setEnabled( enable);
  d->m_paCloseFile->setEnabled( enable);
  updateCaption();

  d->m_manager->setActivePart( d->m_rootDoc, d->m_rootViews.current() );

  oldRootViews.setAutoDelete( true );
  oldRootViews.clear();

  if ( oldRootDoc && oldRootDoc->viewCount() == 0 )
  {
    //kDebug(30003) << "No more views, deleting old doc " << oldRootDoc << endl;
    delete oldRootDoc;
  }
}

void KoMainWindow::updateReloadFileAction(KoDocument *doc)
{
    d->m_reloadfile->setEnabled( doc && !doc->url().isEmpty() );
}

void KoMainWindow::updateVersionsFileAction(KoDocument *doc)
{
    //TODO activate it just when we save it in oasis file format
    d->m_versionsfile->setEnabled( doc && !doc->url().isEmpty()&&doc->isModified());
}


void KoMainWindow::setRootDocumentDirect( KoDocument *doc, const Q3PtrList<KoView> & views )
{
  d->m_rootDoc = doc;
  d->m_rootViews = views;
  bool enable = d->m_rootDoc != 0 ? true : false;
  d->m_paDocInfo->setEnabled( enable );
  d->m_paSave->setEnabled( enable );
  d->m_paSaveAs->setEnabled( enable );
  d->m_exportFile->setEnabled( enable );
  d->m_paPrint->setEnabled( enable );
  d->m_paPrintPreview->setEnabled( enable );
  d->m_sendfile->setEnabled( enable);
  d->m_paCloseFile->setEnabled( enable );
}

void KoMainWindow::addRecentURL( const KUrl& url )
{
    kDebug(30003) << "KoMainWindow::addRecentURL url=" << url.prettyURL() << endl;
    // Add entry to recent documents list
    // (call coming from KoDocument because it must work with cmd line, template dlg, file/open, etc.)
    if ( !url.isEmpty() )
    {
        bool ok = true;
        if ( url.isLocalFile() )
        {
            QString path = url.path( -1 );
            QStringList tmpDirs = KGlobal::dirs()->resourceDirs( "tmp" );
            for ( QStringList::Iterator it = tmpDirs.begin() ; ok && it != tmpDirs.end() ; ++it )
                if ( path.contains( *it ) )
                    ok = false; // it's in the tmp resource
            if ( ok )
                KRecentDocument::add(path);
        }
        else
            KRecentDocument::add(url.url(-1), true);

        if ( ok )
            m_recent->addUrl( url );
        saveRecentFiles();
    }
}

void KoMainWindow::saveRecentFiles()
{
    // Save list of recent files
    KConfig * config = instance() ? instance()->config() : KGlobal::config();
    kDebug(30003) << this << " Saving recent files list into config. instance()=" << instance() << endl;
    m_recent->saveEntries( config );
    config->sync();

    // Tell all windows to reload their list, after saving
    // Doesn't work multi-process, but it's a start
    foreach ( KMainWindow* window, KMainWindow::memberList())
        static_cast<KoMainWindow *>(window)->reloadRecentFileList();
}

void KoMainWindow::reloadRecentFileList()
{
    KConfig * config = instance() ? instance()->config() : KGlobal::config();
    m_recent->loadEntries( config );
}

KoDocument* KoMainWindow::createDoc() const
{
    KoDocumentEntry entry = KoDocumentEntry( KoDocument::readNativeService() );
    QString errorMsg;
    return entry.createDoc( &errorMsg );
}

void KoMainWindow::updateCaption()
{
  //kDebug(30003) << "KoMainWindow::updateCaption()" << endl;
  if ( !d->m_rootDoc )
    setCaption(QString::null);
  else if ( rootDocument()->isCurrent() )
  {
      QString caption;
      // Get caption from document info (title(), in about page)
      if ( rootDocument()->documentInfo() )
      {
          KoDocumentInfoPage * page = rootDocument()->documentInfo()->page( QString::fromLatin1("about") );
          if (page)
              caption = static_cast<KoDocumentInfoAbout *>(page)->title();
      }
      const QString url = rootDocument()->url().pathOrURL();
      if ( !caption.isEmpty() && !url.isEmpty() )
          caption = QString( "%1 - %2" ).arg( caption ).arg( url );
      else if ( caption.isEmpty() )
          caption = url;

      setCaption( caption, rootDocument()->isModified() );
      if ( !rootDocument()->url().fileName(false).isEmpty() )
        d->m_paSave->setToolTip( i18n("Save as %1").arg(rootDocument()->url().fileName(false)) );
      else
        d->m_paSave->setToolTip( i18n("Save") );
  }
}

void KoMainWindow::updateCaption( QString caption, bool mod )
{
  //kDebug(30003)<<"KoMainWindow::updateCaption("<<caption<<","<<mod<<")"<<endl;
  setCaption( caption, mod );
}

KoDocument *KoMainWindow::rootDocument() const
{
    return d->m_rootDoc;
}

KoView *KoMainWindow::rootView() const
{
  if(d->m_rootViews.find(d->m_activeView)!=-1)
    return d->m_activeView;
  return d->m_rootViews.first();
}

KParts::PartManager *KoMainWindow::partManager()
{
  return d->m_manager;
}

bool KoMainWindow::openDocument( const KUrl & url )
{
    if ( !KIO::NetAccess::exists(url,true,0) )
    {
        KMessageBox::error(0L, i18n("The file %1 does not exist.").arg(url.url()) );
        m_recent->removeUrl(url); //remove the file from the recent-opened-file-list
        saveRecentFiles();
        return false;
    }
    return  openDocumentInternal( url );
}

// (not virtual)
bool KoMainWindow::openDocument( KoDocument *newdoc, const KUrl & url )
{
    if (!KIO::NetAccess::exists(url,true,0) )
    {
        if (!newdoc->checkAutoSaveFile())
        {
          newdoc->initEmpty(); //create an emtpy document
        }

        setRootDocument( newdoc );
        newdoc->setURL(url);
        QString mime = KMimeType::findByURL(url)->name();
        if ( mime.isEmpty() || mime == KMimeType::defaultMimeType() )
            mime = newdoc->nativeFormatMimeType();
        if ( url.isLocalFile() ) // workaround for kde<=3.3 kparts bug, fixed for 3.4
            newdoc->setFile(url.path());
        newdoc->setMimeTypeAfterLoading( mime );
        updateCaption();
        return true;
    }
    return openDocumentInternal( url, newdoc );
}

// ## If you modify anything here, please check KoShellWindow::openDocumentInternal
bool KoMainWindow::openDocumentInternal( const KUrl & url, KoDocument *newdoc )
{
    //kDebug(30003) << "KoMainWindow::openDocument " << url.url() << endl;

    if ( !newdoc )
        newdoc = createDoc();
    if ( !newdoc )
        return false;

    d->m_firstTime=true;
    connect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    connect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    connect(newdoc, SIGNAL(canceled( const QString & )), this, SLOT(slotLoadCanceled( const QString & )));
    newdoc->addShell( this ); // used by openURL
    bool openRet = (!isImporting ()) ? newdoc->openURL(url) : newdoc->import(url);
    if(!openRet)
    {
        newdoc->removeShell(this);
        delete newdoc;
        return false;
    }
    updateReloadFileAction(newdoc);
    updateVersionsFileAction( newdoc );
    return true;
}

// Separate from openDocument to handle async loading (remote URLs)
void KoMainWindow::slotLoadCompleted()
{
    kDebug(30003) << "KoMainWindow::slotLoadCompleted" << endl;
    KoDocument* doc = rootDocument();
    KoDocument* newdoc = (KoDocument *)(sender());

    if ( doc && doc->isEmpty() && !doc->isEmbedded() )
    {
        // Replace current empty document
        setRootDocument( newdoc );
    }
    else if ( doc && !doc->isEmpty() )
    {
        // Open in a new shell
        // (Note : could create the shell first and the doc next for this
        // particular case, that would give a better user feedback...)
        KoMainWindow *s = new KoMainWindow( newdoc->instance() );
        s->show();
        newdoc->removeShell( this );
        s->setRootDocument( newdoc );
    }
    else
    {
        // We had no document, set the new one
       setRootDocument( newdoc );
    }
    disconnect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    disconnect(newdoc, SIGNAL(canceled( const QString & )), this, SLOT(slotLoadCanceled( const QString & )));
}

void KoMainWindow::slotLoadCanceled( const QString & errMsg )
{
    kDebug(30003) << "KoMainWindow::slotLoadCanceled" << endl;
    if ( !errMsg.isEmpty() ) // empty when canceled by user
        KMessageBox::error( this, errMsg );
    // ... can't delete the document, it's the one who emitted the signal...

    KoDocument* newdoc = (KoDocument *)(sender());
    disconnect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    disconnect(newdoc, SIGNAL(canceled( const QString & )), this, SLOT(slotLoadCanceled( const QString & )));

    newdoc->removeShell(this);
    delete newdoc;
}

void KoMainWindow::slotSaveCanceled( const QString &errMsg )
{
    kDebug(30003) << "KoMainWindow::slotSaveCanceled" << endl;
    if ( !errMsg.isEmpty() ) // empty when canceled by user
        KMessageBox::error( this, errMsg );
    slotSaveCompleted();
}

void KoMainWindow::slotSaveCompleted()
{
    kDebug(30003) << "KoMainWindow::slotSaveCompleted" << endl;
    KoDocument* pDoc = (KoDocument *)(sender());
    disconnect(pDoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(pDoc, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
    disconnect(pDoc, SIGNAL(canceled( const QString & )),
               this, SLOT(slotSaveCanceled( const QString & )));
}

// returns true if we should save, false otherwise.
bool KoMainWindow::exportConfirmation( const QByteArray &outputFormat )
{
    if (!rootDocument()->wantExportConfirmation()) return true;
    KMimeType::Ptr mime = KMimeType::mimeType( outputFormat );

    const bool neverHeardOfIt = ( mime->name() == KMimeType::defaultMimeType() );
    QString comment = neverHeardOfIt ?
                      i18n( "%1 (unknown file type)" ).arg( QString::fromLatin1(outputFormat) )
                      : mime->comment();

    // Warn the user
    int ret;
    if (!isExporting ()) // File --> Save
    {
        ret = KMessageBox::warningContinueCancel
              (
                  this,
                  i18n( "<qt>Saving as a %1 may result in some loss of formatting."
                        "<p>Do you still want to save in this format?</qt>" )
                  .arg( QString( "<b>%1</b>" ).arg( comment ) ), // in case we want to remove the bold later
                  i18n( "Confirm Save" ),
                  KStdGuiItem::save (),
                  "NonNativeSaveConfirmation"
                  );
    }
    else // File --> Export
    {
        ret = KMessageBox::warningContinueCancel
              (
                  this,
                  i18n( "<qt>Exporting as a %1 may result in some loss of formatting."
                        "<p>Do you still want to export to this format?</qt>" )
                  .arg( QString( "<b>%1</b>" ).arg( comment ) ), // in case we want to remove the bold later
                  i18n( "Confirm Export" ),
                  i18n ("Export"),
                  "NonNativeExportConfirmation" // different to the one used for Save (above)
                  );
    }

    return (ret == KMessageBox::Continue);
}

bool KoMainWindow::saveDocument( bool saveas, bool silent )
{
    KoDocument* pDoc = rootDocument();
    if(!pDoc)
        return true;

    bool reset_url;
    if ( pDoc->url().isEmpty() )
    {
        emit saveDialogShown();
        reset_url = true;
        saveas = true;
    }
    else
        reset_url = false;

    connect(pDoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    connect(pDoc, SIGNAL(completed()), this, SLOT(slotSaveCompleted()));
    connect(pDoc, SIGNAL(canceled( const QString & )),
            this, SLOT(slotSaveCanceled( const QString & )));

    KUrl oldURL = pDoc->url();
    QString oldFile = pDoc->file();
    QByteArray _native_format = pDoc->nativeFormatMimeType();
    QByteArray oldOutputFormat = pDoc->outputMimeType();
    int oldSpecialOutputFlag = pDoc->specialOutputFlag();
    KUrl suggestedURL = pDoc->url();

    QStringList mimeFilter = KoFilterManager::mimeFilter( _native_format, KoFilterManager::Export, pDoc->extraNativeMimeTypes() );
    if( !mimeFilter.contains(oldOutputFormat) < 0 && !isExporting() )
    {
        kDebug(30003) << "KoMainWindow::saveDocument no export filter for '" << oldOutputFormat << "'" << endl;

        // --- don't setOutputMimeType in case the user cancels the Save As
        // dialog and then tries to just plain Save ---

        // suggest a different filename extension (yes, we fortunately don't all live in a world of magic :))
        QString suggestedFilename = suggestedURL.fileName ();
        if ( !suggestedFilename.isEmpty () ) // ".kwd" looks strange for a name
        {
            int c = suggestedFilename.lastIndexOf('.');

            KMimeType::Ptr mime = KMimeType::mimeType( _native_format );
            QString ext = mime->property( "X-KDE-NativeExtension" ).toString();
            if (!ext.isEmpty ())
            {
                if (c < 0)
                    suggestedFilename += ext;
                else
                    suggestedFilename = suggestedFilename.left (c) + ext;
            }
            else  // current filename extension wrong anyway
            {
                if (c > 0)
                {
                    // this assumes that a . signifies an extension, not just a .
                    suggestedFilename = suggestedFilename.left (c);
                }
            }

            suggestedURL.setFileName (suggestedFilename);
        }

        // force the user to choose outputMimeType
        saveas = true;
    }

    bool ret = false;

    if ( pDoc->url().isEmpty() || saveas )
    {
        // if you're just File/Save As'ing to change filter options you
        // don't want to be reminded about overwriting files etc.
        bool justChangingFilterOptions = false;

        KoFileDialog *dialog = new KoFileDialog( (isExporting() && !d->m_lastExportURL.isEmpty() )? d->m_lastExportURL.url () : suggestedURL.url (),
                                                QString::null, this, "file dialog", true);

        if (!isExporting())
            dialog->setCaption( i18n("Save Document As") );
        else
            dialog->setCaption( i18n("Export Document As") );

        dialog->setOperationMode( KFileDialog::Saving );
        dialog->setMode(KFile::File);
        dialog->setSpecialMimeFilter( mimeFilter,
                                      isExporting() ? d->m_lastExportFormat : pDoc->mimeType(),
                                      isExporting() ? d->m_lastExportSpecialOutputFlag : oldSpecialOutputFlag,
                                      _native_format,
                                      pDoc->supportedSpecialFormats() );

        KUrl newURL;
        QByteArray outputFormat = _native_format;
        int specialOutputFlag = 0;
        bool bOk;
        do {
            bOk=true;
            if(dialog->exec()==QDialog::Accepted) {
                newURL=dialog->selectedURL();
                outputFormat=dialog->currentMimeFilter().toLatin1();
                specialOutputFlag = dialog->specialEntrySelected();
                kDebug(30003) << "KoMainWindow::saveDocument outputFormat = " << outputFormat << endl;

                if (!isExporting())
                    justChangingFilterOptions = (newURL == pDoc->url()) &&
                                                (outputFormat == pDoc->mimeType()) &&
                                                (specialOutputFlag == oldSpecialOutputFlag);
                else
                    justChangingFilterOptions = (newURL == d->m_lastExportURL) &&
                                                (outputFormat == d->m_lastExportFormat) &&
                                                (specialOutputFlag == d->m_lastExportSpecialOutputFlag);
            }
            else
            {
                bOk = false;
                break;
            }

            if ( newURL.isEmpty() )
            {
                bOk = false;
                break;
            }

            // this file exists and we are not just clicking "Save As" to change filter options
            // => ask for confirmation
            if ( KIO::NetAccess::exists( newURL, false /*will write*/, this ) && !justChangingFilterOptions )
            {
                bOk = KMessageBox::questionYesNo( this,
                                                  i18n("A document with this name already exists.\n"\
                                                       "Do you want to overwrite it?"),
                                                  i18n("Warning") ) == KMessageBox::Yes;
            }
        } while ( !bOk );

        delete dialog;

        if (bOk)
        {
            bool wantToSave = true;

            // don't change this line unless you know what you're doing :)
            if (!justChangingFilterOptions || pDoc->confirmNonNativeSave (isExporting ())) {
                if ( !pDoc->isNativeFormat( outputFormat ) )
                        wantToSave = exportConfirmation( outputFormat );
            }

            if (wantToSave)
            {
                //
                // Note:
                // If the user is stupid enough to Export to the current URL,
                // we do _not_ change this operation into a Save As.  Reasons
                // follow:
                //
                // 1. A check like "isExporting() && oldURL == newURL"
                //    doesn't _always_ work on case-insensitive filesystems
                //    and inconsistent behaviour is bad.
                // 2. It is probably not a good idea to change pDoc->mimeType
                //    and friends because the next time the user File/Save's,
                //    (not Save As) they won't be expecting that they are
                //    using their File/Export settings
                //
                // As a bad side-effect of this, the modified flag will not
                // be updated and it is possible that what is currently on
                // their screen is not what is stored on disk (through loss
                // of formatting).  But if you are dumb enough to change
                // mimetype but not the filename, then arguably, _you_ are
                // the "bug" :)
                //
                // - Clarence
                //


                pDoc->setOutputMimeType( outputFormat, specialOutputFlag );
                if (!isExporting ())   // Save As
                {
                    ret = pDoc->saveAs( newURL );

                    if (ret)
                    {
                        kDebug(30003) << "Successful Save As!" << endl;
                        addRecentURL( newURL );
                    }
                    else
                    {
                        kDebug(30003) << "Failed Save As!" << endl;
                        pDoc->setURL( oldURL ), pDoc->setFile( oldFile );
                        pDoc->setOutputMimeType( oldOutputFormat, oldSpecialOutputFlag );
                    }
                }
                else    // Export
                {
                    ret = pDoc->exp0rt( newURL );

                    if (ret)
                    {
                        // a few file dialog convenience things
                        d->m_lastExportURL = newURL;
                        d->m_lastExportFormat = outputFormat;
                        d->m_lastExportSpecialOutputFlag = specialOutputFlag;
                    }

                    // always restore output format
                    pDoc->setOutputMimeType( oldOutputFormat, oldSpecialOutputFlag );
                }

                if (silent) // don't let the document change the window caption
                  pDoc->setTitleModified();
            }   // if (wantToSave)  {
            else
                ret = false;
        }   // if (bOk) {
        else
            ret = false;
    }
    else {  // saving
        bool needConfirm = pDoc->confirmNonNativeSave( false ) &&
                           !pDoc->isNativeFormat( oldOutputFormat );
        if (!needConfirm ||
               (needConfirm && exportConfirmation( oldOutputFormat /* not so old :) */ ))
           )
        {
            // be sure pDoc has the correct outputMimeType!
            ret = pDoc->save();

            if (!ret)
            {
                kDebug(30003) << "Failed Save!" << endl;
                pDoc->setURL( oldURL ), pDoc->setFile( oldFile );
            }
        }
        else
            ret = false;
    }

// Now that there's a File/Export option, this is no longer necessary.
// If you continue to use File/Save to export to a foreign format,
// this signals your intention to continue working in a foreign format.
// You have already been warned by the DoNotAskAgain exportConfirmation
// about losing formatting when you first saved so don't set modified
// here or else it will be reported as a bug by some MSOffice user.
// You have been warned!  Do not click DoNotAskAgain!!!
#if 0
    if (ret && !isExporting())
    {
        // When exporting to a non-native format, we don't reset modified.
        // This way the user will be reminded to save it again in the native format,
        // if he/she doesn't want to lose formatting.
        if ( wasModified && pDoc->outputMimeType() != _native_format )
            pDoc->setModified( true );
    }
#endif

    if (!ret && reset_url)
        pDoc->resetURL(); //clean the suggested filename as the save dialog was rejected
    return ret;
}

void KoMainWindow::closeEvent(QCloseEvent *e) {
    if(queryClose()) {
        saveWindowSettings();
        setRootDocument(0L);
        KParts::MainWindow::closeEvent(e);
    }
}

void KoMainWindow::saveWindowSettings()
{
    if (d->m_windowSizeDirty && rootDocument())
    {
        // Save window size into the config file of our instance
        instance()->config()->setGroup( "MainWindow" );
        //kDebug(30003) << "KoMainWindow::saveWindowSettings" << endl;
        saveWindowSize( instance()->config() );
        d->m_windowSizeDirty = false;
        // Save toolbar position into the config file of the app, under the doc's instance name
        //kDebug(30003) << "KoMainWindow::closeEvent -> saveMainWindowSettings rootdoc's instance=" << rootDocument()->instance()->instanceName() << endl;
        saveMainWindowSettings( KGlobal::config(), rootDocument()->instance()->instanceName() );
        KGlobal::config()->sync();
        resetAutoSaveSettings(); // Don't let KMainWindow override the good stuff we wrote down
    }
}

void KoMainWindow::resizeEvent( QResizeEvent * e )
{
    d->m_windowSizeDirty = true;
    KParts::MainWindow::resizeEvent( e );
}

bool KoMainWindow::queryClose()
{
    if ( rootDocument() == 0 )
        return true;
    //kDebug(30003) << "KoMainWindow::queryClose() viewcount=" << rootDocument()->viewCount()
    //               << " shellcount=" << rootDocument()->shellCount() << endl;
    if ( !d->m_forQuit && rootDocument()->shellCount() > 1 )
        // there are more open, and we are closing just one, so no problem for closing
        return true;

    // see DTOR for a descr. of the test
    if ( d->m_rootDoc->isEmbedded() )
        return true;

    // main doc + internally stored child documents
    if ( d->m_rootDoc->isModified() )
    {
        QString name;
        if ( rootDocument()->documentInfo() )
        {
            name = rootDocument()->documentInfo()->title();
        }
        if ( name.isEmpty() )
            name = rootDocument()->url().fileName();

        if ( name.isEmpty() )
            name = i18n( "Untitled" );

        int res = KMessageBox::warningYesNoCancel( this,
                        i18n( "<p>The document <b>'%1'</b> has been modified.</p><p>Do you want to save it?</p>", name ),
                        QString::null,
                        KStdGuiItem::save(),
                        KStdGuiItem::discard());

        switch(res) {
            case KMessageBox::Yes : {
                d->m_rootDoc->setDoNotSaveExtDoc(); // external docs are saved later
                bool isNative = ( d->m_rootDoc->outputMimeType() == d->m_rootDoc->nativeFormatMimeType() );
                if (! saveDocument( !isNative ) )
                    return false;
                break;
            }
            case KMessageBox::No :
                rootDocument()->removeAutoSaveFiles();
                rootDocument()->setModified( false ); // Now when queryClose() is called by closeEvent it won't do anything.
                break;
            default : // case KMessageBox::Cancel :
                return false;
        }
    }

    if ( d->m_rootDoc->queryCloseExternalChildren() == KMessageBox::Cancel )
    {
        return false;
    }

    return true;
}

// Helper method for slotFileNew and slotFileClose
void KoMainWindow::chooseNewDocument( int /*KoDocument::InitDocFlags*/ initDocFlags )
{
    KoDocument* doc = rootDocument();
    KoDocument *newdoc = createDoc();

    if ( !newdoc )
        return;

    //FIXME: This needs to be handled differently
    connect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));

    if ( ( !doc  && ( initDocFlags == KoDocument::InitDocFileNew ) ) || ( doc && !doc->isEmpty() ) )
    {
        KoMainWindow *s = new KoMainWindow( newdoc->instance() );
        s->show();
        newdoc->addShell( s );
        newdoc->showStartUpWidget( s, true /*Always show widget*/ );
        return;
    }

    if( doc ) {
        setRootDocument( 0 );
        delete d->m_rootDoc;
        d->m_rootDoc = 0;
    }

    newdoc->addShell( this );
    newdoc->showStartUpWidget( this, true /*Always show widget*/ );
}

void KoMainWindow::slotFileNew()
{
    chooseNewDocument( KoDocument::InitDocFileNew );
}

void KoMainWindow::slotFileOpen()
{
    KFileDialog *dialog = new KFileDialog(":OpenDialog", QString::null, this);
    dialog->setObjectName( "file dialog" );
    dialog->setMode(KFile::File);
    if (!isImporting())
        dialog->setCaption( i18n("Open Document") );
    else
        dialog->setCaption( i18n("Import Document") );

    const QStringList mimeFilter = KoFilterManager::mimeFilter( KoDocument::readNativeFormatMimeType(),
                                                                KoFilterManager::Import,
                                                                KoDocument::readExtraNativeMimeTypes() );
    dialog->setMimeFilter( mimeFilter );
    if(dialog->exec()!=QDialog::Accepted) {
        delete dialog;
        return;
    }
    KUrl url( dialog->selectedURL() );
    delete dialog;

    if ( url.isEmpty() )
        return;

    (void) openDocument( url );
}

void KoMainWindow::slotFileOpenRecent( const KUrl & url )
{
    (void) openDocument( url );
}

void KoMainWindow::slotFileSave()
{
    if ( saveDocument() )
        emit documentSaved();
}

void KoMainWindow::slotFileSaveAs()
{
    if ( saveDocument( true ) )
        emit documentSaved();
}

void KoMainWindow::slotDocumentInfo()
{
  if ( !rootDocument() )
    return;

  KoDocumentInfo *docInfo = rootDocument()->documentInfo();

  if ( !docInfo )
    return;

  KoDocumentInfoDlg *dlg = new KoDocumentInfoDlg( docInfo, this, "documentInfoDlg" );
  if ( dlg->exec() )
  {
    dlg->save();
    rootDocument()->setModified( true );
    rootDocument()->setTitleModified();
  }

  delete dlg;
}

void KoMainWindow::slotFileClose()
{
    if (queryClose())
    {
        saveWindowSettings();
        setRootDocument( 0 ); // don't delete this shell when deleting the document
        delete d->m_rootDoc;
        d->m_rootDoc = 0;
        chooseNewDocument( KoDocument::InitDocFileClose );
    }
}

void KoMainWindow::slotFileQuit()
{
    close();
}

void KoMainWindow::print(bool quick) {
    if ( !rootView() )
    {
        kDebug(30003) << "KoMainWindow::slotFilePrint : No root view!" << endl;
        return;
    }

    KPrinter printer( true /*, QPrinter::HighResolution*/ );
    QString title = rootView()->koDocument()->documentInfo()->title();
    QString fileName = rootView()->koDocument()->url().fileName();

    // strip off the native extension (I don't want foobar.kwd.ps when printing into a file)
    KMimeType::Ptr mime = KMimeType::mimeType( rootView()->koDocument()->outputMimeType() );
    if ( mime ) {
        QString extension = mime->property( "X-KDE-NativeExtension" ).toString();

        if ( fileName.endsWith( extension ) )
            fileName.truncate( fileName.length() - extension.length() );
    }

    if ( title.isEmpty() )
        title = fileName;
    printer.setDocName( title );
    printer.setDocFileName( fileName );
    printer.setDocDirectory( rootView()->koDocument()->url().directory() );

    // ### TODO: apply global koffice settings here

    rootView()->setupPrinter( printer );

    if ( quick ||  printer.setup( this ) )
        rootView()->print( printer );
}


void KoMainWindow::slotFilePrint()
{
    print(false);
}

void KoMainWindow::slotFilePrintPreview()
{
    if ( !rootView() )
    {
        kWarning() << "KoMainWindow::slotFilePrint : No root view!" << endl;
        return;
    }
    KPrinter printer( false );
    KTempFile tmpFile;
    // The temp file is deleted by KoPrintPreview

    // This line has to be before setupPrinter to let the apps decide what to
    // print and what not (if they want to :)
    printer.setFromTo( printer.minPage(), printer.maxPage() );
    printer.setPreviewOnly( true );
    rootView()->setupPrinter( printer );

    QString oldFileName = printer.outputFileName();
    printer.setOutputFileName( tmpFile.name() );
    int oldNumCopies = printer.numCopies();
    printer.setNumCopies( 1 );
    // Disable kdeprint's own preview, we'd get two. This shows that KPrinter needs
    // a "don't use the previous settings" mode. The current way is really too much of a hack.
    QString oldKDEPreview = printer.option( "kde-preview" );
    printer.setOption( "kde-preview", "0" );

    rootView()->print(printer);
    //KoPrintPreview::preview(this, "KoPrintPreviewDialog", tmpFile.name());

    // Restore previous values
    printer.setOutputFileName( oldFileName );
    printer.setNumCopies( oldNumCopies );
    printer.setOption( "kde-preview", oldKDEPreview );
}

void KoMainWindow::slotConfigureKeys()
{
    guiFactory()->configureShortcuts();
}

void KoMainWindow::slotConfigureToolbars()
{
    if (rootDocument())
        saveMainWindowSettings( KGlobal::config(), rootDocument()->instance()->instanceName() );
    KEditToolbar edit(factory(), this);
    connect(&edit,SIGNAL(newToolbarConfig()),this,SLOT(slotNewToolbarConfig()));
    (void) edit.exec();
}

void KoMainWindow::slotNewToolbarConfig()
{
  if (rootDocument())
      applyMainWindowSettings( KGlobal::config(), rootDocument()->instance()->instanceName() );
  KXMLGUIFactory *factory = guiFactory();

  // Check if there's an active view
  if( !d->m_activeView )
      return;

  // This gets plugged in even for embedded views
  factory->plugActionList(d->m_activeView, "view_closeallviews",
                          d->m_veryHackyActionList);

  // This one only for root views
  if(d->m_rootViews.findRef(d->m_activeView)!=-1)
    factory->plugActionList(d->m_activeView, "view_split",
                            d->m_splitViewActionList );
  plugActionList( "toolbarlist", d->m_toolbarList );
}

void KoMainWindow::slotToolbarToggled( bool toggle )
{
  //kDebug(30003) << "KoMainWindow::slotToolbarToggled " << sender()->name() << " toggle=" << true << endl;
  // The action (sender) and the toolbar have the same name
  KToolBar * bar = toolBar( sender()->objectName() );
  if (bar)
  {
    if (toggle)
      bar->show();
    else
      bar->hide();

    if (rootDocument())
        saveMainWindowSettings( KGlobal::config(), rootDocument()->instance()->instanceName() );
  }
  else
    kWarning(30003) << "slotToolbarToggled : Toolbar " << sender()->objectName() << " not found!" << endl;
}

bool KoMainWindow::toolbarIsVisible(const char *tbName)
{
    QWidget *tb = toolBar( tbName);
    return !tb->isHidden();
}

void KoMainWindow::showToolbar( const char * tbName, bool shown )
{
    QWidget * tb = toolBar( tbName );
    if ( !tb )
    {
        kWarning(30003) << "KoMainWindow: toolbar " << tbName << " not found." << endl;
        return;
    }
    if ( shown )
        tb->show();
    else
        tb->hide();

    // Update the action appropriately
    foreach( KAction* action, d->m_toolbarList )
        if ( action->objectName() != tbName )
        {
            //kDebug(30003) << "KoMainWindow::showToolbar setChecked " << shown << endl;
            static_cast<KToggleAction *>(action)->setChecked( shown );
            break;
        }
}

void KoMainWindow::slotSplitView() {
    d->m_splitted=true;
    d->m_rootViews.append(d->m_rootDoc->createView(d->m_splitter, "splitted-view"));
    d->m_rootViews.current()->show();
    d->m_rootViews.current()->setPartManager( d->m_manager );
    d->m_manager->setActivePart( d->m_rootDoc, d->m_rootViews.current() );
    d->m_removeView->setEnabled(true);
    d->m_orientation->setEnabled(true);
}

void KoMainWindow::slotCloseAllViews() {

    // Attention: Very touchy code... you know what you're doing? Goooood :)
    d->m_forQuit=true;
    if(queryClose()) {
        // In case the document is embedded we close all open "extra-shells"
        if(d->m_rootDoc && d->m_rootDoc->isEmbedded()) {
            hide();
            d->m_rootDoc->removeShell(this);
            Q3PtrListIterator<KoMainWindow> it(d->m_rootDoc->shells());
            while (it.current()) {
                it.current()->hide();
                delete it.current(); // this updates the lists' current pointer and thus
                                     // the iterator (the shell dtor calls removeShell)
            d->m_rootDoc=0;
            }
        }
        // not embedded -> destroy the document and all shells/views ;)
        else
            setRootDocument( 0L );
        close();  // close this window (and quit the app if necessary)
    }
    d->m_forQuit=false;
}

void KoMainWindow::slotRemoveView() {
    KoView *view;
    if(d->m_rootViews.findRef(d->m_activeView)!=-1)
        view=d->m_rootViews.current();
    else
        view=d->m_rootViews.first();
    view->hide();
    if ( !d->m_rootViews.removeRef(view) )
        kWarning() << "view not found in d->m_rootViews!" << endl;

    if(d->m_rootViews.count()==1)
    {
        d->m_removeView->setEnabled(false);
        d->m_orientation->setEnabled(false);
    }
    // Prevent the view's destroyed() signal from triggering GUI rebuilding (too early)
    d->m_manager->setActivePart( 0, 0 );

    delete view;
    view=0L;

    d->m_rootViews.first()->setPartManager( d->m_manager );
    d->m_manager->setActivePart( d->m_rootDoc, d->m_rootViews.first() );

    if(d->m_rootViews.count()==1)
        d->m_splitted=false;
}

void KoMainWindow::slotSetOrientation() {
    d->m_splitter->setOrientation(static_cast<Qt::Orientation>
                                  (d->m_orientation->currentItem()));
}

void KoMainWindow::slotProgress(int value) {
    //kDebug(30003) << "KoMainWindow::slotProgress " << value << endl;
    if(value==-1) {
        if ( d->m_progress )
        {
            statusBar()->removeWidget(d->m_progress);
            delete d->m_progress;
            d->m_progress=0L;
        }
        d->m_firstTime=true;
        return;
    }
    if(d->m_firstTime)
    {
        // The statusbar might not even be created yet.
        // So check for that first, and create it if necessary
        QStatusBar* bar = qFindChild<QStatusBar *>( this );
        if ( !bar ) {
            statusBar()->show();
            QApplication::sendPostedEvents( this, QEvent::ChildAdded );
            // ######## KDE4 porting: removed this call:
            // setUpLayout();
        }

        if ( d->m_progress )
        {
            statusBar()->removeWidget(d->m_progress);
            delete d->m_progress;
            d->m_progress=0L;
        }
        statusBar()->setMaximumHeight(statusBar()->height());
        d->m_progress=new KProgressBar(statusBar());
        //d->m_progress->setMaximumHeight(statusBar()->height());
        statusBar()->addPermanentWidget( d->m_progress );
        d->m_progress->show();
        d->m_firstTime=false;
    }
    d->m_progress->setValue(value);
    kapp->processEvents();
}


void KoMainWindow::slotActivePartChanged( KParts::Part *newPart )
{

  // This looks very much like KParts::MainWindow::createGUI, but we have
  // to reimplement it because it works with an active part, whereas we work
  // with an active view _and_ an active part, depending for what.
  // Both are KXMLGUIClients, but e.g. the plugin query needs a QObject.
  //kDebug(30003) <<  "KoMainWindow::slotActivePartChanged( Part * newPart) newPart = " << newPart << endl;
  //kDebug(30003) <<  "current active part is " << d->m_activePart << endl;

  if ( d->m_activePart && d->m_activePart == newPart && !d->m_splitted )
  {
    //kDebug(30003) << "no need to change the GUI" << endl;
    return;
  }

  KXMLGUIFactory *factory = guiFactory();

  setUpdatesEnabled( false );

  if ( d->m_activeView )
  {
    KParts::GUIActivateEvent ev( false );
    QApplication::sendEvent( d->m_activePart, &ev );
    QApplication::sendEvent( d->m_activeView, &ev );


    factory->removeClient( d->m_activeView );

    unplugActionList( "toolbarlist" );
    qDeleteAll( d->m_toolbarList );
    d->m_toolbarList.clear();
  }

  if ( !d->bMainWindowGUIBuilt )
  {
    // Load mainwindow plugins
    KParts::Plugin::loadPlugins( this, this, instance(), true );
    createShellGUI();
  }

  if ( newPart && d->m_manager->activeWidget() && d->m_manager->activeWidget()->inherits( "KoView" ) )
  {
    d->m_activeView = (KoView *)d->m_manager->activeWidget();
    d->m_activePart = newPart;
    //kDebug(30003) <<  "new active part is " << d->m_activePart << endl;

    factory->addClient( d->m_activeView );


    // This gets plugged in even for embedded views
    factory->plugActionList(d->m_activeView, "view_closeallviews",
                            d->m_veryHackyActionList);
    // This one only for root views
    if(d->m_rootViews.findRef(d->m_activeView)!=-1)
        factory->plugActionList(d->m_activeView, "view_split", d->m_splitViewActionList );

    // Position and show toolbars according to user's preference
    setAutoSaveSettings( newPart->instance()->instanceName(), false );

    // Create and plug toolbar list for Settings menu
    //QPtrListIterator<KToolBar> it = toolBarIterator();
    foreach ( QWidget* it, factory->containers( "ToolBar" ) )
    {
      KToolBar * tb = ::qobject_cast<KToolBar *>(it);
      if ( tb )
      {
          KToggleAction * act = new KToggleAction( i18n("Show %1 Toolbar").arg( tb->windowTitle() ),
                                                   actionCollection(), tb->objectName().toUtf8() );
	  act->setCheckedState(i18n("Hide %1 Toolbar").arg( tb->windowTitle() ));
	  connect( act, SIGNAL( toggled( bool ) ), this, SLOT( slotToolbarToggled( bool ) ) );
          act->setChecked ( !tb->isHidden() );
          d->m_toolbarList.append( act );
      }
      else
          kWarning(30003) << "Toolbar list contains a " << it->metaObject()->className() << " which is not a toolbar!" << endl;
    }
    plugActionList( "toolbarlist", d->m_toolbarList );

    // Send the GUIActivateEvent only now, since it might show/hide toolbars too
    // (and this has priority over applyMainWindowSettings)
    KParts::GUIActivateEvent ev( true );
    QApplication::sendEvent( d->m_activePart, &ev );
    QApplication::sendEvent( d->m_activeView, &ev );
  }
  else
  {
    d->m_activeView = 0L;
    d->m_activePart = 0L;
  }
  setUpdatesEnabled( true );
}

QLabel * KoMainWindow::statusBarLabel()
{
  if ( !d->statusBarLabel )
  {
    d->statusBarLabel = new QLabel( statusBar() );
    statusBar()->addPermanentWidget( d->statusBarLabel, 1 );
  }
  return d->statusBarLabel;
}

void KoMainWindow::setMaxRecentItems(uint _number)
{
        m_recent->setMaxItems( _number );
}

DCOPObject * KoMainWindow::dcopObject()
{
    if ( !d->m_dcopObject )
    {
        d->m_dcopObject = new KoMainWindowIface( this );
    }

    return d->m_dcopObject;
}

void KoMainWindow::slotEmailFile()
{
    if (!rootDocument())
        return;

    // Subject = Document file name
    // Attachment = The current file
    // Message Body = The current document in HTML export? <-- This may be an option.
    QString theSubject;
    QStringList urls;
    QString fileURL;
    if (rootDocument()->url ().isEmpty () ||
        rootDocument()->isModified())
    {
        //Save the file as a temporary file
        bool const tmp_modified = rootDocument()->isModified();
        KUrl const tmp_url = rootDocument()->url();
        QByteArray const tmp_mimetype = rootDocument()->outputMimeType();
        KTempFile tmpfile; //TODO: The temorary file should be deleted when the mail program is closed
        KUrl u;
        u.setPath(tmpfile.name());
        rootDocument()->setURL(u);
        rootDocument()->setModified(true);
        rootDocument()->setOutputMimeType(rootDocument()->nativeFormatMimeType());

        saveDocument(false, true);

        fileURL = tmpfile.name();
        theSubject = i18n("Document");
        urls.append( fileURL );

        rootDocument()->setURL(tmp_url);
        rootDocument()->setModified(tmp_modified);
        rootDocument()->setOutputMimeType(tmp_mimetype);
    }
    else
    {
        fileURL = rootDocument()->url().url();
        theSubject = i18n("Document - %1").arg(rootDocument()->url().fileName(false));
        urls.append( fileURL );
    }

    kDebug(30003) << "(" << fileURL <<")" << endl;

    if (!fileURL.isEmpty())
    {
        KToolInvocation::invokeMailer(QString::null, QString::null, QString::null, theSubject,
                            QString::null, //body
                            QString::null,
                            urls); // attachments
    }
}

void KoMainWindow::slotVersionsFile()
{
    KoVersionDialog *dlg = new KoVersionDialog(  this );
    dlg->exec();
    delete dlg;
}

void KoMainWindow::slotReloadFile()
{
    KoDocument* pDoc = rootDocument();
    if(!pDoc || pDoc->url().isEmpty() || !pDoc->isModified())
        return;

    bool bOk = KMessageBox::questionYesNo( this,
                                      i18n("You will lose all your changes!\n"
                                           "Do you want to continue?"),
                                      i18n("Warning") ) == KMessageBox::Yes;
    if ( !bOk )
        return;

    KUrl url = pDoc->url();
    if ( pDoc && !pDoc->isEmpty() )
    {
        setRootDocument( 0L ); // don't delete this shell when deleting the document
        delete d->m_rootDoc;
        d->m_rootDoc = 0L;
    }
    openDocument( url );
    return;

}

void KoMainWindow::slotImportFile()
{
    kDebug(30003) << "slotImportFile()" << endl;

    d->m_isImporting = true;
    slotFileOpen();
    d->m_isImporting = false;
}

void KoMainWindow::slotExportFile()
{
    kDebug(30003) << "slotExportFile()" << endl;

    d->m_isExporting = true;
    slotFileSaveAs();
    d->m_isExporting = false;
}

bool KoMainWindow::isImporting() const
{
    return d->m_isImporting;
}

bool KoMainWindow::isExporting() const
{
    return d->m_isExporting;
}

void KoMainWindow::setDocToOpen( KoDocument *doc )
{
  d->m_docToOpen = doc;
}

#include "KoMainWindow.moc"
