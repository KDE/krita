/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
                 2001, 2002 Sven Leiber         <s.leiber@web.de>

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

#include "KoAutoFormatDia.h"
#include "KoAutoFormat.h"
#include "KoCharSelectDia.h"
#include <KoSearchDia.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <k3listview.h>
#include <kstandarddirs.h>

#include <QLayout>

#include <QCheckBox>
#include <QPushButton>
#include <QToolTip>
#include <QComboBox>
#include <QDir>
#include <QApplication>
//Added by qt3to4:
#include <Q3GridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <Q3ListBox>
KoAutoFormatLineEdit::KoAutoFormatLineEdit ( QWidget * parent, const char * name )
    : QLineEdit(parent,name)
{
}

void KoAutoFormatLineEdit::keyPressEvent ( QKeyEvent *ke )
{
    if( ke->key()  == Qt::Key_Return ||
        ke->key()  == Qt::Key_Enter )
    {
        emit keyReturnPressed();
        return;
    }
    QLineEdit::keyPressEvent (ke);
}


/******************************************************************/
/* Class: KoAutoFormatExceptionWidget                             */
/******************************************************************/

KoAutoFormatExceptionWidget::KoAutoFormatExceptionWidget(QWidget *parent, const QString &name,const QStringList &_list, bool _autoInclude, bool _abreviation)
    :QWidget( parent )
{
    m_bAbbreviation=_abreviation;
    m_listException=_list;
    Q3GridLayout *grid = new Q3GridLayout(this, 4, 2, 0, KDialog::spacingHint());

    QLabel *lab=new QLabel(name,this);
    grid->addMultiCellWidget(lab,0,0,0,1);

    exceptionLine = new KoAutoFormatLineEdit( this );
    grid->addWidget(exceptionLine,1,0);

    connect(exceptionLine,SIGNAL(keyReturnPressed()),SLOT(slotAddException()));
    connect(exceptionLine ,SIGNAL(textChanged ( const QString & )),
            SLOT(textChanged ( const QString & )));

    pbAddException=new QPushButton(i18n("Add"),this);
    connect(pbAddException, SIGNAL(clicked()),SLOT(slotAddException()));
    grid->addWidget(pbAddException,1,1);

    pbAddException->setEnabled(false);

    pbRemoveException=new QPushButton(i18n("Remove"),this);
    connect(pbRemoveException, SIGNAL(clicked()),SLOT(slotRemoveException()));
    grid->addWidget(pbRemoveException,2,1,Qt::AlignTop);

    exceptionList=new Q3ListBox(this);
    exceptionList->insertStringList(m_listException);
    exceptionList->sort();
    grid->addWidget(exceptionList,2,0);

    grid->setRowStretch( 2, 1 );

    connect( exceptionList , SIGNAL(selectionChanged () ),
            this,SLOT(slotExceptionListSelected()) );

    pbRemoveException->setEnabled( exceptionList->currentItem()!=-1);

    cbAutoInclude = new QCheckBox( i18n("Autoinclude"), this );
    grid->addWidget(cbAutoInclude,3,0);
    cbAutoInclude->setChecked( _autoInclude );
}

void KoAutoFormatExceptionWidget::textChanged ( const QString &_text )
{
    pbAddException->setEnabled(!_text.isEmpty());
}

void KoAutoFormatExceptionWidget::slotAddException()
{
    QString text=exceptionLine->text().trimmed();
    if(!text.isEmpty())
    {
        if(text.at(text.length()-1)!='.' && m_bAbbreviation)
            text=text+".";
        if( m_listException.findIndex( text )==-1)
        {
            m_listException<<text;

            exceptionList->clear();
            exceptionList->insertStringList(m_listException);
            exceptionList->sort();
            pbRemoveException->setEnabled( exceptionList->currentItem()!=-1);
            pbAddException->setEnabled(false);
        }
        exceptionLine->clear();
    }
}

void KoAutoFormatExceptionWidget::slotRemoveException()
{
    if(!exceptionList->currentText().isEmpty())
    {
        m_listException.remove(exceptionList->currentText());
        exceptionList->clear();
        pbAddException->setEnabled(false);
        pbRemoveException->setEnabled( exceptionList->currentItem()!=-1);
        exceptionList->insertStringList(m_listException);
        exceptionLine->clear();
    }
}

bool KoAutoFormatExceptionWidget::autoInclude()
{
    return cbAutoInclude->isChecked();
}

void KoAutoFormatExceptionWidget::setListException( const QStringList &list)
{
    exceptionList->clear();
    exceptionList->insertStringList(list);
}

void KoAutoFormatExceptionWidget::setAutoInclude(bool b)
{
    cbAutoInclude->setChecked( b );
}

void KoAutoFormatExceptionWidget::slotExceptionListSelected()
{
    pbRemoveException->setEnabled( exceptionList->currentItem()!=-1 );
}

/******************************************************************/
/* Class: KoAutoFormatDia                                         */
/******************************************************************/

