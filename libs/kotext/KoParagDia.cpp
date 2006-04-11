/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2005 Martin Ellis <martin.ellis@kdemail.net>

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

#include "KoParagDia.h"
#include "KoParagDia_p.h"
#include "KoDocument.h"
#include "KoTextFormat.h"
#include "KoTextParag.h"
#include "KoTextDocument.h"
#include "KoTextZoomHandler.h"
#include "KoParagDecorationTab.h"

#include <KoCharSelectDia.h>
#include <KoUnitWidgets.h>
#include <kcolorbutton.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knumvalidator.h>
#include <KoGlobal.h>
#include <q3groupbox.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3VBoxLayout>
#include <Q3Frame>
#include <QKeyEvent>
#include <Q3GridLayout>
#include <Q3PtrList>
#include <Q3HBoxLayout>
#include <knuminput.h>
#include <kdeversion.h>
#include <kpushbutton.h>
#include <kcombobox.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <kvbox.h>
#include <q3hbox.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qapplication.h>
#include <q3widgetstack.h>
#include <q3tl.h>
#include <QListWidget>
#include <Q3ListBox>

KoCounterStyleWidget::KoCounterStyleWidget( bool displayDepth, bool onlyStyleTypeLetter, bool disableAll, QWidget * parent  )
    :QWidget( parent ),
    stylesList()
{
    noSignals = true;
    styleBuffer = 999;
    Q3VBoxLayout *vbox = new Q3VBoxLayout( this,0, 0/*KDialog::marginHint(), KDialog::spacingHint()*/ );
    gStyle = new Q3GroupBox( i18n( "St&yle" ), this, "styleLayout" );
    vbox->addWidget( gStyle);
    Q3GridLayout * grid = new Q3GridLayout(gStyle, 12, 5, KDialog::marginHint(), KDialog::spacingHint());
    grid->addRowSpacing(0, fontMetrics().height()/2);

    makeCounterRepresenterList( stylesList, onlyStyleTypeLetter );

    lstStyle = new QListWidget( gStyle );
    grid->addMultiCellWidget( lstStyle, 1, 11, 0, 0);
    fillStyleCombo();
    connect( lstStyle, SIGNAL( itemSelectionChanged() ), this, SLOT( numStyleChanged() ) );


    QLabel *lPrefix = new QLabel( gStyle, "lPrefix" );
    lPrefix->setText( i18n( "Pre&fix text:" ) );
    grid->addWidget( lPrefix, 1, 1);

    sPrefix = new QLineEdit( gStyle, "sPrefix" );
    lPrefix->setBuddy( sPrefix );
    grid->addWidget( sPrefix, 1, 2);

    QLabel *lSuffix = new QLabel( gStyle, "lSuffix" );
    lSuffix->setText( i18n( "Suffi&x text:" ) );
    grid->addWidget( lSuffix, 1, 3);

    sSuffix = new QLineEdit( gStyle, "sSuffix" );
    lSuffix->setBuddy( sSuffix );
    grid->addWidget( sSuffix, 1, 4 );

    lStart = new QLabel( gStyle, "lStart" );
    lStart->setText( i18n( "&Start at:" ) );
    grid->addWidget( lStart, 2, 1 );


    spnDepth = new QSpinBox( 0, 15, 1, gStyle );
    if (  displayDepth )
        grid->addWidget( spnDepth, 3, 2 );
    else
        spnDepth->hide();

    spnDisplayLevels = new QSpinBox( 0, 15, 1, gStyle );
    spnDisplayLevels->setMinValue( 1 );
    if ( displayDepth )
        grid->addWidget( spnDisplayLevels, 3, 4 );
    else
        spnDisplayLevels->hide();


    Q3HBoxLayout *customCharBox = new Q3HBoxLayout(0, 0, 6);
    lCustom = new QLabel( i18n( "Custo&m character:" ), gStyle, "custom char label" );
    customCharBox->addWidget( lCustom );

    bCustom = new QPushButton( "", gStyle, "bCustom" );
    lCustom->setBuddy( bCustom );
    customCharBox->addWidget( bCustom );
    connect( bCustom, SIGNAL( clicked() ), this, SLOT( selectCustomBullet() ) );

    QSpacerItem* spacer_2 = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    customCharBox->addItem( spacer_2 );

    grid->addMultiCellLayout(customCharBox, 4, 4, 1, 4, Qt::AlignLeft);

    spnStart = new KoSpinBox( gStyle );
    spnStart->setMinValue ( 1);
    lStart->setBuddy( spnStart );
    grid->addWidget( spnStart, 2, 2);

    lAlignment = new QLabel( gStyle );
    lAlignment->setText( i18n( "Counter alignment:" ) );
    grid->addWidget( lAlignment, 2, 3 );

    cbAlignment = new KComboBox( gStyle );
    cbAlignment->addItem(i18n("Align Auto"));
    cbAlignment->addItem(i18n("Align Left"));
    cbAlignment->addItem(i18n("Align Right"));
    cbAlignment->setCurrentItem(0);
    grid->addWidget( cbAlignment, 2, 4 );

    QLabel *lDepth = new QLabel( gStyle, "lDepth" );
    lDepth->setText( i18n( "&Depth:" ) );
    lDepth->setBuddy( spnDepth );
    if ( displayDepth )
        grid->addWidget( lDepth, 3, 1 );
    else
        lDepth->hide();

    QLabel *lDisplayLevels = new QLabel( gStyle );
    lDisplayLevels->setText( i18n( "Display le&vels:" ) );
    lDisplayLevels->setBuddy( spnDisplayLevels );
    if ( displayDepth )
        grid->addWidget( lDisplayLevels, 3, 3 );
    else
        lDisplayLevels->hide();

    cbRestart = new QCheckBox( i18n( "&Restart numbering at this paragraph" ), gStyle );
    grid->addMultiCellWidget( cbRestart, 5, 5, 1, 3 );

    if ( onlyStyleTypeLetter )
    {
        lCustom->hide();
        bCustom->hide();
        cbRestart->hide();
    }


    connect( cbRestart, SIGNAL( toggled(bool) ), this, SLOT( restartChanged(bool) ) );

    connect( sSuffix, SIGNAL( textChanged (const QString &) ), this, SLOT( suffixChanged(const QString &) ) );
    connect( sPrefix, SIGNAL( textChanged (const QString &) ), this, SLOT( prefixChanged(const QString &) ) );
    connect( spnStart, SIGNAL( valueChanged (int) ), this, SLOT( startChanged(int) ) );
    connect( spnDepth, SIGNAL( valueChanged (int) ), this, SLOT( depthChanged(int) ) );
    connect( spnDisplayLevels, SIGNAL( valueChanged (int) ), this, SLOT( displayLevelsChanged(int) ) );
    connect( cbAlignment, SIGNAL( activated (const QString&) ), this, SLOT( alignmentChanged(const QString&) ) );
    noSignals = false;
    if ( disableAll )
    {
        gStyle->setEnabled( false );
        lstStyle->setEnabled( false );
        sSuffix->setEnabled( false );
        sPrefix->setEnabled( false );
        bCustom->setEnabled( false );
        spnStart->setEnabled( false );
        spnDepth->setEnabled( false );
        spnDisplayLevels->setEnabled( false );
        lStart->setEnabled( false );
        lCustom->setEnabled( false );
        cbRestart->setEnabled( false );
        cbAlignment->setEnabled( false );
    }
}

void KoCounterStyleWidget::alignmentChanged(const QString& s)
{
    int a;
    if(s==i18n("Align Left"))
        a=Qt::AlignLeft;
    else if(s==i18n("Align Right"))
        a=Qt::AlignRight;
    else if(s==i18n("Align Auto"))
        a=Qt::AlignLeft;
    else {
        kError()<<"Not Implemented"<<endl;
        return;
    }
    m_counter.setAlignment(a);
    emit sig_alignmentChanged(a);
}

void KoCounterStyleWidget::setCounter( const KoParagCounter& counter )
{
    noSignals = true;
    KoParagCounter::Style st = counter.style();
    m_counter = counter;
    // Huh? doesn't the line above do this already?
    //m_counter.setStyle( st );
    changeKWSpinboxType( st);
    displayStyle( st );
    noSignals = false;
}

void KoCounterStyleWidget::changeKWSpinboxType(KoParagCounter::Style st) {
    switch(st)
    {
        case KoParagCounter::STYLE_NONE:
            spnStart->setCounterType(KoSpinBox::NONE);
            break;
        case KoParagCounter::STYLE_NUM:
            spnStart->setCounterType(KoSpinBox::NUM);
            break;
        case KoParagCounter::STYLE_ALPHAB_L:
            spnStart->setCounterType(KoSpinBox::ALPHAB_L);
            break;
        case KoParagCounter::STYLE_ALPHAB_U:
            spnStart->setCounterType(KoSpinBox::ALPHAB_U);
            break;
        case KoParagCounter::STYLE_ROM_NUM_L:
            spnStart->setCounterType(KoSpinBox::ROM_NUM_L);
            break;
        case KoParagCounter::STYLE_ROM_NUM_U:
            spnStart->setCounterType(KoSpinBox::ROM_NUM_U);
            break;
        default:
            spnStart->setCounterType(KoSpinBox::NONE);
    }
}


void KoCounterStyleWidget::fillStyleCombo(KoParagCounter::Numbering type) {
    if(lstStyle==NULL) return;
    noSignals=true;
    unsigned int cur = lstStyle->currentRow();
    lstStyle->clear();
    Q3PtrListIterator<StyleRepresenter> style( stylesList );
    while ( style.current() ) {
        if(style.current()->style() == KoParagCounter::STYLE_NONE) {
            if(type == KoParagCounter::NUM_NONE)
                lstStyle->addItem( style.current()->name() );
        }
        else if(type == KoParagCounter::NUM_LIST || !style.current()->isBullet())
            if(type != KoParagCounter::NUM_NONE)
                lstStyle->addItem( style.current()->name() );
        ++style;
    }

    if(styleBuffer <= lstStyle->count())
        lstStyle->setCurrentRow(styleBuffer);
    else
        if(cur <= lstStyle->count())
            lstStyle->setCurrentRow(cur);

    if(cur > lstStyle->count()) {
        styleBuffer = cur;
    }
    noSignals=false;
}

void KoCounterStyleWidget::displayStyle( KoParagCounter::Style style )
{
    unsigned int i = 0;
    while ( stylesList.count() > i && stylesList.at(i)->style() != style )
        ++i;
    lstStyle->setCurrentRow(i);

    bCustom->setText( m_counter.customBulletCharacter() );
    if ( !m_counter.customBulletFont().isEmpty() )
        bCustom->setFont( QFont( m_counter.customBulletFont() ) );

    sPrefix->setText( m_counter.prefix() );
    sSuffix->setText( m_counter.suffix() );

    spnDepth->setValue( m_counter.depth() );
    spnDisplayLevels->setValue( m_counter.displayLevels() );
    spnStart->setValue( m_counter.startNumber() );

    cbRestart->setChecked( m_counter.restartCounter() );
    if(m_counter.alignment()==Qt::AlignLeft)
        cbAlignment->setCurrentText(i18n("Align Left"));
    else if(m_counter.alignment()==Qt::AlignRight)
        cbAlignment->setCurrentText(i18n("Align Right"));
    else if(m_counter.alignment()==Qt::AlignLeft)
        cbAlignment->setCurrentText(i18n("Align Auto"));
    else
        kError()<<"Not Implemented"<<endl;
}

