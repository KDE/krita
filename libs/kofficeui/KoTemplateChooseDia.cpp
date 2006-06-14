/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   2000, 2001 Werner Trobin <trobin@kde.org>
   2002, 2003 Thomas Nagy <tnagy@eleve.emn.fr>
   2004 David Faure <faure@kde.org>

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

// Description: Template Choose Dialog

/******************************************************************/

#include "KoTemplateChooseDia.h"

#include <kdialog.h>
#include <klocale.h>
#include <kdeversion.h>
#include <kfiledialog.h>
#include <kinstance.h>
#include <KoFilterManager.h>
#include <KoTemplates.h>
#include <KoDocument.h>
#include <kmainwindow.h>

#include <kdebug.h>
#include <kpushbutton.h>
#include <kglobalsettings.h>

#include <kfileiconview.h>
#include <kfileitem.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kaboutdata.h>

#include <QApplication>
#include <QLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPoint>
#include <QObject>
#include <QDesktopWidget>
#include <QToolTip>
#include <QTextEdit>
#include <QByteArray>
#include <QHideEvent>
#include <QGridLayout>
#include <Q3Frame>
#include <QLabel>
#include <QGroupBox>

class MyFileDialog : public KFileDialog
{
    public :
        MyFileDialog(
                const QString& startDir=0,
                const QString& filter =0,
                QWidget *parent=0,
                const char *name=0,
                bool modal=0)
            :  KFileDialog (startDir, filter, parent),
        m_slotOkCalled( false ) {}

        KUrl currentURL()
        {
            setResult( QDialog::Accepted ); // selectedURL tests for it
            return KFileDialog::selectedURL();
        }

        // Return true if the current URL exists, show msg box if not
        bool checkURL()
        {
            bool ok = true;
            KUrl url = currentURL();
            if ( url.isLocalFile() )
            {
                ok = QFile::exists( url.path() );
                if ( !ok ) {
                    // Maybe offer to create a new document with that name? (see alos KoDocument::openFile)
                    KMessageBox::error( this, i18n( "The file %1 does not exist." ,url.path() ) );
                }
            }
            return ok;
        }
        // Called directly by pressing Return in the location combo
        // (so we need to remember that it got called, to avoid calling it twice)
        // Called "by hand" when clicking on our OK button
        void slotOk() {
            m_slotOkCalled = true;
            KFileDialog::slotOk();
        }
        bool slotOkCalled() const { return m_slotOkCalled; }
    protected:
    // Typing a file that doesn't exist closes the file dialog, we have to
    // handle this case better here.
        virtual void accept() {
            if ( checkURL() )
                KFileDialog::accept();
        }

        virtual void reject() {
		KFileDialog::reject();
		emit cancelClicked();
        }
private:
        bool m_slotOkCalled;
};

/*================================================================*/

/*================================================================*/

class KoTemplateChooseDiaPrivate {
    public:
	KoTemplateChooseDiaPrivate(const QByteArray& templateType, KInstance* instance,
                                   const QByteArray &format,
                                   const QString &nativeName,
                                   const QStringList& extraNativeMimeTypes,
                                   const KoTemplateChooseDia::DialogType &dialogType) :
	    m_templateType(templateType), m_instance(instance), m_format(format),
            m_nativeName(nativeName), m_extraNativeMimeTypes( extraNativeMimeTypes ),
            m_dialogType(dialogType), tree(0),
            m_nostartupdlg( false ),
            m_mainwidget(0), m_nodiag( 0 )
	{
	    m_returnType = KoTemplateChooseDia::Empty;
	}

	~KoTemplateChooseDiaPrivate() {}

	QByteArray m_templateType;
	KInstance* m_instance;
	QByteArray m_format;
	QString m_nativeName;
        QStringList m_extraNativeMimeTypes;

        KoTemplateChooseDia::DialogType m_dialogType;
	KoTemplateTree *tree;

