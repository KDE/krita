/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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


#include "KoStyleCollection.h"
#include "KoStyleDialog.h"
#include "KoStyleDialog.moc"
#include <KoFontDia.h>
#include <KoGlobal.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <QTabWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QLayout>
#include <QListWidget>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3PtrList>
#include <Q3Frame>
#include <Q3ValueList>
#include <QResizeEvent>

/******************************************************************/
/* Class: KoStyleDialog                                          */
/******************************************************************/

/* keep 2 qlists with the styles.
   1 of the origs, another with the changed ones (in order of the stylesList)
   When an orig entry is empty and the other is not, a new one has to be made,
   when the orig is present and the other is not, the orig has to be deleted.
   Otherwise all changes are copied from the changed ones to the origs on OK.
   OK updates the doc if styles are deleted.
   The dtor frees all the changed ones.
*/
/* Months later the above seems SOO stupid.. Just should have created a small class
   containing the orig and the copy and an enum plus some simple methods..
   Well; just keep that for those loonly uninspiring days :) (Thomas Z)
*/
class KoStyleManagerPrivate
{
public:
    KoStylePreview* preview;
    QCheckBox* cbIncludeInTOC;
};

KoStyleDialog::KoStyleDialog( QWidget *_parent, KoUnit::Unit unit,
                                const KoStyleCollection& styles, const QString & activeStyleName,
                                int flags )
    : KDialog( _parent )
{
    setCaption( i18n("Style Manager") );
    setModal( true );
    setObjectName( "Stylist" );
    setButtons( Ok|Cancel|Apply );

    d = new KoStyleManagerPrivate;
    //setWFlags(getWFlags() || WDestructiveClose);
    m_currentStyle =0L;
    noSignals=true;
    m_origStyles.setAutoDelete(false);
    m_changedStyles.setAutoDelete(false);
    setupWidget(styles); // build the widget with the buttons and the list selector.
    addGeneralTab( flags );
    KoStyleFontTab * fontTab = new KoStyleFontTab( m_tabs );
    addTab( fontTab );

    KoStyleParagTab *newTab = new KoStyleParagTab( m_tabs );
    newTab->setWidget( new KoIndentSpacingWidget( unit, -1/*no limit*/,newTab ) );
    addTab( newTab );

    newTab = new KoStyleParagTab( m_tabs );
    newTab->setWidget( new KoParagAlignWidget( true, newTab ) );
    addTab( newTab );

    newTab = new KoStyleParagTab( m_tabs );
    KoParagLayoutWidget *decorations = new KoParagDecorationWidget( newTab );
    decorations->layout()->setMargin(KDialog::marginHint());
    newTab->setWidget( decorations );
    addTab( newTab );

    newTab = new KoStyleParagTab( m_tabs );
    newTab->setWidget( new KoParagCounterWidget( false , newTab ) );
    addTab( newTab );

    newTab = new KoStyleParagTab( m_tabs );
    newTab->setWidget( new KoParagTabulatorsWidget( unit, -1, newTab ) );
    addTab( newTab );

    QList<QListWidgetItem *> tmp = m_stylesList->findItems( activeStyleName, Qt::MatchExactly );
    if( !tmp.isEmpty() )
      m_stylesList->setCurrentItem( tmp.first() );
    else
      m_stylesList->setCurrentRow( 0 );

    noSignals=false;
    switchStyle();
    setInitialSize( QSize( 600, 570 ) );
}

KoStyleDialog::~KoStyleDialog()
{
    for (unsigned int i =0 ; m_origStyles.count() > i ; i++) {
        KoParagStyle *orig = m_origStyles.at(i);
        KoParagStyle *changed = m_changedStyles.at(i);
        if( orig && changed && orig != changed ) // modified style, we can delete the changed one now that changes have been applied
            delete changed;
    }

    delete d;
}

void KoStyleDialog::addTab( KoStyleManagerTab * tab )
{
    m_tabsList.append( tab );
    m_tabs->addTab( tab, tab->tabName() );
    tab->layout()->activate();
}

