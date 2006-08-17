/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#include <QVariant>   // first for gcc 2.7.2
#include <q3buttongroup.h>
#include <QCheckBox>
#include <q3groupbox.h>
#include <QLabel>
#include <QLayout>
#include <QMap>
#include <QRadioButton>
#include <QSpinBox>
#include <QStringList>

#include <QWidget>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3Frame>

//#include <algorithm>

#include <kcolorbutton.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfontdialog.h>
#include <k3listview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kpushbutton.h>
#include <kvbox.h>

#include "contextstyle.h"
#include "kformulaconfigpage.h"
#include "symboltable.h"
#include "esstixfontstyle.h"
#include "cmstyle.h"
#include "symbolfontstyle.h"


KFORMULA_NAMESPACE_BEGIN


ConfigurePage::ConfigurePage( Document* document, QWidget* view, KConfig* config, KVBox* box, char* /*name*/ )
    : QObject( box->parent() ), m_document( document ), m_view( view ), m_config( config ), m_changed( false )
{
//    const ContextStyle& contextStyle = document->getContextStyle( true );

    // fonts

    Q3GroupBox *gbox = new Q3GroupBox( i18n( "Fonts" ), box );
    gbox->setColumnLayout( 0, Qt::Horizontal );

    Q3GridLayout* grid = new Q3GridLayout( gbox->layout(), 5, 3 );
    grid->setSpacing( KDialog::spacingHint() );

    grid->setColumnStretch(1, 1);

/*    defaultFont = contextStyle.getDefaultFont();
    nameFont = contextStyle.getNameFont();
    numberFont = contextStyle.getNumberFont();
    operatorFont = contextStyle.getOperatorFont();*/

    connect( buildFontLine( gbox, grid, 0, defaultFont,
            i18n( "Default font:" ), defaultFontName ),
            SIGNAL( clicked() ), SLOT( selectNewDefaultFont() ) );

    connect( buildFontLine( gbox, grid, 1, nameFont,
            i18n( "Name font:" ), nameFontName ),
            SIGNAL( clicked() ), SLOT( selectNewNameFont() ) );

    connect( buildFontLine( gbox, grid, 2, numberFont,
            i18n( "Number font:" ), numberFontName ),
            SIGNAL( clicked() ), SLOT( selectNewNumberFont() ) );

    connect( buildFontLine( gbox, grid, 3, operatorFont,
            i18n( "Operator font:" ), operatorFontName ),
            SIGNAL( clicked() ), SLOT( selectNewOperatorFont() ) );

    QLabel* sizeTitle = new QLabel( i18n( "Default base size:" ), gbox );
    grid->addWidget( sizeTitle, 4, 0 );

//    sizeSpin = new KIntNumInput( contextStyle.baseSize(), gbox );
//    sizeSpin->setRange( 8, 72, 1, true );

    grid->addMultiCellWidget( sizeSpin, 4, 4, 1, 2 );

    connect( sizeSpin, SIGNAL( valueChanged( int ) ),
            SLOT( baseSizeChanged( int ) ) );

    // syntax highlighting

 //   syntaxHighlighting = new QCheckBox( i18n( "Use syntax highlighting" ),box );
 //   syntaxHighlighting->setChecked( contextStyle.syntaxHighlighting() );

//     hlBox = new QGroupBox( i18n( "Highlight Colors" ), box );
//     hlBox->setColumnLayout( 0, Qt::Horizontal );

//     grid = new QGridLayout( hlBox->layout(), 5, 2 );
//     grid->setSpacing( KDialog::spacingHint() );

//     QLabel* defaultLabel = new QLabel( hlBox, "defaultLabel" );
//     defaultLabel->setText( i18n( "Default color:" ) );
//     grid->addWidget( defaultLabel, 0, 0 );

//     defaultColorBtn = new KColorButton( hlBox );
//defaultColorBtn->setObjectName( "defaultColor" );
//     defaultColorBtn->setColor( contextStyle.getDefaultColor() );
//     grid->addWidget( defaultColorBtn, 0, 1 );


//     QLabel* numberLabel = new QLabel( hlBox, "numberLabel" );
//     numberLabel->setText( i18n( "Number color:" ) );
//     grid->addWidget( numberLabel, 1, 0 );

//     numberColorBtn = new KColorButton( hlBox );
//numberColorBtn->setObjectName( "numberColor" );
//     numberColorBtn->setColor( contextStyle.getNumberColorPlain() );
//     grid->addWidget( numberColorBtn, 1, 1 );


//     QLabel* operatorLabel = new QLabel( hlBox, "operatorLabel" );
//     operatorLabel->setText( i18n( "Operator color:" ) );
//     grid->addWidget( operatorLabel, 2, 0 );

//     operatorColorBtn = new KColorButton( hlBox );
//operatorColorBtn->setObjectName( "operatorColor" );
//     operatorColorBtn->setColor( contextStyle.getOperatorColorPlain() );
//     grid->addWidget( operatorColorBtn, 2, 1 );


//     QLabel* emptyLabel = new QLabel( hlBox, "emptyLabel" );
//     emptyLabel->setText( i18n( "Empty color:" ) );
//     grid->addWidget( emptyLabel, 3, 0 );

//     emptyColorBtn = new KColorButton( hlBox );
//emptyColorBtn->setObjectName( "emptyColor" );
//     emptyColorBtn->setColor( contextStyle.getEmptyColorPlain() );
//     grid->addWidget( emptyColorBtn, 3, 1 );


//     QLabel* errorLabel = new QLabel( hlBox, "errorLabel" );
//     errorLabel->setText( i18n( "Error color:" ) );
//     grid->addWidget( errorLabel, 4, 0 );

//     errorColorBtn = new KColorButton( hlBox );
//errorColorBtn->setObjectName( "errorColor" );
//     errorColorBtn->setColor( contextStyle.getErrorColorPlain() );
//     grid->addWidget( errorColorBtn, 4, 1 );

    connect( syntaxHighlighting, SIGNAL( clicked() ),
            SLOT( syntaxHighlightingClicked() ) );

    syntaxHighlightingClicked();

    styleBox = new Q3ButtonGroup( i18n( "Font Style" ), box );
    styleBox->setColumnLayout( 0, Qt::Horizontal );

    grid = new Q3GridLayout( styleBox->layout(), 3, 1 );
    grid->setSpacing( KDialog::spacingHint() );

//    esstixStyle = new QRadioButton( i18n( "Esstix font style" ), styleBox );
//    esstixStyle->setChecked( contextStyle.getFontStyle() == "esstix" );

//    cmStyle = new QRadioButton( i18n( "Computer modern (TeX) style" ), styleBox );
//    cmStyle->setChecked( contextStyle.getFontStyle() == "tex" );

    symbolStyle = new QRadioButton( i18n( "Symbol font style" ), styleBox );
    symbolStyle->setChecked( !esstixStyle->isChecked() && !cmStyle->isChecked() );

    grid->addWidget( symbolStyle, 0, 0 );
    grid->addWidget( esstixStyle, 1, 0 );
    grid->addWidget( cmStyle, 2, 0 );

    connect( styleBox, SIGNAL( clicked( int ) ), this, SLOT( slotChanged() ) );
    connect( syntaxHighlighting, SIGNAL( clicked() ), this, SLOT( slotChanged() ) );
    connect( sizeSpin, SIGNAL( valueChanged( int ) ), this, SLOT( slotChanged() ) );

    Q_ASSERT( !m_changed );
}