void KoCounterStyleWidget::display( const KoParagLayout & lay ) {
    KoParagCounter::Style style = KoParagCounter::STYLE_NONE;
    if ( lay.counter )
    {
        style=lay.counter->style();
        m_counter = *lay.counter;
    }
    else
    {
        m_counter = KoParagCounter();
    }
    styleBuffer = 999;

    numTypeChanged( m_counter.numbering() );
    emit sig_numTypeChanged( m_counter.numbering() );

    displayStyle( style );
}


void KoCounterStyleWidget::numTypeChanged( int nType ) {
    m_counter.setNumbering( static_cast<KoParagCounter::Numbering>( nType ) );
    gStyle->setEnabled( m_counter.numbering() != KoParagCounter::NUM_NONE );
    fillStyleCombo(m_counter.numbering());
    bool state=m_counter.numbering()==KoParagCounter::NUM_LIST;
    bCustom->setEnabled(state);
    lCustom->setEnabled(state);
}


void KoCounterStyleWidget::makeCounterRepresenterList( Q3PtrList<StyleRepresenter>& stylesList, bool onlyStyleTypeLetter )
{
    stylesList.setAutoDelete( true );
    stylesList.append( new StyleRepresenter(i18n( "Arabic Numbers" )
            ,  KoParagCounter::STYLE_NUM));
    stylesList.append( new StyleRepresenter(i18n( "Lower Alphabetical" )
            ,  KoParagCounter::STYLE_ALPHAB_L ));
    stylesList.append( new StyleRepresenter(i18n( "Upper Alphabetical" )
            ,  KoParagCounter::STYLE_ALPHAB_U ));
    stylesList.append( new StyleRepresenter(i18n( "Lower Roman Numbers" )
            ,  KoParagCounter::STYLE_ROM_NUM_L ));
    stylesList.append( new StyleRepresenter(i18n( "Upper Roman Numbers" )
            ,  KoParagCounter::STYLE_ROM_NUM_U ));
    if ( !onlyStyleTypeLetter )
    {
        stylesList.append( new StyleRepresenter(i18n( "Disc Bullet" )
                                                ,  KoParagCounter::STYLE_DISCBULLET , true));
        stylesList.append( new StyleRepresenter(i18n( "Square Bullet" )
                                                ,  KoParagCounter::STYLE_SQUAREBULLET , true));
        stylesList.append( new StyleRepresenter(i18n( "Box Bullet" )
                                                ,  KoParagCounter::STYLE_BOXBULLET , true));
        stylesList.append( new StyleRepresenter(i18n( "Circle Bullet" )
                                                ,  KoParagCounter::STYLE_CIRCLEBULLET , true));
        stylesList.append( new StyleRepresenter(i18n( "Custom Bullet" )
                                                ,  KoParagCounter::STYLE_CUSTOMBULLET , true));
    }
    stylesList.append( new StyleRepresenter(i18n( "None" ), KoParagCounter::STYLE_NONE));
}


void KoCounterStyleWidget::selectCustomBullet() {
    unsigned int i = 0;
    while ( stylesList.count() > i && stylesList.at(i)->style() != KoParagCounter::STYLE_CUSTOMBULLET )
        ++i;
    lstStyle->setCurrentRow(i);

    QString f = m_counter.customBulletFont();
    if ( f.isEmpty() )
        f = "symbol";
    QChar c = m_counter.customBulletCharacter();

    if ( KoCharSelectDia::selectChar( f, c ) ) {
        emit changeCustomBullet( f, c );
        m_counter.setCustomBulletFont( f );
        m_counter.setCustomBulletCharacter( c );
        if ( !f.isEmpty() )
            bCustom->setFont( QFont( f ) );
        bCustom->setText( c );
    }
}

void KoCounterStyleWidget::numStyleChanged() {
    if ( noSignals )
        return;
    // We selected another style from the list box.
    styleBuffer = 999;
    StyleRepresenter *sr = stylesList.at(lstStyle->currentRow());
    emit changeStyle( sr->style() );
    m_counter.setStyle( sr->style() );
    bool isNumbered = !sr->isBullet() && !sr->style() == KoParagCounter::STYLE_NONE;
    lStart->setEnabled( isNumbered );
    spnStart->setEnabled( isNumbered );
    cbRestart->setEnabled( isNumbered );
    spnDisplayLevels->setEnabled( isNumbered );
    changeKWSpinboxType(sr->style() );
}



KoSpinBox::KoSpinBox( QWidget * parent )
    : QSpinBox( parent )
{
    m_Etype=NONE;
    //max value supported by roman number
    setMaxValue ( 3999 );
}
KoSpinBox::~KoSpinBox( )
{
}

KoSpinBox::KoSpinBox( int minValue, int maxValue, int step, QWidget * parent )
    : QSpinBox( minValue, maxValue, step, parent )
{
    m_Etype = NONE;
}

void KoSpinBox::setCounterType(CounterType _type)
{
    m_Etype = _type;
    lineEdit()->setText(mapValueToText(value()));
}


QString KoSpinBox::mapValueToText( int value )
{
    if(value==0 && m_Etype==NUM)
        return QString("0");
    else if(value==0 && m_Etype!=NUM)
        return QString::null;
    switch(m_Etype)
    {
        case NUM:
            return QString::number(value);
        case ALPHAB_L:
            return KoParagCounter::makeAlphaLowerNumber( value );
        case ALPHAB_U:
            return KoParagCounter::makeAlphaUpperNumber( value );
        case ROM_NUM_L:
            return KoParagCounter::makeRomanNumber( value );
        case ROM_NUM_U:
            return KoParagCounter::makeRomanNumber( value ).upper();
        case NONE:
        default:
            return QString::null;
    }
    //never here
    return QString::null;
}

int KoSpinBox::mapTextToValue( bool * ok )
{
    int ret;
    QString txt = text();

    *ok = TRUE;
    switch(m_Etype)
    {
        case NUM:
            ret = txt.toInt ( ok );
            break;
        case ALPHAB_L:
            ret = KoParagCounter::fromAlphaLowerNumber( txt.lower() );
            break;
        case ALPHAB_U:
            ret = KoParagCounter::fromAlphaUpperNumber( txt.upper() );
            break;
        case ROM_NUM_L:
            ret = KoParagCounter::fromRomanNumber( txt.lower() );
            break;
        case ROM_NUM_U:
            ret = KoParagCounter::fromRomanNumber( txt.lower() ); // _not_ upper()
            break;
        case NONE:
        default:
            ret = -1;
            break;
    }

    if (ret == -1)
        *ok = FALSE;

    return ret;
}


/******************************************************************/
/* class KPagePreview                                            */
/******************************************************************/

KPagePreview::KPagePreview( QWidget* parent )
    : Q3GroupBox( i18n( "Preview" ), parent )
{
    left = 0;
    right = 0;
    first = 0;
    spacing = 0;
    before = 0;
    after = 0;
}

void KPagePreview::drawContents( QPainter* p )
{
    int wid = 148;
    int hei = 210;
    int _x = ( width() - wid ) / 5;
    int _y = ( height() - hei ) / 5;

    int dl = convert(left);
    int dr = convert(right);

    //first+left because firstlineIndent is relative to leftIndent
    int df = convert(first) + dl;

    int spc = convert(spacing);

    // draw page
    p->setPen( QPen( Qt::black ) );
    p->setBrush( QBrush( Qt::black ) );

    p->drawRect( _x + 1, _y + 1, wid, hei );

    p->setBrush( QBrush( Qt::white ) );
    p->drawRect( _x, _y, wid, hei );

    // draw parags
    p->setPen( Qt::NoPen );
    p->setBrush( QBrush( Qt::lightGray ) );

    for ( int i = 1; i <= 4; i++ )
        p->drawRect( _x + 6, _y + 6 + ( i - 1 ) * 12 + 2, wid - 12 - ( ( i / 4 ) * 4 == i ? 50 : 0 ), 6 );

    p->setBrush( QBrush( Qt::darkGray ) );

    for ( int i = 5; i <= 8; i++ )
      {
	QRect rect( ( i == 5 ? df : dl ) + _x + 6, _y + 6 + ( i - 1 ) * 12 + 2 + ( i - 5 ) * spc + static_cast<int>( before / 2 ),
		    wid - 12 - ( ( i / 4 ) * 4 == i ? 50 : 0 ) - ( ( i == 12 ? 0 : dr ) + ( i == 5 ? df : dl ) ), 6);

	if(rect.width ()>=0)
	  p->drawRect( rect );
      }
    p->setBrush( QBrush( Qt::lightGray ) );

    for ( int i = 9; i <= 12; i++ )
        p->drawRect( _x + 6, _y + 6 + ( i - 1 ) * 12 + 2 + 3 * spc +
                     static_cast<int>( before / 2 ) + static_cast<int>( after / 2 ),
                     wid - 12 - ( ( i / 4 ) * 4 == i ? 50 : 0 ), 6 );

}

int KPagePreview::convert(double input) {
    if(input < 1) return 0;
    if(input <= 5) return 3;
    if(input <= 10) return 4 + static_cast<int>( (input-5) / 2.5 );
    if(input <= 20) return 6 + static_cast<int>( (input-10) / 4 );
    if(input <= 100) return 10 + static_cast<int>( (input-20) / 8 );
    return static_cast<int>( input / 5);
}

/******************************************************************/
/* class KPagePreview2                                           */
/******************************************************************/

KPagePreview2::KPagePreview2( QWidget* parent )
    : Q3GroupBox( i18n( "Preview" ), parent )
{
    align = Qt::AlignLeft;
}

void KPagePreview2::drawContents( QPainter* p )
{
    int wid = 148;
    int hei = 210;
    int _x = ( width() - wid ) / 2;
    int _y = ( height() - hei ) / 2;

    // draw page
    p->setPen( QPen( Qt::black ) );
    p->setBrush( QBrush( Qt::black ) );

    p->drawRect( _x + 1, _y + 1, wid, hei );

    p->setBrush( QBrush( Qt::white ) );
    p->drawRect( _x, _y, wid, hei );

    // draw parags
    p->setPen( Qt::NoPen );
    p->setBrush( QBrush( Qt::lightGray ) );

    for ( int i = 1; i <= 4; i++ )
        p->drawRect( _x + 6, _y + 6 + ( i - 1 ) * 12 + 2, wid - 12 - ( ( i / 4 ) * 4 == i ? 50 : 0 ), 6 );

    p->setBrush( QBrush( Qt::darkGray ) );

    int __x = 0, __w = 0;
    for ( int i = 5; i <= 8; i++ ) {
        switch ( i ) {
        case 5: __w = wid - 12;
            break;
        case 6: __w = wid - 52;
            break;
        case 7: __w = wid - 33;
            break;
        case 8: __w = wid - 62;
        default: break;
        }

        switch ( align ) {
            case Qt::AlignLeft:
                __x = _x + 6;
                break;
            case Qt::AlignHCenter:
                __x = _x + ( wid - __w ) / 2;
                break;
            case Qt::AlignRight:
                __x = _x + ( wid - __w ) - 6;
                break;
            case Qt::AlignJustify:
            {
                if ( i < 8 ) __w = wid - 12;
                __x = _x + 6;
            } break;
        }

        p->drawRect( __x, _y + 6 + ( i - 1 ) * 12 + 2 + ( i - 5 ), __w, 6 );
    }

    p->setBrush( QBrush( Qt::lightGray ) );

    for ( int i = 9; i <= 12; i++ )
        p->drawRect( _x + 6, _y + 6 + ( i - 1 ) * 12 + 2 + 3, wid - 12 - ( ( i / 4 ) * 4 == i ? 50 : 0 ), 6 );

}

