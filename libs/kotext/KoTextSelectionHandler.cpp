/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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

#include "KoTextSelectionHandler.h"
#include "KoTextDocumentLayout.h"
#include "KoTextShapeData.h"
#include "KoInlineTextObjectManager.h"
#include "KoTextLocator.h"
#include "KoBookmark.h"
#include "KoBookmarkManager.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoTableCellStyle.h"
#include "styles/KoStyleManager.h"
#include "KoTextDocument.h"

#include "changetracker/KoChangeTracker.h"

#include <kdebug.h>
#include <KLocale>
#include <QTextCharFormat>
#include <QtGui/QFontDatabase>
#include <QFont>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextList>
#include <QTextTable>
#include <QTextTableCell>

#include <QTextFormat>


class KoTextSelectionHandler::Private
{
public:
    Private() : textShape(0), textShapeData(0), caret(0) {}

    KoShape *textShape;
    KoTextShapeData *textShapeData;
    QTextCursor *caret;
};

class BlockFormatVisitor
{
public:
    BlockFormatVisitor() {}
    virtual ~BlockFormatVisitor() {}

    virtual void visit(QTextBlockFormat &format) const = 0;

    static void visitSelection(QTextCursor *caret, const BlockFormatVisitor &visitor) {
        int start = caret->position();
        int end = caret->anchor();
        if (start > end) { // swap
            int tmp = start;
            start = end;
            end = tmp;
        }

        QTextBlock block = caret->block();
        if (block.position() > start)
            block = block.document()->findBlock(start);

        // now loop over all blocks that the selection contains and alter the text fragments where applicable.
        while (block.isValid() && block.position() <= end) {
            QTextBlockFormat format = block.blockFormat();
            visitor.visit(format);
            QTextCursor cursor(block);
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

    static void visitSelection(QTextCursor *caret, const CharFormatVisitor &visitor) {
        int start = caret->position();
        int end = caret->anchor();
        if (start > end) { // swap
            int tmp = start;
            start = end;
            end = tmp;
        } else if (start == end) { // just set a new one.
            QTextCharFormat format = caret->charFormat();
            visitor.visit(format);
            caret->setCharFormat(format);
            return;
        }

        QTextBlock block = caret->block();
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
                QTextCharFormat format = cursor.charFormat(); // this gets the format one char before the postion.
                visitor.visit(format);

                cursor.setPosition(qMax(start, fragment.position()));
                int to = qMin(end, fragment.position() + fragment.length());
                cursor.setPosition(to, QTextCursor::KeepAnchor);
                cursors.append(cursor);
                formats.append(format);

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


KoTextSelectionHandler::KoTextSelectionHandler(QObject *parent)
        : KoToolSelection(parent),
        d(new Private())
{
}

KoTextSelectionHandler::~KoTextSelectionHandler()
{
    delete d;
}

void KoTextSelectionHandler::bold(bool bold)
{
    Q_ASSERT(d->caret);
    emit startMacro(i18n("Bold"));
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);

    if (KoTextDocument(d->textShapeData->document()).changeTracker() && KoTextDocument(d->textShapeData->document()).changeTracker()->isEnabled()) {
        QTextCharFormat prevFormat(d->caret->charFormat());

        int changeId = KoTextDocument(d->textShapeData->document()).changeTracker()->getFormatChangeId(i18n("Bold"), format, prevFormat, d->caret->charFormat().property( KoCharacterStyle::ChangeTrackerId ).toInt());
        format.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
    }

    d->caret->mergeCharFormat(format);
    emit stopMacro();
}

void KoTextSelectionHandler::italic(bool italic)
{
    Q_ASSERT(d->caret);
    emit startMacro(i18n("Italic"));
    QTextCharFormat format;
    format.setFontItalic(italic);

    if (KoTextDocument(d->textShapeData->document()).changeTracker() && KoTextDocument(d->textShapeData->document()).changeTracker()->isEnabled()) {
        QTextCharFormat prevFormat(d->caret->charFormat());

        int changeId = KoTextDocument(d->textShapeData->document()).changeTracker()->getFormatChangeId(i18n("Italic"), format, prevFormat, d->caret->charFormat().property( KoCharacterStyle::ChangeTrackerId ).toInt());
        format.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
    }

    d->caret->mergeCharFormat(format);
    emit stopMacro();
}

void KoTextSelectionHandler::underline(bool underline)
{
    Q_ASSERT(d->caret);
    emit startMacro(i18n("Underline"));
    QTextCharFormat format;
    if (underline) {
        format.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::SingleLine);
        format.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::SolidLine);
    } else {
        format.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::NoLineType);
        format.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::NoLineStyle);
    }
    //format.setFontUnderline(underline);
    d->caret->mergeCharFormat(format);
    emit stopMacro();
}