QPushButton* ConfigurePage::buildFontLine( QWidget* parent,
            Q3GridLayout* layout, int number, QFont font, QString name,
            QLabel*& fontName )
{
    QLabel* fontTitle = new QLabel( name, parent );

    QString labelName = font.family() + ' ' + QString::number( font.pointSize() );
    fontName = new QLabel( labelName, parent );
    fontName->setFont( font );
    fontName->setFrameStyle(Q3Frame::StyledPanel | Q3Frame::Sunken);

    QPushButton* chooseButton = new QPushButton( i18n( "Choose..." ), parent );

    layout->addWidget( fontTitle, number, 0 );
    layout->addWidget( fontName, number, 1 );
    layout->addWidget( chooseButton, number, 2 );

    return chooseButton;
}


void ConfigurePage::apply()
{
    if ( !m_changed )
        return;
    QString fontStyle;
    if ( esstixStyle->isChecked() ) {
        fontStyle = "esstix";

        QStringList missing = EsstixFontStyle::missingFonts();

        if ( missing.count() > 0 ) {
            QString text = i18n( "The fonts '%1' are missing."
                                 " Do you want to change the font style anyway?"
                           ,missing.join( "', '" ) );
            if ( KMessageBox::warningContinueCancel( m_view, text ) ==
                 KMessageBox::Cancel ) {
                return;
            }
        }
    }
    else if ( cmStyle->isChecked() ) {
        fontStyle = "tex";

        QStringList missing = CMStyle::missingFonts();

        if ( missing.count() > 0 && !CMStyle::m_installed) {
            QString text = i18n( "The fonts '%1' are missing."
                                 " Do you want to change the font style anyway?"
                           , missing.join( "', '" ) );
            if ( KMessageBox::warningContinueCancel( m_view, text ) ==
                 KMessageBox::Cancel ) {
                return;
            }
        }
    }
    else { // symbolStyle->isChecked ()
        fontStyle = "symbol";

        QStringList missing = SymbolFontStyle::missingFonts();

        if ( missing.count() > 0 ) {
            QString text = i18n( "The font 'symbol' is missing."
                                 " Do you want to change the font style anyway?" );
            if ( KMessageBox::warningContinueCancel( m_view, text ) ==
                 KMessageBox::Cancel ) {
                return;
            }

        }
    }

/*    ContextStyle& contextStyle = m_document->getContextStyle( true );

    contextStyle.setDefaultFont( defaultFont );
    contextStyle.setNameFont( nameFont );
    contextStyle.setNumberFont( numberFont );
    contextStyle.setOperatorFont( operatorFont );
    contextStyle.setBaseSize( sizeSpin->value() );

    contextStyle.setFontStyle( fontStyle );

    contextStyle.setSyntaxHighlighting( syntaxHighlighting->isChecked() );*/
//     contextStyle.setDefaultColor( defaultColorBtn->color() );
//     contextStyle.setNumberColor( numberColorBtn->color() );
//     contextStyle.setOperatorColor( operatorColorBtn->color() );
//     contextStyle.setEmptyColor( emptyColorBtn->color() );
//     contextStyle.setErrorColor( errorColorBtn->color() );

    m_config->setGroup( "kformula Font" );
    m_config->writeEntry( "defaultFont", defaultFont.toString() );
    m_config->writeEntry( "nameFont", nameFont.toString() );
    m_config->writeEntry( "numberFont", numberFont.toString() );
    m_config->writeEntry( "operatorFont", operatorFont.toString() );
    m_config->writeEntry( "baseSize", QString::number( sizeSpin->value() ) );

    m_config->writeEntry( "fontStyle", fontStyle );

//     m_config->setGroup( "kformula Color" );
//     m_config->writeEntry( "syntaxHighlighting", syntaxHighlighting->isChecked() );
//     m_config->writeEntry( "defaultColor", defaultColorBtn->color() );
//     m_config->writeEntry( "numberColor",  numberColorBtn->color() );
//     m_config->writeEntry( "operatorColor", operatorColorBtn->color() );
//     m_config->writeEntry( "emptyColor", emptyColorBtn->color() );
//     m_config->writeEntry( "errorColor", errorColorBtn->color() );

    // notify!!!
//    m_document->updateConfig();
    m_changed = false;
}