void KoStyleDialog::setupWidget(const KoStyleCollection& styleCollection)
{
    QFrame * frame1 = new QFrame();
    setMainWidget(frame1);
    Q3GridLayout *frame1Layout = new Q3GridLayout( frame1, 0, 0, // auto
                                                 0, KDialog::spacingHint() );
    numStyles = styleCollection.count();
    m_stylesList = new QListWidget( frame1 );
    m_stylesList->addItems( styleCollection.displayNameList() );

    const Q3ValueList<KoUserStyle*> styleList = styleCollection.styleList();
    for ( Q3ValueList<KoUserStyle *>::const_iterator it = styleList.begin(), end = styleList.end();
          it != end ; ++it )
    {
        KoParagStyle* style = static_cast<KoParagStyle *>( *it );
        m_origStyles.append( style );
        m_changedStyles.append( style );
        m_styleOrder<< style->name();
    }

    frame1Layout->addMultiCellWidget( m_stylesList, 0, 0, 0, 1 );


    m_moveUpButton = new QPushButton( frame1 );
    m_moveUpButton->setIcon( SmallIconSet( "up" ) );
    connect( m_moveUpButton, SIGNAL( clicked() ), this, SLOT( moveUpStyle() ) );
    frame1Layout->addWidget( m_moveUpButton, 1, 1 );

    m_moveDownButton = new QPushButton( frame1 );
    m_moveDownButton->setIcon( SmallIconSet( "down" ) );
    connect( m_moveDownButton, SIGNAL( clicked() ), this, SLOT( moveDownStyle() ) );
    frame1Layout->addWidget( m_moveDownButton, 1, 0 );


    m_deleteButton = new QPushButton( frame1 );
    m_deleteButton->setText( i18n( "&Delete" ) );
    connect( m_deleteButton, SIGNAL( clicked() ), this, SLOT( deleteStyle() ) );

    frame1Layout->addWidget( m_deleteButton, 2, 1 );

    m_newButton = new QPushButton( frame1 );
    m_newButton->setText( i18n( "New" ) );
    connect( m_newButton, SIGNAL( clicked() ), this, SLOT( addStyle() ) );

    frame1Layout->addWidget( m_newButton, 2, 0 );

    m_tabs = new QTabWidget( frame1 );
    frame1Layout->addMultiCellWidget( m_tabs, 0, 2, 2, 2 );

    connect( m_stylesList, SIGNAL( selectionChanged() ), this, SLOT( switchStyle() ) );
    connect( m_tabs, SIGNAL( currentChanged ( QWidget * ) ), this, SLOT( switchTabs() ) );
}

