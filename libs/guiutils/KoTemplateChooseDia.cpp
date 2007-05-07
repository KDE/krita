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
#include <kcomponentdata.h>
#include <KoFilterManager.h>
#include <KoTemplates.h>
#include <KoDocument.h>
#include <kxmlguiwindow.h>

#include <kdebug.h>
#include <kpushbutton.h>
#include <kglobalsettings.h>
#include <kconfiggroup.h>

#include <kfileitem.h>
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <kpagewidgetmodel.h>
#include <kicon.h>

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
#include <QFrame>
#include <QLabel>
#include <QGroupBox>
#include <QKeyEvent>

class MyFileDialog : public KFileDialog
{
    public :
        MyFileDialog(
                const QString& startDir=0,
                const QString& filter =0,
                QWidget *parent=0)
            :  KFileDialog (startDir, filter, parent),
        m_slotOkCalled( false ) {}

        KUrl currentURL()
        {
            setResult( QDialog::Accepted ); // selectedURL tests for it
            return KFileDialog::selectedUrl();
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
	KoTemplateChooseDiaPrivate(const QByteArray& templateType, const KComponentData &componentData,
                                   const QByteArray &format,
                                   const QString &nativeName,
                                   const QStringList& extraNativeMimeTypes,
                                   const KoTemplateChooseDia::DialogType &dialogType) :
	    m_templateType(templateType), m_componentData(componentData), m_format(format),
            m_nativeName(nativeName), m_extraNativeMimeTypes( extraNativeMimeTypes ),
            m_dialogType(dialogType), tree(0),
            m_mainwidget(0)
	{
	    m_returnType = KoTemplateChooseDia::Empty;
	}

	~KoTemplateChooseDiaPrivate() {}

	QByteArray m_templateType;
	KComponentData m_componentData;
	QByteArray m_format;
	QString m_nativeName;
        QStringList m_extraNativeMimeTypes;

        KoTemplateChooseDia::DialogType m_dialogType;
	KoTemplateTree *tree;

	QString m_templateName;
	QString m_fullTemplateName;
	KoTemplateChooseDia::ReturnType m_returnType;

	// the main widget
	QWidget *m_mainwidget;

	// choose a template
	KPageDialog * m_jwidget;
	QGroupBox * boxdescription;
	QTextEdit * textedit;

	// choose a file
	MyFileDialog *m_filedialog;

	// for the layout
	QTabWidget* tabWidget;
	QWidget* newTab;
	QWidget* existingTab;

};

/******************************************************************/
/* Class: KoTemplateChooseDia                                     */
/******************************************************************/

/*================================================================*/
KoTemplateChooseDia::KoTemplateChooseDia(QWidget *parent, const char *name, const KComponentData &componentData,
                                         const QByteArray &format,
                                         const QString &nativeName,
                                         const QStringList &extraNativeMimeTypes,
                                         const DialogType &dialogType,
                                         const QByteArray& templateType) :
    KPageDialog( parent )
    , d( new KoTemplateChooseDiaPrivate( templateType,
                                         componentData,
                                         format,
                                         nativeName,
                                         extraNativeMimeTypes,
                                         dialogType) )

{

    setModal( true );
    setCaption( i18n("Open Document") );
    setButtons( KDialog::Ok | KDialog::Cancel );
    setDefaultButton( KDialog::Ok );
    setObjectName( name );

//     QPushButton* ok = actionButton( KDialog::Ok );
//     QPushButton* cancel = actionButton( KDialog::Cancel );
//     cancel->setAutoDefault(false);
//     ok->setDefault(true);
//  //enableButtonOk(false);
    if (!templateType.isNull() && !templateType.isEmpty())
        d->tree = new KoTemplateTree(templateType, componentData, true);

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
}

KoTemplateChooseDia::ReturnType KoTemplateChooseDia::choose(const KComponentData &componentData, QString &file,
                                                            const KoTemplateChooseDia::DialogType &dialogType,
                                                            const QByteArray& templateType,
                                                            QWidget* parent)
{
    const QString nativeName = componentData.aboutData()->programName();
    const QByteArray format = KoDocument::readNativeFormatMimeType( componentData );
    const QStringList extraNativeMimeTypes = KoDocument::readExtraNativeMimeTypes( componentData );
    // Maybe the above two can be combined into one call, for speed:
    //KoDocument::getNativeMimeTypeInfo( componentData, nativeName, extraNativeMimeTypes );
    return choose( componentData, file, format, nativeName, extraNativeMimeTypes,
                   dialogType, templateType, parent );
}

KoTemplateChooseDia::ReturnType KoTemplateChooseDia::choose(const KComponentData &componentData, QString &file,
                                       const QByteArray &format,
                                       const QString &nativeName,
                                       const QStringList& extraNativeMimeTypes,
                                       const DialogType &dialogType,
                                       const QByteArray& templateType,
                                       QWidget* parent )
{
    KoTemplateChooseDia dlg(
        parent, "Choose", componentData, format,
        nativeName, extraNativeMimeTypes, dialogType, templateType );

    KoTemplateChooseDia::ReturnType rt = Cancel;

    dlg.resize( 700, 480 );
    if ( dlg.exec() == QDialog::Accepted )
    {
        file = dlg.getFullTemplate();
        rt = dlg.getReturnType();
    }

    return rt;
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
    KConfigGroup grp( d->m_componentData.config(), "TemplateChooserDialog" );
    int templateNum = grp.readEntry( "TemplateTab", -1 );
    QString templateName = grp.readPathEntry( "TemplateName" );
    if ( templateName.isEmpty() && d->tree->defaultTemplate() )
        templateName = d->tree->defaultTemplate()->name(); //select the default template for the app

    // item which will be selected initially
    QListWidgetItem * itemtoselect = 0;

    // count the templates inserted
    int entriesnumber = 0;
    int defaultTemplateGroup = -1;

    QList<KPageWidgetItem*> pageWidgetItems;

    for ( KoTemplateGroup *group = d->tree->first(); group!=0L; group=d->tree->next() )
    {
	if (group->isHidden())
	    continue;

	if ( d->tree->defaultGroup() == group )
            defaultTemplateGroup = entriesnumber; //select the default template group for the app

        QFrame * frame = new QFrame();
        KPageWidgetItem * item = d->m_jwidget->addPage ( frame, group->name() );
        item->setIcon( KIcon(group->first()->loadPicture(d->m_componentData)) );
        item->setHeader( group->name() );
        pageWidgetItems.append( item );

	QGridLayout* layout = new QGridLayout(frame);
	KoTCDIconCanvas *canvas = new KoTCDIconCanvas( frame );
	layout->addWidget(canvas,0,0);

	canvas->setBackgroundRole( QPalette::Base );
	canvas->setResizeMode( QListView::Adjust );
	// TODO canvas->setWordWrapIconText( true );
	canvas->show();

	QListWidgetItem * tempitem = canvas->load(group, templateName, d->m_componentData);
	if (tempitem)
	    itemtoselect = tempitem;

	// TODO canvas->sort();
	canvas->setSelectionMode(QListView::SingleSelection);

	connect( canvas, SIGNAL( itemClicked ( QListWidgetItem * ) ),
                 this, SLOT( currentChanged( QListWidgetItem * ) ) );

	connect( canvas, SIGNAL( itemDoubleClicked( QListWidgetItem * ) ),
                 this, SLOT( chosen(QListWidgetItem *) ) );

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

    //  Set the initially shown page, possibly from the last usage of the dialog
    if (entriesnumber >= templateNum && templateNum != -1 )
 	d->m_jwidget->setCurrentPage(pageWidgetItems[templateNum]);
    else if ( defaultTemplateGroup != -1)
 	d->m_jwidget->setCurrentPage(pageWidgetItems[defaultTemplateGroup]);


    // Set the initially selected template, possibly from the last usage of the dialog
    currentChanged(itemtoselect);
}

/*================================================================*/
// private
void KoTemplateChooseDia::setupDialog()
{
    QGridLayout *maingrid = new QGridLayout( d->m_mainwidget );
    maingrid->setMargin( 2 );
    maingrid->setSpacing( 6 );

    setCaption(i18n( "Choose Template" ));
    setupTemplateDialog(d->m_mainwidget, maingrid);
}

/*================================================================*/
// private SLOT
void KoTemplateChooseDia::currentChanged( QListWidgetItem * item)
{
    if (item)
    {
	// set text in the textarea
	d->textedit->setPlainText( descriptionText(
				item->text(),
				((KoTCDIconViewItem *) item)->getDescr()
				));

	// set the icon in the canvas selected
        item->setSelected(true);

	// register the current template
	d->m_templateName = item->text();
	d->m_fullTemplateName = ((KoTCDIconViewItem *) item)->getFName();
    }
}

/*================================================================*/
// private SLOT
void KoTemplateChooseDia::chosen(QListWidgetItem * item)
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
void KoTemplateChooseDia::recentSelected( QListWidgetItem * item)
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
	KConfigGroup grp( d->m_componentData.config(), "TemplateChooserDialog" );
	static const char* const s_returnTypes[] = { 0 /*Cancel ;)*/, "Template", "Empty" };
	if ( d->m_returnType <= Empty )
	{
	    grp.writeEntry( "LastReturnType", QString::fromLatin1(s_returnTypes[d->m_returnType]) );
	    if (d->m_returnType == Template)
	    {
 		grp.writePathEntry( "TemplateName", d->m_templateName );
 		grp.writePathEntry( "FullTemplateName", d->m_fullTemplateName);
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

    // is it a template or a file ?
    if ( d->m_dialogType==OnlyTemplates || newTabSelected )
    {
	// a template is chosen
	if (d->m_templateName.length() > 0)
	    d->m_returnType = Template;
	else
	    d->m_returnType = Empty;

	return true;
    }

    d->m_returnType = Empty;
    return false;
}

/*================================================================*/
//private
QString KoTemplateChooseDia::descriptionText(const QString &name, const QString &description)
{
	QString descrText(i18n("Name:"));
	descrText += ' ' + name;
	descrText += '\n';
	descrText += i18n("Description:");
	if (description.isEmpty())
	      descrText += ' ' + i18n("No description available");
	else
              descrText += ' ' + description;
	return descrText;
}

/*================================================================*/

QListWidgetItem * KoTCDIconCanvas::load( KoTemplateGroup *group, const QString& name, const KComponentData &componentData )
{
    QListWidgetItem * itemtoreturn = 0;

    for (KoTemplate *t=group->first(); t!=0L; t=group->next()) {
	if (t->isHidden())
	    continue;
	QListWidgetItem *item = new KoTCDIconViewItem(
		this,
		t->name(),
		t->loadPicture(componentData),
		t->description(),
		t->file());

	if (name == t->name())
	{
	    itemtoreturn = item;
	}

#if 0 // TODO
	item->setKey(t->name());
	item->setDragEnabled(false);
	item->setDropEnabled(false);
#endif
    }

    return itemtoreturn;
}

void KoTCDIconCanvas::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
        e->ignore();
    else
        KIconCanvas::keyPressEvent( e );
}

#include "KoTemplateChooseDia.moc"