void ConfigurePage::slotDefault()
{
    defaultFont = QFont( "Times", 12, QFont::Normal, true );
    nameFont = QFont( "Times" );
    numberFont = nameFont;
    operatorFont = nameFont;

    sizeSpin->setValue( 20 );

    updateFontLabel( defaultFont, defaultFontName );
    updateFontLabel( nameFont, nameFontName );
    updateFontLabel( numberFont, numberFontName );
    updateFontLabel( operatorFont, operatorFontName );

    symbolStyle->setChecked( true );
    if (CMStyle::missingFonts().isEmpty())
        cmStyle->setChecked( true );
    else if (EsstixFontStyle::missingFonts().isEmpty())
        esstixStyle->setChecked( true );
    else
        symbolStyle->setChecked( true );

    syntaxHighlighting->setChecked( true );
    syntaxHighlightingClicked();

//     defaultColorBtn->setColor( Qt::black );
//     numberColorBtn->setColor( Qt::blue );
//     operatorColorBtn->setColor( Qt::darkGreen );
//     emptyColorBtn->setColor( Qt::blue );
//     errorColorBtn->setColor( Qt::darkRed );
    slotChanged();
}

void ConfigurePage::syntaxHighlightingClicked()
{
//     bool checked = syntaxHighlighting->isChecked();
//     hlBox->setEnabled( checked );
}