/******************************************************************/
/* class KoBorderPreview                                          */
/******************************************************************/


KoBorderPreview::KoBorderPreview( QWidget* parent )
    :Q3Frame(parent)
{
}

void KoBorderPreview::mousePressEvent( QMouseEvent *_ev )
{
    emit choosearea(_ev);
}

void KoBorderPreview::setBorder( KoBorder::BorderType which, const KoBorder& border)
{
    switch( which ) {
    case KoBorder::TopBorder:
        setTopBorder( border );
        break;
    case KoBorder::BottomBorder:
        setBottomBorder( border );
        break;
    case KoBorder::LeftBorder:
        setLeftBorder( border );
        break;
    case KoBorder::RightBorder:
        setRightBorder( border );
        break;
    default:
        kError() << "KoBorderPreview: unknown border type" << endl;
    }
}

void KoBorderPreview::drawContents( QPainter* painter )
{
    QRect r = contentsRect();
    QFontMetrics fm( font() );

    painter->fillRect( r.x() + fm.width( 'W' ), r.y() + fm.height(), r.width() - 2 * fm.width( 'W' ),
                       r.height() - 2 * fm.height(), Qt::white );
    painter->setClipRect( r.x() + fm.width( 'W' ), r.y() + fm.height(), r.width() - 2 * fm.width( 'W' ),
                          r.height() - 2 * fm.height() );

    bool leftdouble = m_leftBorder.width() > 0 && m_leftBorder.getStyle() == KoBorder::DOUBLE_LINE;
    bool rightdouble = m_rightBorder.width() > 0 && m_rightBorder.getStyle() == KoBorder::DOUBLE_LINE;
    bool topdouble = m_topBorder.width() > 0 && m_topBorder.getStyle() == KoBorder::DOUBLE_LINE;
    bool bottomdouble = m_bottomBorder.width() > 0 && m_bottomBorder.getStyle() == KoBorder::DOUBLE_LINE;

    if ( m_topBorder.width() > 0 ) {
        painter->setPen( setBorderPen( m_topBorder ) );
        painter->drawLine( r.x() + 20, r.y() + 30, r.right() - 19, r.y() + 30 );
        if ( m_topBorder.getStyle()==KoBorder::DOUBLE_LINE)
            painter->drawLine( int(r.x() + 20 + ( leftdouble ? m_leftBorder.width() + 1 : 0) ),
                               int(r.y() + 30 + m_topBorder.width()+1),
                               int(r.right() - 19 - ( rightdouble ? m_rightBorder.width() + 1 : 0) ),
                               int(r.y() + 30 + m_topBorder.width()+1)
                             );
    }

    if ( m_bottomBorder.width() > 0 ) {
        painter->setPen( setBorderPen( m_bottomBorder ) );
        painter->drawLine( r.x() + 20, r.bottom() - 30, r.right() - 19, r.bottom() - 30 );
        if ( m_bottomBorder.getStyle()==KoBorder::DOUBLE_LINE)
            painter->drawLine( int(r.x() + 20 + ( leftdouble ? m_leftBorder.width() + 1 : 0) ),
                               int(r.bottom() - 30 - m_bottomBorder.width()-1),
                               int(r.right() - 19 - ( rightdouble ? m_rightBorder.width() + 1 : 0) ),
                               int(r.bottom() - 30 - m_bottomBorder.width() - 1)
                             );
    }

    if ( m_leftBorder.width() > 0 ) {
        painter->setPen( setBorderPen( m_leftBorder ) );
        painter->drawLine( r.x() + 20, r.y() + 30, r.x() + 20, r.bottom() - 29 );
        if ( m_leftBorder.getStyle()==KoBorder::DOUBLE_LINE)
            painter->drawLine( int(r.x() + 20 + m_leftBorder.width() +1),
                               int(r.y() + 30 + ( topdouble ? m_topBorder.width() + 1 : 0) ),
                               int(r.x() + 20 + m_leftBorder.width() +1),
                               int(r.bottom() - 29 - ( bottomdouble ? m_bottomBorder.width() + 1 : 0) )
                             );
    }

    if ( m_rightBorder.width() > 0 ) {
        painter->setPen( setBorderPen( m_rightBorder ) );
        painter->drawLine( r.right() - 20, r.y() + 30, r.right() - 20, r.bottom() - 29 );
        if ( m_rightBorder.getStyle()==KoBorder::DOUBLE_LINE)
            painter->drawLine( int(r.right() - 20 - m_rightBorder.width() - 1 ),
                               int(r.y() + 30 + ( topdouble ? m_topBorder.width() + 1 : 0) ),
                               int(r.right() - 20 - m_rightBorder.width() - 1),
                               int(r.bottom() - 29 - ( bottomdouble ? m_bottomBorder.width() + 1 : 0) )
                             );
    }
}

QPen KoBorderPreview::setBorderPen( KoBorder _brd )
{
    QPen pen( Qt::black, 1, Qt::SolidLine );

    pen.setWidth( static_cast<int>( _brd.penWidth() ) );
    pen.setColor( _brd.color );

    switch ( _brd.getStyle() ) {
    case KoBorder::SOLID:
        pen.setStyle( Qt::SolidLine );
        break;
    case KoBorder::DASH:
        pen.setStyle( Qt::DashLine );
        break;
    case KoBorder::DOT:
        pen.setStyle( Qt::DotLine );
        break;
    case KoBorder::DASH_DOT:
        pen.setStyle( Qt::DashDotLine );
        break;
    case KoBorder::DASH_DOT_DOT:
        pen.setStyle( Qt::DashDotDotLine );
        break;
    case KoBorder::DOUBLE_LINE:
        pen.setStyle( Qt::SolidLine );
        break;
    }

    return QPen( pen );
}

/******************************************************************/
/* Class: KoStylePreview. Previewing text with style ;)           */
/******************************************************************/
KoStylePreview::KoStylePreview( const QString& title, const QString& text, QWidget* parent )
    : Q3GroupBox( title, parent )
{
    setMinimumHeight(80);
    m_zoomHandler = new KoTextZoomHandler;
    QFont font = KoGlobal::defaultFont();
    m_textdoc = new KoTextDocument( m_zoomHandler, new KoTextFormatCollection( font, QColor(), KGlobal::locale()->language(), false ) );
    //m_textdoc->setWidth( KoTextZoomHandler::ptToLayoutUnitPt( 1000 ) );
    KoTextParag * parag = m_textdoc->firstParag();
    parag->insert( 0, text );
}

KoStylePreview::~KoStylePreview()
{
    delete m_textdoc;
    delete m_zoomHandler;
}

void KoStylePreview::setCounter( const KoParagCounter & counter )
{
    KoTextParag * parag = m_textdoc->firstParag();
    parag->setCounter( counter );
    update();
}

void KoStylePreview::setStyle( KoParagStyle * style )
{
    KoTextParag * parag = m_textdoc->firstParag();
    parag->applyStyle( style );
    update();
}

void KoStylePreview::drawContents( QPainter *painter )
{
    painter->save();
    QRect r = contentsRect();
    //kDebug(32500) << "KoStylePreview::drawContents contentsRect=" << DEBUGRECT(r) << endl;

    QRect whiteRect( r.x() + 10, r.y() + 10,
                     r.width() - 20, r.height() - 20 );
    QColorGroup cg = QApplication::palette().active();
    painter->fillRect( whiteRect, cg.brush( QColorGroup::Base ) );

    KoTextParag * parag = m_textdoc->firstParag();
    int widthLU = m_zoomHandler->pixelToLayoutUnitX( whiteRect.width() - 2 ); // keep one pixel border horizontally
    if ( m_textdoc->width() != widthLU )
    {
        // For centering to work, and to even get word wrapping when the thing is too big :)
        m_textdoc->setWidth( widthLU );
        parag->invalidate(0);
    }

    parag->format();
    QRect textRect = parag->pixelRect( m_zoomHandler );

    // Center vertically, but not horizontally, to keep the parag alignment working,
    textRect.moveTopLeft( QPoint( whiteRect.x(),
                                  whiteRect.y() + ( whiteRect.height() - textRect.height() ) / 2 ) );
    // Move it from the left border a little
    textRect.rLeft() += 4;
    textRect.rRight() += 4;
    //kDebug(32500) << "KoStylePreview::drawContents textRect=" << DEBUGRECT(textRect)
    //          << " textSize=" << textSize.width() << "," << textSize.height() << endl;
    painter->setClipRect( textRect.intersect( whiteRect ) );
    painter->translate( textRect.x(), textRect.y() );

    m_textdoc->drawWYSIWYG( painter, 0, 0, textRect.width(), textRect.height(), cg, m_zoomHandler );
    painter->restore();
}

