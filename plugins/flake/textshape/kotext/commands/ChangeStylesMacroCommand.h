/*
 *  Copyright (c) 2012 C. Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CHANGESTYLESMACROCOMMAND_H
#define CHANGESTYLESMACROCOMMAND_H

#include <kundo2command.h>

#include <QList>
#include <QSet>

class QTextDocument;
class KoCharacterStyle;
class KoParagraphStyle;
class KoStyleManager;

class ChangeStylesMacroCommand : public KUndo2Command
{
public:
    ChangeStylesMacroCommand(const QList<QTextDocument *> &documents, KoStyleManager *styleManager);

    virtual ~ChangeStylesMacroCommand();

    /// redo the command
    void redo();

    /// revert the actions done in redo
    void undo();

    void changedStyle(KoCharacterStyle *s) {m_changedCharacterStyles.append(s);}
    void origStyle(KoCharacterStyle *s) {m_origCharacterStyles.append(s);}
    void changedStyle(KoParagraphStyle *s) {m_changedParagraphStyles.append(s);}
    void origStyle(KoParagraphStyle *s) {m_origParagraphStyles.append(s);}
    void changedStyle(int id) {m_changedStyles.insert(id);}

private:
    QList<QTextDocument *> m_documents;
    QList<KoCharacterStyle *> m_origCharacterStyles;
    QList<KoCharacterStyle *> m_changedCharacterStyles;
    QList<KoParagraphStyle *> m_origParagraphStyles;
    QList<KoParagraphStyle *> m_changedParagraphStyles;
    QSet<int> m_changedStyles;
    KoStyleManager *m_styleManager;
    bool m_first;
};

#endif // CHANGESTYLESMACROCOMMAND_H