void ConfigurePage::selectNewDefaultFont()
{
    if ( selectFont( defaultFont ) )
        updateFontLabel( defaultFont, defaultFontName );
}

void ConfigurePage::selectNewNameFont()
{
    if ( selectFont( nameFont ) )
        updateFontLabel( nameFont, nameFontName );
}

void ConfigurePage::selectNewNumberFont()
{
    if ( selectFont( numberFont ) )
        updateFontLabel( numberFont, numberFontName );
}

void ConfigurePage::selectNewOperatorFont()
{
    if ( selectFont( operatorFont ) )
        updateFontLabel( operatorFont, operatorFontName );
}

bool ConfigurePage::selectFont( QFont & font )
{
    QStringList list;

    KFontChooser::getFontList( list, KFontChooser::SmoothScalableFonts );

    KFontDialog dlg( m_view, false, true, list );
    dlg.setFont( font );

    int result = dlg.exec();
    if (  KDialog::Accepted == result ) {
        font = dlg.font();
        slotChanged();
        return true;
    }

    return false;
}

void ConfigurePage::baseSizeChanged( int /*value*/ )
{
}

void ConfigurePage::updateFontLabel( QFont font, QLabel* label )
{
    label->setText( font.family() + ' ' + QString::number( font.pointSize() ) );
    label->setFont( font );
}

void ConfigurePage::slotChanged()
{
    m_changed = true;
}

// class UsedFontItem : public K3ListViewItem {
// public:
//     UsedFontItem( MathFontsConfigurePage* page, QListView* parent, QString font )
//         : K3ListViewItem( parent, font ), m_page( page ) {}

//     int compare( QListViewItem* i, int col, bool ascending ) const;

// private:
//     MathFontsConfigurePage* m_page;
// };

// int UsedFontItem::compare( QListViewItem* i, int, bool ) const
// {
//     QValueVector<QString>::iterator lhsIt = m_page->findUsedFont( text( 0 ) );
//     QValueVector<QString>::iterator rhsIt = m_page->findUsedFont( i->text( 0 ) );
//     if ( lhsIt < rhsIt ) {
//         return -1;
//     }
//     else if ( lhsIt > rhsIt ) {
//         return 1;
//     }
//     return 0;
// }

// MathFontsConfigurePage::MathFontsConfigurePage( Document* document, QWidget* view,
//                                                 KConfig* config, QVBox* box, char* name )
//     : QObject( box->parent(), name ), m_document( document ), m_view( view ), m_config( config )
// {
//     QWidget* fontWidget = new QWidget( box );
//     QGridLayout* fontLayout = new QGridLayout( fontWidget, 1, 1, KDialog::marginHint(), KDialog::spacingHint() );

