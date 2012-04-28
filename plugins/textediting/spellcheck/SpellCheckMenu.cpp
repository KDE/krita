/* This file is part of the KDE project
 * Copyright (C) 2010 Christoph Goerlich <chgoerlich@gmx.de>
 * Copyright (C) 2012 Shreya Pandit <shreya@shreyapandit.com>
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

#include "SpellCheckMenu.h"
#include "SpellCheck.h"

#include <KDebug>
#include <KActionMenu>
#include <KMenu>
#include <KLocale>

#include <QSignalMapper>

SpellCheckMenu::SpellCheckMenu(const Sonnet::Speller &speller, SpellCheck *spellCheck)
    : QObject(spellCheck),
    m_spellCheck(spellCheck),
    m_speller(speller),
    m_suggestionsMenuAction(0),
    m_ignoreWordAction(0),
    m_addToDictionaryAction(0),
    m_suggestionsMenu(0),
    m_suggestionsSignalMapper(new QSignalMapper(this)),
    m_currentMisspelledPosition(-1)
{
    m_suggestionsMenuAction = new KActionMenu(i18n("Spelling"), this);
    m_suggestionsMenu = m_suggestionsMenuAction->menu();
    connect(m_suggestionsMenu, SIGNAL(aboutToShow()), this, SLOT(createSuggestionsMenu()));

    m_addToDictionaryAction = new KAction(i18n("Add to Dictionary"), this);
    connect(m_addToDictionaryAction, SIGNAL(triggered()), this, SLOT(addWordToDictionary()));

    // disabling this as if it calls the speller it's only changed in a local copy
    // see addWordToDictionary for how it should be done, except background checker
    // doesn't have suche a method for ignoreWord
    // Only option left is to personally ignore words

    // m_ignoreWordAction = new KAction(i18n("Ignore Word"), this);
    // connect(m_ignoreWordAction, SIGNAL(triggered()), this, SLOT(ignoreWord()));

    connect(m_suggestionsSignalMapper, SIGNAL(mapped(const QString&)), 
            this, SLOT(replaceWord(const QString&)));

    setEnabled(false);
    setVisible(false);
}

SpellCheckMenu::~SpellCheckMenu()
{

}

QPair<QString, KAction*> SpellCheckMenu::menuAction()
{
    return QPair<QString, KAction*>("spelling_suggestions", m_suggestionsMenuAction);
}

void SpellCheckMenu::createSuggestionsMenu()
{
    m_suggestions.clear();
    m_suggestionsMenu->clear();

    m_suggestionsMenu->addAction(m_ignoreWordAction);
    m_suggestionsMenu->addAction(m_addToDictionaryAction);
    m_suggestionsMenu->addSeparator();

    if (!m_currentMisspelled.isEmpty()) {
        m_suggestions = m_speller.suggest(m_currentMisspelled);
        for (int i = 0; i < m_suggestions.count(); ++i) {
            const QString &suggestion = m_suggestions.at(i);
            KAction *action = new KAction(suggestion, m_suggestionsMenu);
            connect(action, SIGNAL(triggered()), m_suggestionsSignalMapper, SLOT(map()));
            m_suggestionsSignalMapper->setMapping(action, suggestion);
            m_suggestionsMenu->addAction(action);
        }
    }
}

void SpellCheckMenu::ignoreWord()
{
    if (m_currentMisspelled.isEmpty() || m_currentMisspelledPosition < 0)
        return;

    m_speller.addToSession(m_currentMisspelled);

    emit clearHighlightingForWord(m_currentMisspelledPosition);

    m_currentMisspelled.clear();
    m_currentMisspelledPosition = -1;
}

void SpellCheckMenu::addWordToDictionary()
{
    if (m_currentMisspelled.isEmpty() || m_currentMisspelledPosition < 0)
        return;

    // see comment in ctor above why this will never work
    m_spellCheck->addWordToPersonal(m_currentMisspelled);

    emit clearHighlightingForWord(m_currentMisspelledPosition);

    m_currentMisspelled.clear();
    m_currentMisspelledPosition = -1;
}

void SpellCheckMenu::setMisspelled(const QString word, int position,int length)
{
    m_currentMisspelled = word;
    m_lengthMisspelled=length;
    m_currentMisspelledPosition = position;
}

void SpellCheckMenu::setEnabled(bool b)
{
    if (m_suggestionsMenuAction)
        m_suggestionsMenuAction->setEnabled(b);

    if (m_addToDictionaryAction)
        m_addToDictionaryAction->setEnabled(b);

    if (m_ignoreWordAction)
        m_ignoreWordAction->setEnabled(b);
}

void SpellCheckMenu::setVisible(bool b)
{
    if (m_suggestionsMenuAction)
        m_suggestionsMenuAction->setVisible(b);

    if (m_addToDictionaryAction)
        m_addToDictionaryAction->setVisible(b);

    if (m_ignoreWordAction)
        m_ignoreWordAction->setVisible(b);
}

void SpellCheckMenu::replaceWord(const QString &suggestion)
{
    if (suggestion.isEmpty() || m_currentMisspelledPosition < 0)
        return;

    m_spellCheck->replaceWordBySuggestion(suggestion, m_currentMisspelledPosition,m_lengthMisspelled);

    m_currentMisspelled.clear();
    m_currentMisspelledPosition = -1;
}

void SpellCheckMenu::setCurrentLanguage(const QString &language)
{
    m_speller.setLanguage(language);
}

#include <SpellCheckMenu.moc>
