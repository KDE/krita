/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>

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

#include "KoHighlightingTab.h"
#include "KoTextFormat.h"

#include <QStringList>
#include <q3buttongroup.h>
#include <QRadioButton>
#include <QCheckBox>

#include <kcombobox.h>
#include <kcolorbutton.h>

#include "KoHighlightingTab.moc"


KoHighlightingTab::KoHighlightingTab( QWidget* parent, const char* name, Qt::WFlags fl ) 
        : KoHighlightingTabBase( parent, name, fl )
{
    underlineStyleKComboBox->addItems( KoTextFormat::underlineTypeList() );
    underlineLineStyleKComboBox->addItems( KoTextFormat::underlineStyleList() );
    strikethroughStyleKComboBox->addItems( KoTextFormat::strikeOutTypeList() );
    strikethroughLineStyleKComboBox->addItems( KoTextFormat::strikeOutStyleList() );

    capitalisationButtonGroup->setColumnLayout( 3, Qt::Horizontal );
    QStringList attributes = KoTextFormat::fontAttributeList();
    for ( QStringList::Iterator it = attributes.begin(); it != attributes.end(); ++it ) {
        capitalisationButtonGroup->insert( new QRadioButton( *it, capitalisationButtonGroup ) );
    }
    capitalisationButtonGroup->setButton( 0 );

    connect( underlineStyleKComboBox, SIGNAL( activated( int ) ), this, SLOT( slotUnderlineChanged( int ) ) );
    connect( underlineLineStyleKComboBox, SIGNAL( activated( int ) ), this, SIGNAL( underlineStyleChanged( int ) ) );
    connect( underlineKColorButton, SIGNAL( changed( const QColor & ) ), this, SIGNAL( underlineColorChanged( const QColor & ) ) );
    connect( strikethroughStyleKComboBox, SIGNAL( activated( int ) ), this, SLOT( slotStrikethroughChanged( int ) ) );
    connect( strikethroughLineStyleKComboBox, SIGNAL( activated( int ) ), this, SIGNAL( strikethroughStyleChanged( int ) ) );
    connect( underlineWordByWordCheckBox, SIGNAL( toggled( bool ) ), this, SIGNAL( wordByWordChanged( bool ) ) );
    connect( capitalisationButtonGroup, SIGNAL( clicked( int ) ), this, SIGNAL( capitalisationChanged( int ) ) );
}

KoHighlightingTab::~KoHighlightingTab()
{
}

KoTextFormat::UnderlineType KoHighlightingTab::getUnderline() const
{
    return static_cast<KoTextFormat::UnderlineType>( underlineStyleKComboBox->currentIndex() );
}

KoTextFormat::UnderlineStyle KoHighlightingTab::getUnderlineStyle() const
{
    return static_cast<KoTextFormat::UnderlineStyle>( underlineLineStyleKComboBox->currentIndex() );
}

QColor KoHighlightingTab::getUnderlineColor() const
{
    return underlineKColorButton->color();
}

KoTextFormat::StrikeOutType KoHighlightingTab::getStrikethrough() const
{
    return static_cast<KoTextFormat::StrikeOutType>( strikethroughStyleKComboBox->currentIndex() );
}

KoTextFormat::StrikeOutStyle KoHighlightingTab::getStrikethroughStyle() const
{
    return static_cast<KoTextFormat::StrikeOutStyle>( strikethroughLineStyleKComboBox->currentIndex() );
}

bool KoHighlightingTab::getWordByWord() const
{
    return underlineWordByWordCheckBox->isOn();
}

KoTextFormat::AttributeStyle KoHighlightingTab::getCapitalisation() const
{
    return static_cast<KoTextFormat::AttributeStyle>( capitalisationButtonGroup->selectedId() );
}

void KoHighlightingTab::setUnderline( KoTextFormat::UnderlineType item )
{
    underlineStyleKComboBox->setCurrentIndex( static_cast<int>( item ) );
    slotUnderlineChanged( static_cast<int>( item ) );
}

void KoHighlightingTab::setUnderlineStyle( KoTextFormat::UnderlineStyle item )
{
    underlineLineStyleKComboBox->setCurrentIndex( static_cast<int>( item ) );
    emit underlineStyleChanged( static_cast<int>( item ) );
}

void KoHighlightingTab::setUnderlineColor( const QColor &color )
{
    underlineKColorButton->setColor( color );
}

void KoHighlightingTab::setStrikethrough( int item )
{
    strikethroughStyleKComboBox->setCurrentIndex( static_cast<int>( item ) );
    slotStrikethroughChanged( static_cast<int>( item ) );
}

void KoHighlightingTab::setStrikethroughStyle( int item )
{
    strikethroughLineStyleKComboBox->setCurrentIndex( static_cast<int>( item ) );
    emit strikethroughStyleChanged( static_cast<int>( item ) );
}

void KoHighlightingTab::setWordByWord( bool state )
{
    underlineWordByWordCheckBox->setChecked( state );
}

void KoHighlightingTab::setCapitalisation( int item )
{
    capitalisationButtonGroup->setButton( static_cast<int>( item ) );
    emit capitalisationChanged( static_cast<int>( item ) );
}

void KoHighlightingTab::slotUnderlineChanged( int item )
{
    underlineLineStyleKComboBox->setEnabled( item > 0 );
    underlineKColorButton->setEnabled( item > 0 );
    underlineWordByWordCheckBox->setEnabled( ( item > 0 ) || ( strikethroughStyleKComboBox->currentIndex() > 0 ) );
    emit underlineChanged( item );
}

void KoHighlightingTab::slotStrikethroughChanged( int item )
{
    strikethroughLineStyleKComboBox->setEnabled( item > 0 );
    underlineWordByWordCheckBox->setEnabled( ( item > 0 ) || ( underlineStyleKComboBox->currentIndex() > 0 ) );
    emit strikethroughChanged(  item );
}