void KoStyleDialog::addGeneralTab( int flags ) {
    QWidget *tab = new QWidget( m_tabs );

    Q3GridLayout *tabLayout = new Q3GridLayout( tab );
    tabLayout->setSpacing( KDialog::spacingHint() );
    tabLayout->setMargin( KDialog::marginHint() );

    m_nameString = new QLineEdit( tab );
    m_nameString->resize(m_nameString->sizeHint() );
    connect( m_nameString, SIGNAL( textChanged( const QString &) ), this, SLOT( renameStyle(const QString &) ) );

    tabLayout->addWidget( m_nameString, 0, 1 );

    QLabel *nameLabel = new QLabel( tab );
    nameLabel->setText( i18n( "Name:" ) );
    nameLabel->resize(nameLabel->sizeHint());
    nameLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    tabLayout->addWidget( nameLabel, 0, 0 );

    m_styleCombo = new QComboBox( tab );
    m_styleCombo->setEditable( false );

    tabLayout->addWidget( m_styleCombo, 1, 1 );

    QLabel *nextStyleLabel = new QLabel( tab );
    nextStyleLabel->setText( i18n( "Next style:" ) );
    nextStyleLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    tabLayout->addWidget( nextStyleLabel, 1, 0 );

    m_inheritCombo = new QComboBox( tab );
    m_inheritCombo->setEditable( false );
    tabLayout->addWidget( m_inheritCombo, 2, 1 );

    QLabel *inheritStyleLabel = new QLabel( tab );
    inheritStyleLabel->setText( i18n( "Inherit style:" ) );
    inheritStyleLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );

    tabLayout->addWidget( inheritStyleLabel, 2, 0 );

    int row = 3;

    if ( flags & ShowIncludeInToc ) {
        d->cbIncludeInTOC = new QCheckBox( i18n("Include in table of contents"), tab );
        tabLayout->addMultiCellWidget( d->cbIncludeInTOC, row, row, 0, 1 );
        ++row;
    } else {
        d->cbIncludeInTOC = 0;
    }

    /*i18n: This is a test text.
      ("The quick brown fox jumps over the lazy dog" contains all
      standard Latin characters from A to Z.)
    */
    d->preview = new KoStylePreview( i18n( "Preview" ), i18n( "The quick brown fox jumps over the lazy dog. And, what about the cat, one may ask? Well, the cat is playing cards with the mouse, the bird and the fish. It is, to say the least a hell of a party!" ), tab );

    tabLayout->addMultiCellWidget( d->preview, row, row, 0, 1 );

    m_tabs->addTab( tab, i18n( "General" ) );

    m_inheritCombo->addItem( i18n("<None>") );

    for ( int i = 0; i < m_stylesList->count(); i++ )
    {
        m_styleCombo->addItem( m_stylesList->item(i)->text() );
        m_inheritCombo->addItem( m_stylesList->item(i)->text() );
    }

}

void KoStyleDialog::switchStyle() {
    kDebug(32500) << "KoStyleDialog::switchStyle noSignals=" << noSignals << endl;
    if(noSignals) return;
    noSignals=true;

    if(m_currentStyle !=0L)
        save();

    m_currentStyle = 0L;
    int num = styleIndex( m_stylesList->currentRow() );
    kDebug(32500) << "KoStyleDialog::switchStyle switching to " << num << endl;
    if(m_origStyles.at(num) == m_changedStyles.at(num)) {
        m_currentStyle = new KoParagStyle( *m_origStyles.at(num) );
        m_changedStyles.take(num);
        m_changedStyles.insert(num, m_currentStyle);
    } else {
        m_currentStyle = m_changedStyles.at(num);
    }
    updateGUI();

    noSignals=false;
}

void KoStyleDialog::switchTabs()
{
    // Called when the user switches tabs
    // We call save() to update our style, for the preview on the 1st tab
    save();
    updatePreview();
}

// Return the index of the a style from its position in the GUI
// (e.g. in m_stylesList or m_styleCombo). This index is used in
// the m_origStyles and m_changedStyles lists.
// The reason for the difference is that a deleted style is removed
// from the GUI but not from the internal lists.
int KoStyleDialog::styleIndex( int pos ) {
    int p = 0;
    for(unsigned int i=0; i < m_changedStyles.count(); i++) {
        // Skip deleted styles, they're no in m_stylesList anymore
        KoParagStyle * style = m_changedStyles.at(i);
        if ( !style ) continue;
        if ( p == pos )
            return i;
        ++p;
    }
    kWarning() << "KoStyleDialog::styleIndex no style found at pos " << pos << endl;

#ifdef __GNUC_
#warning implement undo/redo
#endif

    return 0;
}