KoAutoFormatDia::KoAutoFormatDia( QWidget *parent, const char *name,
      KoAutoFormat * autoFormat )
    : KPageDialog(parent),
      oSimpleBegin( autoFormat->getConfigTypographicSimpleQuotes().begin ),
      oSimpleEnd( autoFormat->getConfigTypographicSimpleQuotes().end ),
      oDoubleBegin( autoFormat->getConfigTypographicDoubleQuotes().begin ),
      oDoubleEnd( autoFormat->getConfigTypographicDoubleQuotes().end ),
      bulletStyle( autoFormat->getConfigBulletStyle()),
      m_autoFormat( *autoFormat ),
      m_docAutoFormat( autoFormat )
{
    setFaceType( Tabbed );
    setCaption( i18n("Autocorrection") );
    setModal( true );
    setObjectName( name );
    setButtons( Ok | Cancel | User1 );
    setDefaultButton( Ok );
    setButtonGuiItem( User1, KGuiItem( i18n( "&Reset" ), "undo" ) );
    showButtonSeparator( true );

    noSignal=true;
    newEntry = 0L;
    autocorrectionEntryChanged= false;
    changeLanguage = false;

    setupTab1();
    setupTab2();
    setupTab3();
    setupTab4();
    setInitialSize( QSize(500, 300) );
    connect( this, SIGNAL( user1Clicked() ), this, SLOT(slotResetConf()));
    noSignal=false;
}

KoAutoFormatDia::~KoAutoFormatDia()
{
    delete newEntry;
}

void KoAutoFormatDia::slotResetConf()
{
    if ( currentPage() == p1 )
        initTab1();
    else if ( currentPage() == p2 )
        initTab2();
    else if ( currentPage() == p3 )
        initTab2();
    else if ( currentPage() == p4 )
        initTab4();
}

void KoAutoFormatDia::setupTab1()
{
    
    tab1 = new QWidget();
    p1 = addPage( tab1, i18n( "Simple Autocorrection" ) );
    Q3VBoxLayout *vbox = new Q3VBoxLayout(tab1, 0, KDialog::spacingHint());

    cbUpperCase = new QCheckBox( tab1 );
    cbUpperCase->setText( i18n(
            "Convert &first letter of a sentence automatically to uppercase\n"
            "(e.g. \"my house. in this town\" to \"my house. In this town\")"
            ) );
    cbUpperCase->setWhatsThis( i18n(
            "Detect when a new sentence is started and always ensure that"
            " the first character is an uppercase character."));

    vbox->addWidget(cbUpperCase);


    cbUpperUpper = new QCheckBox( tab1 );
    cbUpperUpper->setText( i18n(
            "Convert &two uppercase characters to one uppercase and one"
            " lowercase character\n (e.g. PErfect to Perfect)" ) );
    cbUpperUpper->setWhatsThis( i18n(
            "All words are checked for the common mistake of holding the "
            "shift key down a bit too long. If some words must have two "
            "uppercase characters, then those exceptions should be added in "
            "the 'Exceptions' tab."));

    vbox->addWidget(cbUpperUpper);

    cbDetectUrl=new QCheckBox( tab1 );
    cbDetectUrl->setText( i18n( "Autoformat &URLs" ) );
    cbDetectUrl->setWhatsThis( i18n(
            "Detect when a URL (Uniform Resource Locator) is typed and "
            "provide formatting that matches the way an Internet browser "
            "would show a URL."));

    vbox->addWidget(cbDetectUrl);

    cbIgnoreDoubleSpace=new QCheckBox( tab1 );
    cbIgnoreDoubleSpace->setText( i18n( "&Suppress double spaces" ) );
    cbIgnoreDoubleSpace->setWhatsThis( i18n(
            "Make sure that more than one space cannot be typed, as this is a "
            "common mistake which is quite hard to find in formatted text."));

    vbox->addWidget(cbIgnoreDoubleSpace);

    cbRemoveSpaceBeginEndLine=new QCheckBox( tab1 );
    cbRemoveSpaceBeginEndLine->setText( i18n(
            "R&emove spaces at the beginning and end of paragraphs" ) );
    cbRemoveSpaceBeginEndLine->setWhatsThis( i18n(
            "Keep correct formatting and indenting of sentences by "
            "automatically removing spaces typed at the beginning and end of "
            "a paragraph."));

    vbox->addWidget(cbRemoveSpaceBeginEndLine);

    cbAutoChangeFormat=new QCheckBox( tab1 );
    cbAutoChangeFormat->setText( i18n(
            "Automatically do &bold and underline formatting") );
    cbAutoChangeFormat->setWhatsThis( i18n(
            "When you use _underline_ or *bold*, the text between the "
            "underscores or asterisks will be converted to underlined or "
            "bold text.") );

    vbox->addWidget(cbAutoChangeFormat);

    cbAutoReplaceNumber=new QCheckBox( tab1 );
    cbAutoReplaceNumber->setText( i18nc(
            "We add the 1/2 char at the %1", "Re&place 1/2... with %1...",
            QString( "" )  ));
    cbAutoReplaceNumber->setWhatsThis( i18n(
            "Most standard fraction notations will be converted when available"
            ) );

    vbox->addWidget(cbAutoReplaceNumber);

    cbUseNumberStyle=new QCheckBox( tab1 );
    cbUseNumberStyle->setText( i18n(
            "Use &autonumbering for numbered paragraphs" ) );
    cbUseNumberStyle->setWhatsThis( i18n(
            "When typing '1)' or similar in front of a paragraph, "
            "automatically convert the paragraph to use that numbering style. "
            "This has the advantage that further paragraphs will also be "
            "numbered and the spacing is done correctly.") );

    vbox->addWidget(cbUseNumberStyle);

    cbAutoSuperScript = new QCheckBox( tab1 );
    cbAutoSuperScript->setText( i18n("Rep&lace 1st... with 1^st..."));
    cbAutoSuperScript->setEnabled( m_docAutoFormat->nbSuperScriptEntry()>0 );

    vbox->addWidget(cbAutoSuperScript);
    cbCapitalizeDaysName = new QCheckBox( tab1 );
    cbCapitalizeDaysName->setText( i18n("Capitalize name of days"));
    vbox->addWidget(cbCapitalizeDaysName);

    cbUseBulletStyle=new QCheckBox( tab1 );
    cbUseBulletStyle->setText( i18n(
            "Use l&ist-formatting for bulleted paragraphs" ) );
    cbUseBulletStyle->setWhatsThis( i18n(
            "When typing '*' or '-' in front of a paragraph, automatically "
            "convert the paragraph to use that list-style. Using a list-style "
            "formatting means that a correct bullet is used to draw the list."
            ) );

    connect( cbUseBulletStyle, SIGNAL( toggled( bool ) ),
            SLOT( slotBulletStyleToggled( bool ) ) );

    vbox->addWidget(cbUseBulletStyle);
    Q3HBoxLayout *hbox = new Q3HBoxLayout();

    hbox->addSpacing( 20 );
    hbox->setSpacing(KDialog::spacingHint());
    pbBulletStyle = new QPushButton( tab1 );
    pbBulletStyle->setFixedSize( pbBulletStyle->sizeHint() );
    hbox->addWidget( pbBulletStyle );
    pbDefaultBulletStyle = new QPushButton( tab1 );
    pbDefaultBulletStyle->setText(i18n("Default"));
    pbDefaultBulletStyle->setFixedSize( pbDefaultBulletStyle->sizeHint() );
    hbox->addWidget( pbDefaultBulletStyle );

    hbox->addStretch( 1 );

    vbox->addItem(hbox);
    vbox->addStretch( 1 );

    initTab1();

    connect( pbBulletStyle, SIGNAL( clicked() ), SLOT( chooseBulletStyle() ) );
    connect( pbDefaultBulletStyle, SIGNAL( clicked()),
            SLOT( defaultBulletStyle() ) );
}

