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
#ifndef KOTEXTEDITINGPLUGIN_H
#define KOTEXTEDITINGPLUGIN_H

#include <QObject>
#include <QString>
#include <QHash>
#include "kotext_export.h"

class KAction;
class QTextDocument;
class QTextCursor;
class QUndoCommand;

/**
 * This is a base class for a text editing plugin as used by the text tool.
 * When the user types text into the text shape text editing plugins will be notified of
 * changes in the text document.  The plugin is meant to be for altering or checking text as the user
 * types it,
 * To ensure a good user experience this plugin will only be called when it makes sense from
 * a users perspective.
 * The finishedWord() method will be called when the user makes at least one change to
 * a word and then moves the cursor out of the word, a similar approach happens with the
 * finishedParagraph(), it will only be called after the cursor has been moved out of the paragraph.
 */
class KOTEXT_EXPORT KoTextEditingPlugin : public QObject
{
    Q_OBJECT
public:
    /// constructor
    KoTextEditingPlugin();
    virtual ~KoTextEditingPlugin();

    /**
     * This method will be called when the user makes at least one change to
     * a word and then moves the cursor out of the word.
     * You are free to alter the word via the textDocument.  Be aware that operations should be done
     * via a QTextCursor and should retain any formatting already present on the text.
     * @param document the text document that was altered.
     * @param cursorPosition the last altered position in the word.
     */
    virtual void finishedWord(QTextDocument *document, int cursorPosition) = 0;

    /**
     * This method will be called when the user makes at least one change to
     * a paragraph and then moves the cursor out of the paragraph.
     * You are free to alter the paragraph via the textDocument.  Be aware that operations should be done
     * via a QTextCursor and should retain any formatting already present on the text.
     * Note that finishedWord() is always called just prior to the call to this method.
     * @param document the text document that was altered.
     * @param cursorPosition the last altered position in the paragraph.
     */
    virtual void finishedParagraph(QTextDocument *document, int cursorPosition) = 0;

    /**
     * This method will be called when the user selects a portion of text and selects this plugin
     * to handle it.
     * You are free to alter the text via the textDocument.  Be aware that operations should be done
     * via a QTextCursor and should retain any formatting already present on the text.
     * @param document the text document that was altered.
     * @param startPosition the position at the start of the selection
     * @param endPosition the position at the end of the selection
     */
    virtual void checkSection(QTextDocument *document, int startPosition, int endPosition);

    /// can be called when this plugin needs the current position of the textcursor
    virtual void setCurrentCursorPosition(QTextDocument *document, int cursorPosition);

    /**
     * Retrieves the entire collection of actions for the plugin
     */
    QHash<QString, KAction*> actions() const;

signals:
    /// emitted when a series of commands is started that together need to become 1 undo action.
    void startMacro(const QString &name);
    /// emitted when a series of commands has ended that together should be 1 undo action.
    void stopMacro();

protected:
    /**
     * Helper method that allows you to easily get the word out of the document.
     * This method will create a selection on the parameter cursor where the altered word
     * is selected.
     * Example usage:
     * @code
        QTextCursor cursor(document);
        selectWord(cursor, cursorPosition);
        QString word = cursor.selectedText();
     * @endcode
     * @param cursor the cursor to alter.
     * @param cursorPosition the position of the cursor somewhere in the word.
     */
    void selectWord(QTextCursor &cursor, int cursorPosition) const;
    /**
     * Helper method that allows you to easily get the text of the paragraph which
     * holds the cursor position.
     * Please realize that altering of the paragraph text should be done on a
     * QTextCursor and not by altering the returned string.  Doing so would loose
     * all text formatting of the paragraph.
     * @param document the document.
     * @param cursorPosition the position of the cursor somewhere in the word.
     */
    QString paragraph(QTextDocument *document, int cursorPosition) const;

    /**
     * Add an action under the given name to the action collection.
     *
     * @param name The name by which the action be retrieved again from the collection.
     * @param action The action to add.
     */
    void addAction(const QString &name, KAction *action);

private:
    class Private;
    Private * const d;
};

#endif