void KoTextSelectionHandler::strikeOut(bool strikeout)
{
    Q_ASSERT(d->caret);
    emit startMacro(i18n("Strike Out"));
    QTextCharFormat format;
    if (strikeout) {
        format.setProperty(KoCharacterStyle::StrikeOutType, KoCharacterStyle::SingleLine);
        format.setProperty(KoCharacterStyle::StrikeOutStyle, KoCharacterStyle::SolidLine);
    } else {
        format.setProperty(KoCharacterStyle::StrikeOutType, KoCharacterStyle::NoLineType);
        format.setProperty(KoCharacterStyle::StrikeOutStyle, KoCharacterStyle::NoLineStyle);
    }
    //format.setFontStrikeOut(strikeout ? Qt::SolidLine : Qt::NoPen);
    d->caret->mergeCharFormat(format);
    emit stopMacro();
}

void KoTextSelectionHandler::insertFrameBreak()
{
    emit startMacro(i18n("Insert Break"));
    QTextBlock block = d->caret->block();
    /*
        if(d->caret->position() == block.position() && block.length() > 0) { // start of parag
            QTextBlockFormat bf = d->caret->blockFormat();
            bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysAfter);
            d->caret->setBlockFormat(bf);
        } else { */
    QTextBlockFormat bf = d->caret->blockFormat();
//       if(d->caret->position() != block.position() + block.length() -1 ||
//               bf.pageBreakPolicy() != QTextFormat::PageBreak_Auto) // end of parag or already a pagebreak
    nextParagraph();
    bf = d->caret->blockFormat();
    bf.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
    d->caret->setBlockFormat(bf);
    //}
    emit stopMacro();
}

void KoTextSelectionHandler::setFontSize(int size)
{
    QTextCharFormat newSize;
    newSize.setFontPointSize(size);
    d->caret->mergeCharFormat(newSize);
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

void KoTextSelectionHandler::increaseFontSize()
{
    emit startMacro(i18n("Increase font size"));
    FontResizer sizer(FontResizer::Grow);
    CharFormatVisitor::visitSelection(d->caret, sizer);
    emit stopMacro();
}

void KoTextSelectionHandler::decreaseFontSize()
{
    emit startMacro(i18n("Decrease font size"));
    FontResizer sizer(FontResizer::Shrink);
    CharFormatVisitor::visitSelection(d->caret, sizer);
    emit stopMacro();
}

void KoTextSelectionHandler::setDefaultFormat()
{
    KoTextDocument koDocument(d->textShapeData->document());
    emit startMacro(i18n("Set default format"));
    if (KoStyleManager *styleManager = koDocument.styleManager()) {
        KoCharacterStyle *defaultCharStyle = styleManager->defaultParagraphStyle()->characterStyle();
        QTextCharFormat defaultFormat;
        defaultCharStyle->applyStyle(defaultFormat);
        d->caret->setCharFormat(defaultFormat);
    }
    emit stopMacro();
}

void KoTextSelectionHandler::setHorizontalTextAlignment(Qt::Alignment align)
{
    Q_ASSERT(d->caret);
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
    emit startMacro(i18n("Set Horizontal Alignment"));
    BlockFormatVisitor::visitSelection(d->caret, aligner);
    emit stopMacro();
}

void KoTextSelectionHandler::setVerticalTextAlignment(Qt::Alignment align)
{
    Q_ASSERT(d->caret);
    QTextCharFormat::VerticalAlignment charAlign = QTextCharFormat::AlignNormal;
    if (align == Qt::AlignTop)
        charAlign = QTextCharFormat::AlignSuperScript;
    else if (align == Qt::AlignBottom)
        charAlign = QTextCharFormat::AlignSubScript;

    emit startMacro(i18n("Set Vertical Alignment"));
    QTextCharFormat format;
    format.setVerticalAlignment(charAlign);
    d->caret->mergeCharFormat(format);
    emit stopMacro();
}

void KoTextSelectionHandler::increaseIndent()
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
    emit startMacro(i18n("Increase Indent"));
    BlockFormatVisitor::visitSelection(d->caret, indenter);
    emit stopMacro();
}