	QString m_templateName;
	QString m_fullTemplateName;
	KoTemplateChooseDia::ReturnType m_returnType;

	bool m_nostartupdlg;

	// the main widget
	QWidget *m_mainwidget;

	// do not show this dialog at startup
	QCheckBox *m_nodiag;

	// choose a template
	KPageDialog * m_jwidget;
	KFileIconView *m_recent;
	QGroupBox * boxdescription;
	QTextEdit * textedit;

	// choose a file
	MyFileDialog *m_filedialog;

	// for the layout
	QTabWidget* tabWidget;
	QWidget* newTab;
	QWidget* existingTab;
	QWidget* recentTab;

};

/******************************************************************/
/* Class: KoTemplateChooseDia                                     */
/******************************************************************/

/*================================================================*/
KoTemplateChooseDia::KoTemplateChooseDia(QWidget *parent, const char *name, KInstance* instance,
                                         const QByteArray &format,
                                         const QString &nativeName,
                                         const QStringList &extraNativeMimeTypes,
                                         const DialogType &dialogType,
                                         const QByteArray& templateType) :
    KPageDialog( parent )
{

    setModal( true );
    setCaption( i18n("Open Document") );
    setButtons( KDialog::Ok | KDialog::Cancel );
    setDefaultButton( KDialog::Ok );
    setObjectName( name );

    d = new KoTemplateChooseDiaPrivate(
        templateType,
        instance,
        format,
        nativeName,
        extraNativeMimeTypes,
        dialogType);

//     QPushButton* ok = actionButton( KDialog::Ok );
//     QPushButton* cancel = actionButton( KDialog::Cancel );
//     cancel->setAutoDefault(false);
//     ok->setDefault(true);
//  //enableButtonOK(false);
    if (!templateType.isNull() && !templateType.isEmpty() && dialogType!=NoTemplates)
        d->tree = new KoTemplateTree(templateType, instance, true);

    d->m_mainwidget = new QWidget();
    setMainWidget( d->m_mainwidget ); 

    d->m_templateName = "";
    d->m_fullTemplateName = "";
    d->m_returnType = Cancel;

    setupDialog();
}

KoTemplateChooseDia::~KoTemplateChooseDia()
{
    delete d->tree;
    delete d;
    d=0L;
}

// Keep in sync with KoMainWindow::chooseNewDocument
static bool cancelQuits() {
    bool onlyDoc = !KoDocument::documentList() || KoDocument::documentList()->count() <= 1;
    bool onlyMainWindow = KMainWindow::memberList().count() <= 1;
    return onlyDoc && onlyMainWindow && kapp->instanceName() != "koshell"; // hack for koshell
}

KoTemplateChooseDia::ReturnType KoTemplateChooseDia::choose(KInstance* instance, QString &file,
                                                            const KoTemplateChooseDia::DialogType &dialogType,
                                                            const QByteArray& templateType,
                                                            QWidget* parent)
{
    const QString nativeName = instance->aboutData()->programName();
    const QByteArray format = KoDocument::readNativeFormatMimeType( instance );
    const QStringList extraNativeMimeTypes = KoDocument::readExtraNativeMimeTypes( instance );
    // Maybe the above two can be combined into one call, for speed:
    //KoDocument::getNativeMimeTypeInfo( instance, nativeName, extraNativeMimeTypes );
    return choose( instance, file, format, nativeName, extraNativeMimeTypes,
                   dialogType, templateType, parent );
}