// Update the GUI so that it shows m_currentStyle
void KoStyleDialog::updateGUI() {
    kDebug(32500) << "KoStyleDialog::updateGUI m_currentStyle=" << m_currentStyle << " " << m_currentStyle->name() << endl;
    Q3PtrListIterator<KoStyleManagerTab> it( m_tabsList );
    for ( ; it.current() ; ++it )
    {
        it.current()->setStyle( m_currentStyle );
        it.current()->update();
    }

    m_nameString->setText(m_currentStyle->displayName());

    QString followingName = m_currentStyle->followingStyle() ? m_currentStyle->followingStyle()->displayName() : QString::null;
    kDebug(32500) << "KoStyleDialog::updateGUI updating combo to " << followingName << endl;
    for ( int i = 0; i < m_styleCombo->count(); i++ ) {
        if ( m_styleCombo->itemText( i ) == followingName ) {
            m_styleCombo->setCurrentIndex( i );
            kDebug(32500) << "found at " << i << endl;
            break;
        }
    }

    QString inheritName = m_currentStyle->parentStyle() ? m_currentStyle->parentStyle()->displayName() : QString::null;
    kDebug(32500) << "KoStyleDialog::updateGUI updating combo to " << inheritName << endl;
    for ( int i = 0; i < m_inheritCombo->count(); i++ ) {
        if ( m_inheritCombo->itemText( i ) == inheritName ) {
            m_inheritCombo->setCurrentIndex( i );
            kDebug(32500) << "found at " << i << endl;
            break;
        }
        else
            m_inheritCombo->setCurrentIndex( 0 );//none !!!
    }

    if ( d->cbIncludeInTOC )
        d->cbIncludeInTOC->setChecked( m_currentStyle->isOutline() );

    // update delete button (can't delete first style);
    m_deleteButton->setEnabled(m_stylesList->currentRow() != 0);

    m_moveUpButton->setEnabled(m_stylesList->currentRow() != 0);
    m_moveDownButton->setEnabled(m_stylesList->currentRow()!=(int)m_stylesList->count()-1);

    updatePreview();
}

void KoStyleDialog::updatePreview()
{
    d->preview->setStyle(m_currentStyle);
    d->preview->update();
}

void KoStyleDialog::save() {
    if(m_currentStyle) {
        // save changes from UI to object.
        Q3PtrListIterator<KoStyleManagerTab> it( m_tabsList );
        for ( ; it.current() ; ++it )
            it.current()->save();

	// Rename the style - only if it's actually been renamed.
        if ( m_currentStyle->name() != m_nameString->text() &&
            m_currentStyle->displayName() != m_nameString->text() )
        {
            m_currentStyle->setDisplayName( m_nameString->text() );
        }

        int indexNextStyle = styleIndex( m_styleCombo->currentIndex() );
        m_currentStyle->setFollowingStyle( m_origStyles.at( indexNextStyle ) ); // point to orig, not changed! (#47377)

        if ( m_inheritCombo->currentItem() == 0 )  //<None> selected
            m_currentStyle->setParentStyle(0);
        else
        {
            int indexParentStyle=styleIndex( m_inheritCombo->currentItem()-1 );
            KoParagStyle *parent=m_origStyles.at(indexParentStyle);
            if( parent==0L )  //If not found in the orig list (means its a new Style) look in the changeStyles list
                parent=m_changedStyles.at(indexParentStyle);
            m_currentStyle->setParentStyle( parent );
        }

        if ( d->cbIncludeInTOC )
            m_currentStyle->setOutline( d->cbIncludeInTOC->isChecked() );
    }
}

KoParagStyle * KoStyleDialog::style( const QString & _name )
{
    for(unsigned int i=0; i < m_changedStyles.count(); i++) {
        // Skip deleted styles, they're no in m_stylesList anymore
        KoParagStyle * style = m_changedStyles.at(i);
        if ( !style ) continue;
        if ( style->name() == _name)
            return style;
    }
    return 0;
}

QString KoStyleDialog::generateUniqueName()
{
    int count = 1;
    QString name;
    do {
        name = "new" + QString::number( count++ );
    } while ( style( name ) );
    return name;
}


void KoStyleDialog::addStyle() {
    save();

    QString str = i18n( "New Style Template (%1)" ,numStyles++);
    if ( m_currentStyle )
    {
        m_currentStyle = new KoParagStyle( *m_currentStyle ); // Create a new style, initializing from the current one
        m_currentStyle->setDisplayName( str );
        m_currentStyle->setName( generateUniqueName() );
    }
    else
        m_currentStyle = new KoParagStyle( str );
    m_currentStyle->setFollowingStyle( m_currentStyle ); // #45868

    noSignals=true;
    m_origStyles.append(0L);
    m_changedStyles.append(m_currentStyle);
    m_stylesList->addItem( str );
    m_styleCombo->addItem( str );
    m_inheritCombo->addItem( str );
    m_stylesList->setCurrentRow( m_stylesList->count() - 1 );
    noSignals=false;
    m_styleOrder << m_currentStyle->name();

    updateGUI();
}

