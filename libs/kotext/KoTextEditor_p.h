/* This file is part of the KDE project
* Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
* Copyright (C) 2009 Thomas Zander <zander@kde.org>
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

#ifndef KOTEXTEDITOR_P_H
#define KOTEXTEDITOR_P_H

#include "KoTextEditor.h"

#include "KoTextDocument.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoStyleManager.h"
#include "changetracker/KoChangeTracker.h"

#include <KLocale>
#include <kdebug.h>

#include <QStack>
#include <QTextBlock>
#include <QTextDocument>
#include <QTimer>

class KUndo2Command;

class KoTextEditor::Private
{
public:
    enum State {
        NoOp,
        KeyPress,
        Delete,
        Format,
        Custom
    };

    explicit Private(KoTextEditor *qq, QTextDocument *document);

    ~Private() {}

    void documentCommandAdded();
    void updateState(State newState, const QString &title = QString());

    void newLine(KUndo2Command *parent);
    void clearCharFormatProperty(int propertyId);

    void emitTextFormatChanged();

    KoTextEditor *q;
    QTextCursor caret;
    QTextDocument *document;
    QStack<KUndo2Command*> commandStack;
    bool addNewCommand;
    bool dummyMacroAdded;
    int customCommandCount;
    QString commandTitle;

    State editorState;

    bool editProtected;
    bool editProtectionCached;
};

class KoTextVisitor
{
public:
    KoTextVisitor(KoTextEditor *editor)
        : m_abortVisiting(false)
        , m_editor(editor)
    {
    }

    virtual ~KoTextVisitor() {}
    // called whenever a visit was prevented by editprotection
    virtual void nonVisit() {}

    virtual void visitFragmentSelection(QTextCursor &)
    {
    }

    // The default implementation calls visitFragmentSelection on each fragment.intersect.selection
    virtual void visitBlock(QTextBlock &block, const QTextCursor &caret)
    {
        for (QTextBlock::iterator it = block.begin(); it != block.end(); ++it) {
            QTextCursor fragmentSelection(caret);
            fragmentSelection.setPosition(qMax(caret.selectionStart(), it.fragment().position()));
            fragmentSelection.setPosition(qMin(caret.selectionEnd(), it.fragment().position() + it.fragment().length()), QTextCursor::KeepAnchor);

            if (fragmentSelection.anchor() >= fragmentSelection.position()) {
                continue;
            }

            visitFragmentSelection(fragmentSelection);
        }
    }

    bool abortVisiting() { return m_abortVisiting;}
    void setAbortVisiting(bool abort) {m_abortVisiting = abort;}
    KoTextEditor * editor() {return m_editor;}
private:
    bool m_abortVisiting;
    KoTextEditor *m_editor;
};

class BlockFormatVisitor
{
public:
    BlockFormatVisitor() {}
    virtual ~BlockFormatVisitor() {}

    virtual void visit(QTextBlock &block) const = 0;

    static void visitSelection(KoTextEditor *editor, const BlockFormatVisitor &visitor, const QString &title = i18n("Format"), bool resetProperties = false, bool registerChange = true) {
        int start = qMin(editor->position(), editor->anchor());
        int end = qMax(editor->position(), editor->anchor());

        QTextBlock block = editor->block();
        if (block.position() > start)
            block = block.document()->findBlock(start);

        // now loop over all blocks that the selection contains and alter the text fragments where applicable.
        while (block.isValid() && block.position() <= end) {
            QTextBlockFormat prevFormat = block.blockFormat();
            if (resetProperties) {
                if (KoTextDocument(editor->document()).styleManager()) {
                    KoParagraphStyle *old = KoTextDocument(editor->document()).styleManager()->paragraphStyle(block.blockFormat().intProperty(KoParagraphStyle::StyleId));
                    if (old)
                        old->unapplyStyle(block);
                }
            }
            visitor.visit(block);
            QTextCursor cursor(block);
            QTextBlockFormat format = cursor.blockFormat();
            if (registerChange)
                editor->registerTrackedChange(cursor, KoGenChange::FormatChange, title, format, prevFormat, true);
            block = block.next();
        }
    }
};

class CharFormatVisitor
{
public:
    CharFormatVisitor() {}
    virtual ~CharFormatVisitor() {}

    virtual void visit(QTextCharFormat &format) const = 0;

    static void visitSelection(KoTextEditor *editor, const CharFormatVisitor &visitor, const QString &title = i18n("Format"), bool registerChange = true) {
        int start = qMin(editor->position(), editor->anchor());
        int end = qMax(editor->position(), editor->anchor());
        if (start == end) { // just set a new one.
            QTextCharFormat format = editor->charFormat();
            visitor.visit(format);

            if (registerChange && KoTextDocument(editor->document()).changeTracker() && KoTextDocument(editor->document()).changeTracker()->recordChanges()) {
                QTextCharFormat prevFormat(editor->charFormat());

                int changeId = KoTextDocument(editor->document()).changeTracker()->getFormatChangeId(title, format, prevFormat, editor->charFormat().property( KoCharacterStyle::ChangeTrackerId ).toInt());
                format.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
            }

            editor->cursor()->setCharFormat(format);
            return;
        }

        QTextBlock block = editor->block();
        if (block.position() > start)
            block = block.document()->findBlock(start);

        QList<QTextCursor> cursors;
        QList<QTextCharFormat> formats;
        // now loop over all blocks that the selection contains and alter the text fragments where applicable.
        while (block.isValid() && block.position() < end) {
            QTextBlock::iterator iter = block.begin();
            while (! iter.atEnd()) {
                QTextFragment fragment = iter.fragment();
                if (fragment.position() > end)
                    break;
                if (fragment.position() + fragment.length() <= start) {
                    ++iter;
                    continue;
                }

                QTextCursor cursor(block);
                cursor.setPosition(fragment.position() + 1);
                QTextCharFormat format = cursor.charFormat(); // this gets the format one char after the postion.
                visitor.visit(format);

                if (registerChange && KoTextDocument(editor->document()).changeTracker() && KoTextDocument(editor->document()).changeTracker()->recordChanges()) {
                    QTextCharFormat prevFormat(cursor.charFormat());

                    int changeId = KoTextDocument(editor->document()).changeTracker()->getFormatChangeId(title, format, prevFormat, cursor.charFormat().property( KoCharacterStyle::ChangeTrackerId ).toInt());
                    format.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
                }

                cursor.setPosition(qMax(start, fragment.position()));
                int to = qMin(end, fragment.position() + fragment.length());
                cursor.setPosition(to, QTextCursor::KeepAnchor);
                cursors.append(cursor);
                formats.append(format);

                QTextCharFormat prevFormat(cursor.charFormat());
                if (registerChange)
                    editor->registerTrackedChange(cursor,KoGenChange::FormatChange,title, format, prevFormat, false); //this will lead to every fragment having a different change untill the change merging in registerTrackedChange checks also for formatChange or not?

                ++iter;
            }
            block = block.next();
        }
        QList<QTextCharFormat>::Iterator iter = formats.begin();
        foreach(QTextCursor cursor, cursors) {
            cursor.setCharFormat(*iter);
            ++iter;
        }
    }
};

#endif //KOTEXTEDITOR_P_H