void KoAutoFormatDia::initTab1()
{
    cbUpperCase->setChecked( m_autoFormat.getConfigUpperCase() );
    cbUpperUpper->setChecked( m_autoFormat.getConfigUpperUpper() );
    cbDetectUrl->setChecked( m_autoFormat.getConfigAutoDetectUrl());
    cbIgnoreDoubleSpace->setChecked( m_autoFormat.getConfigIgnoreDoubleSpace());
    cbRemoveSpaceBeginEndLine->setChecked( m_autoFormat.getConfigRemoveSpaceBeginEndLine());
    cbAutoChangeFormat->setChecked( m_autoFormat.getConfigAutoChangeFormat());
    cbAutoReplaceNumber->setChecked( m_autoFormat.getConfigAutoReplaceNumber());
    cbUseNumberStyle->setChecked( m_autoFormat.getConfigAutoNumberStyle());
    cbUseBulletStyle->setChecked( m_autoFormat.getConfigUseBulletSyle());
    cbAutoSuperScript->setChecked( m_docAutoFormat->getConfigAutoSuperScript());
    pbBulletStyle->setText( bulletStyle );
    cbCapitalizeDaysName->setChecked( m_autoFormat.getConfigCapitalizeNameOfDays());

    slotBulletStyleToggled( cbUseBulletStyle->isChecked() );
}

void KoAutoFormatDia::slotBulletStyleToggled( bool b )
{
    pbBulletStyle->setEnabled( b );
    pbDefaultBulletStyle->setEnabled( b );
}

void KoAutoFormatDia::setupTab2()
{
    tab2 = new QWidget();
    p2 = addPage( tab2, i18n( "Custom Quotes" ) );

    Q3VBoxLayout *vbox = new Q3VBoxLayout(tab2, 0, KDialog::spacingHint());

    cbTypographicDoubleQuotes = new QCheckBox( tab2 );
    cbTypographicDoubleQuotes->setText( i18n(
            "Replace &double quotes with typographical quotes" ) );

    connect( cbTypographicDoubleQuotes,SIGNAL(toggled ( bool)),
            SLOT(slotChangeStateDouble(bool)));

    vbox->addWidget( cbTypographicDoubleQuotes );

    Q3HBoxLayout *hbox = new Q3HBoxLayout( );
    hbox->addSpacing( 20 );

    pbDoubleQuote1 = new QPushButton( tab2 );
    pbDoubleQuote1->setFixedSize( pbDoubleQuote1->sizeHint() );

    pbDoubleQuote2 = new QPushButton( tab2 );
    pbDoubleQuote2->setFixedSize( pbDoubleQuote2->sizeHint() );

    if (QApplication::isRightToLeft()) {
        hbox->addWidget( pbDoubleQuote2 );
        hbox->addWidget( pbDoubleQuote1 );
    } else {
        hbox->addWidget( pbDoubleQuote1 );
        hbox->addWidget( pbDoubleQuote2 );
    }

    hbox->addSpacing( KDialog::spacingHint() );

    pbDoubleDefault = new QPushButton( tab2 );
    pbDoubleDefault->setText(i18n("Default"));
    pbDoubleDefault->setFixedSize( pbDoubleDefault->sizeHint() );
    hbox->addWidget( pbDoubleDefault );

    hbox->addStretch( 1 );

    connect(pbDoubleQuote1, SIGNAL( clicked() ), SLOT( chooseDoubleQuote1() ));
    connect(pbDoubleQuote2, SIGNAL( clicked() ), SLOT( chooseDoubleQuote2() ));
    connect(pbDoubleDefault, SIGNAL( clicked()), SLOT( defaultDoubleQuote() ));

    vbox->addItem( hbox );

    cbTypographicSimpleQuotes = new QCheckBox( tab2 );
    cbTypographicSimpleQuotes->setText( i18n(
            "Replace &single quotes with typographical quotes" ) );

    connect( cbTypographicSimpleQuotes,SIGNAL(toggled ( bool)),
            SLOT(slotChangeStateSimple(bool)));

    vbox->addWidget( cbTypographicSimpleQuotes );

    hbox = new Q3HBoxLayout( );
    hbox->addSpacing( 20 );

    pbSimpleQuote1 = new QPushButton( tab2 );
    pbSimpleQuote1->setFixedSize( pbSimpleQuote1->sizeHint() );

    pbSimpleQuote2 = new QPushButton( tab2 );
    pbSimpleQuote2->setFixedSize( pbSimpleQuote2->sizeHint() );

    if (QApplication::isRightToLeft()) {
        hbox->addWidget( pbSimpleQuote2 );
        hbox->addWidget( pbSimpleQuote1 );
    } else {
        hbox->addWidget( pbSimpleQuote1 );
        hbox->addWidget( pbSimpleQuote2 );
    }

    hbox->addSpacing( KDialog::spacingHint() );

    pbSimpleDefault = new QPushButton( tab2 );
    pbSimpleDefault->setText(i18n("Default"));
    pbSimpleDefault->setFixedSize( pbSimpleDefault->sizeHint() );
    hbox->addWidget( pbSimpleDefault );

    hbox->addStretch( 1 );

    connect(pbSimpleQuote1, SIGNAL( clicked() ), SLOT( chooseSimpleQuote1() ));
    connect(pbSimpleQuote2, SIGNAL( clicked() ), SLOT( chooseSimpleQuote2() ));
    connect(pbSimpleDefault, SIGNAL( clicked()), SLOT( defaultSimpleQuote() ));

    vbox->addItem( hbox );
    vbox->addStretch( 1 );

    initTab2();
}