KoTemplateChooseDia::ReturnType KoTemplateChooseDia::choose(KInstance* instance, QString &file,
                                       const QByteArray &format,
                                       const QString &nativeName,
                                       const QStringList& extraNativeMimeTypes,
                                       const DialogType &dialogType,
                                       const QByteArray& templateType,
                                       QWidget* parent )
{
    KoTemplateChooseDia *dlg = new KoTemplateChooseDia(
        parent, "Choose", instance, format,
        nativeName, extraNativeMimeTypes, dialogType, templateType );

    KoTemplateChooseDia::ReturnType rt = Cancel;

    if (dlg->noStartupDlg())
    {
	// start with the default template
	file = dlg->getFullTemplate();
	rt = dlg->getReturnType();
    }
    else
    {
	dlg->resize( 700, 480 );
	if ( dlg->exec() == QDialog::Accepted )
	{
	    file = dlg->getFullTemplate();
	    rt = dlg->getReturnType();
	}
    }

    delete dlg;
    return rt;
}

bool KoTemplateChooseDia::noStartupDlg() const {
    return d->m_nostartupdlg;
}


QString KoTemplateChooseDia::getTemplate() const{
    return d->m_templateName;
}

QString KoTemplateChooseDia::getFullTemplate() const{
    return d->m_fullTemplateName;
}

KoTemplateChooseDia::ReturnType KoTemplateChooseDia::getReturnType() const {
    return d->m_returnType;
}

KoTemplateChooseDia::DialogType KoTemplateChooseDia::getDialogType() const {
    return d->m_dialogType;
}

/*================================================================*/
// private
void KoTemplateChooseDia::setupRecentDialog(QWidget * widgetbase, QGridLayout * layout)
{

        d->m_recent = new KoTCDRecentFilesIconView(widgetbase, "recent files");
        // I prefer the icons to be in "most recent first" order (DF)
        d->m_recent->setSorting( static_cast<QDir::SortFlags>( QDir::Time | QDir::Reversed ) );
        layout->addWidget(d->m_recent,0,0);

        QString oldGroup = d->m_instance->config()->group();
        d->m_instance->config()->setGroup( "RecentFiles" );

        int i = 0;
        QString value;
        do {
                QString key=QString( "File%1" ).arg( i );
                value=d->m_instance->config()->readPathEntry( key );
                if ( !value.isEmpty() ) {
                    // Support for kdelibs-3.5's new RecentFiles format: name[url]
                    QString s = value;
                    if ( s.endsWith("]") )
                    {
                        int pos = s.indexOf("[");
                        s = s.mid( pos + 1, s.length() - pos - 2);
                    }
                    KUrl url(s);

                    if(!url.isLocalFile() || QFile::exists(url.path())) {
                        KFileItem *item = new KFileItem( KFileItem::Unknown, KFileItem::Unknown, url );
                        d->m_recent->insertItem(item);
                    }
                }
                i++;
        } while ( !value.isEmpty() || i<=10 );

        d->m_instance->config()->setGroup( oldGroup );
        d->m_recent->showPreviews();

	connect(d->m_recent, SIGNAL( doubleClicked ( Q3IconViewItem * ) ),
			this, SLOT( recentSelected( Q3IconViewItem * ) ) );

}

/*================================================================*/
// private
void KoTemplateChooseDia::setupFileDialog(QWidget * widgetbase, QGridLayout * layout)
{
    QString dir = QString::null;
    QPoint point( 0, 0 );

    d->m_filedialog=new MyFileDialog(dir,
	    QString::null,
	    widgetbase,
	    "file dialog",
	    false);

    layout->addWidget(d->m_filedialog,0,0);
    d->m_filedialog->setParent( widgetbase );
    //d->m_filedialog->setOperationMode( KFileDialog::Opening);

    const QList<QPushButton *> buttons = qFindChildren<QPushButton *>( d->m_filedialog );
    foreach( QPushButton* button, buttons )
        button->hide();

    d->m_filedialog->setSizeGripEnabled ( FALSE );

    const QStringList mimeFilter = KoFilterManager::mimeFilter( d->m_format,
                                                                KoFilterManager::Import,
                                                                d->m_extraNativeMimeTypes );
    d->m_filedialog->setMimeFilter( mimeFilter );

    connect(d->m_filedialog, SIGNAL(  okClicked() ),
	    this, SLOT (  slotOk() ));

    connect(d->m_filedialog, SIGNAL( cancelClicked() ),
	    this, SLOT (  slotCancel() ));

}

