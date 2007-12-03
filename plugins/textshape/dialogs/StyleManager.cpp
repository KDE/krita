/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#define PARAGRAPH_STYLE 1000
#define CHARACTER_STYLE 1001

#include "StyleManager.h"

#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>

StyleManager::StyleManager(QWidget *parent)
    :QWidget(parent),
    m_blockSignals(false)
{
    widget.setupUi(this);
    layout()->setMargin(0);

    connect (widget.styles, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
                this, SLOT(setStyle(QListWidgetItem*, QListWidgetItem*)));
    connect(widget.bNew, SIGNAL(pressed()), this, SLOT(buttonNewPressed()));
    connect(widget.bDelete, SIGNAL(pressed()), this, SLOT(buttonDeletePressed()));

    connect(widget.paragraphStylePage, SIGNAL(nameChanged(const QString&)), this, SLOT(setStyleName(const QString&)));
    connect(widget.characterStylePage, SIGNAL(nameChanged(const QString&)), this, SLOT(setStyleName(const QString&)));

    connect(widget.createPage, SIGNAL(newParagraphStyle(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
    connect(widget.createPage, SIGNAL(newCharacterStyle(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
}

StyleManager::~StyleManager() {
    foreach(int styleId, m_alteredParagraphStyles.keys()) {
        KoParagraphStyle *s = m_alteredParagraphStyles[styleId];
        delete s;
    }
    m_alteredParagraphStyles.clear();
    foreach(int styleId, m_alteredCharacterStyles.keys()) {
        KoCharacterStyle *s = m_alteredCharacterStyles[styleId];
        delete s;
    }
    m_alteredCharacterStyles.clear();
}

void StyleManager::setStyleManager(KoStyleManager *sm) {
    Q_ASSERT(sm);
    m_styleManager = sm;
    widget.styles->clear();
    bool defaultOne = true;
    foreach(KoParagraphStyle *style, m_styleManager->paragraphStyles()) {
        if(defaultOne) {
            defaultOne = false;
            continue;
        }
        QListWidgetItem *item = new QListWidgetItem(style->name(), widget.styles, PARAGRAPH_STYLE);
        item->setData(PARAGRAPH_STYLE, style->styleId());
        widget.styles->addItem(item);
        m_paragraphStyles.append(style);
    }
    QListWidgetItem *separator = new QListWidgetItem(widget.styles);
    separator->setBackgroundColor(QColor(Qt::black)); // TODO use theme
    separator->setSizeHint(QSize(20, 2));
    defaultOne = true;
    foreach(KoCharacterStyle *style, m_styleManager->characterStyles()) {
        if(defaultOne) {
            defaultOne = false;
            continue;
        }
        if(separator)
            widget.styles->addItem(separator);
        separator = 0;

        QListWidgetItem *item = new QListWidgetItem(style->name(), widget.styles, CHARACTER_STYLE);
        item->setData(CHARACTER_STYLE, style->styleId());
        widget.styles->addItem(item);
    }
    delete separator;

    widget.paragraphStylePage->setParagraphStyles(m_paragraphStyles);
    widget.styles->setCurrentRow(0);

    connect(sm, SIGNAL(styleAdded(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
    connect(sm, SIGNAL(styleAdded(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
    connect(sm, SIGNAL(styleRemoved(KoParagraphStyle*)), this, SLOT(removeParagraphStyle(KoParagraphStyle*)));
    connect(sm, SIGNAL(styleRemoved(KoCharacterStyle*)), this, SLOT(removeCharacterStyle(KoCharacterStyle*)));
}

void StyleManager::setStyle(QListWidgetItem *item, QListWidgetItem *previous) {
    int styleId = item->data(PARAGRAPH_STYLE).toInt();
    if(styleId > 0) {
        widget.characterStylePage->save();
        widget.characterStylePage->setStyle(0);
        widget.paragraphStylePage->save();
        KoParagraphStyle *style;
        if( m_alteredParagraphStyles.contains(styleId) )
            style = m_alteredParagraphStyles.value(styleId);
        else {
            style = m_styleManager->paragraphStyle(styleId);
            style = new KoParagraphStyle(*style);
            m_alteredParagraphStyles.insert(styleId, style);
        }
        widget.paragraphStylePage->setStyle(style);
        widget.stackedWidget->setCurrentWidget(widget.paragraphStylePage);
    }
    else {
        widget.paragraphStylePage->save();
        widget.paragraphStylePage->setStyle(0);
        widget.characterStylePage->save();
        styleId = item->data(CHARACTER_STYLE).toInt();
        if(styleId > 0) {
            KoCharacterStyle *style;
            if(m_alteredCharacterStyles.contains(styleId))
                style = m_alteredCharacterStyles.value(styleId);
            else {
                style = m_styleManager->characterStyle(styleId);
                style = new KoCharacterStyle(*style);
                m_alteredCharacterStyles.insert(styleId, style);
            }
            widget.characterStylePage->setStyle(style);
            widget.stackedWidget->setCurrentWidget(widget.characterStylePage);
        }
        else {
            // separator clicked.
            const int row = widget.styles->row(item);
            if(widget.styles->row(previous) == row + 1) // moving up.
                widget.styles->setCurrentRow(row -1);
            else if(widget.styles->row(previous) == row - 1) // moving down.
                widget.styles->setCurrentRow(row +1);
            else
                widget.styles->setCurrentItem(previous);
        }
    }
}

void StyleManager::setUnit(const KoUnit &unit) {
    widget.paragraphStylePage->setUnit(unit);
}

void StyleManager::save() {
    m_blockSignals = true;
    widget.paragraphStylePage->save();

    foreach(int styleId, m_alteredCharacterStyles.keys()) {
        KoCharacterStyle *orig = m_styleManager->characterStyle(styleId);
        KoCharacterStyle *altered = m_alteredCharacterStyles[styleId];
        if(orig == 0) { // new one
            QListWidgetItem *item;
            for(int i=0; i < widget.styles->count(); i++) {
                item = widget.styles->item(i);
                if(item->data(CHARACTER_STYLE).toInt() == altered->styleId())
                    break;
            }
            Q_ASSERT(item->data(CHARACTER_STYLE).toInt() == altered->styleId()); // assert that the style is in the list
            m_styleManager->add(altered);
            item->setData(CHARACTER_STYLE, altered->styleId());
        }
        else if(altered == 0) // deleted one.
            m_styleManager->remove(orig);
        else {
            orig->copyProperties(altered);
            m_styleManager->alteredStyle(orig);
            delete altered;
        }
    }
    m_alteredCharacterStyles.clear();

    foreach(int styleId, m_alteredParagraphStyles.keys()) {
        KoParagraphStyle *orig = m_styleManager->paragraphStyle(styleId);
        KoParagraphStyle *altered = m_alteredParagraphStyles[styleId];
        if(orig == 0) { // new one
            QListWidgetItem *item;
            for(int i=0; i < widget.styles->count(); i++) {
                item = widget.styles->item(i);
                if(item->data(PARAGRAPH_STYLE).toInt() == altered->styleId())
                    break;
            }
            Q_ASSERT(item->data(PARAGRAPH_STYLE).toInt() == altered->styleId()); // assert that the style is in the list
            m_styleManager->add(altered);
            item->setData(PARAGRAPH_STYLE, altered->styleId());
        }
        else if(altered == 0) // deleted one.
            m_styleManager->remove(orig);
        else {
            orig->copyProperties(altered);
            m_styleManager->alteredStyle(orig);
            delete altered;
        }
    }
    m_alteredParagraphStyles.clear();

    widget.characterStylePage->setStyle(0);
    widget.paragraphStylePage->setStyle(0);
    setStyle(widget.styles->currentItem(), 0);
    m_blockSignals = false;
}

void StyleManager::buttonNewPressed() {
    widget.stackedWidget->setCurrentWidget(widget.createPage);
    // that widget will emit a new style which we will add using addParagraphStyle or addCharacterStyle
}

void StyleManager::addParagraphStyle(KoParagraphStyle *style) {
    if(m_blockSignals) return;
    KoCharacterStyle *cs = style->characterStyle();
    if(cs->name().isEmpty())
        cs->setName(style->name());
    addCharacterStyle( cs );
    QListWidgetItem *item = new QListWidgetItem(style->name(), 0, PARAGRAPH_STYLE);
    int i=1000;
    while(m_alteredParagraphStyles.contains(i))
        i++;
    style->setStyleId(i);
    m_alteredParagraphStyles.insert(i, style);
    item->setData(PARAGRAPH_STYLE, i);
    widget.styles->insertItem(m_paragraphStyles.count(), item);
    m_paragraphStyles.append(style);
    widget.styles->setCurrentItem(item);
    widget.paragraphStylePage->switchToGeneralTab();
}

void StyleManager::addCharacterStyle(KoCharacterStyle *style) {
    if(m_blockSignals) return;
    QListWidgetItem *item = new QListWidgetItem(style->name(), 0, CHARACTER_STYLE);
    int i=1000;
    while(m_alteredCharacterStyles.contains(i))
        i++;
    style->setStyleId(i);
    m_alteredCharacterStyles.insert(i, style);
    item->setData(CHARACTER_STYLE, i);
    widget.styles->insertItem(widget.styles->count(), item);
    widget.styles->setCurrentItem(item);
    widget.characterStylePage->switchToGeneralTab();
}

void StyleManager::buttonDeletePressed() {
    // TODO
}

void StyleManager::setStyleName(const QString &name) {
    widget.styles->currentItem()->setText(name);
}

void StyleManager::removeParagraphStyle(KoParagraphStyle* style) {
    // TODO  signal incoming from style manager to remove this style
}

void StyleManager::removeCharacterStyle(KoCharacterStyle* style) {
    // TODO  signal incoming from style manager to remove this style
}

/* TODO
    On new move focus to name text field.
    Add a connection to the same 'name' text field when I press enter it should press the create button.
    on 'new' use the currently selected style as a template
*/

#include <StyleManager.moc>
