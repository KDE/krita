/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Pierre Stirnweiss <pierre.stirnweiss_koffice@gadz.org>
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

#ifndef KOTEXTDOCUMENT_H
#define KOTEXTDOCUMENT_H

#include <QTextDocument>
#include <QUrl>

#include "KoList.h"

#include "changetracker/KoChangeTracker.h"

class KoStyleManager;
class KoInlineTextObjectManager;
class KUndoStack;
class KoTextEditor;

/**
 * KoTextDocument provides an easy mechanism to set and access the
 * editing members of a QTextDocument. The meta data are stored as resources
 * in the QTextDocument using QTextDocument::addResource() and fetched
 * using QTextDocument::resource().
 *
 */
class KOTEXT_EXPORT KoTextDocument
{
public:
    /// Constructor
    KoTextDocument(QTextDocument *document);
    /// Constructor
    KoTextDocument(const QTextDocument *document);

    /// Destructor
    ~KoTextDocument();

    /// Returns the document that was passed in the constructor
    QTextDocument *document() const;

    ///Returns the text editor for that document
    KoTextEditor *textEditor();

    ///Sets the text editor for the document
    void setTextEditor(KoTextEditor *textEditor);

    /// Sets the style manager that defines the named styles in the document
    void setStyleManager(KoStyleManager *styleManager);

    /// Returns the style manager
    KoStyleManager *styleManager() const;

    /// Sets the change tracker of the document
    void setChangeTracker(KoChangeTracker *changeTracker);

    ///Returns the change tracker of the document
    KoChangeTracker *changeTracker() const;

    ///Sets the global undo stack
    void setUndoStack(KUndoStack *undoStack);

    ///Returns the global undo stack
    KUndoStack *undoStack() const;

    /// Sets the lists of the document
    void setLists(const QList<KoList *> &lists);

    /// Returns the lists in the document
    QList<KoList *> lists() const;

    /// Adds a list to the document
    void addList(KoList *list);

    /// Removes a list from the document
    void removeList(KoList *list);

    /// Returns the KoList that holds \a block; 0 if block is not part of any list
    KoList *list(const QTextBlock &block) const;

    /// Returns the KoList that holds \a list
    KoList *list(QTextList *textList) const;

    /// Returns the KoInlineTextObjectManager
    KoInlineTextObjectManager *inlineTextObjectManager() const;

    /// Set the KoInlineTextObjectManager
    void setInlineTextObjectManager(KoInlineTextObjectManager *manager);

    /**
     * Clears the text in the document. Unlike QTextDocument::clear(), this
     * function does not clear the resources of the QTextDocument.
     */
    void clearText();

    /// Enum (type) used to add resources using QTextDocument::addResource()
    enum ResourceType {
        StyleManager = QTextDocument::UserResource,
        Lists,
        InlineTextManager,
        ChangeTrackerResource,
        UndoStack,
        TextEditor
    };
    static const QUrl StyleManagerURL;
    static const QUrl ListsURL;
    static const QUrl InlineObjectTextManagerURL;
    static const QUrl ChangeTrackerURL;
    static const QUrl UndoStackURL;
    static const QUrl TextEditorURL;

private:
    QTextDocument *m_document;
};

#endif // KOTEXTDOCUMENT_H