/*================================================================*/
// private
void KoTemplateChooseDia::setupTemplateDialog(QWidget * widgetbase, QGridLayout * layout)
{

    d->m_jwidget = new KPageDialog( widgetbase );
    d->m_jwidget->setFaceType( KPageDialog::List );
    layout->addWidget(d->m_jwidget,0,0);

    d->boxdescription = new QGroupBox(
	    i18n("Selected Template"),
	    widgetbase );
    layout->addWidget(d->boxdescription, 1, 0 );

    // config
    KConfigGroup grp( d->m_instance->config(), "TemplateChooserDialog" );
    int templateNum = grp.readEntry( "TemplateTab", -1 );
    QString templateName = grp.readPathEntry( "TemplateName" );
	if ( templateName.isEmpty() && d->tree->defaultTemplate() )
		templateName = d->tree->defaultTemplate()->name(); //select the default template for the app

    // item which will be selected initially
    Q3IconViewItem * itemtoselect = 0;

    // count the templates inserted
    int entriesnumber = 0;
	int defaultTemplateGroup = -1;

    for ( KoTemplateGroup *group = d->tree->first(); group!=0L; group=d->tree->next() )
    {
	if (group->isHidden())
	    continue;

	if ( d->tree->defaultGroup() == group )
		defaultTemplateGroup = entriesnumber; //select the default template group for the app

    QFrame * frame = new QFrame();
    KPageWidgetItem * item = d->m_jwidget->addPage ( frame, group->name() );
    item->setIcon( group->first()->loadPicture(d->m_instance) );
    item->setHeader( group->name() );

	QGridLayout* layout = new QGridLayout(frame);
	KoTCDIconCanvas *canvas = new KoTCDIconCanvas( frame );
	layout->addWidget(canvas,0,0);

	canvas->setBackgroundRole( QPalette::Base );
	canvas->setResizeMode(Q3IconView::Adjust);
	canvas->setWordWrapIconText( true );
	canvas->show();

	Q3IconViewItem * tempitem = canvas->load(group, templateName, d->m_instance);
	if (tempitem)
	    itemtoselect = tempitem;

	canvas->sort();
	canvas->setSelectionMode(Q3IconView::Single);

	connect( canvas, SIGNAL( clicked ( Q3IconViewItem * ) ),
		this, SLOT( currentChanged( Q3IconViewItem * ) ) );

	connect( canvas, SIGNAL( doubleClicked( Q3IconViewItem * ) ),
		this, SLOT( chosen(Q3IconViewItem *) ) );

	entriesnumber++;
    }

    // Disabled during Qt4 porting. Is this needed? Maybe set margin/spacing on layout?
    //d->boxdescription->setInsideMargin ( 3 );
    //d->boxdescription->setInsideSpacing ( 3 );

    d->textedit = new QTextEdit( d->boxdescription );
    d->textedit->setReadOnly(1);
    d->textedit->setPlainText(descriptionText(i18n("Empty Document"), i18n("Creates an empty document")));
    d->textedit->setLineWidth(0);
    d->textedit->setMaximumHeight(50);

    // Hide the widget if there is no template available. This should never happen ;-)
    if (!entriesnumber)
	d->m_jwidget->hide();

// FIXME: Port this!
//  Set the initially shown page, possibly from the last usage of the dialog
//     if (entriesnumber >= templateNum && templateNum != -1 )
// 	d->m_jwidget->showPage(templateNum);
//     else if ( defaultTemplateGroup != -1)
// 	d->m_jwidget->showPage(defaultTemplateGroup);


    // Set the initially selected template, possibly from the last usage of the dialog
    currentChanged(itemtoselect);

    // setup the checkbox
    QString translatedstring = i18n("Always start %1 with the selected template",d->m_nativeName);

    d->m_nodiag = new QCheckBox ( translatedstring , widgetbase);
    layout->addWidget(d->m_nodiag, 2, 0);
    QString  startwithoutdialog = grp.readEntry( "NoStartDlg" );
    bool ischecked = startwithoutdialog == QString("yes");

    // When not starting up, display a tri-state button telling whether
    // the user actually choosed the template to start with next times (bug:77542)
    if (d->m_dialogType == Everything)
    {
        d->m_nodiag->setChecked( ischecked );
    }
    else
    {
        d->m_nodiag->setTristate();
        d->m_nodiag->setCheckState( Qt::PartiallyChecked );
    }
}

