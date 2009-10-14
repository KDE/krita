/* This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
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

#include "KoTextEditor.h"

#include "KoBookmark.h"
#include "KoInlineTextObjectManager.h"
#include <KoOdf.h>
#include "KoTextDocument.h"
#include "KoTextDocumentLayout.h"
#include "KoTextDrag.h"
#include "KoTextLocator.h"
#include "KoTextOdfSaveHelper.h"
#include "KoTextPaste.h"
#include "KoUndoStack.h"
#include "changetracker/KoChangeTracker.h"
#include "changetracker/KoChangeTrackerElement.h"
#include "changetracker/KoDeleteChangeMarker.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoStyleManager.h"
#include "styles/KoTableCellStyle.h"

#include <KLocale>

#include <QApplication>
#include <QFontDatabase>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextFormat>
#include <QTextTable>
#include <QTimer>
#include <QString>
#include <QUndoCommand>

#include <kdebug.h>

static bool isRightToLeft(const QString &text)
{
    int ltr = 0, rtl = 0;

    QString::const_iterator iter = text.begin();
    while (iter != text.end()) {
        switch (QChar::direction((*iter).unicode())) {
        case QChar::DirL:
        case QChar::DirLRO:
        case QChar::DirLRE:
            ltr++;
            break;
        case QChar::DirR:
        case QChar::DirAL:
        case QChar::DirRLO:
        case QChar::DirRLE:
            rtl++;
        default:
            break;
        }
        ++iter;
    }
    return ltr < rtl;
}

/*Private*/

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
    void updateState(State newState, QString title = QString());

    bool deleteInlineObjects(bool backwards = false);
    void deleteSelection();
    void runDirectionUpdater();

    KoTextEditor *q;
    QTextCursor caret;
    QTextDocument *document;
    QUndoCommand *headCommand;
    QString commandTitle;
    KoText::Direction direction;
    bool isBidiDocument;

    State editorState;

    QTimer updateRtlTimer;
    QList<int> dirtyBlocks;
};

KoTextEditor::Private::Private(KoTextEditor *qq, QTextDocument *document)
    : q(qq),
    document (document),
    headCommand(0),
    isBidiDocument(false)
{
    caret = QTextCursor(document);
    editorState = NoOp;
    updateRtlTimer.setSingleShot(true);
    updateRtlTimer.setInterval(250);
    QObject::connect(&updateRtlTimer, SIGNAL(timeout()), q, SLOT(runDirectionUpdater()));
}

void KoTextEditor::Private::documentCommandAdded()
{
    class UndoTextCommand : public QUndoCommand
    {
    public:
        UndoTextCommand(QTextDocument *document, QUndoCommand *parent = 0)
        : QUndoCommand(i18n("Text"), parent),
        m_document(document)
        {}

        void undo() {
            if (m_document.isNull())
                return;
            m_document->undo(KoTextDocument(m_document).textEditor()->cursor());
        }

        void redo() {
            if (m_document.isNull())
                return;
            m_document->redo(KoTextDocument(m_document).textEditor()->cursor());
        }

        QPointer<QTextDocument> m_document;
    };
//kDebug() << "editor state: " << editorState << " headcommand: " << headCommand;
    if (!headCommand || editorState == NoOp) {
        headCommand = new QUndoCommand(commandTitle);
        if (KoTextDocument(document).undoStack()) {
            //kDebug() << "pushing head: " << headCommand->text();
            KoTextDocument(document).undoStack()->push(headCommand);
        }
    }
    else if ((editorState == KeyPress || editorState == Delete) && headCommand->childCount()) {
        headCommand = new QUndoCommand(commandTitle);
        if (KoTextDocument(document).undoStack()) {
            //kDebug() << "pushing head: " << headCommand->text();
            KoTextDocument(document).undoStack()->push(headCommand);
        }
    }

    new UndoTextCommand(document, headCommand);
}

void KoTextEditor::Private::updateState(KoTextEditor::Private::State newState, QString title)
{
    if (editorState == Custom && newState !=NoOp)
        return;
    //kDebug() << "updateState from: " << editorState << " to: " << newState;
    if (editorState != newState || commandTitle != title) {
        if (headCommand /*&& headCommand->childCount() && KoTextDocument(document).undoStack()*/) {
            //kDebug() << "reset headCommand";
            //            KoTextDocument(document).undoStack()->push(headCommand);
            headCommand = 0;
        }
    }
    editorState = newState;
    if (!title.isEmpty())
        commandTitle = title;
    else
        commandTitle = i18n("Text");
    //kDebug() << "commandTitle is now: " << commandTitle;
}