void KoStyleDialog::updateFollowingStyle( KoParagStyle *s )
{
    for ( KoParagStyle* p = m_changedStyles.first(); p != 0L; p = m_changedStyles.next() )
    {
        if ( p->followingStyle() == s)
            p->setFollowingStyle(p);
    }

}

void KoStyleDialog::updateInheritStyle( KoParagStyle *s )
{
    for ( KoParagStyle* p = m_changedStyles.first(); p != 0L; p = m_changedStyles.next() )
    {
        //when we remove style, we must replace inherite style to 0L
        //when parent style was removed.
        //##########Laurent change inherited style attribute
        if ( p->parentStyle() == s)
            p->setParentStyle(0L);
    }

}

void KoStyleDialog::deleteStyle() {

    unsigned int cur = styleIndex( m_stylesList->currentRow() );
    unsigned int curItem = m_stylesList->currentRow();
    QString name = m_stylesList->currentItem()->text();
    KoParagStyle *s = m_changedStyles.at(cur);
    m_styleOrder.removeAll( s->name());
    updateFollowingStyle( s );
    updateInheritStyle( s );
    Q_ASSERT( s == m_currentStyle );
    delete s;
    m_currentStyle = 0L;
    m_changedStyles.remove(cur);
    m_changedStyles.insert(cur,0L);

    // Done with noSignals still false, so that when m_stylesList changes the current item
    // we display it automatically
    m_stylesList->takeItem(curItem);
    m_styleCombo->removeItem(curItem);

    m_inheritCombo->removeItem( m_inheritCombo->findText( name ) );

    numStyles--;
    m_stylesList->setItemSelected( m_stylesList->currentItem(), true );
}

void KoStyleDialog::moveUpStyle()
{
    Q_ASSERT( m_currentStyle );
    if ( m_currentStyle )
        save();
    const QString currentStyleName = m_currentStyle->name();
    const QString currentStyleDisplayName = m_stylesList->currentItem()->text();
    int pos2 = m_styleOrder.indexOf( currentStyleName );
    if ( pos2 != -1 )
    {
        m_styleOrder.removeAt(pos2);
        m_styleOrder.insert( pos2-1, currentStyleName);
    }

    int pos = m_stylesList->currentRow();
    noSignals=true;
    m_stylesList->item( pos )->setText( m_stylesList->item( pos-1 )->text() );
    m_styleCombo->setItemText( pos, m_stylesList->item( pos-1 )->text() );

    m_stylesList->item(pos-1)->setText( currentStyleDisplayName );
    m_styleCombo->setItemText( pos-1, currentStyleDisplayName );

    m_stylesList->setCurrentItem( m_stylesList->currentItem() );
    noSignals=false;

    updateGUI();
}

void KoStyleDialog::moveDownStyle()
{
    Q_ASSERT( m_currentStyle );
    if ( m_currentStyle )
        save();
    const QString currentStyleName = m_currentStyle->name();
    const QString currentStyleDisplayName = m_stylesList->currentItem()->text();
    int pos2 = m_styleOrder.indexOf( currentStyleName );
    if ( pos2 != -1 )
    {
        m_styleOrder.removeAt( pos2 );
        m_styleOrder.insert( pos2+1, currentStyleName);
    }

    int pos = m_stylesList->currentRow();
    noSignals=true;
    m_stylesList->item( pos )->setText( m_stylesList->item( pos+1 )->text() );
    m_styleCombo->setItemText( pos, m_stylesList->item( pos+1 )->text() );
    m_stylesList->item( pos+1 )->setText( currentStyleDisplayName );
    m_styleCombo->setItemText( pos+1, currentStyleDisplayName );
    m_stylesList->setCurrentItem( m_stylesList->currentItem() );
    noSignals=false;

    updateGUI();
}