void KoAutoFormatDia::initTab2()
{
    bool state=m_autoFormat.getConfigTypographicDoubleQuotes().replace;
    cbTypographicDoubleQuotes->setChecked( state );
    pbDoubleQuote1->setText( oDoubleBegin );
    pbDoubleQuote2->setText(oDoubleEnd );
    slotChangeStateDouble(state);

    state=m_autoFormat.getConfigTypographicSimpleQuotes().replace;
    cbTypographicSimpleQuotes->setChecked( state );
    pbSimpleQuote1->setText( oSimpleBegin );
    pbSimpleQuote2->setText(oSimpleEnd );
    slotChangeStateSimple(state);

}

void KoAutoFormatDia::setupTab3()
{
    tab3 = new QWidget();
    p3 = addPage( tab3, i18n( "Advanced Autocorrection" ) );

    QLabel *lblFind, *lblReplace;

    Q3GridLayout *grid = new Q3GridLayout( tab3, 11, 7, 0, KDialog::spacingHint() );

    autoFormatLanguage = new QComboBox(tab3);

    QStringList lst;
    lst<<i18n("Default");
    lst<<i18n("All Languages");
    exceptionLanguageName.insert( i18n("Default"), "");
    exceptionLanguageName.insert( i18n("All Languages"), "all_languages");

    KStandardDirs *standard = new KStandardDirs();
    QStringList tmp = standard->findDirs("data", "koffice/autocorrect/");
    QString path = *(tmp.end());
    for ( QStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it )
    {
        path =*it;
    }
    delete standard;
    QDir dir( path);
    tmp =dir.entryList (QDir::Files);
    for ( QStringList::Iterator it = tmp.begin(); it != tmp.end(); ++it )
    {
        if ( !(*it).contains("autocorrect"))
        {
            QString readableName = KGlobal::locale()->twoAlphaToCountryName((*it).left((*it).length()-4));
            QString tmp;
            if ( readableName.isEmpty() )
                tmp =(*it).left((*it).length()-4);
            else
                tmp =readableName;
            exceptionLanguageName.insert( tmp, (*it).left((*it).length()-4));
            lst<<tmp;
        }
    }
    autoFormatLanguage->addItems(lst);
#warning "kde4: port it"
    //connect(autoFormatLanguage->listBox(), SIGNAL(selected ( const QString & )), this, SLOT(changeAutoformatLanguage(const QString & )));

    grid->addMultiCellWidget( autoFormatLanguage, 0, 0, 4, 6 );
    QLabel *lblAutoFormatLanguage = new QLabel( i18n("Replacements and exceptions for language:"), tab3);
    grid->addMultiCellWidget( lblAutoFormatLanguage, 0, 0, 0, 3 );

    cbAdvancedAutoCorrection = new QCheckBox( tab3 );
    cbAdvancedAutoCorrection->setText( i18n("Enable word replacement") );
    connect( cbAdvancedAutoCorrection, SIGNAL(clicked ()), this, SLOT( slotChangeAdvancedAutoCorrection()));
    grid->addMultiCellWidget( cbAdvancedAutoCorrection, 1, 1, 0, 6 );

    cbAutoCorrectionWithFormat = new QCheckBox( tab3 );
    cbAutoCorrectionWithFormat->setText( i18n("Replace text with format") );
    grid->addMultiCellWidget( cbAutoCorrectionWithFormat, 2, 2, 0, 6 );

    lblFind = new QLabel( i18n( "&Find:" ), tab3 );
    grid->addWidget( lblFind, 3, 0 );

    m_find = new KoAutoFormatLineEdit( tab3 );
    grid->addWidget( m_find, 3, 1 );

    lblFind->setBuddy( m_find );

    connect( m_find, SIGNAL( textChanged( const QString & ) ),
	     SLOT( slotfind( const QString & ) ) );
    connect( m_find, SIGNAL( keyReturnPressed() ),
             SLOT( slotAddEntry()));

    pbSpecialChar1 = new QPushButton( "...", tab3 );
    pbSpecialChar1->setToolTip( i18n( "Insert a special character..." ) );
    pbSpecialChar1->setFixedWidth( 40 );
    grid->addWidget( pbSpecialChar1, 3, 2 );

    connect(pbSpecialChar1,SIGNAL(clicked()), SLOT(chooseSpecialChar1()));

    lblReplace = new QLabel( i18n( "&Replace:" ), tab3 );
    grid->addWidget( lblReplace, 3, 3 );

    m_replace = new KoAutoFormatLineEdit( tab3 );
    grid->addWidget( m_replace, 3, 4 );

    lblReplace->setBuddy( m_replace );

    connect( m_replace, SIGNAL( textChanged( const QString & ) ),
	        SLOT( slotfind2( const QString & ) ) );
    connect( m_replace, SIGNAL( keyReturnPressed() ),
            SLOT( slotAddEntry()));

    pbSpecialChar2 = new QPushButton( "...", tab3 );
    pbSpecialChar2->setToolTip( i18n( "Insert a special character..." ) );
    pbSpecialChar2->setFixedWidth( 40 );
    grid->addWidget( pbSpecialChar2, 3, 5 );

    connect(pbSpecialChar2,SIGNAL(clicked()), SLOT(chooseSpecialChar2()));

    pbAdd = new QPushButton( i18n( "&Add"), tab3  );
    grid->addWidget( pbAdd, 3, 6 );

    connect(pbAdd,SIGNAL(clicked()),this, SLOT(slotAddEntry()));

    m_pListView = new K3ListView( tab3 );
    m_pListView->addColumn( i18n( "Find" ) );
    m_pListView->addColumn( i18n( "Replace" ) );
    m_pListView->setAllColumnsShowFocus( true );
    grid->addMultiCellWidget( m_pListView, 4, 10, 0, 5 );

    connect(m_pListView, SIGNAL(doubleClicked ( Q3ListViewItem * )),
             SLOT(slotChangeTextFormatEntry()) );
    connect(m_pListView, SIGNAL(clicked ( Q3ListViewItem * ) ),
             SLOT(slotEditEntry()) );

    pbRemove = new QPushButton( i18n( "Remove" ), tab3 );
    grid->addWidget( pbRemove, 4, 6, Qt::AlignTop );

    connect(pbRemove,SIGNAL(clicked()), SLOT(slotRemoveEntry()));

    pbChangeFormat= new QPushButton( i18n( "Change Format..." ), tab3 );
    grid->addWidget( pbChangeFormat, 5, 6, Qt::AlignTop );

    connect( pbChangeFormat, SIGNAL(clicked()), SLOT(slotChangeTextFormatEntry()));

    pbClearFormat= new QPushButton( i18n( "Clear Format" ), tab3 );
    grid->addWidget( pbClearFormat, 6, 6, Qt::AlignTop );

    connect( pbClearFormat, SIGNAL(clicked()), SLOT(slotClearTextFormatEntry()));
    grid->setRowStretch( 10, 1 );

    initTab3();
    slotChangeAdvancedAutoCorrection();
    pbRemove->setEnabled(false);
    pbChangeFormat->setEnabled( false );
    pbAdd->setEnabled(false);
    pbClearFormat->setEnabled( false);

}