KoIndentSpacingWidget::KoIndentSpacingWidget( KoUnit::Unit unit, double _frameWidth, QWidget * parent )
        : KoParagLayoutWidget( KoParagDia::PD_SPACING, parent ), m_unit( unit )
{
    QString unitName = KoUnit::unitName( m_unit );
    Q3GridLayout *mainGrid = new Q3GridLayout( this, 3, 2, KDialog::marginHint(), KDialog::spacingHint() );

    // mainGrid gives equal space to each groupbox, apparently
    // I tried setRowStretch but the result is awful (much space between them and not equal!)
    // Any other way (in order to make the 2nd, the one with a single checkbox, a bit
    // smaller than the other 3) ? (DF)


    // --------------- indent ---------------
    double frameWidth=_frameWidth;
    QString length;
    if(frameWidth==-1) {
        frameWidth=9999;
    } else {
        length=i18n("Frame width: %1 %2"
		,KoUnit::toUserStringValue(frameWidth,m_unit)
		,KoUnit::unitName(m_unit));
        frameWidth=KoUnit::toUserValue(frameWidth,m_unit);
    }

    Q3GroupBox * indentFrame = new Q3GroupBox( i18n( "Indent" ), this );
    Q3GridLayout * indentGrid = new Q3GridLayout( indentFrame, 5, 2, KDialog::marginHint(), KDialog::spacingHint() );

    QLabel * lLimit = new QLabel(length , indentFrame );
    if(frameWidth!=-1)
    {
        lLimit->setAlignment( Qt::AlignRight );
        indentGrid->addWidget( lLimit, 1,0 );
    }

    QLabel * lLeft = new QLabel( i18n("&Left:"), indentFrame );
    lLeft->setAlignment( Qt::AlignRight );
    indentGrid->addWidget( lLeft, 2, 0 );

    eLeft = new KoUnitDoubleSpinBox( indentFrame, 0, 9999, 1, 0.0, m_unit );
    lLeft->setBuddy( eLeft );
    indentGrid->addWidget( eLeft, 2, 1 );
    connect( eLeft, SIGNAL( valueChanged(double ) ), this, SLOT( leftChanged( double ) ) );

    QLabel * lRight = new QLabel( i18n("&Right:"), indentFrame );
    lRight->setAlignment( Qt::AlignRight );
    indentGrid->addWidget( lRight, 3, 0 );

    eRight = new KoUnitDoubleSpinBox( indentFrame, 0, 9999, 1, 0.0, m_unit );
    lRight->setBuddy( eRight );
    indentGrid->addWidget( eRight, 3, 1 );
    connect( eRight, SIGNAL( valueChanged( double ) ), this, SLOT( rightChanged( double ) ) );

    QLabel * lFirstLine = new QLabel( i18n("&First line:"), indentFrame );
    lFirstLine->setAlignment( Qt::AlignRight );
    indentGrid->addWidget( lFirstLine, 4, 0 );

    eFirstLine = new KoUnitDoubleSpinBox( indentFrame, -9999, 9999, 1, 0.0, m_unit );
    lFirstLine->setBuddy( eFirstLine );
    connect( eFirstLine, SIGNAL( valueChanged( double ) ), this, SLOT( firstChanged( double ) ) );
    indentGrid->addWidget( eFirstLine, 4, 1 );

    // grid row spacing
    indentGrid->addRowSpacing( 0, fontMetrics().height() / 2 ); // groupbox title
    for ( int i = 1 ; i < indentGrid->numRows() ; ++i )
        indentGrid->setRowStretch( i, 1 );
    mainGrid->addWidget( indentFrame, 0, 0 );

        // --------------- line spacing ---------------
    Q3GroupBox * spacingFrame = new Q3GroupBox( i18n( "Line &Spacing" ), this, "spacingFrame" );
    Q3GridLayout * spacingGrid = new Q3GridLayout( spacingFrame, 2, 1,
                                                 KDialog::marginHint(), KDialog::spacingHint() );

    cSpacing = new QComboBox( false, spacingFrame, "" );
    // Keep order in sync with lineSpacingType() and display()
    cSpacing->addItem( i18nc( "Line spacing value", "Single" ) );
    cSpacing->addItem( i18nc( "Line spacing value", "1.5 Lines" ) );
    cSpacing->addItem( i18nc( "Line spacing value", "Double" ) );
    cSpacing->addItem( i18n( "Proportional") ); // LS_MULTIPLE, called Proportional like in OO
    cSpacing->addItem( i18n( "Line Distance (%1)" ,unitName) ); // LS_CUSTOM
    cSpacing->addItem( i18n( "At Least (%1)" ,unitName) );
    cSpacing->addItem( i18n( "Fixed (%1)" ,unitName) ); // LS_FIXED

    connect( cSpacing, SIGNAL( activated( int ) ), this, SLOT( spacingActivated( int ) ) );
    spacingGrid->addWidget( cSpacing, 1, 0 );

    sSpacingStack = new Q3WidgetStack( spacingFrame );

    eSpacing = new KoUnitDoubleSpinBox( spacingFrame, 0, 9999, CM_TO_POINT(1),
					0.0, m_unit );
    eSpacing->setRange( 0, 9999, 1, false);
    connect( eSpacing, SIGNAL( valueChanged( double ) ), this, SLOT( spacingChanged( double ) ) );
    eSpacingPercent = new KIntNumInput( 100, spacingFrame );
    eSpacingPercent->setRange( 0, 1000, 10, false );
    eSpacingPercent->setSuffix( " %" );
    connect( eSpacingPercent, SIGNAL( valueChanged( int ) ), this, SLOT( spacingChanged( int ) ) );

    sSpacingStack->addWidget( eSpacing );
    sSpacingStack->addWidget( eSpacingPercent );
    spacingGrid->addWidget( sSpacingStack, 1, 1 );

    // grid row spacing
    spacingGrid->addRowSpacing( 0, fontMetrics().height() / 2 ); // groupbox title
    for ( int i = 1 ; i < spacingGrid->numRows() ; ++i )
        spacingGrid->setRowStretch( i, 1 );
    mainGrid->addWidget( spacingFrame, 1, 0 );

    eSpacing->setEnabled( true );

    // --------------- paragraph spacing ---------------
    Q3GroupBox * pSpaceFrame = new Q3GroupBox( i18n( "Para&graph Space" ), this, "pSpaceFrame" );
    Q3GridLayout * pSpaceGrid = new Q3GridLayout( pSpaceFrame, 3, 2,
                                                KDialog::marginHint(), KDialog::spacingHint() );

    QLabel * lBefore = new QLabel( i18n("Before:"), pSpaceFrame );
    lBefore->setAlignment( Qt::AlignRight );
    pSpaceGrid->addWidget( lBefore, 1, 0 );

    eBefore = new KoUnitDoubleSpinBox( pSpaceFrame, 0, 9999, CM_TO_POINT(1), 0.0, m_unit );
    eBefore->setRange( 0 , 9999, 1, false);
    connect( eBefore, SIGNAL( valueChanged( double ) ), this, SLOT( beforeChanged( double ) ) );
    pSpaceGrid->addWidget( eBefore, 1, 1 );

    QLabel * lAfter = new QLabel( i18n("After:"), pSpaceFrame );
    lAfter->setAlignment( Qt::AlignRight );
    pSpaceGrid->addWidget( lAfter, 2, 0 );

    eAfter = new KoUnitDoubleSpinBox( pSpaceFrame, 0, 9999, 1, 0.0, m_unit );
    eAfter->setRange( 0, 9999, 1, false);
    connect( eAfter, SIGNAL( valueChanged( double ) ), this, SLOT( afterChanged( double ) ) );
    pSpaceGrid->addWidget( eAfter, 2, 1 );

    // grid row spacing
    pSpaceGrid->addRowSpacing( 0, fontMetrics().height() / 2 ); // groupbox title
    for ( int i = 1 ; i < pSpaceGrid->numRows() ; ++i )
        pSpaceGrid->setRowStretch( i, 1 );
    mainGrid->addWidget( pSpaceFrame, 2, 0 );

    // --------------- preview --------------------
    prev1 = new KPagePreview( this );
    mainGrid->addMultiCellWidget( prev1, 0, mainGrid->numRows()-1, 1, 1 );

    mainGrid->setColStretch( 1, 1 );
    //mainGrid->setRowStretch( 4, 1 );
}

double KoIndentSpacingWidget::leftIndent() const
{
    return qMax(0.0, eLeft->value() );
}

double KoIndentSpacingWidget::rightIndent() const
{
    return qMax(0.0,eRight->value() );
}

double KoIndentSpacingWidget::firstLineIndent() const
{
    return eFirstLine->value();
}

double KoIndentSpacingWidget::spaceBeforeParag() const
{
    return qMax(0.0, eBefore->value() );
}

double KoIndentSpacingWidget::spaceAfterParag() const
{
    return qMax(0.0, eAfter->value() );
}

KoParagLayout::SpacingType KoIndentSpacingWidget::lineSpacingType() const
{
    int index = cSpacing->currentItem();
    switch ( index ) {
    case 0:
        return KoParagLayout::LS_SINGLE;
    case 1:
        return KoParagLayout::LS_ONEANDHALF;
    case 2:
        return KoParagLayout::LS_DOUBLE;
    case 3:
        return KoParagLayout::LS_MULTIPLE;
    case 4:
        return KoParagLayout::LS_CUSTOM;
    case 5:
        return KoParagLayout::LS_AT_LEAST;
    case 6:
        return KoParagLayout::LS_FIXED;
    default:
        kError(32500) << "Error in KoIndentSpacingWidget::lineSpacingType" << endl;
        return KoParagLayout::LS_SINGLE;
    }
}

double KoIndentSpacingWidget::lineSpacing() const
{
    return (lineSpacingType() == KoParagLayout::LS_MULTIPLE)
                               ? qMax( 1, eSpacingPercent->value() ) / 100.0
                               : qMax( 0.0, eSpacing->value() );
}


void KoIndentSpacingWidget::display( const KoParagLayout & lay )
{
    double _left = lay.margins[Q3StyleSheetItem::MarginLeft];
    eLeft->changeValue( _left );
    //prev1->setLeft( _left );  done by leftChanged() below
    leftChanged( _left ); // sets min value for eFirstLine

    double _right = lay.margins[Q3StyleSheetItem::MarginRight];
    eRight->changeValue( _right );
    prev1->setRight( _right );

    double _first = lay.margins[Q3StyleSheetItem::MarginFirstLine];
    eFirstLine->changeValue( _first );
    prev1->setFirst( _first );

    double _before = lay.margins[Q3StyleSheetItem::MarginTop];
    eBefore->changeValue( _before );
    prev1->setBefore( _before );

    double _after = lay.margins[Q3StyleSheetItem::MarginBottom];
    eAfter->changeValue( _after );
    prev1->setAfter( _after );

    double _spacing = lay.lineSpacingValue();
    KoParagLayout::SpacingType _type = lay.lineSpacingType;
    switch ( _type ) {
    case KoParagLayout::LS_SINGLE: // single
        cSpacing->setCurrentItem( 0 );
        break;
    case KoParagLayout::LS_ONEANDHALF:
        cSpacing->setCurrentItem( 1 );
        break;
    case KoParagLayout::LS_DOUBLE:
        cSpacing->setCurrentItem( 2 );
        break;
    case KoParagLayout::LS_MULTIPLE:
        cSpacing->setCurrentItem( 3 );
        break;
    case KoParagLayout::LS_CUSTOM:
        cSpacing->setCurrentItem( 4 );
        break;
    case KoParagLayout::LS_AT_LEAST:
        cSpacing->setCurrentItem( 5 );
        break;
    case KoParagLayout::LS_FIXED:
        cSpacing->setCurrentItem( 6 );
        break;
    default:
        cSpacing->setCurrentItem( 0 );
        break;
    }

    updateLineSpacing( _type );
    eSpacing->setValue( (_type == KoParagLayout::LS_MULTIPLE) ? qMax( 1.0, _spacing )
                        : KoUnit::toUserValue( _spacing, m_unit ) );
    eSpacingPercent->setValue( ( _type == KoParagLayout::LS_MULTIPLE ) ? qRound( _spacing * 100 ) : 100 );
}

void KoIndentSpacingWidget::save( KoParagLayout & lay )
{
    lay.setLineSpacingValue(lineSpacing());
    lay.lineSpacingType = lineSpacingType();
    lay.margins[Q3StyleSheetItem::MarginLeft] = leftIndent();
    lay.margins[Q3StyleSheetItem::MarginRight] = rightIndent();
    lay.margins[Q3StyleSheetItem::MarginFirstLine] = firstLineIndent();
    lay.margins[Q3StyleSheetItem::MarginTop] = spaceBeforeParag();
    lay.margins[Q3StyleSheetItem::MarginBottom] = spaceAfterParag();
}

QString KoIndentSpacingWidget::tabName()
{
    return i18n( "Indent && S&pacing" );
}

void KoIndentSpacingWidget::leftChanged( double _val )
{
    prev1->setLeft( KoUnit::fromUserValue( _val, m_unit ) );
    // The minimum first-line margin is -leftMargin() (where leftMargin>=0)
    eFirstLine->setMinValue( -qMax( 0.0, _val ) );
}

void KoIndentSpacingWidget::rightChanged( double _val )
{
    prev1->setRight( KoUnit::fromUserValue( _val, m_unit ) );
}

void KoIndentSpacingWidget::firstChanged( double _val )
{
    prev1->setFirst( KoUnit::fromUserValue( _val, m_unit ) );
}

void KoIndentSpacingWidget::updateLineSpacing( KoParagLayout::SpacingType _type )
{
    bool needsValue = (_type != KoParagLayout::LS_SINGLE &&
                       _type != KoParagLayout::LS_ONEANDHALF &&
                       _type != KoParagLayout::LS_DOUBLE);

    if ( _type == KoParagLayout::LS_MULTIPLE )
    {
        sSpacingStack->raiseWidget( eSpacingPercent );
    }
    else
    {
        sSpacingStack->raiseWidget( eSpacing );
    }
    eSpacing->setEnabled( needsValue );
    if ( needsValue )
        prev1->setSpacing( eSpacing->value() );
    else
    {
        prev1->setSpacing( _type == KoParagLayout::LS_ONEANDHALF ? 8 :
                           _type == KoParagLayout::LS_DOUBLE ? 16 :0 );
    }
}