void KoStyleDialog::slotOk() {
    save();
    apply();
    slotButtonClicked( Ok );
}

void KoStyleDialog::slotApply() {
    save();
    apply();
    slotButtonClicked( Apply );
}

void KoStyleDialog::apply() {
    noSignals=true;
    KoStyleChangeDefMap styleChanged;
    Q3PtrList<KoParagStyle> removeStyle;
    for (unsigned int i =0 ; m_origStyles.count() > i ; i++) {
        if(m_origStyles.at(i) == 0L && m_changedStyles.at(i)!=0L) {           // newly added style
            kDebug(32500) << "adding new " << m_changedStyles.at(i)->name() << " (" << i << ")" << endl;
            KoParagStyle *tmp = addStyleTemplate(m_changedStyles.take(i));
            m_changedStyles.insert(i, tmp);
        } else if(m_changedStyles.at(i) == 0L && m_origStyles.at(i) != 0L) { // deleted style
            kDebug(32500) << "deleting orig " << m_origStyles.at(i)->name() << " (" << i << ")" << endl;

            KoParagStyle *orig = m_origStyles.at(i);
            //applyStyleChange( orig, -1, -1 );
            KoStyleChangeDef tmp( -1,-1);
            styleChanged.insert( orig, tmp);

            removeStyle.append( orig );
            // Note that the style is never deleted (we'll need it for undo/redo purposes)

        } else if(m_changedStyles.at(i) != 0L && m_origStyles.at(i)!=0L) { // simply updated style
            kDebug(32500) << "update style " << m_changedStyles.at(i)->name() << " (" << i << ")" << endl;
            KoParagStyle *orig = m_origStyles.at(i);
            KoParagStyle *changed = m_changedStyles.at(i);
            if ( orig != changed )
            {
                int paragLayoutChanged = orig->paragLayout().compare( changed->paragLayout() );
                int formatChanged = orig->format().compare( changed->format() );
                //kDebug(32500) << "old format " << orig->format().key() << " pointsize " << orig->format().pointSizeF() << endl;
                //kDebug(32500) << "new format " << changed->format().key() << " pointsize " << changed->format().pointSizeF() << endl;

                // Copy everything from changed to orig
                *orig = *changed;

                // Apply the change selectively - i.e. only what changed
                //applyStyleChange( orig, paragLayoutChanged, formatChanged );
                if ( formatChanged != 0 || paragLayoutChanged != 0 ) {
                    KoStyleChangeDef tmp(paragLayoutChanged, formatChanged);
                    styleChanged.insert( orig, tmp );
                }

            }

        }// else
         //     kDebug(32500) << "has not changed " <<  m_changedStyles.at(i)->name() << " (" << i << ")" <<  endl;
    }

    applyStyleChange( styleChanged );

    KoParagStyle *tmp = 0L;
    for ( tmp = removeStyle.first(); tmp ;tmp = removeStyle.next() )
        removeStyleTemplate( tmp );

    updateStyleListOrder( m_styleOrder );
    updateAllStyleLists();
    noSignals=false;
}

