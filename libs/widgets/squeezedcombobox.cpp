/* ============================================================
 * Author: Tom Albers <tomalbers@kde.nl>
 * Date  : 2005-01-01
 * Description :
 *
 * Copyright 2005 by Tom Albers <tomalbers@kde.nl>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)* any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "squeezedcombobox.h"
/** @file squeezedcombobox.cpp */

// Qt includes.

#include <QComboBox>
#include <QPair>
#include <QTimer>
#include <QStyle>
#include <QApplication>
#include <QResizeEvent>

SqueezedComboBox::SqueezedComboBox(QWidget *parent, const char *name)
        : QComboBox(parent)
{
    setObjectName(name);
    setMinimumWidth(100);
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()),
            SLOT(slotTimeOut()));
}

SqueezedComboBox::~SqueezedComboBox()
{
    delete m_timer;
}

bool SqueezedComboBox::contains(const QString& _text) const
{
    if (_text.isEmpty())
        return false;

    for (QMap<int, QString>::const_iterator it = m_originalItems.begin() ; it != m_originalItems.end();
            ++it) {
        if (it.value() == _text) {
            return true;
        }
    }
    return false;
}

qint32 SqueezedComboBox::findOriginalText(const QString& text) const
{
    for (int i = 0; i < m_originalItems.size(); i++) {
        if(m_originalItems.value(i) == text) {
            return i;
        }
    }
    return -1;
}

QStringList SqueezedComboBox::originalTexts() const
{
    return m_originalItems.values();
}

void SqueezedComboBox::resetOriginalTexts(const QStringList &texts)
{
    if (texts == m_originalItems.values()) return;

    clear();
    m_originalItems.clear();

    Q_FOREACH (const QString &item, texts) {
        addSqueezedItem(item);
    }
}

QSize SqueezedComboBox::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();

    int maxW = count() ? 18 : 7 * fm.width(QChar('x')) + 18;
    int maxH = qMax(fm.lineSpacing(), 14) + 2;

    QStyleOptionComboBox options;
    options.initFrom(this);

    return style()->sizeFromContents(QStyle::CT_ComboBox, &options,
                                     QSize(maxW, maxH), this).expandedTo(QApplication::globalStrut());
}

void SqueezedComboBox::insertSqueezedItem(const QString& newItem, int index, QVariant userData)
{
    m_originalItems[index] = newItem;
    QComboBox::insertItem(index, squeezeText(newItem), userData);
}

void SqueezedComboBox::insertSqueezedItem(const QIcon &icon, const QString &newItem, int index, QVariant userData)
{
    m_originalItems[index] = newItem;
    QComboBox::insertItem(index, icon, squeezeText(newItem), userData);
}

void SqueezedComboBox::addSqueezedItem(const QString& newItem, QVariant userData)
{
    insertSqueezedItem(newItem, count(), userData);
}

void SqueezedComboBox::addSqueezedItem(const QIcon &icon, const QString &newItem, QVariant userData)
{
    insertSqueezedItem(icon, newItem, count(), userData);
}

void SqueezedComboBox::setCurrent(const QString& itemText)
{
    qint32 itemIndex = findOriginalText(itemText);
    if (itemIndex >= 0) {
        setCurrentIndex(itemIndex);
    }
}

void SqueezedComboBox::resizeEvent(QResizeEvent *)
{
    m_timer->start(200);
}

void SqueezedComboBox::slotTimeOut()
{
    for (QMap<int, QString>::iterator it = m_originalItems.begin() ; it != m_originalItems.end();
            ++it) {
        setItemText(it.key(), squeezeText(it.value()));
    }
}

QString SqueezedComboBox::squeezeText(const QString& original)
{
    // not the complete widgetSize is usable. Need to compensate for that.
    int widgetSize = width() - 30;
    QFontMetrics fm(fontMetrics());

    // If we can fit the full text, return that.
    if (fm.width(original) < widgetSize)
        return(original);

    // We need to squeeze.
    QString sqItem = original; // prevent empty return value;
    widgetSize = widgetSize - fm.width("...");
    for (int i = 0 ; i != original.length(); ++i) {
        if ((int)fm.width(original.right(i)) > widgetSize) {
            sqItem = QString("..." + original.right(--i));
            break;
        }
    }
    return sqItem;
}

QString SqueezedComboBox::itemHighlighted()
{
    int curItem = currentIndex();
    return m_originalItems[curItem];
}

void SqueezedComboBox::removeSqueezedItem(int index)
{
    removeItem(index);
    m_originalItems.remove(index);
}