void KoIndentSpacingWidget::spacingActivated( int /*_index*/ )
{
    updateLineSpacing( lineSpacingType() );
    if ( eSpacing->isEnabled() ) // i.e. needsValue = true
        eSpacing->setFocus();
}

void KoIndentSpacingWidget::spacingChanged( double _val )
{
    prev1->setSpacing( _val );
}

void KoIndentSpacingWidget::spacingChanged( int _val )
{
    prev1->setSpacing( _val / 100.0 );
}

void KoIndentSpacingWidget::beforeChanged( double _val )
{
    prev1->setBefore( KoUnit::fromUserValue( _val, m_unit ) );
}

void KoIndentSpacingWidget::afterChanged( double _val )
{
    prev1->setAfter( KoUnit::fromUserValue( _val, m_unit ) );
}


KoParagAlignWidget::KoParagAlignWidget( bool breakLine, QWidget * parent )
        : KoParagLayoutWidget( KoParagDia::PD_ALIGN, parent )
{
    Q3GridLayout *grid = new Q3GridLayout( this, 3, 2, KDialog::marginHint(), KDialog::spacingHint() );

    QGroupBox * AlignGroup = new QGroupBox( i18n( "Alignment" ), this );

    rLeft = new QRadioButton( i18n( "&Left" ), AlignGroup );
    connect( rLeft, SIGNAL( clicked() ), this, SLOT( alignLeft() ) );

    rCenter = new QRadioButton( i18n( "C&enter" ), AlignGroup );
    connect( rCenter, SIGNAL( clicked() ), this, SLOT( alignCenter() ) );

    rRight = new QRadioButton( i18n( "&Right" ), AlignGroup );
    connect( rRight, SIGNAL( clicked() ), this, SLOT( alignRight() ) );

    rJustify = new QRadioButton( i18n( "&Justify" ), AlignGroup );
    connect( rJustify, SIGNAL( clicked() ), this, SLOT( alignJustify() ) );

    clearAligns();
    rLeft->setChecked( true );

    grid->addWidget(AlignGroup, 0, 0);

    // --------------- End of page /frame ---------------
    Q3GroupBox * endFramePage = new Q3GroupBox( i18n( "Behavior at &End of Frame/Page" ), this );
    Q3GridLayout * endFramePageGrid = new Q3GridLayout( endFramePage, 4, 1,
                                                      KDialog::marginHint(), KDialog::spacingHint() );

    cKeepLinesTogether = new QCheckBox( i18n("&Keep lines together"),endFramePage);
    endFramePageGrid->addWidget( cKeepLinesTogether, 1, 0 );
    cHardBreakBefore = new QCheckBox( i18n("Insert break before paragraph"),endFramePage);
    endFramePageGrid->addWidget( cHardBreakBefore, 2, 0 );
    cHardBreakAfter = new QCheckBox( i18n("Insert break after paragraph"),endFramePage);
    endFramePageGrid->addWidget( cHardBreakAfter, 3, 0 );

    endFramePageGrid->addRowSpacing( 0, fontMetrics().height() / 2 ); // groupbox title
    for ( int i = 0 ; i < endFramePageGrid->numRows()-1 ; ++i )
        endFramePageGrid->setRowStretch( 0, 0 );
    endFramePageGrid->setRowStretch( endFramePageGrid->numRows()-1, 1 );
    grid->addWidget( endFramePage, 2, 0 );

    endFramePage->setEnabled(breakLine);

    // --------------- preview --------------------
    prev2 = new KPagePreview2( this );
    grid->addMultiCellWidget( prev2, 0, 2, 1, 1 );

    // --------------- main grid ------------------
    grid->setColStretch( 1, 1 );
    grid->setRowStretch( 1, 1 );
}

int KoParagAlignWidget::pageBreaking() const
{
    int pb = 0;
    if ( cKeepLinesTogether->isChecked() )
        pb |= KoParagLayout::KeepLinesTogether;
    if ( cHardBreakBefore->isChecked() )
        pb |= KoParagLayout::HardFrameBreakBefore;
    if ( cHardBreakAfter->isChecked() )
        pb |= KoParagLayout::HardFrameBreakAfter;
    return pb;
}


void KoParagAlignWidget::display( const KoParagLayout & lay )
{
    int align = lay.alignment;
    prev2->setAlign( align );

    clearAligns();
    switch ( align ) {
        case Qt::AlignLeft: // see KoView::setAlign
            rLeft->setChecked( true );
            break;
        case Qt::AlignHCenter:
            rCenter->setChecked( true );
            break;
        case Qt::AlignRight:
            rRight->setChecked( true );
            break;
        case Qt::AlignJustify:
            rJustify->setChecked( true );
    }

    cKeepLinesTogether->setChecked( lay.pageBreaking & KoParagLayout::KeepLinesTogether );
    cHardBreakBefore->setChecked( lay.pageBreaking & KoParagLayout::HardFrameBreakBefore );
    cHardBreakAfter->setChecked( lay.pageBreaking & KoParagLayout::HardFrameBreakAfter );
    // ## preview support for end-of-frame ?
}

void KoParagAlignWidget::save( KoParagLayout & lay )
{
    lay.alignment = align();
    lay.pageBreaking = pageBreaking();
}

int KoParagAlignWidget::align() const
{
    if ( rLeft->isChecked() ) return Qt::AlignLeft;
    else if ( rCenter->isChecked() ) return Qt::AlignHCenter;
    else if ( rRight->isChecked() ) return Qt::AlignRight;
    else if ( rJustify->isChecked() ) return Qt::AlignJustify;

    return Qt::AlignLeft;
}

QString KoParagAlignWidget::tabName()
{
    return i18n( "General &Layout" );
}

void KoParagAlignWidget::alignLeft()
{
    prev2->setAlign( Qt::AlignLeft );
    clearAligns();
    rLeft->setChecked( true );
}

void KoParagAlignWidget::alignCenter()
{
    prev2->setAlign( Qt::AlignHCenter );
    clearAligns();
    rCenter->setChecked( true );
}

void KoParagAlignWidget::alignRight()
{
    prev2->setAlign( Qt::AlignRight );
    clearAligns();
    rRight->setChecked( true );
}

void KoParagAlignWidget::alignJustify()
{
    prev2->setAlign( Qt::AlignJustify );
    clearAligns();
    rJustify->setChecked( true );
}

void KoParagAlignWidget::clearAligns()
{
    rLeft->setChecked( false );
    rCenter->setChecked( false );
    rRight->setChecked( false );
    rJustify->setChecked( false );
}

////////////////////////////////////////////////////////////////////////////////

KoParagDecorationWidget::KoParagDecorationWidget( QWidget * parent )
    : KoParagLayoutWidget( KoParagDia::PD_DECORATION, parent )
{
    Q3VBoxLayout *tabLayout = new Q3VBoxLayout( this );
    wDeco = new KoParagDecorationTab( this );
    tabLayout->add( wDeco );

    // Set up Border Style combo box
    wDeco->cbBorderStyle->addItem( KoBorder::getStyle( KoBorder::SOLID ) );
    wDeco->cbBorderStyle->addItem( KoBorder::getStyle( KoBorder::DASH ) );
    wDeco->cbBorderStyle->addItem( KoBorder::getStyle( KoBorder::DOT ) );
    wDeco->cbBorderStyle->addItem( KoBorder::getStyle( KoBorder::DASH_DOT ) );
    wDeco->cbBorderStyle->addItem( KoBorder::getStyle( KoBorder::DASH_DOT_DOT ) );
    wDeco->cbBorderStyle->addItem( KoBorder::getStyle( KoBorder::DOUBLE_LINE  ) );

    // Set up Border Width combo box
    for( unsigned int i = 1; i <= 10; i++ )
        wDeco->cbBorderWidth->addItem(QString::number(i));

    // Setup the border toggle buttons, and merge checkbox
    connect( wDeco->bBorderLeft, SIGNAL( toggled( bool ) ),
             this, SLOT( brdLeftToggled( bool ) ) );
    connect( wDeco->bBorderRight, SIGNAL( toggled( bool ) ),
             this, SLOT( brdRightToggled( bool ) ) );
    connect( wDeco->bBorderTop, SIGNAL( toggled( bool ) ),
             this, SLOT( brdTopToggled( bool ) ) );
    connect( wDeco->bBorderBottom, SIGNAL( toggled( bool ) ),
             this, SLOT( brdBottomToggled( bool ) ) );
    connect( wDeco->cbJoinBorder, SIGNAL( toggled( bool ) ),
             this, SLOT( brdJoinToggled( bool ) ) );

    // Set up Border preview widget
    wPreview  = new KoBorderPreview( wDeco->borderPreview );
    Q3VBoxLayout *previewLayout = new Q3VBoxLayout( wDeco->borderPreview );
    previewLayout->addWidget( wPreview );
    connect( wPreview, SIGNAL( choosearea(QMouseEvent * ) ),
             this, SLOT( slotPressEvent(QMouseEvent *) ) );
}

///////
// Current GUI selections
KoBorder::BorderStyle KoParagDecorationWidget::curBorderStyle() const
{
    QString selection = wDeco->cbBorderStyle->currentText();
    return KoBorder::getStyle( selection );
}

unsigned int KoParagDecorationWidget::curBorderWidth() const {
    return wDeco->cbBorderWidth->currentText().toUInt();
}

QColor KoParagDecorationWidget::curBorderColor() const {
    return wDeco->bBorderColor->color();
}
///////

// Check whether a border is the same as that selected in the GUI
bool KoParagDecorationWidget::borderChanged( const KoBorder& border ) {
    return (unsigned int)border.penWidth() != curBorderWidth() ||
           border.color != curBorderColor() ||
           border.getStyle() != curBorderStyle();
}

// Set a given border according to the values selected in the GUI
void KoParagDecorationWidget::updateBorder( KoBorder& border )
{
    border.setPenWidth( curBorderWidth() );
    border.color = curBorderColor();
    border.setStyle( curBorderStyle () );
}

void KoParagDecorationWidget::clickedBorderPreview( KoBorder& border,
                                                     KoBorder::BorderType position,
                                                     KPushButton *corresponding )
{
    if ( borderChanged( border ) && corresponding->isOn() ) {
        updateBorder( border );
        wPreview->setBorder( position, border );
    }
    else
        corresponding->setOn( !corresponding->isOn() );
}

