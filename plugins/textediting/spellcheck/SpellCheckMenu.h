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

#ifndef SPELLCHECKMENU_H
#define SPELLCHECKMENU_H

#include <QObject>
#include <QPair>

#include <sonnet/speller.h>

class KActionMenu;
class KAction;
class KMenu;
class QSignalMapper;
class SpellCheck;

class SpellCheckMenu : public QObject
{
    Q_OBJECT
public:
    explicit SpellCheckMenu(const Sonnet::Speller &speller, SpellCheck *spellCheck);
    ~SpellCheckMenu();

    QPair<QString, KAction*> menuAction();
    void setMisspelled(const QString word, int position,int length);
    void setEnabled(bool b);
    void setVisible(bool b);
    void setCurrentLanguage(const QString &language);

signals:
    void clearHighlightingForWord(int startPosition);

private slots:
    void createSuggestionsMenu();
    void replaceWord(const QString &suggestion);
    void ignoreWord();
    void addWordToDictionary();

private:
    SpellCheck *m_spellCheck;
    Sonnet::Speller m_speller;
    KActionMenu *m_suggestionsMenuAction;
    KAction *m_ignoreWordAction;
    KAction *m_addToDictionaryAction;
    KMenu *m_suggestionsMenu;
    int m_lengthMisspelled;
    QSignalMapper *m_suggestionsSignalMapper;
    int m_currentMisspelledPosition;
    QString m_currentMisspelled;
    QStringList m_suggestions;
};

#endif // SPELLCHECKMENU_H
