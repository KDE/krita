/* ============================================================
 * Author: Tom Albers <tomalbers@kde.nl>
 * Date  : 2005-01-01
 * Description : 
 * 
 * Copyright 2005 by Tom Albers
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

/** @file squeezedcombobox.cpp */

// Qt includes.

#include <qlistbox.h>
#include <qcombobox.h>
#include <qpair.h>
#include <qtimer.h>
#include <qvaluelist.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qtooltip.h>

// Local includes.

#include "squeezedcombobox.h"

SqueezedComboBoxTip::SqueezedComboBoxTip( QWidget * parent, SqueezedComboBox* name )
    : QToolTip( parent )
{
    m_originalWidget = name;
}

void SqueezedComboBoxTip::maybeTip( const QPoint &pos )
{
    QListBox* listBox = m_originalWidget->listBox();
    if (!listBox)
        return;

    QListBoxItem* selectedItem = listBox->itemAt( pos );
    if (selectedItem)
    {
        QRect positionToolTip = listBox->itemRect( selectedItem );
        QString toolTipText = m_originalWidget->itemHighlighted();
        if (!toolTipText.isNull())
            tip(positionToolTip, toolTipText);
    }
}

SqueezedComboBox::SqueezedComboBox( QWidget *parent, const char *name )
    : QComboBox( parent, name )
{
    setMinimumWidth(100);
    m_timer = new QTimer(this);
    m_tooltip = new SqueezedComboBoxTip( listBox()->viewport(), this );

    connect(m_timer, SIGNAL(timeout()),
            SLOT(slotTimeOut()));
    connect(this, SIGNAL(activated( int )),
            SLOT(slotUpdateToolTip( int )));
}

SqueezedComboBox::~SqueezedComboBox()
{
    delete m_tooltip;
    delete m_timer;
}

bool SqueezedComboBox::contains( const QString& _text ) const
{
    if ( _text.isEmpty() )
        return false;

    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i )
    {
        if ( text(i) == _text )
            return true;
    }
    return false;
}

QSize SqueezedComboBox::sizeHint() const
{
    constPolish();
    QFontMetrics fm = fontMetrics();

    int maxW = count() ? 18 : 7 * fm.width(QChar('x')) + 18;
    int maxH = QMAX( fm.lineSpacing(), 14 ) + 2;

    return style().sizeFromContents(QStyle::CT_ComboBox, this,
    QSize(maxW, maxH)).
            expandedTo(QApplication::globalStrut());
}

void SqueezedComboBox::insertSqueezedItem(const QString& newItem, int index)
{
    m_originalItems[index] = newItem;
    insertItem( squeezeText(newItem), index );

    // if this is the first item, set the tooltip.
    if (index == 0)
        slotUpdateToolTip(0);
}

void SqueezedComboBox::resizeEvent ( QResizeEvent * )
{
    m_timer->start(200, true);
}

void SqueezedComboBox::slotTimeOut()
{
    QMapIterator<int,QString> it;
    for (it = m_originalItems.begin() ; it != m_originalItems.end();
         ++it)
    {
        changeItem( squeezeText( it.data() ), it.key() );
    }
}

QString SqueezedComboBox::squeezeText( const QString& original)
{
    // not the complete widgetSize is usable. Need to compensate for that.
    int widgetSize = width()-30;
    QFontMetrics fm( fontMetrics() );

    // If we can fit the full text, return that.
    if (fm.width(original) < widgetSize)
        return(original);

    // We need to squeeze.
    QString sqItem = original; // prevent empty return value;
    widgetSize = widgetSize-fm.width("...");
    for (uint i = 0 ; i != original.length(); ++i)
    {
        if ( (int)fm.width(original.right(i)) > widgetSize)
        {
            sqItem = QString("..." + original.right(--i));
            break;
        }
    }
    return sqItem;
}

void SqueezedComboBox::slotUpdateToolTip( int index )
{
    QToolTip::remove(this);
    QToolTip::add(this, m_originalItems[index]);
}

QString SqueezedComboBox::itemHighlighted()
{
    int curItem = this->listBox()->currentItem();
    return m_originalItems[curItem];
}

#include "squeezedcombobox.moc"