void KoAutoFormatDia::initTab3()
{
    if ( !changeLanguage || noSignal)
    {
        initialLanguage=m_autoFormat.getConfigAutoFormatLanguage( );
        if ( initialLanguage.isEmpty() )
            autoFormatLanguage->setCurrentIndex(0);
        else
        {
            KoExceptionLanguageName::Iterator it = exceptionLanguageName.begin();
            for ( ; it != exceptionLanguageName.end() ; ++it )
            {
                if ( it.data() == initialLanguage)
                {
                    autoFormatLanguage->setCurrentText(it.key());
                    break;
                }

            }
        }
    }
    //force to re-readconfig when we reset config and we change a entry
    if ( autocorrectionEntryChanged )
    {
        if ( !changeLanguage )
            m_docAutoFormat->configAutoFormatLanguage( initialLanguage);
        m_docAutoFormat->readConfig( true );
    }
    cbAdvancedAutoCorrection->setChecked(m_autoFormat.getConfigAdvancedAutoCorrect());
    cbAutoCorrectionWithFormat->setChecked( m_autoFormat.getConfigCorrectionWithFormat());
    m_pListView->clear();

    Q3DictIterator<KoAutoFormatEntry> it( m_docAutoFormat->getAutoFormatEntries());
    for( ; it.current(); ++it )
    {
        ( void )new Q3ListViewItem( m_pListView, it.currentKey(), it.current()->replace() );
    }
}

void KoAutoFormatDia::slotChangeAdvancedAutoCorrection()
{
    bool state = cbAdvancedAutoCorrection->isChecked();
    cbAutoCorrectionWithFormat->setEnabled( state );
    pbSpecialChar2->setEnabled( state );
    pbSpecialChar1->setEnabled( state );
    m_replace->setEnabled( state);
    m_find->setEnabled( state);
    m_pListView->setEnabled( state);

    state = state && !m_replace->text().isEmpty() && !m_find->text().isEmpty();
    KoAutoFormatEntry * entry=m_docAutoFormat->findFormatEntry(m_find->text());
    pbRemove->setEnabled(state && entry);
    pbChangeFormat->setEnabled(state && entry);
    pbClearFormat->setEnabled(state && entry);
    pbAdd->setEnabled(state);
}