bool KoTextEditor::Private::deleteInlineObjects(bool backwards)
{
    Q_UNUSED(backwards)
    return false;
    // TODO don't just blindly delete, make this a command so we can undo it later.
    // Also note that the below code needs unit testing since I found some issues already
    /*
    QTextCursor cursor(*d->caret);
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(d->textShapeData->document()->documentLayout());
    Q_ASSERT(layout);
    KoInlineTextObjectManager *manager = layout->inlineObjectTextManager();
    KoInlineObject *object;
    bool found = false;

    if (d->caret->hasSelection()) {
   QString selected = cursor.selectedText();
   cursor.setPosition(cursor.selectionStart() + 1);
   int position = cursor.position();
   const QChar *data = selected.constData();
   for (int i = 0; i < selected.length(); i++) {
   if (data->unicode() == QChar::ObjectReplacementCharacter) {
   found = true;
   cursor.setPosition(position);
   object = manager->inlineTextObject(cursor);

   if (object)
   manager->removeInlineObject(cursor);
}
// if there is an inline object, the InlineTextObjectManager will also delete the char
// so only need to update position if inline object not found
else
    position++;
data++;
}
} else {
    if (!backward)
    cursor.movePosition(QTextCursor::Right);
    object = manager->inlineTextObject(cursor);

    if (object) {
   manager->removeInlineObject(cursor);
   found = true;
}
}
return found;
*/
}

void KoTextEditor::Private::deleteSelection()
{//TODO
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());

    QTextCursor delText = QTextCursor(caret);
    if (!delText.hasSelection())
        delText.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    QString text = delText.selectedText();
    //kDebug() << "delText: " << text;
    QTextDocumentFragment selection = delText.selection();
    caret.deleteChar();
    //kDebug() << "inlineObject property: " << caret.charFormat().intProperty(KoCharacterStyle::InlineInstanceId);
    if (KoTextDocument(document).changeTracker() && KoTextDocument(document).changeTracker()->isEnabled()) {
        //            KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
        //            Q_ASSERT(layout);
        //            Q_ASSERT(layout->inlineTextObjectManager());
        //            if (!m_changeRegistered) {
        int changeId = KoTextDocument(document).changeTracker()->getDeleteChangeId(i18n("Delete"), selection, caret.charFormat().property( KoCharacterStyle::ChangeTrackerId ).toInt());
        //                format.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
        //            }
       KoDeleteChangeMarker *deleteMarker = new KoDeleteChangeMarker;
       deleteMarker->setChangeId(changeId);
        //            deleteMarker->setId(format.property(KoCharacterStyle::ChangeTrackerId).toInt());
        //            deleteMarker->setText(text);
        layout->inlineTextObjectManager()->insertInlineObject(caret, deleteMarker);
        //            m_caret.setCharFormat(oldFormat);
    }
}

void KoTextEditor::Private::runDirectionUpdater()
{
    while (! dirtyBlocks.isEmpty()) {
        const int blockNumber = dirtyBlocks.first();
        dirtyBlocks.removeAll(blockNumber);
        QTextBlock block = document->findBlockByNumber(blockNumber);
        if (block.isValid()) {
            KoText::Direction newDirection = KoText::AutoDirection;
            QTextBlockFormat format = block.blockFormat();
            KoText::Direction dir =
                static_cast<KoText::Direction>(format.intProperty(KoParagraphStyle::TextProgressionDirection));

            if (dir == KoText::AutoDirection || dir == KoText::PerhapsLeftRightTopBottom ||
                    dir == KoText::PerhapsRightLeftTopBottom) {
                bool rtl = isRightToLeft(block.text());
                if (rtl && (dir != KoText::AutoDirection || QApplication::isLeftToRight()))
                    newDirection = KoText::PerhapsRightLeftTopBottom;
                else if (!rtl && (dir != KoText::AutoDirection || QApplication::isRightToLeft())) // remove previously set one if needed.
                    newDirection = KoText::PerhapsLeftRightTopBottom;

                QTextCursor cursor(block);
                if (format.property(KoParagraphStyle::TextProgressionDirection).toInt() != newDirection) {
                    format.setProperty(KoParagraphStyle::TextProgressionDirection, newDirection);
                    cursor.setBlockFormat(format); // note that setting this causes a re-layout.
                }
                if (!isBidiDocument) {
                    if ((QApplication::isLeftToRight() && (newDirection == KoText::RightLeftTopBottom
                                    || newDirection == KoText::PerhapsRightLeftTopBottom))
                            || (QApplication::isRightToLeft() && (newDirection == KoText::LeftRightTopBottom
                                    || newDirection == KoText::PerhapsLeftRightTopBottom))) {
                        isBidiDocument = true;
                        emit q->isBidiUpdated();
                    }
                }
            }
        }
    }
}

/*KoTextEditor*/

//TODO factor out the changeTracking charFormat setting from all individual slots to a public slot, which will be available for external commands (TextShape)

//The BlockFormatVisitor and CharFormatVisitor are used when a property needs to be modified relative to its current value (which could be different over the selection). For example: increase indentation by 10pt.
//The BlockFormatVisitor is also used for the change tracking of a blockFormat. The changeTracker stores the information about the changeId in the charFormat. The BlockFormatVisitor ensures that thd changeId is set on the whole block (even if only a part of the block is actually selected).
//Should such mechanisms be later provided directly by Qt, we could dispose of these classes.

