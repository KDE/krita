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

#include "StylesWidget.h"

#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>

StyleManager::StyleManager(QWidget *parent)
        : QWidget(parent),
        m_styleManager(0),
        m_selectedParagStyle(0),
        m_selectedCharStyle(0),
        m_blockSignals(false)
{
    widget.setupUi(this);
    layout()->setMargin(0);

    connect(widget.styles, SIGNAL(paragraphStyleSelected(KoParagraphStyle *, bool)), this, SLOT(setParagraphStyle(KoParagraphStyle*,bool)));
    connect(widget.styles, SIGNAL(characterStyleSelected(KoCharacterStyle *, bool)), this, SLOT(setCharacterStyle(KoCharacterStyle*,bool)));

    connect(widget.bNew, SIGNAL(pressed()), this, SLOT(buttonNewPressed()));
    connect(widget.bDelete, SIGNAL(pressed()), this, SLOT(buttonDeletePressed()));

    connect(widget.createPage, SIGNAL(newParagraphStyle(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
    connect(widget.createPage, SIGNAL(newCharacterStyle(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
}

StyleManager::~StyleManager()
{
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

void StyleManager::setStyleManager(KoStyleManager *sm)
{
    Q_ASSERT(sm);
    m_styleManager = sm;
    widget.styles->setStyleManager(sm);
    widget.styles->setEmbedded(true);
    widget.stackedWidget->setCurrentWidget(widget.welcomePage);
    widget.paragraphStylePage->setParagraphStyles(sm->paragraphStyles());
    connect(sm, SIGNAL(styleAdded(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
    connect(sm, SIGNAL(styleAdded(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
    connect(sm, SIGNAL(styleRemoved(KoParagraphStyle*)), this, SLOT(removeParagraphStyle(KoParagraphStyle*)));
    connect(sm, SIGNAL(styleRemoved(KoCharacterStyle*)), this, SLOT(removeCharacterStyle(KoCharacterStyle*)));
}

void StyleManager::setParagraphStyle(KoParagraphStyle *style, bool canDelete)
{
    m_selectedCharStyle = 0;
    m_selectedParagStyle = style;
    widget.characterStylePage->save();
    widget.characterStylePage->setStyle(0);
    widget.paragraphStylePage->save();
    KoParagraphStyle * localStyle = style->clone();

    if (!m_alteredParagraphStyles.contains(style->styleId()))
        m_alteredParagraphStyles.insert(style->styleId(), localStyle);
    else
        localStyle = m_alteredParagraphStyles.value(style->styleId());

    widget.paragraphStylePage->setStyle(localStyle);
    widget.stackedWidget->setCurrentWidget(widget.paragraphStylePage);
    widget.bDelete->setEnabled(canDelete);
}

void StyleManager::setCharacterStyle(KoCharacterStyle *style, bool canDelete)
{
    m_selectedParagStyle = 0;
    m_selectedCharStyle = style;
    widget.paragraphStylePage->save();
    widget.paragraphStylePage->setStyle(0);
    widget.characterStylePage->save();

    KoCharacterStyle *localStyle = style->clone();

    if (!m_alteredCharacterStyles.contains(style->styleId()))
        m_alteredCharacterStyles.insert(style->styleId(), localStyle);
    else
        localStyle = m_alteredCharacterStyles.value(style->styleId());

    widget.characterStylePage->setStyle(localStyle);
    widget.stackedWidget->setCurrentWidget(widget.characterStylePage);
    widget.bDelete->setEnabled(canDelete);
}

void StyleManager::setUnit(const KoUnit &unit)
{
    widget.paragraphStylePage->setUnit(unit);
}

void StyleManager::save()
{
    m_blockSignals = true;
    widget.paragraphStylePage->save();
    widget.characterStylePage->save();

    foreach(int styleId, m_alteredCharacterStyles.keys()) {
        KoCharacterStyle *orig = m_styleManager->characterStyle(styleId);
        KoCharacterStyle *altered = m_alteredCharacterStyles[styleId];
        orig->copyProperties(altered);
        m_styleManager->alteredStyle(orig);
        delete altered;
    }
    m_alteredCharacterStyles.clear();

    foreach(int styleId, m_alteredParagraphStyles.keys()) {
        KoParagraphStyle *orig = m_styleManager->paragraphStyle(styleId);
        KoParagraphStyle *altered = m_alteredParagraphStyles[styleId];
        orig->copyProperties(altered);
        m_styleManager->alteredStyle(orig);
        delete altered;
    }
    m_alteredParagraphStyles.clear();
//Reset the active style
    if (m_selectedCharStyle) {
        KoCharacterStyle *localStyle = m_selectedCharStyle->clone();

        if (!m_alteredCharacterStyles.contains(m_selectedCharStyle->styleId()))
            m_alteredCharacterStyles.insert(m_selectedCharStyle->styleId(), localStyle);
        else
            localStyle = m_alteredCharacterStyles.value(m_selectedCharStyle->styleId());

        widget.characterStylePage->setStyle(localStyle);
    }
    else
        widget.characterStylePage->setStyle(0);

    if (m_selectedParagStyle) {
        KoParagraphStyle * localStyle = m_selectedParagStyle->clone();

        if (!m_alteredParagraphStyles.contains(m_selectedParagStyle->styleId()))
            m_alteredParagraphStyles.insert(m_selectedParagStyle->styleId(), localStyle);
        else
            localStyle = m_alteredParagraphStyles.value(m_selectedParagStyle->styleId());

        widget.paragraphStylePage->setStyle(localStyle);
    }
    else
        widget.paragraphStylePage->setStyle(0);
    m_blockSignals = false;
}

void StyleManager::buttonNewPressed()
{
    widget.stackedWidget->setCurrentWidget(widget.createPage);
    // that widget will emit a new style which we will add using addParagraphStyle or addCharacterStyle
}

void StyleManager::addParagraphStyle(KoParagraphStyle *style)
{
    widget.paragraphStylePage->save();
    widget.characterStylePage->save();
    widget.characterStylePage->setStyle(0);
    widget.paragraphStylePage->setStyle(0);

    if (m_blockSignals) return;

    KoCharacterStyle *cs = style->characterStyle();
    if (cs->name().isEmpty())
        cs->setName(style->name());
    addCharacterStyle(cs);

    m_styleManager->add(style);
    widget.paragraphStylePage->setParagraphStyles(m_styleManager->paragraphStyles());
    widget.stackedWidget->setCurrentWidget(widget.welcomePage);
}

void StyleManager::addCharacterStyle(KoCharacterStyle *style)
{
    if (m_blockSignals) return;

    m_styleManager->add(style);
    widget.stackedWidget->setCurrentWidget(widget.welcomePage);
}

void StyleManager::buttonDeletePressed()
{
    widget.styles->deleteStyleClicked();
}

void StyleManager::removeParagraphStyle(KoParagraphStyle* style)
{
    if (m_alteredParagraphStyles.contains(style->styleId()))
        m_alteredParagraphStyles.remove(style->styleId());
    widget.paragraphStylePage->setParagraphStyles(m_styleManager->paragraphStyles());
}

void StyleManager::removeCharacterStyle(KoCharacterStyle* style)
{
    if (m_alteredCharacterStyles.contains(style->styleId()))
        m_alteredCharacterStyles.remove(style->styleId());
}

/* TODO
    On new move focus to name text field.
    Add a connection to the same 'name' text field when I press enter it should press the create button.
    on 'new' use the currently selected style as a template
*/

#include <StyleManager.moc>
