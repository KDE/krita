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

#include <QComboBox>
#include <QPair>
#include <QTimer>
#include <QStyle>
#include <QApplication>
#include <QToolTip>
#include <QResizeEvent>

// Local includes.

#include "squeezedcombobox.h"

#warning kde4 port
// SqueezedComboBoxTip::SqueezedComboBoxTip( QWidget * parent, SqueezedComboBox* name )
//     : QToolTip( parent )
// {
//     m_originalWidget = name;
// }
//
// void SqueezedComboBoxTip::maybeTip( const QPoint &pos )
// {
//     Q3ListBox* listBox = m_originalWidget->listBox();
//     if (!listBox)
//         return;
//
//     Q3ListBoxItem* selectedItem = listBox->itemAt( pos );
//     if (selectedItem)
//     {
//         QRect positionToolTip = listBox->itemRect( selectedItem );
//         QString toolTipText = m_originalWidget->itemHighlighted();
//         if (!toolTipText.isNull())
//             tip(positionToolTip, toolTipText);
//     }
// }

SqueezedComboBox::SqueezedComboBox( QWidget *parent, const char *name )
    : QComboBox(parent)
{
    setObjectName(name);
    setMinimumWidth(100);
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    //m_tooltip = new SqueezedComboBoxTip( listBox()->viewport(), this ); XXX

    connect(m_timer, SIGNAL(timeout()),
            SLOT(slotTimeOut()));
    //connect(this, SIGNAL(activated( int )), XXX
    //        SLOT(slotUpdateToolTip( int )));
}

SqueezedComboBox::~SqueezedComboBox()
{
    //delete m_tooltip; XXX
    delete m_timer;
}

bool SqueezedComboBox::contains( const QString& _text ) const
{
    if ( _text.isEmpty() )
        return false;

    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i )
    {
        if ( itemText(i) == _text )
            return true;
    }
    return false;
}

QSize SqueezedComboBox::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();

    int maxW = count() ? 18 : 7 * fm.width(QChar('x')) + 18;
    int maxH = qMax( fm.lineSpacing(), 14 ) + 2;

    QStyleOptionComboBox options;
    options.initFrom(this);

    return style()->sizeFromContents(QStyle::CT_ComboBox, &options,
                                     QSize(maxW, maxH), this).expandedTo(QApplication::globalStrut());
}

void SqueezedComboBox::insertSqueezedItem(const QString& newItem, int index)
{
    m_originalItems[index] = newItem;
    QComboBox::insertItem(index, squeezeText(newItem));

    // if this is the first item, set the tooltip.
    if (index == 0)
        slotUpdateToolTip(0);
}

void SqueezedComboBox::addSqueezedItem(const QString& newItem)
{
    insertSqueezedItem(newItem, count());
}

void SqueezedComboBox::setCurrent(const QString& itemText)
{
    QString squeezedText = squeezeText(itemText);
    qint32 itemIndex = findText(squeezedText);
    if (itemIndex >= 0) {
        setCurrentIndex(itemIndex);
    }
}

void SqueezedComboBox::resizeEvent ( QResizeEvent * )
{
    m_timer->start(200);
}

void SqueezedComboBox::slotTimeOut()
{
    for (QMap<int, QString>::iterator it = m_originalItems.begin() ; it != m_originalItems.end();
         ++it)
    {
        setItemText( it.key(), squeezeText( it.value() ) );
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
    for (int i = 0 ; i != original.length(); ++i)
    {
        if ( (int)fm.width(original.right(i)) > widgetSize)
        {
            sqItem = QString("..." + original.right(--i));
            break;
        }
    }
    return sqItem;
}

void SqueezedComboBox::slotUpdateToolTip( int /*index*/ )
{
//     QToolTip::remove(this); XXX
//     this->setToolTip( m_originalItems[index]);
}

QString SqueezedComboBox::itemHighlighted()
{
    int curItem = currentIndex();
    return m_originalItems[curItem];
}

#include "squeezedcombobox.moc"