// Establish which border position was clicked in the border preview,
// and update the appropriate border
void KoParagDecorationWidget::slotPressEvent(QMouseEvent *_ev)
{
    const int OFFSETX = 15;
    const int OFFSETY = 7;
    const int Ko_SPACE = 30;

    QRect r = wPreview->contentsRect();
    QRect rect(r.x() + OFFSETX, r.y() + OFFSETY,
               r.width() - OFFSETX, r.y() + OFFSETY + Ko_SPACE);
    if(rect.contains(QPoint(_ev->x(),_ev->y())))
    {
        clickedBorderPreview( m_topBorder, KoBorder::TopBorder,
                              wDeco->bBorderTop );
    }

    rect.setCoords(r.x() + OFFSETX, r.height() - OFFSETY - Ko_SPACE,
                   r.width() - OFFSETX, r.height() - OFFSETY);
    if(rect.contains(QPoint(_ev->x(),_ev->y())))
    {
        clickedBorderPreview( m_bottomBorder, KoBorder::BottomBorder,
                              wDeco->bBorderBottom );
    }

    rect.setCoords(r.x() + OFFSETX, r.y() + OFFSETY,
                   r.x() + Ko_SPACE + OFFSETX, r.height() - OFFSETY);
    if(rect.contains(QPoint(_ev->x(),_ev->y())))
    {
        clickedBorderPreview( m_leftBorder, KoBorder::LeftBorder,
                              wDeco->bBorderLeft );
    }

    rect.setCoords(r.width() - OFFSETX - Ko_SPACE, r.y() + OFFSETY,
                   r.width() - OFFSETX, r.height() - OFFSETY);
    if(rect.contains(QPoint(_ev->x(),_ev->y())))
    {
        clickedBorderPreview( m_rightBorder, KoBorder::RightBorder,
                              wDeco->bBorderRight );
    }
}

void KoParagDecorationWidget::display( const KoParagLayout & lay )
{
    wDeco->bBackgroundColor->setColor( lay.backgroundColor );

    m_leftBorder = lay.leftBorder;
    m_rightBorder = lay.rightBorder;
    m_topBorder = lay.topBorder;
    m_bottomBorder = lay.bottomBorder;
    m_joinBorder = lay.joinBorder;

    wDeco->bBorderLeft->blockSignals( true );
    wDeco->bBorderRight->blockSignals( true );
    wDeco->bBorderTop->blockSignals( true );
    wDeco->bBorderBottom->blockSignals( true );
    updateBorders();
    wDeco->bBorderLeft->blockSignals( false );
    wDeco->bBorderRight->blockSignals( false );
    wDeco->bBorderTop->blockSignals( false );
    wDeco->bBorderBottom->blockSignals( false );
}

void KoParagDecorationWidget::updateBorders()
{
    wDeco->bBorderLeft->setOn( m_leftBorder.penWidth() > 0 );
    wDeco->bBorderRight->setOn( m_rightBorder.penWidth() > 0 );
    wDeco->bBorderTop->setOn( m_topBorder.penWidth() > 0 );
    wDeco->bBorderBottom->setOn( m_bottomBorder.penWidth() > 0 );
    wDeco->cbJoinBorder->setChecked( m_joinBorder );
    wPreview->setLeftBorder( m_leftBorder );
    wPreview->setRightBorder( m_rightBorder );
    wPreview->setTopBorder( m_topBorder );
    wPreview->setBottomBorder( m_bottomBorder );
}


void KoParagDecorationWidget::save( KoParagLayout & lay )
{
    lay.backgroundColor = wDeco->bBackgroundColor->color();
    lay.topBorder = m_topBorder;
    lay.bottomBorder = m_bottomBorder;
    lay.leftBorder = m_leftBorder;
    lay.rightBorder = m_rightBorder;
    lay.joinBorder = m_joinBorder;
}

QColor KoParagDecorationWidget::backgroundColor() const {
    return wDeco->bBackgroundColor->color();
}

QString KoParagDecorationWidget::tabName() {
    // Why D&e..?  Because &De.. conflicts with &Delete in
    // the style manager.
    return i18n( "D&ecorations" );
}

void KoParagDecorationWidget::brdLeftToggled( bool _on )
{
    if ( !_on )
        m_leftBorder.setPenWidth(0);
    else {
        m_leftBorder.setPenWidth( curBorderWidth() );
        m_leftBorder.color = curBorderColor();
        m_leftBorder.setStyle( curBorderStyle() );
    }
    wPreview->setLeftBorder( m_leftBorder );
}

void KoParagDecorationWidget::brdRightToggled( bool _on )
{
    if ( !_on )
        m_rightBorder.setPenWidth(0);
    else {
        m_rightBorder.setPenWidth( curBorderWidth() );
        m_rightBorder.color = curBorderColor();
        m_rightBorder.setStyle( curBorderStyle() );
    }
    wPreview->setRightBorder( m_rightBorder );
}

void KoParagDecorationWidget::brdTopToggled( bool _on )
{
    if ( !_on )
        m_topBorder.setPenWidth(0);
    else {
        m_topBorder.setPenWidth( curBorderWidth() );
        m_topBorder.color = curBorderColor();
        m_topBorder.setStyle( curBorderStyle() );
    }
    wPreview->setTopBorder( m_topBorder );
}

void KoParagDecorationWidget::brdBottomToggled( bool _on )
{
    if ( !_on )
        m_bottomBorder.setPenWidth ( 0 );
    else {
        m_bottomBorder.setPenWidth( curBorderWidth() );
        m_bottomBorder.color = curBorderColor();
        m_bottomBorder.setStyle( curBorderStyle() );
    }
    wPreview->setBottomBorder( m_bottomBorder );
}

void KoParagDecorationWidget::brdJoinToggled( bool _on ) {
    m_joinBorder = _on;
}
////////////////////////////////////////////////////////////////////////////////


KoParagCounterWidget::KoParagCounterWidget( bool disableAll, QWidget * parent )
    : KoParagLayoutWidget( KoParagDia::PD_NUMBERING, parent )
{

    Q3VBoxLayout *Form1Layout = new Q3VBoxLayout( this );
    Form1Layout->setSpacing( KDialog::spacingHint() );
    Form1Layout->setMargin( KDialog::marginHint() );

    gNumbering = new Q3ButtonGroup( this, "numberingGroup" );
    gNumbering->setTitle( i18n( "Numbering" ) );
    gNumbering->setColumnLayout(0, Qt::Vertical );
    gNumbering->layout()->setSpacing( 0 );
    gNumbering->layout()->setMargin( 0 );
    Q3HBoxLayout *numberingGroupLayout = new Q3HBoxLayout( gNumbering->layout() );
    numberingGroupLayout->setAlignment( Qt::AlignTop );
    numberingGroupLayout->setSpacing( KDialog::spacingHint() );
    numberingGroupLayout->setMargin( KDialog::marginHint() );

    // What type of numbering is required?
    QRadioButton *rNone = new QRadioButton( gNumbering, "rNone" );
    rNone->setText( i18n( "&None" ) );
    numberingGroupLayout->addWidget( rNone );

    gNumbering->insert( rNone , KoParagCounter::NUM_NONE);

    QRadioButton *rList = new QRadioButton( gNumbering, "rList" );
    rList->setText( i18n( "&List" ) );
    gNumbering->insert( rList , KoParagCounter::NUM_LIST);
    numberingGroupLayout->addWidget( rList );

    QRadioButton *rChapter = new QRadioButton( gNumbering, "rChapter" );
    rChapter->setText( i18n( "Chapt&er" ) );
    gNumbering->insert( rChapter , KoParagCounter::NUM_CHAPTER);
    numberingGroupLayout->addWidget( rChapter );
    Form1Layout->addWidget( gNumbering );
    connect( gNumbering, SIGNAL( clicked( int ) ), this, SLOT( numTypeChanged( int ) ) );

    m_styleWidget = new KoCounterStyleWidget( true, false, disableAll, this );

    connect( m_styleWidget, SIGNAL( sig_suffixChanged (const QString &) ), this, SLOT( suffixChanged(const QString &) ) );
    connect( m_styleWidget, SIGNAL( sig_prefixChanged (const QString &) ), this, SLOT( prefixChanged(const QString &) ) );
    connect( m_styleWidget, SIGNAL( sig_startChanged(int) ), this, SLOT( startChanged(int) ) );
    connect( m_styleWidget, SIGNAL( sig_restartChanged(bool) ), this, SLOT( restartChanged(bool) ) );
    connect( m_styleWidget, SIGNAL( sig_depthChanged (int) ), this, SLOT( depthChanged(int) ) );
    connect( m_styleWidget, SIGNAL( sig_displayLevelsChanged (int) ), this, SLOT( displayLevelsChanged(int) ) );
    connect( m_styleWidget, SIGNAL( sig_alignmentChanged (int) ), this, SLOT( alignmentChanged(int) ) );
    connect( m_styleWidget, SIGNAL( changeCustomBullet( const QString & , QChar ) ), this, SLOT( slotChangeCustomBullet( const QString & , QChar ) ) );

    connect( m_styleWidget, SIGNAL( sig_numTypeChanged( int ) ), this, SLOT( numTypeChanged(int ) ) );
    connect( m_styleWidget, SIGNAL( changeStyle( KoParagCounter::Style ) ), this, SLOT( styleChanged (KoParagCounter::Style ) ) );

    Form1Layout->addWidget( m_styleWidget );


    preview = new KoStylePreview( i18n( "Preview" ), i18n("Normal paragraph text"), this );
    preview->setObjectName( "counter preview" );
    Form1Layout->addWidget( preview );
    if ( disableAll)
    {
        gNumbering->setEnabled( false);
        preview->setEnabled( false );
    }

    QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    Form1Layout->addItem( spacer );
}

void KoParagCounterWidget::styleChanged( KoParagCounter::Style st )
{
    m_counter.setStyle( st );
    updatePreview();
}

void KoParagCounterWidget::slotChangeCustomBullet( const QString & f, QChar c)
{
    m_counter.setCustomBulletFont( f );
    m_counter.setCustomBulletCharacter( c );
    preview->setCounter( m_counter );
}

QString KoParagCounterWidget::tabName() {
    return i18n( "B&ullets/Numbers" );
}

void KoParagCounterWidget::numTypeChanged( int nType ) {
    // radio buttons pressed to change numbering type
    m_counter.setNumbering( static_cast<KoParagCounter::Numbering>( nType ) );
    preview->setEnabled( m_counter.numbering() != KoParagCounter::NUM_NONE );
    m_styleWidget->numTypeChanged( nType );

    updatePreview();
}

void KoParagCounterWidget::display( const KoParagLayout & lay ) {
    KoParagCounter::Style style = KoParagCounter::STYLE_NONE;
    if ( lay.counter )
    {
        style=lay.counter->style();
        m_counter = *lay.counter;
    }
    else
    {
        m_counter = KoParagCounter();
    }
    gNumbering->setButton( m_counter.numbering() );
    preview->setStyle( lay.style );
    preview->setCounter( m_counter );
    m_styleWidget->display( lay );
}

void KoParagCounterWidget::updatePreview() {
    preview->setCounter(m_counter);
    preview->update();
}

void KoParagCounterWidget::save( KoParagLayout & lay ) {
/*    m_counter.setDepth(spnDepth->value());
    m_counter.setStartNumber(spnStart->value());
    m_counter.setPrefix(sPrefix->text());
    m_counter.setSuffix(sSuffix->text()); */

    if ( lay.counter )
        *lay.counter = m_counter;
    else
        lay.counter = new KoParagCounter( m_counter );
}

KoTabulatorsLineEdit::KoTabulatorsLineEdit( QWidget *parent, double lower, double upper, double step, double value /*= 0.0*/, KoUnit::Unit unit /*= KoUnit::U_PT*/, unsigned int precision /*= 2*/ )
    : KoUnitDoubleSpinBox ( parent, lower, upper, step, value, unit, precision )
{
    setRange( 0, 9999, 1, false);
}

void KoTabulatorsLineEdit::keyPressEvent ( QKeyEvent *ke )
{
    if( ke->key()  == Qt::Key_Return ||
        ke->key()  == Qt::Key_Enter )
    {
        emit keyReturnPressed();
        return;
    }
    KoUnitDoubleSpinBox::keyPressEvent (ke);
}