void KoAutoFormatDia::changeAutoformatLanguage(const QString & text)
{
    if ( text==i18n("Default"))
        m_docAutoFormat->configAutoFormatLanguage( QString::null);
    else
    {
        m_docAutoFormat->configAutoFormatLanguage( exceptionLanguageName.find(text).data());
    }
    if ( !noSignal )
    {
        changeLanguage=true;
        m_docAutoFormat->readConfig( true );
        initTab3();
        initTab4();
        autocorrectionEntryChanged=true;
        cbAutoSuperScript->setEnabled( m_docAutoFormat->nbSuperScriptEntry()>0 );
        oSimpleBegin= m_docAutoFormat->getConfigTypographicSimpleQuotes().begin ;
        oSimpleEnd= m_docAutoFormat->getConfigTypographicSimpleQuotes().end;
        oDoubleBegin= m_docAutoFormat->getConfigTypographicDoubleQuotes().begin;
        oDoubleEnd= m_docAutoFormat->getConfigTypographicDoubleQuotes().end;
        bulletStyle= m_docAutoFormat->getConfigBulletStyle();
        delete newEntry;
        newEntry=0L;
        changeLanguage=false;
    }
}

void KoAutoFormatDia::setupTab4()
{
    tab4 = new QWidget();
    p4 = addPage( tab4, i18n( "Exceptions" ) );
    Q3VBoxLayout *vbox = new Q3VBoxLayout(tab4, 0, KDialog::spacingHint());

    abbreviation=new KoAutoFormatExceptionWidget(tab4,
            i18n("Do not treat as the end of a sentence:"),
            m_autoFormat.listException(),
            m_autoFormat.getConfigIncludeAbbreviation() , true);

    vbox->addWidget( abbreviation );

    twoUpperLetter=new KoAutoFormatExceptionWidget(tab4,
            i18n("Accept two uppercase letters in:"),
            m_autoFormat.listTwoUpperLetterException(),
            m_autoFormat.getConfigIncludeTwoUpperUpperLetterException());

    vbox->addWidget( twoUpperLetter );

    initTab4();
}

void KoAutoFormatDia::initTab4()
{
    abbreviation->setListException( !changeLanguage ? m_autoFormat.listException(): m_docAutoFormat->listException() );
    if ( !changeLanguage )
    {
        abbreviation->setAutoInclude( m_docAutoFormat->getConfigIncludeAbbreviation() );
        twoUpperLetter->setAutoInclude( m_docAutoFormat->getConfigIncludeTwoUpperUpperLetterException() );
    }
    twoUpperLetter->setListException( !changeLanguage ? m_autoFormat.listTwoUpperLetterException():m_docAutoFormat->listTwoUpperLetterException() );
}

void KoAutoFormatDia::slotClearTextFormatEntry()
{
    bool addNewEntry = (pbAdd->text() == i18n( "&Add" ));
    if ( m_pListView->currentItem() || addNewEntry)
    {
        if ( addNewEntry )
        {
            if (newEntry)
                newEntry->clearFormatEntryContext();
        }
        else
        {
            KoAutoFormatEntry *entry = m_docAutoFormat->findFormatEntry(m_pListView->currentItem()->text(0));
            entry->clearFormatEntryContext();
        }
        autocorrectionEntryChanged= true;
    }
}

void KoAutoFormatDia::slotChangeTextFormatEntry()
{
    bool addNewEntry = (pbAdd->text() == i18n( "&Add" ));
    if ( m_pListView->currentItem() || addNewEntry)
    {
        KoAutoFormatEntry *entry = 0L;
        if ( addNewEntry )
        {
            if ( m_replace->text().isEmpty() )
                return;
            if ( !newEntry )
                newEntry = new KoAutoFormatEntry( m_replace->text());
            entry =newEntry;
        }
        else
            entry = m_docAutoFormat->findFormatEntry(m_pListView->currentItem()->text(0));
        KoSearchContext *tmpFormat = entry->formatEntryContext();
        bool createNewFormat = false;

        if ( !tmpFormat )
        {
            tmpFormat = new KoSearchContext();
            createNewFormat = true;
        }

        KoFormatDia *dia = new KoFormatDia( this, i18n("Change Text Format"), tmpFormat ,  0L);
        if ( dia->exec())
        {
            dia->ctxOptions( );
            if ( createNewFormat )
                entry->setFormatEntryContext( tmpFormat );
            autocorrectionEntryChanged= true;

        }
        else
        {
            if ( createNewFormat )
                delete tmpFormat;
        }
        delete dia;
    }
}

void KoAutoFormatDia::slotRemoveEntry()
{
    //find entry in listbox
   if(m_pListView->currentItem())
    {
        m_docAutoFormat->removeAutoFormatEntry(m_pListView->currentItem()->text(0));
        pbAdd->setText(i18n("&Add"));
        refreshEntryList();
        autocorrectionEntryChanged= true;
    }
}


void KoAutoFormatDia::slotfind( const QString & )
{
    KoAutoFormatEntry *entry = m_docAutoFormat->findFormatEntry(m_find->text());
    if ( entry )
    {
        m_replace->setText(entry->replace().toLatin1());
        pbAdd->setText(i18n("&Modify"));
        m_pListView->setCurrentItem(m_pListView->findItem(m_find->text(),0));

    } else {
        m_replace->clear();
        pbAdd->setText(i18n("&Add"));
        m_pListView->setCurrentItem(0L);
    }
    slotfind2("");
}


