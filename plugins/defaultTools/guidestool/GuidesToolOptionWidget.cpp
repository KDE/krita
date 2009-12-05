/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "GuidesToolOptionWidget.h"
#include "KoSnapGuide.h"

#include <KLocale>

GuidesToolOptionWidget::GuidesToolOptionWidget( QWidget * parent )
    :QWidget(parent)
{
    widget.setupUi(this);

    widget.orientation->addItem( i18n("Horizontal") );
    widget.orientation->addItem( i18n("Vertical") );
    widget.orientation->setCurrentIndex( 0 );
    widget.addButton->setIcon( KIcon( "list-add" ) );
    widget.removeButton->setIcon( KIcon( "list-remove" ) );

    widget.position->setUnit( m_unit );

    connect( widget.orientation, SIGNAL(activated(int)),
             this, SLOT(updateList(int)) );
    connect( widget.positionList, SIGNAL(currentRowChanged(int)),
             this, SLOT(updatePosition(int)) );
    connect( widget.position, SIGNAL(valueChangedPt(qreal)),
             this, SLOT(positionChanged(qreal)) );
    connect( widget.removeButton, SIGNAL(clicked(bool)),
             this, SLOT(removeLine()) );
    connect( widget.addButton, SIGNAL(clicked(bool)),
             this, SLOT(addLine()) );
}

GuidesToolOptionWidget::~GuidesToolOptionWidget()
{
}

void GuidesToolOptionWidget::setHorizontalGuideLines( const QList<qreal> &lines )
{
    m_hGuides = lines;
    if( orientation() == Qt::Horizontal )
        updateList( widget.orientation->currentIndex() );
}

void GuidesToolOptionWidget::setVerticalGuideLines( const QList<qreal> &lines )
{
    m_vGuides = lines;
    if( orientation() == Qt::Vertical )
        updateList( widget.orientation->currentIndex() );
}

QList<qreal> GuidesToolOptionWidget::horizontalGuideLines() const
{
    return m_hGuides;
}

QList<qreal> GuidesToolOptionWidget::verticalGuideLines() const
{
    return m_vGuides;
}

Qt::Orientation GuidesToolOptionWidget::orientation() const
{
    return static_cast<Qt::Orientation>( widget.orientation->currentIndex() + 1 );
}

void GuidesToolOptionWidget::setOrientation( Qt::Orientation orientation )
{
    widget.orientation->setCurrentIndex( orientation-1 );
}

void GuidesToolOptionWidget::selectGuideLine( Qt::Orientation orientation, uint index )
{
    widget.orientation->blockSignals( true );
    widget.positionList->blockSignals( true );

    widget.orientation->setCurrentIndex( orientation-1 );
    updateList( widget.orientation->currentIndex() );
    widget.positionList->setCurrentRow( index );
    updatePosition( index );

    widget.orientation->blockSignals( false );
    widget.positionList->blockSignals( false );
}

void GuidesToolOptionWidget::updateList( int orientation )
{
    widget.positionList->clear();
    if( orientation == Qt::Horizontal-1 )
    {
        foreach( qreal p, m_hGuides )
            widget.positionList->addItem( m_unit.toUserStringValue( p ) );
    }
    else
    {
        foreach( qreal p, m_vGuides )
            widget.positionList->addItem( m_unit.toUserStringValue( p ) );
    }

    widget.removeButton->setEnabled( widget.positionList->count() > 0 );
}

void GuidesToolOptionWidget::updatePosition( int index )
{
    widget.position->blockSignals( true );

    if( index < 0 )
    {
        widget.position->changeValue( 0.0 );
    }
    else
    {
        if( orientation() == Qt::Horizontal )
            widget.position->changeValue( m_hGuides[index] );
        else
            widget.position->changeValue( m_vGuides[index] );

        emit guideLineSelected( orientation(), index );
    }

    widget.position->blockSignals( false );
}

void GuidesToolOptionWidget::positionChanged( qreal position )
{
    int index = widget.positionList->currentRow();
    if( index < 0 )
        return;

    if( orientation() == Qt::Horizontal )
        m_hGuides[index] = position;
    else
        m_vGuides[index] = position;

    // update the current item text int the list
    QListWidgetItem * item = widget.positionList->currentItem();
    if( item )
        item->setText( m_unit.toUserStringValue( position ) );

    emit guideLinesChanged( orientation() );
}

void GuidesToolOptionWidget::removeLine()
{
    widget.positionList->blockSignals( true );

    int index = widget.positionList->currentRow();
    if( index < 0 )
        return;

    if( orientation() == Qt::Horizontal )
        m_hGuides.removeAt(index);
    else
        m_vGuides.removeAt(index);

    delete widget.positionList->takeItem( index );

    widget.positionList->blockSignals( false );

    emit guideLinesChanged( orientation() );
}

void GuidesToolOptionWidget::addLine()
{
    Qt::Orientation o = orientation();

    if( o == Qt::Horizontal )
        m_hGuides.append( widget.position->value() );
    else
        m_vGuides.append( widget.position->value() );

    widget.positionList->blockSignals( true );
    updateList( widget.orientation->currentIndex() );
    widget.positionList->setCurrentRow( widget.positionList->count()-1 );
    widget.positionList->blockSignals( false );

    emit guideLinesChanged( o );
    emit guideLineSelected( o, widget.positionList->currentRow() );
}

void GuidesToolOptionWidget::setUnit( const KoUnit &unit )
{
    m_unit = unit;
    widget.positionList->blockSignals( true );
    widget.position->blockSignals( true );

    int currentRow = widget.positionList->currentRow();
    widget.position->setUnit( unit );
    updateList( widget.orientation->currentIndex() );
    widget.positionList->setCurrentRow( currentRow );

    widget.positionList->blockSignals( false );
    widget.position->blockSignals( false );
}

#include "GuidesToolOptionWidget.moc"