void KoTextSelectionHandler::decreaseIndent()
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
    emit startMacro(i18n("Decrease Indent"));
    BlockFormatVisitor::visitSelection(d->caret, indenter);
    emit stopMacro();
}

void KoTextSelectionHandler::setFontFamily(const QString &familyName)
{
    QTextCharFormat format;
    format.setFontFamily(familyName);
    d->caret->mergeCharFormat(format);
}

void KoTextSelectionHandler::setTextColor(const QColor &color)
{
    QTextCharFormat format;
    format.setForeground(QBrush(color));
    d->caret->mergeCharFormat(format);
}

void KoTextSelectionHandler::setTextBackgroundColor(const QColor &color)
{
    QTextCharFormat format;
    format.setBackground(QBrush(color));
    d->caret->mergeCharFormat(format);
}

QString KoTextSelectionHandler::selectedText() const
{
    return d->caret->selectedText();
}

void KoTextSelectionHandler::insertTable(int rows, int columns)
{
    QTextTableFormat tableFormat;

    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    tableFormat.setMargin(5);

    QTextTable *table = d->caret->insertTable(rows, columns, tableFormat);

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
}

void KoTextSelectionHandler::insert(const QString &text)
{
    d->caret->insertText(text);
}

void KoTextSelectionHandler::insertInlineObject(KoInlineObject *inliner)
{
    emit startMacro(i18n("Insert"));
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(d->textShapeData->document()->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());
    layout->inlineTextObjectManager()->insertInlineObject(*d->caret, inliner);
    emit stopMacro();
}

void KoTextSelectionHandler::setStyle(KoParagraphStyle* style)
{
    Q_ASSERT(style);
    emit startMacro(i18n("Set Paragraph Style"));
    int from = d->caret->position();
    int to = d->caret->anchor();
    if (to < from)
        qSwap(to, from);
    QTextBlock block = d->caret->block().document()->findBlock(from);
    Q_ASSERT(block.isValid());

    KoTextDocument doc(block.document());
    KoStyleManager *styleManager = doc.styleManager();

    while (block.isValid() && block.position() <= to) {
        if (styleManager) {
            KoParagraphStyle *old = styleManager->paragraphStyle(block.blockFormat().intProperty(KoParagraphStyle::StyleId));
            if (old)
                old->unapplyStyle(block);
        }

        style->applyStyle(block);
        block = block.next();
    }
    emit stopMacro();
}

void KoTextSelectionHandler::setStyle(KoCharacterStyle* style)
{
    Q_ASSERT(style);
    emit startMacro(i18n("Set Character Style"));
    style->applyStyle(d->caret);
    emit stopMacro();
}

QTextCursor KoTextSelectionHandler::caret() const
{
    return d->caret ? *d->caret : QTextCursor();
}