void KoStyleDialog::renameStyle(const QString &theText) {
    if(noSignals) return;
    noSignals=true;

    int index = m_stylesList->currentRow();
    kDebug(32500) << "KoStyleDialog::renameStyle " << index << " to " << theText << endl;

    // rename only in the GUI, not even in the underlying objects (save() does it).
    kDebug(32500) << "KoStyleDialog::renameStyle before " << m_styleCombo->currentText() << endl;
    m_styleCombo->setItemText( index, theText );
    m_inheritCombo->setItemText( index+1, theText );
    //m_styleOrder[index]=theText; // not needed anymore, we use internal names
    kDebug(32500) << "KoStyleDialog::renameStyle after " << m_styleCombo->currentText() << endl;
    m_stylesList->item( index )->setText( theText );

    // Check how many styles with that name we have now
    int synonyms = 0;
    for ( int i = 0; i < m_styleCombo->count(); i++ ) {
        if ( m_styleCombo->currentText() == m_stylesList->currentItem()->text() )
            ++synonyms;
    }
    Q_ASSERT( synonyms > 0 ); // should have found 'index' at least !
    noSignals=false;
    // Can't close the dialog if two styles have the same name
    bool state=!theText.isEmpty() && (synonyms == 1);
    enableButtonOk(state );
    enableButtonApply(state);
    m_deleteButton->setEnabled(state&&(m_stylesList->currentItem() != 0));
    m_newButton->setEnabled(state);
    m_stylesList->setEnabled( state );
    if ( state )
    {
        m_moveUpButton->setEnabled(m_stylesList->currentRow() != 0);
        m_moveDownButton->setEnabled(m_stylesList->currentRow()!=(int)m_stylesList->count()-1);
    }
    else
    {
        m_moveUpButton->setEnabled(false);
        m_moveDownButton->setEnabled(false);
    }
}

/////////////

KoStyleParagTab::KoStyleParagTab( QWidget * parent )
    : KoStyleManagerTab( parent )
{
     m_widget = 0L;
}

void KoStyleParagTab::update()
{
     m_widget->display( m_style->paragLayout() );
}

void KoStyleParagTab::save()
{
     m_widget->save( m_style->paragLayout() );
}

void KoStyleParagTab::setWidget( KoParagLayoutWidget * widget )
{
    m_widget = widget;
}

void KoStyleParagTab::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    if ( m_widget ) m_widget->resize( size() );
}

KoStyleFontTab::KoStyleFontTab( QWidget * parent )
    : KoStyleManagerTab( parent )
{
	QTabWidget *fontTabContainer = new QTabWidget( this );

	m_fontTab = new KoFontTab( KFontChooser::SmoothScalableFonts, this );
	m_decorationTab = new KoDecorationTab( this );
	m_highlightingTab = new KoHighlightingTab( this );
	m_layoutTab = new KoLayoutTab( true, this );
	m_languageTab = new KoLanguageTab( );

	fontTabContainer->addTab( m_fontTab, i18n( "Font" ) );
	fontTabContainer->addTab( m_decorationTab, i18n( "Decoration" ) );
	fontTabContainer->addTab( m_highlightingTab, i18n( "Highlighting" ) );
	fontTabContainer->addTab( m_layoutTab, i18n( "Layout" ) );
	fontTabContainer->addTab( m_languageTab, i18n( "Language" ) );
}

KoStyleFontTab::~KoStyleFontTab()
{
}