KoParagTabulatorsWidget::KoParagTabulatorsWidget( KoUnit::Unit unit, double frameWidth, QWidget * parent )
    : KoParagLayoutWidget( KoParagDia::PD_TABS, parent ), m_unit(unit) {
    QString length;
    if(frameWidth==-1) {
        frameWidth=9999;
        m_toplimit=9999;
    } else {
        m_toplimit=frameWidth;
        length=i18n("Frame width: %1 %2")
		.arg(KoUnit::toUserStringValue(frameWidth,m_unit))
		.arg(KoUnit::unitName(m_unit));
        frameWidth=KoUnit::toUserValue(frameWidth,m_unit);
    }
    Q3VBoxLayout* Form1Layout = new Q3VBoxLayout( this );
    Form1Layout->setSpacing( KDialog::spacingHint() );
    Form1Layout->setMargin( KDialog::marginHint() );

    Q3HBoxLayout* Layout13 = new Q3HBoxLayout;
    Layout13->setSpacing( KDialog::spacingHint() );
    Layout13->setMargin( 0 ); //?

    lstTabs = new Q3ListBox( this);
    lstTabs->insertItem( "mytabvalue" );
    lstTabs->setMaximumSize( QSize( 300, 32767 ) );
    Layout13->addWidget( lstTabs );

    editLayout = new Q3VBoxLayout;
    editLayout->setSpacing( KDialog::spacingHint() );
    editLayout->setMargin( 0 ); //?

    gPosition = new Q3GroupBox( this, "gPosition" );
    gPosition->setTitle( i18n( "Po&sition" ) );
    gPosition->setColumnLayout(0, Qt::Vertical );
    gPosition->layout()->setSpacing( 0 );
    gPosition->layout()->setMargin( 0 );
    Q3VBoxLayout* GroupBox2Layout = new Q3VBoxLayout( gPosition->layout() );
    GroupBox2Layout->setAlignment( Qt::AlignTop );
    GroupBox2Layout->setSpacing( KDialog::spacingHint() );
    GroupBox2Layout->setMargin( KDialog::marginHint() );

    Q3HBoxLayout* Layout5 = new Q3HBoxLayout;
    Layout5->setSpacing( KDialog::spacingHint() );
    Layout5->setMargin( 0 ); //?

    sTabPos = new KoTabulatorsLineEdit( gPosition, 0, 9999, 1, 0.0, m_unit );
    sTabPos->setRange( 0, 9999, 1 );
    sTabPos->setMaximumSize( QSize( 100, 32767 ) );
    Layout5->addWidget( sTabPos );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout5->addItem( spacer );
    GroupBox2Layout->addLayout( Layout5 );
    editLayout->addWidget( gPosition );

    QLabel* TextLabel1 = new QLabel( gPosition );
    QString unitDescription = KoUnit::unitDescription( m_unit );
    TextLabel1->setText( length );
    GroupBox2Layout->addWidget( TextLabel1 );

    bgAlign = new Q3ButtonGroup( this );
    bgAlign->setTitle( i18n( "Alignment" ) );
    bgAlign->setColumnLayout(0, Qt::Vertical );
    bgAlign->layout()->setSpacing( 0 );
    bgAlign->layout()->setMargin( 0 );
    Q3VBoxLayout* ButtonGroup1Layout = new Q3VBoxLayout( bgAlign->layout() );
    ButtonGroup1Layout->setAlignment( Qt::AlignTop );
    ButtonGroup1Layout->setSpacing( KDialog::spacingHint() );
    ButtonGroup1Layout->setMargin( KDialog::marginHint() );

    rAlignLeft = new QRadioButton( bgAlign );
    rAlignLeft->setText( i18n( "&Left" ) );
    ButtonGroup1Layout->addWidget( rAlignLeft );

    rAlignCentre = new QRadioButton( bgAlign );
    rAlignCentre->setText( i18n( "C&enter" ) );
    ButtonGroup1Layout->addWidget( rAlignCentre );

    rAlignRight = new QRadioButton( bgAlign );
    rAlignRight->setText( i18n( "&Right" ) );
    ButtonGroup1Layout->addWidget( rAlignRight );

    Q3HBoxLayout* Layout8 = new Q3HBoxLayout;
    Layout8->setSpacing( KDialog::spacingHint() );
    Layout8->setMargin( 0 );

    rAlignVar = new QRadioButton( bgAlign );
    rAlignVar->setText( i18n( "On followin&g character: " ) );
    Layout8->addWidget( rAlignVar );

    sAlignChar = new QLineEdit( bgAlign);
    sAlignChar->setMaximumSize( QSize( 60, 32767 ) );
    sAlignChar->setText(QString(KGlobal::locale()->decimalSymbol()[0]));
    Layout8->addWidget( sAlignChar );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout8->addItem( spacer_2 );
    ButtonGroup1Layout->addLayout( Layout8 );
    editLayout->addWidget( bgAlign );

    gTabLeader = new Q3GroupBox( this, "gTabLeader" );
    gTabLeader->setTitle( i18n( "Tab Leader" ) );
    Q3VBoxLayout* GroupBox5Layout = new Q3VBoxLayout( gTabLeader );
    GroupBox5Layout->setAlignment( Qt::AlignTop );
    GroupBox5Layout->setSpacing( KDialog::spacingHint() );
    GroupBox5Layout->setMargin( KDialog::marginHint() );
    GroupBox5Layout->addSpacing( fontMetrics().height() / 2 ); // groupbox title

    QLabel* TextLabel1_2 = new QLabel( gTabLeader );
    TextLabel1_2->setText( i18n( "The space a tab uses can be filled with a pattern." ) );
    GroupBox5Layout->addWidget( TextLabel1_2 );

    Q3GridLayout *fillingGrid = new Q3GridLayout( 0L, 2, 2, 0, KDialog::spacingHint() );

    QLabel* TextLabel2 = new QLabel( gTabLeader);
    TextLabel2->setText( i18n( "&Filling:" ) );
    TextLabel2->setAlignment( Qt::AlignRight );
    fillingGrid->addWidget( TextLabel2, 0, 0 );

    cFilling = new QComboBox( FALSE, gTabLeader);
    cFilling->addItem( i18n( "Blank" ) );
    cFilling->addItem( "_ _ _ _ _ _"); // DOT
    cFilling->addItem( "_________");   // SOLID
    cFilling->addItem( "___ ___ __");  // DASH
    cFilling->addItem( "___ _ ___ _"); // DASH_DOT
    cFilling->addItem( "___ _ _ ___"); // DASH_DOT_DOT
    TextLabel2->setBuddy( cFilling );
    fillingGrid->addWidget( cFilling, 0, 1 );

    QLabel * TextLabel3 = new QLabel( i18n("&Width:"), gTabLeader );
    TextLabel3->setAlignment( Qt::AlignRight );
    fillingGrid->addWidget( TextLabel3, 1, 0 );

    eWidth = new KoUnitDoubleSpinBox( gTabLeader );
    eWidth->setUnit( m_unit );
    TextLabel3->setBuddy( eWidth );
    fillingGrid->addWidget( eWidth, 1, 1 );

    GroupBox5Layout->addLayout( fillingGrid );
    editLayout->addWidget( gTabLeader );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    editLayout->addItem( spacer_4 );
    Layout13->addLayout( editLayout );
    Form1Layout->addLayout( Layout13 );

    Q3HBoxLayout* Layout4 = new Q3HBoxLayout;
    Layout4->setSpacing( KDialog::spacingHint() );
    Layout4->setMargin( 0 );

    bNew = new QPushButton( this);
    bNew->setText( i18n( "&New" ) );
    Layout4->addWidget( bNew );

    bDelete = new QPushButton( this);
    bDelete->setText( i18n( "&Delete" ) );
    Layout4->addWidget( bDelete );

    bDeleteAll = new QPushButton( this);
    bDeleteAll->setText( i18n( "Delete All" ) );
    Layout4->addWidget( bDeleteAll );

    QSpacerItem* spacer_5 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout4->addItem( spacer_5 );
    Form1Layout->addLayout( Layout4 );

    //signal valueChanged passes value which the user see (unlike the value() function)
    //so fromUserValue has to be used in slotTabValueChanged
    connect(sTabPos,SIGNAL(valueChanged(double)), this, SLOT(slotTabValueChanged(double )));
    connect(sTabPos,SIGNAL( keyReturnPressed()),this,SLOT(newClicked()));
    connect(sAlignChar,SIGNAL(textChanged( const QString & )), this, SLOT(slotAlignCharChanged( const QString & )));
    connect(bNew,SIGNAL(clicked ()),this,SLOT(newClicked()));
    connect(bDelete,SIGNAL(clicked ()),this,SLOT(deleteClicked()));
    connect(bDeleteAll,SIGNAL(clicked ()),this,SLOT(deleteAllClicked()));
    connect(bgAlign,SIGNAL(clicked (int)),this,SLOT(updateAlign(int)));
    connect(cFilling,SIGNAL(activated (int)),this,SLOT(updateFilling(int)));
    connect(eWidth,SIGNAL(valueChanged ( double ) ),this,SLOT(updateWidth()));
    connect(lstTabs,SIGNAL(highlighted (int)),this,SLOT(setActiveItem(int)));
    noSignals=false;
}

void KoParagTabulatorsWidget::slotTabValueChanged( double val ) {
    if(noSignals) return;
    noSignals=true;
    //see comment where this slot is connected
    m_tabList[lstTabs->currentItem()].ptPos = KoUnit::fromUserValue( val, m_unit );

    lstTabs->changeItem(tabToString(m_tabList[lstTabs->currentItem()]), lstTabs->currentItem());

    sortLists();
    noSignals=false;
}

void KoParagTabulatorsWidget::slotAlignCharChanged( const QString &/*_text*/ ) {
    // select align 3 and update data structures.
    bgAlign->setButton(3);
    m_tabList[lstTabs->currentItem()].alignChar=sAlignChar->text()[0];
    m_tabList[lstTabs->currentItem()].type=T_DEC_PNT;
}

void KoParagTabulatorsWidget::newClicked() {
    int selected=lstTabs->currentItem();
    KoTabulator newTab;
    if(selected < 0) {
        newTab.ptPos=0;
        newTab.type=T_LEFT;
        newTab.filling=TF_BLANK;
        newTab.ptWidth=0.5;
        m_tabList.append(newTab);
        lstTabs->insertItem(tabToString(newTab));
        lstTabs->setCurrentItem(0);
    } else {
        double pos = m_tabList[selected].ptPos;
        double add=1.0;
        if(m_unit==KoUnit::U_INCH) // inches are 25 times as big as mm, take it easy with adding..
            add=0.1;

        pos=pos + KoUnit::fromUserValue( add, m_unit );
        if(pos<m_toplimit)
        {
            newTab.ptPos=pos + KoUnit::fromUserValue( add, m_unit );
            newTab.type=m_tabList[selected].type;
            newTab.filling=m_tabList[selected].filling;
            newTab.ptWidth=m_tabList[selected].ptWidth;
            m_tabList.insert(m_tabList.at(selected), newTab);
            lstTabs->insertItem( tabToString(newTab), selected);
            lstTabs->setCurrentItem(lstTabs->findItem(tabToString(newTab)));
            sortLists();
        }
    }
}