void KoAutoFormatDia::slotfind2( const QString & )
{
    bool state = !m_replace->text().isEmpty() && !m_find->text().isEmpty();
    KoAutoFormatEntry * entry=m_docAutoFormat->findFormatEntry(m_find->text());
    pbRemove->setEnabled(state && entry);
    if ( state && entry )
    {
        delete newEntry;
        newEntry = 0L;
    }
    pbChangeFormat->setEnabled(state);
    pbClearFormat->setEnabled(state);
    pbAdd->setEnabled(state);
}


void KoAutoFormatDia::refreshEntryList()
{
    m_pListView->clear();
    Q3DictIterator<KoAutoFormatEntry> it( m_docAutoFormat->getAutoFormatEntries());
    for( ; it.current(); ++it )
    {
        ( void )new Q3ListViewItem( m_pListView, it.currentKey(), it.current()->replace() );
    }
    m_pListView->setCurrentItem(m_pListView->firstChild ());
    bool state = !(m_replace->text().isEmpty()) && !(m_find->text().isEmpty());
    //we can delete item, as we search now in listbox and not in m_find lineedit
    pbRemove->setEnabled(m_pListView->currentItem() && m_pListView->selectedItem()!=0 );
    pbChangeFormat->setEnabled(state && m_pListView->currentItem() && m_pListView->selectedItem()!=0 );
    pbClearFormat->setEnabled(state && m_pListView->currentItem() && m_pListView->selectedItem()!=0 );

    pbAdd->setEnabled(state);
}


void KoAutoFormatDia::addEntryList(const QString &key, KoAutoFormatEntry *_autoEntry)
{
    m_docAutoFormat->addAutoFormatEntry( key, _autoEntry );
}



void KoAutoFormatDia::editEntryList(const QString &key,const QString &newFindString, KoAutoFormatEntry *_autoEntry)
{
    if ( m_docAutoFormat->findFormatEntry(key) && m_docAutoFormat->findFormatEntry(key)->formatEntryContext())
        _autoEntry->setFormatEntryContext( new KoSearchContext(*(m_docAutoFormat->findFormatEntry(key)->formatEntryContext()) ));
    m_docAutoFormat->removeAutoFormatEntry( key );
    m_docAutoFormat->addAutoFormatEntry( newFindString, _autoEntry );
}


void KoAutoFormatDia::slotAddEntry()
{
    if(!pbAdd->isEnabled())
        return;
    QString repl = m_replace->text();
    QString find = m_find->text();
    if(repl.isEmpty() || find.isEmpty())
    {
        KMessageBox::sorry( 0L, i18n( "An area is empty" ) );
        return;
    }
    if(repl==find)
    {
        KMessageBox::sorry( 0L, i18n( "Find string is the same as replace string!" ) );
	return;
    }
    KoAutoFormatEntry *tmp = new KoAutoFormatEntry( repl );

    if(pbAdd->text() == i18n( "&Add" ))
    {
        if ( newEntry )
        {
            newEntry->changeReplace( m_replace->text());
            addEntryList(find, newEntry);
            delete tmp;
            newEntry = 0L;
        }
        else
            addEntryList(find, tmp);
    }
    else
        editEntryList(find, find, tmp);
    m_replace->clear();
    m_find->clear();

    refreshEntryList();
    autocorrectionEntryChanged= true;
}


void KoAutoFormatDia::chooseSpecialChar1()
{
    QString f = font().family();
    QChar c = ' ';
    bool const focus = m_find->hasFocus();
    if ( KoCharSelectDia::selectChar( f, c, false ) )
    {
        int const cursorpos = m_find->cursorPosition();
        if (focus)
            m_find->setText( m_find->text().insert( cursorpos, c ) );
        else
            m_find->setText( m_find->text().append(c) );
        m_find->setCursorPosition( cursorpos+1 );
    }
}


void KoAutoFormatDia::chooseSpecialChar2()
{
    QString f = font().family();
    QChar c = ' ';
    bool const focus = m_replace->hasFocus();
    if ( KoCharSelectDia::selectChar( f, c, false ) )
    {
        int const cursorpos = m_replace->cursorPosition();
        if (focus)
            m_replace->setText( m_replace->text().insert(m_replace->cursorPosition(),  c ) );
        else
        m_replace->setText( m_replace->text().append(c) );
        m_replace->setCursorPosition( cursorpos+1 );
    }
}


void KoAutoFormatDia::slotItemRenamed(Q3ListViewItem *, const QString & , int )
{
    // Wow. This need a redesign (we don't have the old key anymore at this point !)
    // -> inherit QListViewItem and store the KoAutoFormatEntry pointer in it.
}


void KoAutoFormatDia::slotEditEntry()
{
    if(m_pListView->currentItem()==0)
        return;
    delete newEntry;
    newEntry=0L;
    m_find->setText(m_pListView->currentItem()->text(0));
    m_replace->setText(m_pListView->currentItem()->text(1));
    bool state = !m_replace->text().isEmpty() && !m_find->text().isEmpty();
    pbRemove->setEnabled(state);
    pbChangeFormat->setEnabled( state );
    pbClearFormat->setEnabled(state);
    pbAdd->setEnabled(state);
}


