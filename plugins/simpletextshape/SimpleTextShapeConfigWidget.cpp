/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
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

#include "SimpleTextShapeConfigWidget.h"
#include "SimpleTextShape.h"
#include <QtGui/QButtonGroup>

#include <KoCanvasController.h>
#include <KoToolManager.h>
#include <KoShapeManager.h>
#include <KoCanvasBase.h>

SimpleTextShapeConfigWidget::SimpleTextShapeConfigWidget()
    : m_shape(0), m_anchorGroup(0)
{
    widget.setupUi( this );

    widget.bold->setCheckable( true );
    widget.bold->setIcon( KIcon( "format-text-bold" ) );
    widget.italic->setCheckable( true );
    widget.italic->setIcon( KIcon( "format-text-italic" ) );
    widget.anchorStart->setIcon( KIcon( "format-justify-left" ) );
    widget.anchorStart->setCheckable( true );
    widget.anchorMiddle->setIcon( KIcon( "format-justify-center" ) );
    widget.anchorMiddle->setCheckable( true );
    widget.anchorEnd->setIcon( KIcon( "format-justify-right" ) );
    widget.anchorEnd->setCheckable( true );
    widget.fontSize->setRange( 2, 1000 );

    m_anchorGroup = new QButtonGroup(this);
    m_anchorGroup->addButton( widget.anchorStart );
    m_anchorGroup->addButton( widget.anchorMiddle );
    m_anchorGroup->addButton( widget.anchorEnd );

    connect( widget.fontFamily, SIGNAL(currentFontChanged(const QFont&)), this, SIGNAL(propertyChanged()));
    connect( widget.fontSize, SIGNAL(valueChanged(int)), this, SIGNAL(propertyChanged()));
    connect( widget.bold, SIGNAL(toggled(bool)), this, SIGNAL(propertyChanged()));
    connect( widget.italic, SIGNAL(toggled(bool)), this, SIGNAL(propertyChanged()));
    connect( widget.text, SIGNAL(textChanged(const QString&)), this, SIGNAL(propertyChanged()));
    connect( widget.startOffset, SIGNAL(valueChanged(int)), this, SIGNAL(propertyChanged()));
    connect( m_anchorGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(propertyChanged()));

    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
    if ( canvasController ) {
        KoShapeManager *manager = canvasController->canvas()->shapeManager();
        connect( manager, SIGNAL(selectionContentChanged()), this, SLOT(slotTextChanged()));
    }
}

void SimpleTextShapeConfigWidget::blockChildSignals( bool block )
{
    widget.fontFamily->blockSignals( block );
    widget.fontSize->blockSignals( block );
    widget.text->blockSignals( block );
    widget.bold->blockSignals( block );
    widget.italic->blockSignals( block );
    widget.startOffset->blockSignals( block );
    m_anchorGroup->blockSignals( block );
}

void SimpleTextShapeConfigWidget::open(KoShape *shape)
{
    m_shape = dynamic_cast<SimpleTextShape*>( shape );
    if( ! m_shape )
        return;

    blockChildSignals( true );

    QFont font = m_shape->font();

    int cursorPos = widget.text->cursorPosition();
    widget.text->setText( m_shape->text() );
    widget.text->setCursorPosition( cursorPos );
    widget.fontSize->setValue( font.pointSize() );
    font.setPointSize( 8 );

    widget.fontFamily->setFont( font );
    widget.bold->setChecked( font.bold() );
    widget.italic->setChecked( font.italic() );
    if( m_shape->textAnchor() == SimpleTextShape::AnchorStart )
        widget.anchorStart->setChecked( true );
    else if( m_shape->textAnchor() == SimpleTextShape::AnchorMiddle )
        widget.anchorMiddle->setChecked( true );
    else
        widget.anchorEnd->setChecked( true );
    widget.startOffset->setValue( static_cast<int>( m_shape->startOffset() * 100.0 ) );
    widget.startOffset->setVisible( m_shape->isOnPath() );
    widget.labelStartOffset->setVisible( m_shape->isOnPath() );

    blockChildSignals( false );
}

void SimpleTextShapeConfigWidget::save()
{
    if( ! m_shape )
        return;

    QFont font = widget.fontFamily->currentFont();
    font.setBold( widget.bold->isChecked() );
    font.setItalic( widget.italic->isChecked() );
    font.setPointSize( widget.fontSize->value() );

    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();
    if ( canvasController ) {
        KoCanvasBase *canvas = canvasController->canvas();
	if ( widget.text->text() != m_shape->text() ) {
            canvas->addCommand( new ChangeText( this, widget.text->text() ) );
	} else if ( font != m_shape->font() ) {
            canvas->addCommand( new ChangeFont( this, font ) );
	} else {
	    SimpleTextShape::TextAnchor anchor;
            if ( widget.anchorStart->isChecked() )
                anchor = SimpleTextShape::AnchorStart;
            else if ( widget.anchorMiddle->isChecked() )
                anchor = SimpleTextShape::AnchorMiddle;
            else
                anchor = SimpleTextShape::AnchorEnd;
            if ( anchor != m_shape->textAnchor() ) {
                canvas->addCommand( new ChangeAnchor( this, anchor ) );
	    }
	}
    }

    m_shape->setStartOffset( static_cast<qreal>(widget.startOffset->value()) / 100.0 );
}

QUndoCommand * SimpleTextShapeConfigWidget::createCommand()
{
    save();

    return 0;
}

void SimpleTextShapeConfigWidget::slotTextChanged()
{
    if ( ! m_shape )
        return;

    int cursorPos = widget.text->cursorPosition();
    blockChildSignals( true );
    widget.text->setText( m_shape->text() );
    blockChildSignals( false );
    widget.text->setCursorPosition( cursorPos );
}

