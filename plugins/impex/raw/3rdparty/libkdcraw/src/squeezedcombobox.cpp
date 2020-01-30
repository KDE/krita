/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2008-08-21
 * @brief  a combo box with a width not depending of text
 *         content size
 *
 * @author Copyright (C) 2006-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2008 by Andi Clemens
 *         <a href="mailto:andi dot clemens at googlemail dot com">andi dot clemens at googlemail dot com</a>
 * @author Copyright (C) 2005 by Tom Albers
 *         <a href="mailto:tomalbers at kde dot nl">tomalbers at kde dot nl</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "squeezedcombobox.h"

// Qt includes

#include <QComboBox>
#include <QTimer>
#include <QStyle>
#include <QApplication>
#include <QResizeEvent>

namespace KDcrawIface
{

class Q_DECL_HIDDEN SqueezedComboBox::Private
{
public:

    Private()
    {
        timer = 0;
    }

    QMap<int, QString> originalItems;

    QTimer*            timer;
};

SqueezedComboBox::SqueezedComboBox(QWidget* const parent, const char* name)
    : QComboBox(parent), d(new Private)
{
    setObjectName(name);
    setMinimumWidth(100);
    d->timer = new QTimer(this);
    d->timer->setSingleShot(true);

    connect(d->timer, &QTimer::timeout, this, &SqueezedComboBox::slotTimeOut);

    connect(this, static_cast<void (SqueezedComboBox::*)(int)>(&SqueezedComboBox::activated), this, &SqueezedComboBox::slotUpdateToolTip);
}

SqueezedComboBox::~SqueezedComboBox()
{
    d->originalItems.clear();
    delete d->timer;
    delete d;
}

bool SqueezedComboBox::contains(const QString& text) const
{
    if (text.isEmpty())
        return false;

    for (QMap<int, QString>::const_iterator it = d->originalItems.constBegin() ; it != d->originalItems.constEnd(); ++it)
    {
        if (it.value() == text)
            return true;
    }

    return false;
}

QSize SqueezedComboBox::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();
    int maxW        = count() ? 18 : 7 * fm.width(QChar('x')) + 18;
    int maxH        = qMax( fm.lineSpacing(), 14 ) + 2;

    QStyleOptionComboBox options;
    options.initFrom(this);

    return style()->sizeFromContents(QStyle::CT_ComboBox, &options, QSize(maxW, maxH), this);
}

void SqueezedComboBox::insertSqueezedItem(const QString& newItem, int index,
                                          const QVariant& userData)
{
    d->originalItems[index] = newItem;
    QComboBox::insertItem(index, squeezeText(newItem), userData);

    // if this is the first item, set the tooltip.
    if (index == 0)
        slotUpdateToolTip(0);
}

void SqueezedComboBox::insertSqueezedList(const QStringList& newItems, int index)
{
    for(QStringList::const_iterator it = newItems.constBegin() ; it != newItems.constEnd() ; ++it)
    {
        insertSqueezedItem(*it, index);
        index++;
    }
}

void SqueezedComboBox::addSqueezedItem(const QString& newItem,
                                       const QVariant& userData)
{
    insertSqueezedItem(newItem, count(), userData);
}

void SqueezedComboBox::setCurrent(const QString& itemText)
{
    QString squeezedText = squeezeText(itemText);
    qint32 itemIndex     = findText(squeezedText);

    if (itemIndex >= 0)
        setCurrentIndex(itemIndex);
}

void SqueezedComboBox::resizeEvent(QResizeEvent *)
{
    d->timer->start(200);
}

void SqueezedComboBox::slotTimeOut()
{
    for (QMap<int, QString>::iterator it = d->originalItems.begin() ; it != d->originalItems.end(); ++it)
    {
        setItemText( it.key(), squeezeText( it.value() ) );
    }
}

QString SqueezedComboBox::squeezeText(const QString& original) const
{
    // not the complete widgetSize is usable. Need to compensate for that.
    int widgetSize = width()-30;
    QFontMetrics fm( fontMetrics() );

    // If we can fit the full text, return that.
    if (fm.width(original) < widgetSize)
        return(original);

    // We need to squeeze.
    QString sqItem = original; // prevent empty return value;
    widgetSize     = widgetSize-fm.width("...");

    for (int i = 0 ; i != original.length(); ++i)
    {
        if ((int)fm.width(original.right(i)) > widgetSize)
        {
            sqItem = QString(original.left(i) + "...");
            break;
        }
    }

    return sqItem;
}

void SqueezedComboBox::slotUpdateToolTip(int index)
{
     setToolTip(d->originalItems[index]);
}

QString SqueezedComboBox::currentUnsqueezedText() const
{
    int curItem = currentIndex();
    return d->originalItems[curItem];
}

QString SqueezedComboBox::item(int index) const
{
    return d->originalItems[index];
}

}  // namespace KDcrawIface