bool KoAutoFormatDia::applyConfig()
{
    // First tab
    KoAutoFormat::TypographicQuotes tq = m_autoFormat.getConfigTypographicSimpleQuotes();
    tq.replace = cbTypographicSimpleQuotes->isChecked();
    tq.begin = pbSimpleQuote1->text()[ 0 ];
    tq.end = pbSimpleQuote2->text()[ 0 ];
    m_docAutoFormat->configTypographicSimpleQuotes( tq );

    tq = m_autoFormat.getConfigTypographicDoubleQuotes();
    tq.replace = cbTypographicDoubleQuotes->isChecked();
    tq.begin = pbDoubleQuote1->text()[ 0 ];
    tq.end = pbDoubleQuote2->text()[ 0 ];
    m_docAutoFormat->configTypographicDoubleQuotes( tq );


    m_docAutoFormat->configUpperCase( cbUpperCase->isChecked() );
    m_docAutoFormat->configUpperUpper( cbUpperUpper->isChecked() );
    m_docAutoFormat->configAutoDetectUrl( cbDetectUrl->isChecked() );

    m_docAutoFormat->configIgnoreDoubleSpace( cbIgnoreDoubleSpace->isChecked());
    m_docAutoFormat->configRemoveSpaceBeginEndLine( cbRemoveSpaceBeginEndLine->isChecked());
    m_docAutoFormat->configUseBulletStyle(cbUseBulletStyle->isChecked());

    m_docAutoFormat->configBulletStyle(pbBulletStyle->text()[ 0 ]);

    m_docAutoFormat->configAutoChangeFormat( cbAutoChangeFormat->isChecked());

    m_docAutoFormat->configAutoReplaceNumber( cbAutoReplaceNumber->isChecked());
    m_docAutoFormat->configAutoNumberStyle(cbUseNumberStyle->isChecked());

    m_docAutoFormat->configAutoSuperScript ( cbAutoSuperScript->isChecked() );
    m_docAutoFormat->configCapitalizeNameOfDays( cbCapitalizeDaysName->isChecked());


    // Second tab
    //m_docAutoFormat->copyAutoFormatEntries( m_autoFormat );
    m_docAutoFormat->copyListException(abbreviation->getListException());
    m_docAutoFormat->copyListTwoUpperCaseException(twoUpperLetter->getListException());
    m_docAutoFormat->configAdvancedAutocorrect( cbAdvancedAutoCorrection->isChecked() );
    m_docAutoFormat->configCorrectionWithFormat( cbAutoCorrectionWithFormat->isChecked());

    m_docAutoFormat->configIncludeTwoUpperUpperLetterException( twoUpperLetter->autoInclude());
    m_docAutoFormat->configIncludeAbbreviation( abbreviation->autoInclude());

    QString lang = exceptionLanguageName.find(autoFormatLanguage->currentText()).data();
    if ( lang == i18n("Default") )
        m_docAutoFormat->configAutoFormatLanguage(QString::null);
    else
        m_docAutoFormat->configAutoFormatLanguage(lang);

    // Save to config file
    m_docAutoFormat->saveConfig();
    return true;
}

void KoAutoFormatDia::slotOk()
{
    if (applyConfig())
    {
       slotButtonClicked( Ok );
    }
}

void KoAutoFormatDia::slotCancel()
{
    //force to reload
    if ( autocorrectionEntryChanged )
    {
        m_docAutoFormat->configAutoFormatLanguage( initialLanguage);
        m_docAutoFormat->readConfig( true );
    }
    slotButtonClicked( Cancel );
}

void KoAutoFormatDia::chooseDoubleQuote1()
{
    QString f = font().family();
    QChar c = oDoubleBegin;
    if ( KoCharSelectDia::selectChar( f, c, false ) )
    {
        pbDoubleQuote1->setText( c );
    }

}

void KoAutoFormatDia::chooseDoubleQuote2()
{
    QString f = font().family();
    QChar c = oDoubleEnd;
    if ( KoCharSelectDia::selectChar( f, c, false ) )
    {
        pbDoubleQuote2->setText( c );
    }
}


void KoAutoFormatDia::defaultDoubleQuote()
{
    pbDoubleQuote1->setText(m_docAutoFormat->getDefaultTypographicDoubleQuotes().begin);
    pbDoubleQuote2->setText(m_docAutoFormat->getDefaultTypographicDoubleQuotes().end);
}

void KoAutoFormatDia::chooseSimpleQuote1()
{
    QString f = font().family();
    QChar c = oSimpleBegin;
    if ( KoCharSelectDia::selectChar( f, c, false ) )
    {
        pbSimpleQuote1->setText( c );
    }
}

void KoAutoFormatDia::chooseSimpleQuote2()
{
    QString f = font().family();
    QChar c = oSimpleEnd;
    if ( KoCharSelectDia::selectChar( f, c, false ) )
    {
        pbSimpleQuote2->setText( c );
    }
}

void KoAutoFormatDia::defaultSimpleQuote()
{

    pbSimpleQuote1->setText(m_docAutoFormat->getDefaultTypographicSimpleQuotes().begin);
    pbSimpleQuote2->setText(m_docAutoFormat->getDefaultTypographicSimpleQuotes().end);
}


void KoAutoFormatDia::chooseBulletStyle()
{
    QString f = font().family();
    QChar c = bulletStyle;
    if ( KoCharSelectDia::selectChar( f, c, false ) )
    {
        pbBulletStyle->setText( c );
    }
}

void KoAutoFormatDia::defaultBulletStyle()
{
    pbBulletStyle->setText( "" );
}

void KoAutoFormatDia::slotChangeStateSimple(bool b)
{
    pbSimpleQuote1->setEnabled(b);
    pbSimpleQuote2->setEnabled(b);
    pbSimpleDefault->setEnabled(b);
}

void KoAutoFormatDia::slotChangeStateDouble(bool b)
{
    pbDoubleQuote1->setEnabled(b);
    pbDoubleQuote2->setEnabled(b);
    pbDoubleDefault->setEnabled(b);
}

#include "KoAutoFormatDia.moc"