/*================================================================*/
// private
void KoTemplateChooseDia::setupDialog()
{

    QGridLayout *maingrid = new QGridLayout( d->m_mainwidget );
    maingrid->setMargin( 2 );
    maingrid->setSpacing( 6 );
    KConfigGroup grp( d->m_instance->config(), "TemplateChooserDialog" );

    if (d->m_dialogType == Everything)
    {

	// the user may want to start with his favorite template
	if (grp.readEntry( "NoStartDlg" ) == QString("yes") )
	{
	    d->m_nostartupdlg = true;
	    d->m_returnType = Empty;

	    // no default template, just start with an empty document
	    if (grp.readEntry("LastReturnType") == QString("Empty") )
		return;

	    // start with the default template
	    d->m_templateName = grp.readPathEntry( "TemplateName" );
	    d->m_fullTemplateName = grp.readPathEntry( "FullTemplateName" );

	    // be paranoid : invalid template means empty template
	    if (!QFile::exists(d->m_fullTemplateName))
		return;

	    if (d->m_fullTemplateName.length() < 2)
		return;

	    d->m_returnType = Template;
	    return;
	}

	if ( cancelQuits() )
	    setButtonGuiItem( KDialog::Cancel, KStdGuiItem::quit() ); 

	d->tabWidget = new QTabWidget( d->m_mainwidget );
	maingrid->addWidget( d->tabWidget, 0, 0 );

	// new document
	d->newTab = new QWidget( d->tabWidget );
	d->tabWidget->addTab( d->newTab, i18n( "&Create Document" ) );
	QGridLayout * newTabLayout = new QGridLayout( d->newTab );
	newTabLayout->setMargin( KDialog::marginHint() );
	newTabLayout->setSpacing( KDialog::spacingHint() );

	// existing document
	d->existingTab = new QWidget( d->tabWidget );
	d->tabWidget->addTab( d->existingTab, i18n( "Open &Existing Document" ) );
	QGridLayout * existingTabLayout = new QGridLayout( d->existingTab );
	existingTabLayout->setSpacing( KDialog::spacingHint() );

        // recent document
        d->recentTab = new QWidget( d->tabWidget );
        d->tabWidget->addTab( d->recentTab, i18n( "Open &Recent Document" ) );
        QGridLayout * recentTabLayout = new QGridLayout( d->recentTab );
	recentTabLayout->setMargin( KDialog::marginHint() );
	recentTabLayout->setSpacing( KDialog::spacingHint() );

	setupTemplateDialog(d->newTab, newTabLayout);
	setupFileDialog(d->existingTab, existingTabLayout);
	setupRecentDialog(d->recentTab, recentTabLayout);

	QString tabhighlighted = grp.readEntry("LastReturnType");
	if ( tabhighlighted == "Template" )
	    d->tabWidget->setCurrentIndex(0); // CreateDocument tab
	else if (tabhighlighted == "File" )
	    d->tabWidget->setCurrentIndex(2); // RecentDocument tab
	else
		d->tabWidget->setCurrentIndex(0); // Default setting: CreateDocument tab
    }
    else
    {

	// open a file
	if (d->m_dialogType == NoTemplates)
	{
	    setupFileDialog(d->m_mainwidget, maingrid);
	}
	// create a new document from a template
	if (d->m_dialogType == OnlyTemplates)
	{
	    setCaption(i18n( "Create Document" ));
	    setupTemplateDialog(d->m_mainwidget, maingrid);
	}
    }
}

