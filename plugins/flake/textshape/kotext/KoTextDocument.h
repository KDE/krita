/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Pierre Stirnweiss <pierre.stirnweiss_calligra@gadz.org>
 * Copyright (C) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>
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
#include <QWeakPointer>
#include <QAbstractTextDocumentLayout>
#include <QUrl>

#include <styles/KoListStyle.h>

class KoList;
class KoStyleManager;
class KoInlineTextObjectManager;
class KoTextRangeManager;
class KUndo2Stack;
class KoTextEditor;
class KoOdfLineNumberingConfiguration;
class KoChangeTracker;
class KoShapeController;
class KoSectionModel;

class QTextCharFormat;

/**
 * KoTextDocument provides an easy mechanism to set and access the
 * editing members of a QTextDocument. The meta data are stored as resources
 * in the QTextDocument using QTextDocument::addResource() and fetched
 * using QTextDocument::resource().
 *
 */
class KRITATEXT_EXPORT KoTextDocument
{
public:
    /// Constructor
    KoTextDocument(QTextDocument *document); // krazy:exclude=explicit
    /// Constructor
    KoTextDocument(const QTextDocument *document); // krazy:exclude=explicit
    /// Constructor
    KoTextDocument(QWeakPointer<QTextDocument> document); // krazy:exclude=explicit

    /// Destructor
    ~KoTextDocument();

    /// Returns the document that was passed in the constructor
    QTextDocument *document() const;

    ///Returns the text editor for that document
    KoTextEditor *textEditor() const;

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

    void setLineNumberingConfiguration(KoOdfLineNumberingConfiguration *lineNumberingConfiguration);

    /// @return the notes configuration
    KoOdfLineNumberingConfiguration *lineNumberingConfiguration() const;

    ///Sets the global undo stack
    void setUndoStack(KUndo2Stack *undoStack);

    ///Returns the global undo stack
    KUndo2Stack *undoStack() const;

    ///Sets the global heading list
    void setHeadingList(KoList *list);

    ///Returns the global heading list
    KoList *headingList() const;

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

    /// Return the KoList that holds \a listId
    KoList *list(KoListStyle::ListIdType listId) const;

    /// Return the selections used during painting.
    QVector<QAbstractTextDocumentLayout::Selection> selections() const;

    /**
     * Set the selections to use for painting.
     *
     * The selections are used to apply temporary styling to
     * parts of a document.
     *
     * \param selections The new selections to use.
     */
    void setSelections(const QVector<QAbstractTextDocumentLayout::Selection> &selections);

    /// Returns the KoInlineTextObjectManager
    KoInlineTextObjectManager *inlineTextObjectManager() const;

    /// Set the KoInlineTextObjectManager
    void setInlineTextObjectManager(KoInlineTextObjectManager *manager);

    /// @return section model for the document
    KoSectionModel *sectionModel();

    /// Sets the section model for the document
    void setSectionModel(KoSectionModel *model);

    /// Returns the KoTextRangeManager
    KoTextRangeManager *textRangeManager() const;

    /// Set the KoTextRangeManager
    void setTextRangeManager(KoTextRangeManager *manager);

    /// Set the KoDocument's shapeController. This controller exists as long as KoDocument exists. It should only be used for deleting shapes.
    void setShapeController(KoShapeController *controller);

    /// Returns the shapeController
    KoShapeController *shapeController() const;

    QTextFrame* auxillaryFrame();

    /**
     * Specifies if tabs are relative to paragraph indent.
     *
     * By default it's false.
     */
    void setRelativeTabs(bool relative);

    /**
     * Returns if tabs are placed relative to paragraph indent.
     *
     * By default, this is false.
     *
     * @see setRelativeTabs
     */
    bool relativeTabs() const;

    void setParaTableSpacingAtStart(bool spacingAtStart);
    bool paraTableSpacingAtStart() const;

    /**
     * Returns the character format for the frame of this document.
     *
     * @return the character format for the frame of this document.
     * @see setFrameCharFormat
     */
    QTextCharFormat frameCharFormat() const;

    /**
     * Sets the character format for the frame of this document.
     *
     * @param format the character format for the frame of this document.
     * @see frameCharFormat
     */
    void setFrameCharFormat(const QTextCharFormat &format);

    /**
     * Returns the block format for the frame of this document.
     *
     * @return the block format for the frame of this document.
     * @see setFrameBlockFormat
     */
    QTextBlockFormat frameBlockFormat() const;

    /**
     * Sets the block format for the frame of this document.
     *
     * @param format the block format for the frame of this document.
     * @see frameBlockFormat
     */
    void setFrameBlockFormat(const QTextBlockFormat &format);

    /**
     * Clears the text in the document. Unlike QTextDocument::clear(), this
     * function does not clear the resources of the QTextDocument.
     */
    void clearText();

    /// Enum (type) used to add resources using QTextDocument::addResource()
    enum ResourceType {
        StyleManager = QTextDocument::UserResource,
        Lists,
        TextRangeManager,
        InlineTextManager,
        ChangeTrackerResource,
        UndoStack,
        TextEditor,
        LineNumberingConfiguration,
        RelativeTabs,
        HeadingList,
        Selections,
        LayoutTextPage, /// this is used for setting the correct page variable on the first resize and should not be used for other purposes
        ParaTableSpacingAtStart, /// this is used during layouting to specify if at the first paragraph margin-top should be applied.
        IndexGeneratorManager,
        FrameCharFormat,
        FrameBlockFormat,
        ShapeController,
        SectionModel
    };

    static const QUrl StyleManagerURL;
    static const QUrl ListsURL;
    static const QUrl TextRangeManagerURL;
    static const QUrl InlineObjectTextManagerURL;
    static const QUrl ChangeTrackerURL;
    static const QUrl UndoStackURL;
    static const QUrl TextEditorURL;
    static const QUrl LineNumberingConfigurationURL;
    static const QUrl BibliographyConfigurationURL;
    static const QUrl RelativeTabsURL;
    static const QUrl HeadingListURL;
    static const QUrl SelectionsURL;
    static const QUrl LayoutTextPageUrl;
    static const QUrl ParaTableSpacingAtStartUrl;
    static const QUrl IndexGeneratorManagerUrl;
    static const QUrl FrameCharFormatUrl;
    static const QUrl FrameBlockFormatUrl;
    static const QUrl ShapeControllerUrl;
    static const QUrl SectionModelUrl;

private:
    QTextDocument *m_document;
};

#endif // KOTEXTDOCUMENT_H