//     QHBoxLayout* hLayout = new QHBoxLayout( 0, 0, 6 );

//     availableFonts = new K3ListView( fontWidget );
//     availableFonts->addColumn( i18n( "Available Fonts" ) );
//     hLayout->addWidget( availableFonts );

//     QVBoxLayout* vLayout = new QVBoxLayout( 0, 0, 6 );
//     QSpacerItem* spacer1 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
//     vLayout->addItem( spacer1 );

//     addFont = new KPushButton( fontWidget );
//     addFont->setText( "->" );
//     vLayout->addWidget( addFont );

//     removeFont = new KPushButton( fontWidget );
//     removeFont->setText( "<-" );
//     vLayout->addWidget( removeFont );

//     QSpacerItem* spacer2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
//     vLayout->addItem( spacer2 );

//     hLayout->addLayout( vLayout );

//     vLayout = new QVBoxLayout( 0, 0, 6 );

//     moveUp = new KPushButton( fontWidget );
//     moveUp->setText( i18n( "Up" ) );
//     vLayout->addWidget( moveUp );

//     requestedFonts = new K3ListView( fontWidget );
//     requestedFonts->addColumn( i18n( "Used Fonts" ) );
//     vLayout->addWidget( requestedFonts );

//     moveDown = new KPushButton( fontWidget );
//     moveDown->setText( i18n( "Down" ) );
//     vLayout->addWidget( moveDown );

//     hLayout->addLayout( vLayout );

//     fontLayout->addLayout( hLayout, 0, 0 );

// //     connect( availableFonts, SIGNAL( executed( QListViewItem* ) ),
// //              this, SLOT( slotAddFont() ) );
// //     connect( requestedFonts, SIGNAL( executed( QListViewItem* ) ),
// //              this, SLOT( slotRemoveFont() ) );
//     connect( addFont, SIGNAL( clicked() ), this, SLOT( slotAddFont() ) );
//     connect( removeFont, SIGNAL( clicked() ), this, SLOT( slotRemoveFont() ) );
//     connect( moveUp, SIGNAL( clicked() ), this, SLOT( slotMoveUp() ) );
//     connect( moveDown, SIGNAL( clicked() ), this, SLOT( slotMoveDown() ) );

//     const ContextStyle& contextStyle = document->getContextStyle( true );
//     const SymbolTable& symbolTable = contextStyle.symbolTable();
//     const QStringList& usedFonts = contextStyle.requestedFonts();

//     QMap<QString, QString> fontMap;
// //    symbolTable.findAvailableFonts( &fontMap );

//     setupLists( usedFonts );
// }

// void MathFontsConfigurePage::apply()
// {
//     QStringList strings;
//     std::copy( usedFontList.begin(), usedFontList.end(), std::back_inserter( strings ) );

//     m_config->setGroup( "kformula Font" );
//     m_config->writeEntry( "usedMathFonts", strings );

//     ContextStyle& contextStyle = m_document->getContextStyle( true );
//     contextStyle.setRequestedFonts( strings );
// }

// void MathFontsConfigurePage::slotDefault()
// {
//     QStringList usedFonts;

//     usedFonts.push_back( "esstixone" );
//     usedFonts.push_back( "esstixtwo" );
//     usedFonts.push_back( "esstixthree" );
//     usedFonts.push_back( "esstixfour" );
//     usedFonts.push_back( "esstixfive" );
//     usedFonts.push_back( "esstixsix" );
//     usedFonts.push_back( "esstixseven" );
//     usedFonts.push_back( "esstixeight" );
//     usedFonts.push_back( "esstixnine" );
//     usedFonts.push_back( "esstixten" );
//     usedFonts.push_back( "esstixeleven" );
//     usedFonts.push_back( "esstixtwelve" );
//     usedFonts.push_back( "esstixthirteen" );
//     usedFonts.push_back( "esstixfourteen" );
//     usedFonts.push_back( "esstixfifteen" );
//     usedFonts.push_back( "esstixsixteen" );
//     usedFonts.push_back( "esstixseventeen" );