/*================================================================*/
// private SLOT
void KoTemplateChooseDia::currentChanged( Q3IconViewItem * item)
{
    if (item)
    {
	Q3IconView* canvas =  item->iconView();

	// set text in the textarea
	d->textedit->setPlainText( descriptionText(
				item->text(),
				((KoTCDIconViewItem *) item)->getDescr()
				));

	// set the icon in the canvas selected
	if (canvas)
	    canvas->setSelected(item,1,0);

	// register the current template
	d->m_templateName = item->text();
	d->m_fullTemplateName = ((KoTCDIconViewItem *) item)->getFName();
    }
}

/*================================================================*/
// private SLOT
void KoTemplateChooseDia::chosen(Q3IconViewItem * item)
{
    // the user double clicked on a template
    if (item)
    {
	currentChanged(item);
	slotOk();
    }
}

/* */
// private SLOT
void KoTemplateChooseDia::recentSelected( Q3IconViewItem * item)
{
	if (item)
	{
		slotOk();
	}
}

/*================================================================*/
// protected SLOT
void KoTemplateChooseDia::slotOk()
{
    // Collect info from the dialog into d->m_returnType and d->m_templateName etc.
    if (collectInfo())
    {
	// Save it for the next time
	KConfigGroup grp( d->m_instance->config(), "TemplateChooserDialog" );
	static const char* const s_returnTypes[] = { 0 /*Cancel ;)*/, "Template", "File", "Empty" };
	if ( d->m_returnType <= Empty )
	{
	    grp.writeEntry( "LastReturnType", QString::fromLatin1(s_returnTypes[d->m_returnType]) );
	    if (d->m_returnType == Template)
	    {
// TODO: Port this!
// 		grp.writeEntry( "TemplateTab", d->m_jwidget->activePageIndex() );
// 		grp.writePathEntry( "TemplateName", d->m_templateName );
// 		grp.writePathEntry( "FullTemplateName", d->m_fullTemplateName);
	    }

	    if (d->m_nodiag)
	    {
		// The checkbox m_nodiag is in tri-state mode for new documents
		// fixes bug:77542
		if (d->m_nodiag->checkState() == Qt::Checked) {
		    grp.writeEntry( "NoStartDlg", "yes");
		}
		else if (d->m_nodiag->checkState() == Qt::Unchecked) {
		    grp.writeEntry( "NoStartDlg", "no");
		}
	    }
	}
	else
	{
	    kWarning(30003) << "Unsupported template chooser result: " << d->m_returnType << endl;
	    grp.writeEntry( "LastReturnType", QString() );
	}
    slotButtonClicked( KDialog::Ok );
    }
}

/*================================================================*/
// private
bool KoTemplateChooseDia::collectInfo()
{


    // to determine what tab is selected in "Everything" mode
    bool newTabSelected = false;
    if ( d->m_dialogType == Everything)
	if ( d->tabWidget->currentWidget() == d->newTab )
	    newTabSelected = true;

    // is it a template or a file ?
    if ( d->m_dialogType==OnlyTemplates || newTabSelected )
    {
	// a template is chosen
	if (d->m_templateName.length() > 0)
	    d->m_returnType = Template;
	else
	    d->m_returnType=Empty;

	return true;
    }
    else if ( d->m_dialogType != OnlyTemplates )
    {
	// a file is chosen
	if (d->m_dialogType == Everything && d->tabWidget->currentWidget() == d->recentTab)
	{
		// Recent file
		KFileItem * item = d->m_recent->currentFileItem();
		if (! item)
			return false;
		KUrl url = item->url();
		if(url.isLocalFile() && !QFile::exists(url.path()))
		{
			KMessageBox::error( this, i18n( "The file %1 does not exist." , url.path() ) );
			return false;
		}
		d->m_fullTemplateName = url.url();
		d->m_returnType = File;
	}
	else
	{
		// Existing file from file dialog
	        if ( !d->m_filedialog->slotOkCalled() )
	            d->m_filedialog->slotOk();
		KUrl url = d->m_filedialog->currentURL();
		d->m_fullTemplateName = url.url();
	        d->m_returnType = File;
	        return d->m_filedialog->checkURL();
	}
	return true;
    }

    d->m_returnType=Empty;
    return false;
}