void KoParagTabulatorsWidget::deleteClicked() {
    int selected = lstTabs->currentItem();
    if (selected < 0) return;
    noSignals=true;
    sTabPos->changeValue(0.0);
    noSignals=false;
    lstTabs->removeItem(selected);
    m_tabList.remove(m_tabList[selected]);
    if(lstTabs->count() >0) {
        lstTabs->setCurrentItem(qMin(static_cast<unsigned int>(selected), lstTabs->count()-1 ));
    } else {
        bDeleteAll->setEnabled(false);
        bDelete->setEnabled(false);
        gPosition->setEnabled(false);
        bgAlign->setEnabled(false);
        gTabLeader->setEnabled(false);
    }
}

void KoParagTabulatorsWidget::deleteAllClicked()
{
    noSignals=true;
    sTabPos->changeValue(0.0);
    noSignals=false;
    lstTabs->clear();
    m_tabList.clear();
    bDeleteAll->setEnabled(false);
    bDelete->setEnabled(false);
    gPosition->setEnabled(false);
    bgAlign->setEnabled(false);
    gTabLeader->setEnabled(false);
}

void KoParagTabulatorsWidget::setActiveItem(int selected) {
    if(noSignals) return;
    if(selected < 0) return;
    noSignals=true;
    KoTabulator *selectedTab = &m_tabList[selected];
    switch( selectedTab->type) {
        case T_CENTER:
            bgAlign->setButton(1); break;
        case  T_RIGHT:
            bgAlign->setButton(2); break;
        case T_DEC_PNT:
            bgAlign->setButton(3);
	    sAlignChar->setText(QString(selectedTab->alignChar));
	    break;
        case T_LEFT:
        default:
            bgAlign->setButton(0);
    }
    switch( selectedTab->filling) {
        case TF_DOTS:
            cFilling->setCurrentItem(1); break;
        case TF_LINE:
            cFilling->setCurrentItem(2); break;
        case TF_DASH:
            cFilling->setCurrentItem(3); break;
        case TF_DASH_DOT:
            cFilling->setCurrentItem(4); break;
        case TF_DASH_DOT_DOT:
            cFilling->setCurrentItem(5); break;
        case TF_BLANK:
        default:
            cFilling->setCurrentItem(0);
    }
    eWidth->setValue( selectedTab->ptWidth );
    sTabPos->setValue( KoUnit::toUserValue(selectedTab->ptPos, m_unit));
    bDelete->setEnabled(true);
    bDeleteAll->setEnabled(true);
    gPosition->setEnabled(true);
    bgAlign->setEnabled(true);
    gTabLeader->setEnabled(true);
    noSignals=false;
}

void KoParagTabulatorsWidget::setCurrentTab( double tabPos ) {
    KoTabulatorList::ConstIterator it = m_tabList.begin();
    for ( int i = 0; it != m_tabList.end(); ++it, ++i )
        if ( (*it).ptPos == tabPos ) {
            lstTabs->setCurrentItem(i);
            setActiveItem( i );
            return;
        }
    kWarning() << "KoParagTabulatorsWidget::setCurrentTab: no tab found at pos=" << tabPos << endl;
}

QString KoParagTabulatorsWidget::tabToString(const KoTabulator &tab) {
    return KoUnit::toUserStringValue( tab.ptPos, m_unit);
}

void KoParagTabulatorsWidget::updateAlign(int selected) {
    KoTabulator *selectedTab = &m_tabList[lstTabs->currentItem()];

    switch( selected) {
        case 1:
            selectedTab->type=T_CENTER; break;
        case  2:
            selectedTab->type=T_RIGHT; break;
        case 3:
            selectedTab->type=T_DEC_PNT;
	    selectedTab->alignChar=sAlignChar->text()[0];
	    break;
        case 0:
        default:
            selectedTab->type=T_LEFT;
    }
}

void KoParagTabulatorsWidget::updateFilling(int selected) {
    KoTabulator *selectedTab = &m_tabList[lstTabs->currentItem()];

    switch( selected) {
        case 1:
            selectedTab->filling=TF_DOTS; break;
        case 2:
            selectedTab->filling=TF_LINE; break;
        case 3:
            selectedTab->filling=TF_DASH; break;
        case 4:
            selectedTab->filling=TF_DASH_DOT; break;
        case 5:
            selectedTab->filling=TF_DASH_DOT_DOT; break;
    case 0:
        default:
            selectedTab->filling=TF_BLANK;
    }
}

void KoParagTabulatorsWidget::updateWidth() {
    KoTabulator *selectedTab = &m_tabList[lstTabs->currentItem()];
    selectedTab->ptWidth = qMax( 0.0, eWidth->value() );
}

void KoParagTabulatorsWidget::sortLists() {

    noSignals=true;
    qHeapSort( m_tabList );

    // we could just sort the listView, but to make sure we never have any problems with
    // inconsistent lists, just re-add..
    QString curValue=lstTabs->currentText();
    lstTabs->clear();
    KoTabulatorList::ConstIterator it = m_tabList.begin();
    for ( ; it != m_tabList.end(); ++it )
        lstTabs->insertItem( KoUnit::toUserStringValue( (*it).ptPos, m_unit ) );

    lstTabs->setCurrentItem(lstTabs->findItem(curValue));
    noSignals=false;
}

void KoParagTabulatorsWidget::display( const KoParagLayout &lay ) {
    m_tabList.clear();
    lstTabs->clear();
    m_tabList = lay.tabList();
    KoTabulatorList::ConstIterator it = m_tabList.begin();
    for ( ; it != m_tabList.end(); ++it )
        lstTabs->insertItem( KoUnit::toUserStringValue( (*it).ptPos, m_unit ) );

    if(lstTabs->count() > 0)
        lstTabs->setCurrentItem(0);
    else {
        bDelete->setEnabled(false);
        bDeleteAll->setEnabled(false);
        gPosition->setEnabled(false);
        bgAlign->setEnabled(false);
        gTabLeader->setEnabled(false);
    }
}

void KoParagTabulatorsWidget::save( KoParagLayout & lay ) {
    lay.setTabList( m_tabList );
}

QString KoParagTabulatorsWidget::tabName() {
    return i18n( "&Tabulators" );
}

/******************************************************************/
/* Class: KoParagDia                                              */
/******************************************************************/
KoParagDia::KoParagDia( QWidget* parent,
                        int flags, KoUnit::Unit unit, double _frameWidth, bool breakLine, bool disableAll )
    : KDialogBase(Tabbed, QString::null, Ok | Cancel | User1 | Apply, Ok, parent )
{
    m_decorationsWidget = 0;
    m_flags = flags;
    setButtonText( KDialogBase::User1, i18n("Reset") );

    if ( m_flags & PD_SPACING )
    {
        KVBox * page = addVBoxPage( i18n( "Indent && S&pacing" ) );
        m_indentSpacingWidget = new KoIndentSpacingWidget( unit, _frameWidth, page );
        m_indentSpacingWidget->layout()->setMargin(0);
    }
    if ( m_flags & PD_ALIGN )
    {
        KVBox * page = addVBoxPage( i18n( "General &Layout" ) );
        m_alignWidget = new KoParagAlignWidget( breakLine, page );
        m_alignWidget->layout()->setMargin(0);
    }
    if ( m_flags & PD_DECORATION )
    {
        KVBox * page = addVBoxPage( i18n( "D&ecorations" ) );
        m_decorationsWidget = new KoParagDecorationWidget( page);
        m_decorationsWidget->layout()->setMargin(0);
    }
    if ( m_flags & PD_NUMBERING )
    {
        KVBox * page = addVBoxPage( i18n( "B&ullets/Numbers" ) );
        m_counterWidget = new KoParagCounterWidget( disableAll, page );
        m_counterWidget->layout()->setMargin(0);
    }
    if ( m_flags & PD_TABS )
    {
        KVBox * page = addVBoxPage( i18n( "&Tabulators" ) );
        m_tabulatorsWidget = new KoParagTabulatorsWidget( unit, _frameWidth, page );
        m_tabulatorsWidget->layout()->setMargin(0);
    }

    connect( this, SIGNAL( user1Clicked() ), this, SLOT(slotReset()));
    setInitialSize( QSize(630, 500) );
}

KoParagDia::~KoParagDia()
{
}

void KoParagDia::slotApply()
{
    emit applyParagStyle();
}

void KoParagDia::slotOk()
{
    slotApply();
    KDialogBase::slotOk();
}

void KoParagDia::setCurrentPage( int page )
{
    switch( page )
    {
    case PD_SPACING:
        showPage( pageIndex( m_indentSpacingWidget->parentWidget() ) );
        break;
    case PD_ALIGN:
        showPage( pageIndex( m_alignWidget->parentWidget() ) );
        break;
    case PD_DECORATION:
        showPage( pageIndex( m_decorationsWidget->parentWidget() ) );
        break;
    case PD_NUMBERING:
        showPage( pageIndex( m_counterWidget->parentWidget() ) );
        break;
    case PD_TABS:
        showPage( pageIndex( m_tabulatorsWidget->parentWidget() ) );
        break;
    default:
        break;
    }
}

void KoParagDia::setParagLayout( const KoParagLayout & lay )
{
    m_indentSpacingWidget->display( lay );
    m_alignWidget->display( lay );
    m_decorationsWidget->display( lay );
    m_counterWidget->display( lay );
    m_tabulatorsWidget->display( lay );
    oldLayout = lay;
}

void KoParagDia::slotReset()
{
    if( m_indentSpacingWidget )
        m_indentSpacingWidget->display( oldLayout );
    if( m_alignWidget )
        m_alignWidget->display( oldLayout );
    if ( m_decorationsWidget )
        m_decorationsWidget->display( oldLayout );
    if( m_counterWidget )
        m_counterWidget->display( oldLayout );
    if( m_tabulatorsWidget )
        m_tabulatorsWidget->display( oldLayout );
}

bool KoParagDia::isCounterChanged() const
{
    if ( oldLayout.counter ) // We had a counter
        return ! ( *oldLayout.counter == counter() );
    else // We had no counter -> changed if we have one now
        return counter().numbering() != KoParagCounter::NUM_NONE;
}

int KoParagDia::changedFlags() const
{
    return paragLayout().compare( oldLayout );
}

KoParagLayout KoParagDia::paragLayout() const
{
    KoParagLayout newLayout;
    newLayout.setLineSpacingValue( lineSpacing() );
    newLayout.lineSpacingType = lineSpacingType();
    newLayout.setTabList( tabListTabulator() );
    newLayout.alignment = align();
    newLayout.margins[Q3StyleSheetItem::MarginFirstLine] = firstLineIndent();
    newLayout.margins[Q3StyleSheetItem::MarginLeft] = leftIndent();
    newLayout.margins[Q3StyleSheetItem::MarginRight] = rightIndent();
    newLayout.margins[Q3StyleSheetItem::MarginTop] = spaceBeforeParag();
    newLayout.margins[Q3StyleSheetItem::MarginBottom] = spaceAfterParag();
    newLayout.pageBreaking = pageBreaking();
    newLayout.leftBorder = leftBorder();
    newLayout.rightBorder = rightBorder();
    newLayout.topBorder = topBorder();
    newLayout.bottomBorder = bottomBorder();
    newLayout.joinBorder = joinBorder();
    newLayout.backgroundColor = backgroundColor();
    newLayout.counter = new KoParagCounter( counter() );
    return newLayout;
}

#include "KoParagDia.moc"
#include "KoParagDia_p.moc"