class BlockFormatVisitor
{
public:
    BlockFormatVisitor() {}
    virtual ~BlockFormatVisitor() {}

    virtual void visit(QTextBlockFormat &format) const = 0;

    static void visitSelection(KoTextEditor *editor, const BlockFormatVisitor &visitor, QString title = i18n("Format"), bool resetProperties = false) {
        int start = qMin(editor->position(), editor->anchor());
        int end = qMax(editor->position(), editor->anchor());

        QTextBlock block = editor->block();
        if (block.position() > start)
            block = block.document()->findBlock(start);

        // now loop over all blocks that the selection contains and alter the text fragments where applicable.
        while (block.isValid() && block.position() <= end) {
            QTextBlockFormat format = block.blockFormat();
            if (resetProperties) {
                if (KoTextDocument(editor->document()).styleManager()) {
                    KoParagraphStyle *old = KoTextDocument(editor->document()).styleManager()->paragraphStyle(block.blockFormat().intProperty(KoParagraphStyle::StyleId));
                    if (old)
                        old->unapplyStyle(block);
                }
            }
            visitor.visit(format);
            QTextCursor cursor(block);
            QTextBlockFormat prevFormat = cursor.blockFormat();
            editor->registerTrackedChange(cursor, KoGenChange::formatChange, title, format, prevFormat, true);
            cursor.setBlockFormat(format);
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

    static void visitSelection(KoTextEditor *editor, const CharFormatVisitor &visitor, QString title = i18n("Format")) {
        int start = qMin(editor->position(), editor->anchor());
        int end = qMax(editor->position(), editor->anchor());
        if (start == end) { // just set a new one.
            QTextCharFormat format = editor->charFormat();
            visitor.visit(format);

            if (KoTextDocument(editor->document()).changeTracker() && KoTextDocument(editor->document()).changeTracker()->isEnabled()) {
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
                    iter++;
                    continue;
                }

                QTextCursor cursor(block);
                cursor.setPosition(fragment.position() + 1);
                QTextCharFormat format = cursor.charFormat(); // this gets the format one char after the postion.
                visitor.visit(format);

                if (KoTextDocument(editor->document()).changeTracker() && KoTextDocument(editor->document()).changeTracker()->isEnabled()) {
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
                editor->registerTrackedChange(cursor,KoGenChange::formatChange,title, format, prevFormat, false); //this will lead to every fragment having a different change untill the change merging in registerTrackedChange checks also for formatChange or not?

                iter++;
            }
            block = block.next();
        }
        QList<QTextCharFormat>::Iterator iter = formats.begin();
        foreach(QTextCursor cursor, cursors) {
            cursor.mergeCharFormat(*iter);
            ++iter;
        }
    }
};

KoTextEditor::KoTextEditor(QTextDocument *document)
    : KoToolSelection(document),
    d (new Private(this, document))
{
    connect (d->document, SIGNAL (undoCommandAdded()), this, SLOT (documentCommandAdded()));
}

KoTextEditor::~KoTextEditor()
{
    delete d;
}

void KoTextEditor::updateDefaultTextDirection(KoText::Direction direction)
{
    d->direction = direction;
}

QTextCursor* KoTextEditor::cursor()
{
    return &(d->caret);
}

void KoTextEditor::addCommand(QUndoCommand *command)
{
    d->updateState(KoTextEditor::Private::Custom, (!command->text().isEmpty())?command->text():i18n("Text"));
    //kDebug() << "will push the custom command: " << command->text();
    d->headCommand = command;
    KoTextDocument(d->document).undoStack()->push(command);
    //kDebug() << "custom command pushed";
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::registerTrackedChange(QTextCursor &selection, KoGenChange::Type changeType, QString title, QTextFormat &format, QTextFormat &prevFormat, bool applyToWholeBlock)
{
//    kDebug(31000) << "in registerTrackedChange; enabled tracking: " << (KoTextDocument(d->document).changeTracker() || KoTextDocument(d->document).changeTracker()->isEnabled());
    if (!KoTextDocument(d->document).changeTracker() || !KoTextDocument(d->document).changeTracker()->isEnabled()) {
        QTextCharFormat charFormat = selection.charFormat();
        charFormat.clearProperty(KoCharacterStyle::ChangeTrackerId);
        selection.setCharFormat(charFormat);
        return;
    }

    //first check if there already is an identical change registered just before or just after the selection. If so, merge appropriatly.
    //TODO implement for format change. handle the prevFormat/newFormat check.
    QTextCursor checker = QTextCursor(selection);
    int idBefore = 0;
    int idAfter = 0;
    int changeId = 0;
    int selectionBegin = qMin(checker.anchor(), checker.position());
    int selectionEnd = qMax(checker.anchor(), checker.position());
    checker.setPosition(selectionBegin);
    idBefore = KoTextDocument(d->document).changeTracker()->mergeableId(changeType, title, checker.charFormat().property( KoCharacterStyle::ChangeTrackerId ).toInt());
    checker.setPosition(selectionEnd);
    if (!checker.atEnd()) {
        checker.movePosition(QTextCursor::NextCharacter);
        idAfter = KoTextDocument(d->document).changeTracker()->mergeableId(changeType, title, checker.charFormat().property( KoCharacterStyle::ChangeTrackerId ).toInt());
    }
    changeId = (idBefore)?idBefore:idAfter;

    switch (changeType) {
        case KoGenChange::insertChange:
            if (!changeId)
                changeId = KoTextDocument(d->document).changeTracker()->getInsertChangeId(title, selection.charFormat().property( KoCharacterStyle::ChangeTrackerId ).toInt());
            break;
        case KoGenChange::formatChange:
            if (!changeId)
                changeId = KoTextDocument(d->document).changeTracker()->getFormatChangeId(title, format, prevFormat, d->caret.charFormat().property( KoCharacterStyle::ChangeTrackerId ).toInt());
            break;
        case KoGenChange::deleteChange:
            //if (!changeId)
            break;
    }

    if (applyToWholeBlock) {
        selection.movePosition(QTextCursor::StartOfBlock);
        selection.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    }

    QTextCharFormat f;
    f.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
    selection.mergeCharFormat(f);
/* // old code...
   if (KoTextDocument(m_textShapeData->document()).changeTracker() && KoTextDocument(m_textShapeData->document()).changeTracker()->isEnabled()) {
        if ( !m_changeRegistered ) {
            int changeId = KoTextDocument(m_textShapeData->document()).changeTracker()->getInsertChangeId(i18n("Key Press"), m_caret.charFormat().property( KoCharacterStyle::ChangeTrackerId ).toInt());
            QTextCharFormat format;
            format.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
            m_caret.mergeCharFormat(format);
        }
    }*/
}

void KoTextEditor::bold(bool bold)
{
    d->updateState(KoTextEditor::Private::Format, i18n("Bold"));
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);

    QTextCharFormat prevFormat(d->caret.charFormat());
    registerTrackedChange(d->caret, KoGenChange::formatChange, i18n("Bold"), format, prevFormat, false);
    d->caret.mergeCharFormat(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::italic(bool italic)
{
    d->updateState(KoTextEditor::Private::Format, i18n("Italic"));
    QTextCharFormat format;
    format.setFontItalic(italic);

    QTextCharFormat prevFormat(d->caret.charFormat());
    registerTrackedChange(d->caret, KoGenChange::formatChange, i18n("Italic"), format, prevFormat, false);
    d->caret.mergeCharFormat(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::underline(bool underline)
{
    d->updateState(KoTextEditor::Private::Format, i18n("Underline"));
    QTextCharFormat format;
    if (underline) {
        format.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::SingleLine);
        format.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::SolidLine);
    } else {
        format.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::NoLineType);
        format.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::NoLineStyle);
    }

    QTextCharFormat prevFormat(d->caret.charFormat());
    registerTrackedChange(d->caret, KoGenChange::formatChange, i18n("Underline"), format, prevFormat, false);
    d->caret.mergeCharFormat(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::strikeOut(bool strikeout)
{
    d->updateState(KoTextEditor::Private::Format, i18n("Strike Out"));
    QTextCharFormat format;
    if (strikeout) {
        format.setProperty(KoCharacterStyle::StrikeOutType, KoCharacterStyle::SingleLine);
        format.setProperty(KoCharacterStyle::StrikeOutStyle, KoCharacterStyle::SolidLine);
    } else {
        format.setProperty(KoCharacterStyle::StrikeOutType, KoCharacterStyle::NoLineType);
        format.setProperty(KoCharacterStyle::StrikeOutStyle, KoCharacterStyle::NoLineStyle);
    }
    QTextCharFormat prevFormat(d->caret.charFormat());
    registerTrackedChange(d->caret, KoGenChange::formatChange, i18n("Strike Out"), format, prevFormat, false);
    d->caret.mergeCharFormat(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setHorizontalTextAlignment(Qt::Alignment align)
{
    class Aligner : public BlockFormatVisitor
    {
    public:
        Aligner(Qt::Alignment align) : alignment(align) {}
        void visit(QTextBlockFormat &format) const {
            format.setAlignment(alignment);
        }
        Qt::Alignment alignment;
    };

    Aligner aligner(align);
    d->updateState(KoTextEditor::Private::Format, i18n("Set Horizontal Alignment"));
    BlockFormatVisitor::visitSelection(this, aligner, i18n("Set Horizontal Alignment"));
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setVerticalTextAlignment(Qt::Alignment align)
{
    QTextCharFormat::VerticalAlignment charAlign = QTextCharFormat::AlignNormal;
    if (align == Qt::AlignTop)
        charAlign = QTextCharFormat::AlignSuperScript;
    else if (align == Qt::AlignBottom)
        charAlign = QTextCharFormat::AlignSubScript;

    d->updateState(KoTextEditor::Private::Format, i18n("Set Vertical Alignment"));
    QTextCharFormat format;
    format.setVerticalAlignment(charAlign);
    QTextCharFormat prevFormat(d->caret.charFormat());
    registerTrackedChange(d->caret, KoGenChange::formatChange, i18n("Set Vertical Alignment"), format, prevFormat, false);
    d->caret.mergeCharFormat(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::decreaseIndent()
{
    class Indenter : public BlockFormatVisitor
    {
    public:
        void visit(QTextBlockFormat &format) const {
            // TODO make the 10 configurable.
            format.setLeftMargin(qMax(qreal(0.0), format.leftMargin() - 10));
        }
        Qt::Alignment alignment;
    };

    Indenter indenter;
    d->updateState(KoTextEditor::Private::Format, i18n("Decrease Indent"));
    BlockFormatVisitor::visitSelection(this, indenter, i18n("Decrease Indent"));
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::increaseIndent()
{
    class Indenter : public BlockFormatVisitor
    {
    public:
        void visit(QTextBlockFormat &format) const {
            // TODO make the 10 configurable.
            format.setLeftMargin(format.leftMargin() + 10);
        }
        Qt::Alignment alignment;
    };

    Indenter indenter;
    d->updateState(KoTextEditor::Private::Format, i18n("Increase Indent"));
    BlockFormatVisitor::visitSelection(this, indenter, i18n("Increase Indent"));
    d->updateState(KoTextEditor::Private::NoOp);
}

class FontResizer : public CharFormatVisitor
{
public:
    enum Type { Grow, Shrink };
    FontResizer(Type type_) : type(type_) {
        QFontDatabase fontDB;
        defaultSizes = fontDB.standardSizes();
    }
    void visit(QTextCharFormat &format) const {
        const qreal current = format.fontPointSize();
        int prev = 1;
        foreach(int pt, defaultSizes) {
            if ((type == Grow && pt > current) || (type == Shrink && pt >= current)) {
                format.setFontPointSize(type == Grow ? pt : prev);
                return;
            }
            prev = pt;
        }
    }

    QList<int> defaultSizes;
    const Type type;
};

void KoTextEditor::decreaseFontSize()
{
    d->updateState(KoTextEditor::Private::Format, i18n("Decrease font size"));
    FontResizer sizer(FontResizer::Shrink);
    CharFormatVisitor::visitSelection(this, sizer, i18n("Decrease font size"));
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::increaseFontSize()
{
    d->updateState(KoTextEditor::Private::Format, i18n("Increase font size"));
    FontResizer sizer(FontResizer::Grow);
    CharFormatVisitor::visitSelection(this, sizer, i18n("Increase font size"));
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setFontFamily(const QString &font)
{
    d->updateState(KoTextEditor::Private::Format, i18n("Set Font"));
    QTextCharFormat format;
    format.setFontFamily(font);
    QTextCharFormat prevFormat(d->caret.charFormat());
    registerTrackedChange(d->caret, KoGenChange::formatChange, i18n("Set Font"), format, prevFormat, false);
    d->caret.mergeCharFormat(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setFontSize(int size)
{
    d->updateState(KoTextEditor::Private::Format, i18n("Set Font Size"));
    QTextCharFormat format;
    format.setFontPointSize(size);
    QTextCharFormat prevFormat(d->caret.charFormat());
    registerTrackedChange(d->caret, KoGenChange::formatChange, i18n("Set Font Size"), format, prevFormat, false);
    d->caret.mergeCharFormat(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setTextBackgroundColor(const QColor &color)
{
    d->updateState(KoTextEditor::Private::Format, i18n("Set Background Color"));
    QTextCharFormat format;
    format.setBackground(QBrush(color));
    QTextCharFormat prevFormat(d->caret.charFormat());
    registerTrackedChange(d->caret, KoGenChange::formatChange, i18n("Set Background Color"), format, prevFormat, false);
    d->caret.mergeCharFormat(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setTextColor(const QColor &color)
{
    d->updateState(KoTextEditor::Private::Format, i18n("Set Text Color"));
    QTextCharFormat format;
    format.setForeground(QBrush(color));
    QTextCharFormat prevFormat(d->caret.charFormat());
    registerTrackedChange(d->caret, KoGenChange::formatChange, i18n("Set Text Color"), format, prevFormat, false);
    d->caret.mergeCharFormat(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setStyle(KoCharacterStyle *style)
{
    Q_ASSERT(style);
    d->updateState(KoTextEditor::Private::Format, i18n("Set Character Style"));
    QTextCharFormat format;
    style->applyStyle(format);
    QTextCharFormat prevFormat(d->caret.charFormat());
    registerTrackedChange(d->caret, KoGenChange::formatChange, i18n("Set Character Style"), format, prevFormat, false);
    d->caret.setCharFormat(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setStyle(KoParagraphStyle *style)
{
    d->updateState(KoTextEditor::Private::Format, i18n("Set Paragraph Style"));
    const int start = qMin(position(), anchor());
    const int end = qMax(position(), anchor());
    QTextBlock block = d->document->findBlock(start);
    KoStyleManager *styleManager = KoTextDocument(d->document).styleManager();
    while (block.isValid() && block.position() <= end) { // now loop over all blocks
        QTextBlockFormat bf = block.blockFormat();
        if (styleManager) {
            KoParagraphStyle *old = styleManager->paragraphStyle(bf.intProperty(KoParagraphStyle::StyleId));
            if (old)
                old->unapplyStyle(block);
        }
        style->applyStyle(block);
        block = block.next();
    }
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setDefaultFormat()
{
    d->updateState(KoTextEditor::Private::Format, i18n("Set default format"));
    if (KoStyleManager *styleManager = KoTextDocument(d->document).styleManager()) {
        KoCharacterStyle *defaultCharStyle = styleManager->defaultParagraphStyle()->characterStyle();
        QTextCharFormat format;
        defaultCharStyle->applyStyle(format);
        QTextCharFormat prevFormat(d->caret.charFormat());
        registerTrackedChange(d->caret, KoGenChange::formatChange, i18n("Set default format"), format, prevFormat, false);
        d->caret.setCharFormat(format);
    }
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::addBookmark(const QString &name)
{//TODO changeTracking
    d->updateState(KoTextEditor::Private::Custom, i18n("Insert Bookmark"));
    KoBookmark *bookmark = new KoBookmark(name, d->document);
    int startPos = -1, endPos = -1, caretPos = -1;

    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(d->document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());
    if (d->caret.hasSelection()) {
        startPos = d->caret.selectionStart();
        endPos = d->caret.selectionEnd();
        caretPos = d->caret.position();

        d->caret.setPosition(endPos);
        KoBookmark *endBookmark = new KoBookmark(name, d->document);
        bookmark->setType(KoBookmark::StartBookmark);
        endBookmark->setType(KoBookmark::EndBookmark);
        layout->inlineTextObjectManager()->insertInlineObject(d->caret, endBookmark);
        bookmark->setEndBookmark(endBookmark);
        d->caret.setPosition(startPos);
    } else {
        bookmark->setType(KoBookmark::SinglePosition);
    }
    // TODO the macro & undo things
    layout->inlineTextObjectManager()->insertInlineObject(d->caret, bookmark);
    if (startPos != -1) {
        // TODO repaint selection properly
        if (caretPos == startPos) {
            startPos = endPos + 1;
            endPos = caretPos;
        } else {
            endPos += 2;
        }
        d->caret.setPosition(startPos);
        d->caret.setPosition(endPos, QTextCursor::KeepAnchor);
    }
    d->updateState(KoTextEditor::Private::NoOp);
}

bool KoTextEditor::insertIndexMarker()
{//TODO changeTracking
    QTextBlock block = d->caret.block();
    if (d->caret.position() >= block.position() + block.length() - 1)
        return false; // can't insert one at end of text
    if (block.text()[ d->caret.position() - block.position()].isSpace())
        return false; // can't insert one on a whitespace as that does not indicate a word.

    d->updateState(KoTextEditor::Private::Custom, i18n("Insert Index"));
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(d->document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());
    KoTextLocator *tl = new KoTextLocator();
    layout->inlineTextObjectManager()->insertInlineObject(d->caret, tl);
    d->updateState(KoTextEditor::Private::NoOp);
    return true;
}

void KoTextEditor::insertInlineObject(KoInlineObject *inliner)
{//TODO changeTracking
    d->updateState(KoTextEditor::Private::Custom, i18n("Insert Variable"));
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(d->document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());
    layout->inlineTextObjectManager()->insertInlineObject(d->caret, inliner);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::insertFrameBreak()
{//TODO split newLine method in two.
    d->updateState(KoTextEditor::Private::KeyPress, i18n("Insert Break"));
    QTextBlock block = d->caret.block();
    /*
    if(d->caret->position() == block.position() && block.length() > 0) { // start of parag
        QTextBlockFormat bf = d->caret->blockFormat();
        bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);
        d->caret->setBlockFormat(bf);
} else { */
    QTextBlockFormat bf = d->caret.blockFormat();
    //       if(d->caret->position() != block.position() + block.length() -1 ||
    //               bf.pageBreakPolicy() != QTextFormat::PageBreak_Auto) // end of parag or already a pagebreak
    newLine();
    bf = d->caret.blockFormat();
    bf.setProperty(KoParagraphStyle::BreakBefore, true);
    d->caret.setBlockFormat(bf);
    //}
    d->updateState(KoTextEditor::Private::NoOp);
}

bool KoTextEditor::deleteInlineObjects(bool backward)
{
    return d->deleteInlineObjects(backward);
}

int KoTextEditor::anchor() const
{
    return d->caret.anchor();
}

bool KoTextEditor::atBlockEnd() const
{
    return d->caret.atBlockEnd();
}

bool KoTextEditor::atBlockStart() const
{
    return d->caret.atBlockStart();
}

bool KoTextEditor::atEnd() const
{
    return d->caret.atEnd();
}

bool KoTextEditor::atStart() const
{
    return d->caret.atStart();
}

QTextBlock KoTextEditor::block() const
{
    return d->caret.block();
}

QTextCharFormat KoTextEditor::blockCharFormat() const
{
    return d->caret.blockCharFormat();
}

QTextBlockFormat KoTextEditor::blockFormat() const
{
    return d->caret.blockFormat();
}

int KoTextEditor::blockNumber() const
{
    return d->caret.blockNumber();
}

QTextCharFormat KoTextEditor::charFormat() const
{
    return d->caret.charFormat();
}

void KoTextEditor::clearSelection()
{
    d->caret.clearSelection();
}

int KoTextEditor::columnNumber() const
{
    return d->caret.columnNumber();
}

void KoTextEditor::deleteChar()
{//TODO
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(d->document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());

//TODO this should not be needed. this case should be handled by the generic code. neighbouring dels should be merged
    if (!d->caret.hasSelection() && dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(d->caret))) {
    //kDebug() << "move over existing del";
    d->caret.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
    return;
    }

    if (!d->deleteInlineObjects(false) || d->caret.hasSelection()) {
        d->updateState(KoTextEditor::Private::Delete, i18n("Delete"));

        if (!d->caret.hasSelection())
            d->caret.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        d->deleteSelection();
    /*        QTextCursor cur = QTextCursor(m_caret);
    if (!cur.hasSelection())
    cur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    //find out if we have a delete marker before
    cur.setPosition((qMin(m_caret.position(), m_caret.anchor()) -1);
    KoDeleteChangeMarker *prev = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(m_caret));
    //find out if we have a delete marker after
    cur.setPosition(qMax(m_caret.anchor(), m_caret.position()));
    KoDeleteChangeMarker *next = dynamic_cast<KoDeleteChangeMarker*>(layout->inlineTextObjectManager()->inlineTextObject(m_caret));



    bool m_changeRegistered = m_textDeleting;
    kDebug() << "delete. registered: " << m_changeRegistered;
    startMacro(i18n("Delete"));
    *///        QTextCharFormat oldFormat = m_caret.charFormat();
    //        QTextCharFormat format = m_caret.charFormat();
    }
}

void KoTextEditor::deletePreviousChar()
{
    if (!d->deleteInlineObjects(false) || d->caret.hasSelection()) {
        d->updateState(KoTextEditor::Private::Delete, i18n("Delete"));

        if (!d->caret.hasSelection())
            d->caret.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        d->deleteSelection();
    }
}

QTextDocument* KoTextEditor::document() const
{
    return d->caret.document();
}

bool KoTextEditor::hasComplexSelection() const
{
    return d->caret.hasComplexSelection();
}

bool KoTextEditor::hasSelection()
{
    return d->caret.hasSelection();
}

void KoTextEditor::insertBlock()
{
//TODO
}

void KoTextEditor::insertBlock(const QTextBlockFormat &format)
{
    Q_UNUSED(format)
//TODO
}

void KoTextEditor::insertBlock(const QTextBlockFormat &format, const QTextCharFormat &charFormat)
{
    Q_UNUSED(format)
    Q_UNUSED(charFormat)
//TODO
}

void KoTextEditor::insertFragment(const QTextDocumentFragment &fragment)
{
    Q_UNUSED(fragment)
//TODO
}

void KoTextEditor::insertTable(int rows, int columns)
{
    d->updateState(KoTextEditor::Private::Custom, i18n("Insert Table"));
    QTextTableFormat tableFormat;

    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    tableFormat.setMargin(5);

    QTextTable *table = d->caret.insertTable(rows, columns, tableFormat);

    // Format the cells a bit.
    for (int row = 0; row < table->rows(); ++row) {
        for (int col = 0; col < table->columns(); ++col) {
            QTextTableCell cell = table->cellAt(row, col);
            QTextTableCellFormat format;
            KoTableCellStyle cellStyle;
            cellStyle.setEdge(KoTableCellStyle::Top, KoTableCellStyle::BorderSolid, 2, QColor(Qt::black));
            cellStyle.setEdge(KoTableCellStyle::Left, KoTableCellStyle::BorderSolid, 2, QColor(Qt::black));
            cellStyle.setEdge(KoTableCellStyle::Bottom, KoTableCellStyle::BorderSolid, 2, QColor(Qt::black));
            cellStyle.setEdge(KoTableCellStyle::Right, KoTableCellStyle::BorderSolid, 2, QColor(Qt::black));
            cellStyle.setPadding(5);

            cellStyle.applyStyle(format);
            cell.setFormat(format);
        }
    }
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::insertText(const QString &text)
{
    d->updateState(KoTextEditor::Private::KeyPress, i18n("Key Press"));
//    if (!d->headCommand){//TODO review this
        QTextCharFormat format = d->caret.charFormat();
        registerTrackedChange(d->caret, KoGenChange::insertChange, i18n("Key Press"), format, format, false);
//    }
    int blockNumber = d->caret.blockNumber();
    d->caret.insertText(text);

    while (blockNumber <= d->caret.blockNumber()) {
        d->dirtyBlocks << blockNumber;
        ++blockNumber;
    }
    d->updateRtlTimer.stop();
    d->updateRtlTimer.start();
}

void KoTextEditor::insertText(const QString &text, const QTextCharFormat &format)
{
    Q_UNUSED(text)
    Q_UNUSED(format)
//TODO
}

void KoTextEditor::mergeBlockCharFormat(const QTextCharFormat &modifier)
{
    Q_UNUSED(modifier)
//TODO
}

void KoTextEditor::mergeBlockFormat(const QTextBlockFormat &modifier)
{
    Q_UNUSED(modifier)
//TODO
}

void KoTextEditor::mergeCharFormat(const QTextCharFormat &modifier)
{
    Q_UNUSED(modifier)
//TODO
}

bool KoTextEditor::movePosition(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode, int n)
{
    return d->caret.movePosition (operation, mode, n);
}

void KoTextEditor::newLine()
{
    d->updateState(KoTextEditor::Private::Custom, i18n("Line Break"));
    if (d->caret.hasSelection())
        d->deleteInlineObjects();
    KoTextDocument koDocument(d->document);
    KoStyleManager *styleManager = koDocument.styleManager();
    KoParagraphStyle *nextStyle = 0;
    KoParagraphStyle *currentStyle = 0;
    if (styleManager) {
        int id = d->caret.blockFormat().intProperty(KoParagraphStyle::StyleId);
        currentStyle = styleManager->paragraphStyle(id);
        if (currentStyle == 0) // not a style based parag.  Lets make the next one correct.
            nextStyle = styleManager->defaultParagraphStyle();
        else
            nextStyle = styleManager->paragraphStyle(currentStyle->nextStyle());
        Q_ASSERT(nextStyle);
        if (currentStyle == nextStyle)
            nextStyle = 0;
    }
    d->caret.insertBlock();
    QTextBlockFormat bf = d->caret.blockFormat();
    QVariant direction = bf.property(KoParagraphStyle::TextProgressionDirection);
    bf.setPageBreakPolicy(QTextFormat::PageBreak_Auto);
    bf.clearProperty(KoParagraphStyle::ListStartValue);
    bf.clearProperty(KoParagraphStyle::UnnumberedListItem);
    bf.clearProperty(KoParagraphStyle::IsListHeader);
    bf.clearProperty(KoParagraphStyle::MasterPageName);
    d->caret.setBlockFormat(bf);
    if (nextStyle) {
        QTextBlock block = d->caret.block();
        if (currentStyle)
            currentStyle->unapplyStyle(block);
        nextStyle->applyStyle(block);
    }

    bf = d->caret.blockFormat();
    if (d->direction != KoText::AutoDirection) { // inherit from shape
        KoText::Direction dir;
        switch (d->direction) {
        case KoText::RightLeftTopBottom:
            dir = KoText::PerhapsRightLeftTopBottom;
            break;
        case KoText::LeftRightTopBottom:
        default:
            dir = KoText::PerhapsLeftRightTopBottom;
        }
        bf.setProperty(KoParagraphStyle::TextProgressionDirection, dir);
    } else if (! direction.isNull()) { // then we inherit from the previous paragraph.
        bf.setProperty(KoParagraphStyle::TextProgressionDirection, direction);
    }
    d->caret.setBlockFormat(bf);
    d->updateState(KoTextEditor::Private::NoOp);
}

int KoTextEditor::position() const
{
    return d->caret.position();
}

void KoTextEditor::removeSelectedText()
{
//TODO
}

void KoTextEditor::select(QTextCursor::SelectionType selection)
{
    //TODO add selection of previous/next char, and option about hasSelection
    d->caret.select (selection);
}

QString KoTextEditor::selectedText() const
{
    return d->caret.selectedText();
}

QTextDocumentFragment KoTextEditor::selection() const
{
    return d->caret.selection();
}

int KoTextEditor::selectionEnd() const
{
    return d->caret.selectionEnd();
}

int KoTextEditor::selectionStart() const
{
    return d->caret.selectionStart();
}

void KoTextEditor::setBlockCharFormat(const QTextCharFormat &format)
{
    Q_UNUSED(format)
//TODO
}

void KoTextEditor::setBlockFormat(const QTextBlockFormat &format)
{
    Q_UNUSED(format)
//TODO
}

void KoTextEditor::setCharFormat(const QTextCharFormat &format)
{
    Q_UNUSED(format)
//TODO
}

void KoTextEditor::setPosition(int pos, QTextCursor::MoveMode m)
{
    d->caret.setPosition (pos, m);
}

void KoTextEditor::setVisualNavigation(bool b)
{
    d->caret.setVisualNavigation (b);
}

bool KoTextEditor::visualNavigation() const
{
    return d->caret.visualNavigation();
}

bool KoTextEditor::isBidiDocument() const
{
    return d->isBidiDocument;
}

#include "KoTextEditor.moc"