/*================================================================*/
//private
QString KoTemplateChooseDia::descriptionText(const QString &name, const QString &description)
{
	QString descrText(i18n("Name:"));
	descrText += " " + name;
	descrText += "\n";
	descrText += i18n("Description:");
	if (description.isEmpty())
	      descrText += " " + i18n("No description available");
	else
              descrText += " " + description;
	return descrText;
}

/*================================================================*/

Q3IconViewItem * KoTCDIconCanvas::load( KoTemplateGroup *group, const QString& name, KInstance* instance )
{
    Q3IconViewItem * itemtoreturn = 0;

    for (KoTemplate *t=group->first(); t!=0L; t=group->next()) {
	if (t->isHidden())
	    continue;
	Q3IconViewItem *item = new KoTCDIconViewItem(
		this,
		t->name(),
		t->loadPicture(instance),
		t->description(),
		t->file());

	if (name == t->name())
	{
	    itemtoreturn = item;
	}

	item->setKey(t->name());
	item->setDragEnabled(false);
	item->setDropEnabled(false);
    }

    return itemtoreturn;
}

/*================================================================*/

KoTCDRecentFilesIconView::~KoTCDRecentFilesIconView()
{
    removeToolTip();
}

void KoTCDRecentFilesIconView::showToolTip( Q3IconViewItem* item )
{
    removeToolTip();
    if ( !item )
        return;

    // Mostly duplicated from KFileIconView, because it only shows tooltips
    // for truncated icon texts, and we want tooltips on all icons,
    // with the full path...
    const KFileItem *fi = ( (KFileIconViewItem*)item )->fileInfo();
    QString toolTipText = fi->url().pathOrUrl( );
#if 0 // qt3 code
    toolTip = new QLabel( QString::fromLatin1(" %1 ").arg(toolTipText), 0,
                          "myToolTip",
                          WStyle_StaysOnTop | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WX11BypassWM );
    toolTip->setFrameStyle( Q3Frame::Plain | Q3Frame::Box );
    toolTip->setLineWidth( 1 );
    toolTip->setAlignment( Qt::AlignLeft | Qt::AlignTop );
    toolTip->move( QCursor::pos() + QPoint( 14, 14 ) );
    toolTip->adjustSize();
    QRect screen = QApplication::desktop()->screenGeometry(
        QApplication::desktop()->screenNumber(QCursor::pos()));
    if (toolTip->x()+toolTip->width() > screen.right()) {
        toolTip->move(toolTip->x()+screen.right()-toolTip->x()-toolTip->width(), toolTip->y());
    }
    if (toolTip->y()+toolTip->height() > screen.bottom()) {
        toolTip->move(toolTip->x(), screen.bottom()-toolTip->y()-toolTip->height()+toolTip->y());
    }
    toolTip->setFont( QToolTip::font() );
    toolTip->setPalette( QToolTip::palette(), TRUE );
    toolTip->show();
#endif

    QToolTip::showText(QCursor::pos() + QPoint( 14, 14 ), QString::fromLatin1(" %1 ").arg(toolTipText), this);

}

void KoTCDRecentFilesIconView::removeToolTip()
{
    delete toolTip;
    toolTip = 0;
}

void KoTCDRecentFilesIconView::hideEvent( QHideEvent *ev )
{
    removeToolTip();
    KFileIconView::hideEvent( ev );
}

#include "KoTemplateChooseDia.moc"