void KoTextSelectionHandler::nextParagraph()
{
    emit startMacro(i18n("Insert Linebreak"));
    KoTextDocument koDocument(d->textShapeData->document());
    KoStyleManager *styleManager = koDocument.styleManager();
    KoParagraphStyle *nextStyle = 0;
    KoParagraphStyle *currentStyle = 0;
    if (styleManager) {
        int id = d->caret->blockFormat().intProperty(KoParagraphStyle::StyleId);
        currentStyle = styleManager->paragraphStyle(id);
        if (currentStyle == 0) // not a style based parag.  Lets make the next one correct.
            nextStyle = styleManager->defaultParagraphStyle();
        else
            nextStyle = styleManager->paragraphStyle(currentStyle->nextStyle());
        Q_ASSERT(nextStyle);
        if (currentStyle == nextStyle)
            nextStyle = 0;
    }
    d->caret->insertBlock();
    QTextBlockFormat bf = d->caret->blockFormat();
    QVariant direction = bf.property(KoParagraphStyle::TextProgressionDirection);
    bf.setPageBreakPolicy(QTextFormat::PageBreak_Auto);
    bf.clearProperty(KoParagraphStyle::ListStartValue);
    bf.clearProperty(KoParagraphStyle::UnnumberedListItem);
    bf.clearProperty(KoParagraphStyle::IsListHeader);
    bf.clearProperty(KoParagraphStyle::MasterPageName);
    d->caret->setBlockFormat(bf);
    if (nextStyle) {
        QTextBlock block = d->caret->block();
        if (currentStyle)
            currentStyle->unapplyStyle(block);
        nextStyle->applyStyle(block);
    }

    bf = d->caret->blockFormat();
    if (d->textShapeData->pageDirection() != KoText::AutoDirection) { // inherit from shape
        KoText::Direction dir;
        switch (d->textShapeData->pageDirection()) {
        case KoText::RightLeftTopBottom:
            dir = KoText::PerhapsRightLeftTopBottom;
            break;
        case KoText::LeftRightTopBottom:
        default:
            dir = KoText::PerhapsLeftRightTopBottom;
        }
        bf.setProperty(KoParagraphStyle::TextProgressionDirection, dir);
    } else if (! direction.isNull()) // then we inherit from the previous paragraph.
        bf.setProperty(KoParagraphStyle::TextProgressionDirection, direction);
    d->caret->setBlockFormat(bf);
    emit stopMacro();
}

bool KoTextSelectionHandler::insertIndexMarker()
{
    QTextBlock block = d->caret->block();
    if (d->caret->position() >= block.position() + block.length() - 1)
        return false; // can't insert one at end of text
    if (block.text()[ d->caret->position() - block.position()].isSpace())
        return false; // can't insert one on a whitespace as that does not indicate a word.

    emit startMacro(i18n("Insert Index"));
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(d->textShapeData->document()->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());
    KoTextLocator *tl = new KoTextLocator();
    layout->inlineTextObjectManager()->insertInlineObject(*d->caret, tl);
    emit stopMacro();
    return true;
}

void KoTextSelectionHandler::addBookmark(const QString &name)
{
    QTextDocument *document = d->textShapeData->document();
    KoBookmark *bookmark = new KoBookmark(name, document);
    int startPos = -1, endPos = -1, caretPos = -1;

    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(d->textShapeData->document()->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineTextObjectManager());
    if (d->caret->hasSelection()) {
        startPos = d->caret->selectionStart();
        endPos = d->caret->selectionEnd();
        caretPos = d->caret->position();

        d->caret->setPosition(endPos);
        KoBookmark *endBookmark = new KoBookmark(name, document);
        bookmark->setType(KoBookmark::StartBookmark);
        endBookmark->setType(KoBookmark::EndBookmark);
        layout->inlineTextObjectManager()->insertInlineObject(*d->caret, endBookmark);
        bookmark->setEndBookmark(endBookmark);
        d->caret->setPosition(startPos);
    } else
        bookmark->setType(KoBookmark::SinglePosition);
    // TODO the macro & undo things
    emit startMacro(i18n("Add Bookmark"));
    layout->inlineTextObjectManager()->insertInlineObject(*d->caret, bookmark);
    emit stopMacro();
    if (startPos != -1) {
        // TODO repaint selection properly
        if (caretPos == startPos) {
            startPos = endPos + 1;
            endPos = caretPos;
        } else
            endPos += 2;
        d->caret->setPosition(startPos);
        d->caret->setPosition(endPos, QTextCursor::KeepAnchor);
    }
}

bool KoTextSelectionHandler::deleteInlineObjects(bool backward)
{
    return false;
    // TODO don't just blindly delete, make this a command so we can undo it later.
    // Also note that the below code needs unit testing since I found some issues already
/*
    QTextCursor cursor(*d->caret);
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(d->textShapeData->document()->documentLayout());
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

void KoTextSelectionHandler::setShape(KoShape *shape)
{
    d->textShape = shape;
}

void KoTextSelectionHandler::setShapeData(KoTextShapeData *data)
{
    d->textShapeData = data;
}

void KoTextSelectionHandler::setCaret(QTextCursor *caret)
{
    d->caret = caret;
}

bool KoTextSelectionHandler::hasSelection()
{
    return d->caret && d->caret->hasSelection();
}

#include <KoTextSelectionHandler.moc>