//     usedFontList.clear();
//     requestedFonts->clear();
//     availableFonts->clear();

//     setupLists( usedFonts );
// }

// QValueVector<QString>::iterator MathFontsConfigurePage::findUsedFont( QString name )
// {
//     return std::find( usedFontList.begin(), usedFontList.end(), name );
// }

// void MathFontsConfigurePage::setupLists( const QStringList& usedFonts )
// {
//     const ContextStyle& contextStyle = m_document->getContextStyle( true );
//     const SymbolTable& symbolTable = contextStyle.symbolTable();

//     QMap<QString, QString> fontMap;
// //    symbolTable.findAvailableFonts( &fontMap );

//     for ( QStringList::const_iterator it = usedFonts.begin(); it != usedFonts.end(); ++it ) {
//         QMap<QString, QString>::iterator font = fontMap.find( *it );
//         if ( font != fontMap.end() ) {
//             fontMap.erase( font );
//             new UsedFontItem( this, requestedFonts, *it );
//             usedFontList.push_back( *it );
//         }
//     }
//     for ( QMap<QString, QString>::iterator it = fontMap.begin(); it != fontMap.end(); ++it ) {
//         new K3ListViewItem( availableFonts, it.key() );
//     }
// }

// void MathFontsConfigurePage::slotAddFont()
// {
//     QListViewItem* fontItem = availableFonts->selectedItem();
//     if ( fontItem ) {
//         QString fontName = fontItem->text( 0 );
//         //availableFonts->takeItem( fontItem );
//         delete fontItem;

//         new UsedFontItem( this, requestedFonts, fontName );
//         usedFontList.push_back( fontName );
//     }
// }

// void MathFontsConfigurePage::slotRemoveFont()
// {
//     QListViewItem* fontItem = requestedFonts->selectedItem();
//     if ( fontItem ) {
//         QString fontName = fontItem->text( 0 );
//         QValueVector<QString>::iterator it = std::find( usedFontList.begin(), usedFontList.end(), fontName );
//         if ( it != usedFontList.end() ) {
//             usedFontList.erase( it );
//         }
//         //requestedFonts->takeItem( fontItem );
//         delete fontItem;
//         new K3ListViewItem( availableFonts, fontName );
//     }
// }

// void MathFontsConfigurePage::slotMoveUp()
// {
//     QListViewItem* fontItem = requestedFonts->selectedItem();
//     if ( fontItem ) {
//         QString fontName = fontItem->text( 0 );
//         QValueVector<QString>::iterator it = std::find( usedFontList.begin(), usedFontList.end(), fontName );
//         if ( it != usedFontList.end() ) {
//             uint pos = it - usedFontList.begin();
//             if ( pos > 0 ) {
//                 QValueVector<QString>::iterator before = it-1;
//                 std::swap( *it, *before );
//                 requestedFonts->sort();
//             }
//         }
//     }
// }

// void MathFontsConfigurePage::slotMoveDown()
// {
//     QListViewItem* fontItem = requestedFonts->selectedItem();
//     if ( fontItem ) {
//         QString fontName = fontItem->text( 0 );
//         QValueVector<QString>::iterator it = std::find( usedFontList.begin(), usedFontList.end(), fontName );
//         if ( it != usedFontList.end() ) {
//             uint pos = it - usedFontList.begin();
//             if ( pos < usedFontList.size()-1 ) {
//                 QValueVector<QString>::iterator after = it+1;
//                 std::swap( *it, *after );
//                 requestedFonts->sort();
//             }
//         }
//     }
// }

KFORMULA_NAMESPACE_END

using namespace KFormula;
#include "kformulaconfigpage.moc"