void KoStyleFontTab::update()
{
	m_fontTab->setSelection( m_style->format().font() );
	m_highlightingTab->setUnderline( m_style->format().underlineType() );
	m_highlightingTab->setUnderlineStyle( m_style->format().underlineStyle() );
	m_highlightingTab->setUnderlineColor( m_style->format().textUnderlineColor() );
	m_highlightingTab->setStrikethrough( m_style->format().strikeOutType() );
	m_highlightingTab->setStrikethroughStyle( m_style->format().strikeOutStyle() );
	m_highlightingTab->setWordByWord( m_style->format().wordByWord() );
	m_highlightingTab->setCapitalisation( m_style->format().attributeFont() );
	m_decorationTab->setTextColor( m_style->format().color() );
	m_decorationTab->setBackgroundColor( m_style->format().textBackgroundColor() );
	m_decorationTab->setShadow( m_style->format().shadowDistanceX(), m_style->format().shadowDistanceY(), m_style->format().shadowColor() );
	m_layoutTab->setSubSuperScript( m_style->format().vAlign(), m_style->format().offsetFromBaseLine(), m_style->format().relativeTextSize() );
	m_layoutTab->setAutoHyphenation( m_style->format().hyphenation() );
	m_languageTab->setLanguage( m_style->format().language() );
/*
#if 0
    bool subScript = m_style->format().vAlign() == KoTextFormat::AlignSubScript;
    bool superScript = m_style->format().vAlign() == KoTextFormat::AlignSuperScript;
    QFont fn = m_style->format().font();
    kDebug()<<" fn.bold() :"<<fn.bold()<<" fn.italic():"<<fn.italic()<<endl;
    kDebug()<<" fn.family() :"<<fn.family()<<endl;
    m_chooser->setFont( fn, subScript, superScript );
    m_chooser->setColor( m_style->format().color() );
    QColor col=m_style->format().textBackgroundColor();
    col=col.isValid() ? col : QApplication::palette().color( QPalette::Active, QColorGroup::Base );
    m_chooser->setBackGroundColor(col);

    m_chooser->setUnderlineColor( m_style->format().textUnderlineColor());

    m_chooser->setUnderlineType(m_style->format().underlineType());
    m_chooser->setUnderlineStyle(m_style->format().underlineStyle());
    m_chooser->setStrikeOutStyle(m_style->format().strikeOutStyle());
    m_chooser->setStrikeOutlineType(m_style->format().strikeOutType());
    m_chooser->setShadowText( m_style->format().shadowText());
    m_chooser->setFontAttribute( m_style->format().attributeFont());
    m_chooser->setWordByWord( m_style->format().wordByWord());
    m_chooser->setRelativeTextSize( m_style->format().relativeTextSize());
    m_chooser->setOffsetFromBaseLine( m_style->format().offsetFromBaseLine());
    m_chooser->setLanguage( m_style->format().language());
    m_chooser->setHyphenation( m_style->format().hyphenation());
#endif
*/}

void KoStyleFontTab::save()
{
	m_style->format() = KoTextFormat( m_fontTab->getSelection(),
                         m_layoutTab->getSubSuperScript(),
                         m_decorationTab->getTextColor(),
                         m_decorationTab->getBackgroundColor(),
                         m_highlightingTab->getUnderlineColor(),
                         m_highlightingTab->getUnderline(),
                         m_highlightingTab->getUnderlineStyle(),
                         m_highlightingTab->getStrikethrough(),
                         m_highlightingTab->getStrikethroughStyle(),
                         m_highlightingTab->getCapitalisation(),
                         m_languageTab->getLanguage(),
                         m_layoutTab->getRelativeTextSize(),
                         m_layoutTab->getOffsetFromBaseline(),
                         m_highlightingTab->getWordByWord(),
                         m_layoutTab->getAutoHyphenation(),
                         m_decorationTab->getShadowDistanceX(),
                         m_decorationTab->getShadowDistanceY(),
                         m_decorationTab->getShadowColor()
			);
/*
	m_style->format().setFont( m_fontTab->getSelection() );
	m_style->format().setColor( m_decorationTab->getTextColor() );
	if( m_decorationTab->getBackGroundColor()!=QApplication::palette().color( QPalette::Active, QColorGroup::Base ))
        m_style->format().setTextBackgroundColor( m_decorationTab->getBackGroundColor() );

	m_style->format().setTextUnderlineColor(m_chooser->underlineColor());
    m_style->format().setUnderlineType (m_chooser->getUnderlineType());
    m_style->format().setUnderlineStyle (m_chooser->getUnderlineStyle());
    m_style->format().setStrikeOutStyle( m_chooser->getStrikeOutStyle() );
    m_style->format().setStrikeOutType (m_chooser->getStrikeOutType());
    m_style->format().setShadowText(m_chooser->getShadowText());
    m_style->format().setWordByWord( m_chooser->getWordByWord());
    m_style->format().setRelativeTextSize( m_chooser->getRelativeTextSize());
    m_style->format().setAttributeFont( m_chooser->getFontAttribute());
    m_style->format().setOffsetFromBaseLine( m_chooser->getOffsetFromBaseLine());
	m_style->format().setVAlign( m_layoutTab->getSubSuperScript() );

    m_style->format().setLanguage( m_chooser->getLanguage());
    m_style->format().setHyphenation( m_chooser->getHyphenation());
*/}

QString KoStyleFontTab::tabName()
{
    return i18n("Font");
}
