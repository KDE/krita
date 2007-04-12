/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#include "KoFontDia.h"
#include "KoTextDocumentLayout.h"
#include "KoTextShapeData.h"
#include "KoInlineTextObjectManager.h"
// #include "KoInlineObject.h"
#include "KoTextLocator.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoStyleManager.h"

#include <kdebug.h>
#include <QTextCharFormat>
#include <QFont>
#include <QTextCursor>
#include <QTextBlock>

class KoTextSelectionHandler::Private {
public:
    Private() :textShape(0), textShapeData(0), caret(0) {}

    KoShape *textShape;
    KoTextShapeData *textShapeData;
    QTextCursor *caret;
};

class CharFormatVisitor {
public:
    CharFormatVisitor() {}
    virtual ~CharFormatVisitor() {}

    virtual void visit(QTextCharFormat &format) const = 0;

    static void visitSelection(QTextCursor *caret, const CharFormatVisitor &visitor) {
        int start = caret->position();
        int end = caret->anchor();
        if(start > end) { // swap
            int tmp = start;
            start = end;
            end = tmp;
        }
        else if(start == end) { // just set a new one.
            QTextCharFormat format = caret->charFormat();
            visitor.visit(format);
            caret->setCharFormat(format);
            return;
        }

        QTextBlock block = caret->block();
        if(block.position() > start)
            block = block.document()->findBlock(start);

        // now loop over all blocks that the selection contains and alter the text fragments where applicable.
        while(block.isValid() && block.position() < end) {
            QTextBlock::iterator iter = block.begin();
            while(! iter.atEnd()) {
                QTextFragment fragment = iter.fragment();
                if(fragment.position() > end)
                    break;
                if(fragment.position() + fragment.length() <= start) {
                    iter++;
                    continue;
                }

                QTextCursor cursor(block);
                cursor.setPosition(fragment.position() +1);
                QTextCharFormat format = cursor.charFormat(); // this gets the format one char before the postion.
                visitor.visit(format);

                cursor.setPosition(qMax(start, fragment.position()));
                int to = qMin(end, fragment.position() + fragment.length()) -1;
                cursor.setPosition(to, QTextCursor::KeepAnchor);
                cursor.mergeCharFormat(format);

                iter++;
            }
            block = block.next();
        }
    }
};

class BlockFormatVisitor {
public:
    BlockFormatVisitor() {}
    virtual ~BlockFormatVisitor() {}

    virtual void visit(QTextBlockFormat &format) const = 0;

    static void visitSelection(QTextCursor *caret, const BlockFormatVisitor &visitor) {
        int start = caret->position();
        int end = caret->anchor();
        if(start > end) { // swap
            int tmp = start;
            start = end;
            end = tmp;
        }

        QTextBlock block = caret->block();
        if(block.position() > start)
            block = block.document()->findBlock(start);

        // now loop over all blocks that the selection contains and alter the text fragments where applicable.
        while(block.isValid() && block.position() < end) {
            QTextBlockFormat format = block.blockFormat();
            visitor.visit(format);
            QTextCursor cursor(block);
            cursor.setBlockFormat(format);
            block = block.next();
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

void KoTextSelectionHandler::bold(bool bold) {
    Q_ASSERT(d->caret);
    class Bolder : public CharFormatVisitor {
    public:
        Bolder(bool on) : bold(on) {}
        void visit(QTextCharFormat &format) const {
            format.setFontWeight( bold ? QFont::Bold : QFont::Normal );
        }
        bool bold;
    };
    Bolder bolder(bold);
    CharFormatVisitor::visitSelection(d->caret, bolder);
}

void KoTextSelectionHandler::italic(bool italic) {
    Q_ASSERT(d->caret);
    class Italic : public CharFormatVisitor {
    public:
        Italic(bool on) : italic(on) {}
        void visit(QTextCharFormat &format) const {
            format.setFontItalic(italic);
        }
        bool italic;
    };
    Italic ital(italic);
    CharFormatVisitor::visitSelection(d->caret, ital);
}

void KoTextSelectionHandler::underline(bool underline) {
    Q_ASSERT(d->caret);
    class Underliner : public CharFormatVisitor {
    public:
        Underliner(bool on) : underline(on) {}
        void visit(QTextCharFormat &format) const {
            format.setFontUnderline(underline);
        }
        bool underline;
    };
    Underliner underliner(underline);
    CharFormatVisitor::visitSelection(d->caret, underliner);
}

void KoTextSelectionHandler::strikeOut(bool strikeout) {
    Q_ASSERT(d->caret);
    class Striker : public CharFormatVisitor {
    public:
        Striker(bool on) : strikeout(on) {}
        void visit(QTextCharFormat &format) const {
            format.setFontStrikeOut(strikeout);
        }
        bool strikeout;
    };
    Striker striker(strikeout);
    CharFormatVisitor::visitSelection(d->caret, striker);
}

void KoTextSelectionHandler::insertFrameBreak() {
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
}

void KoTextSelectionHandler::setFontSize(int size) {
    // TODO
}

void KoTextSelectionHandler::increaseFontSize() {
    // TODO
}

void KoTextSelectionHandler::decreaseFontSize() {
    // TODO
}

void KoTextSelectionHandler::setHorizontalTextAlignment(Qt::Alignment align) {
    Q_ASSERT(d->caret);
    class Aligner : public BlockFormatVisitor {
    public:
        Aligner(Qt::Alignment align) : alignment(align) {}
        void visit(QTextBlockFormat &format) const {
            format.setAlignment(alignment);
        }
        Qt::Alignment alignment;
    };
    Aligner aligner(align);
    BlockFormatVisitor::visitSelection(d->caret, aligner);
}

void KoTextSelectionHandler::setVerticalTextAlignment(Qt::Alignment align) {
    Q_ASSERT(d->caret);
    class Aligner : public CharFormatVisitor {
    public:
        Aligner(QTextCharFormat::VerticalAlignment align) : alignment(align) {}
        void visit(QTextCharFormat &format) const {
            format.setVerticalAlignment(alignment);
        }
        QTextCharFormat::VerticalAlignment alignment;
    };

    QTextCharFormat::VerticalAlignment charAlign = QTextCharFormat::AlignNormal;
    if(align == Qt::AlignTop)
        charAlign = QTextCharFormat::AlignSuperScript;
    else if(align == Qt::AlignBottom)
        charAlign = QTextCharFormat::AlignSubScript;

    Aligner aligner(charAlign);
    CharFormatVisitor::visitSelection(d->caret, aligner);
}

void KoTextSelectionHandler::increaseIndent() {
    class Indenter : public BlockFormatVisitor {
    public:
        void visit(QTextBlockFormat &format) const {
            // TODO make the 10 configurable.
            format.setLeftMargin(format.leftMargin() + 10);
        }
        Qt::Alignment alignment;
    };
    Indenter indenter;
    BlockFormatVisitor::visitSelection(d->caret, indenter);
}

void KoTextSelectionHandler::decreaseIndent() {
    class Indenter : public BlockFormatVisitor {
    public:
        void visit(QTextBlockFormat &format) const {
            // TODO make the 10 configurable.
            format.setLeftMargin(qMax(0.0, format.leftMargin() - 10));
        }
        Qt::Alignment alignment;
    };
    Indenter indenter;
    BlockFormatVisitor::visitSelection(d->caret, indenter);
}

void KoTextSelectionHandler::setTextColor(const QColor &color) {
    // TODO
}

void KoTextSelectionHandler::setTextBackgroundColor(const QColor &color) {
    // TODO
}

QString KoTextSelectionHandler::selectedText() const {
    return d->caret->selectedText();
}

void KoTextSelectionHandler::insert(const QString &text) {
    d->caret->insertText(text);
}

void KoTextSelectionHandler::selectFont(QWidget *parent) {
    KoFontDia *fontDlg = new KoFontDia( d->caret->charFormat()); // , 0, parent);
    fontDlg->exec();
    d->caret->setCharFormat(fontDlg->format());
    delete fontDlg;
}

void KoTextSelectionHandler::insertInlineObject(KoInlineObject *inliner) {
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*> (d->textShapeData->document()->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineObjectTextManager());
    layout->inlineObjectTextManager()->insertInlineObject(*d->caret, inliner);
}

void KoTextSelectionHandler::setStyle(KoParagraphStyle* style) {
    Q_ASSERT(style);
    QTextBlock block = d->caret->block();
    style->applyStyle(block);
}

void KoTextSelectionHandler::setStyle(KoCharacterStyle* style) {
    Q_ASSERT(style);
    style->applyStyle(d->caret);
}

QTextCursor KoTextSelectionHandler::caret() const {
    return d->caret ? *d->caret : QTextCursor();
}

void KoTextSelectionHandler::nextParagraph() {
    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (d->textShapeData->document()->documentLayout());
    KoParagraphStyle *nextStyle = 0;
    if(lay && lay->styleManager()) {
        int id = d->caret->blockFormat().intProperty(KoParagraphStyle::StyleId);
        KoParagraphStyle *currentStyle = lay->styleManager()->paragraphStyle(id);
        if(currentStyle == 0) // not a style based parag.  Lets make the next one correct.
            nextStyle = lay->styleManager()->defaultParagraphStyle();
        else
            nextStyle = lay->styleManager()->paragraphStyle(currentStyle->nextStyle());
        Q_ASSERT(nextStyle);
        if(currentStyle == nextStyle)
            nextStyle = 0;
    }
    d->caret->insertText("\n");
    QTextBlockFormat bf = d->caret->blockFormat();
    bf.setPageBreakPolicy(QTextFormat::PageBreak_Auto);
    d->caret->setBlockFormat(bf);
    if(nextStyle) {
        QTextBlock block = d->caret->block();
        nextStyle->applyStyle(block);
    }
}

bool KoTextSelectionHandler::insertIndexMarker() {
    QTextBlock block = d->caret->block();
    if(d->caret->position() >= block.position() + block.length() -1)
        return false; // can't insert one at end of text
    if(block.text()[ d->caret->position() - block.position() ].isSpace())
        return false; // can't insert one on a whitespace as that does not indicate a word.

    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*> (d->textShapeData->document()->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineObjectTextManager());
    KoTextLocator *tl = new KoTextLocator();
    layout->inlineObjectTextManager()->insertInlineObject(*d->caret, tl);
    return true;
}

void KoTextSelectionHandler::setShape(KoShape *shape) {
    d->textShape = shape;
}

void KoTextSelectionHandler::setShapeData(KoTextShapeData *data) {
    d->textShapeData = data;
}

void KoTextSelectionHandler::setCaret(QTextCursor *caret) {
    d->caret = caret;
}

bool KoTextSelectionHandler::hasSelection() {
    return d->caret && d->caret->hasSelection();
}

#include <KoTextSelectionHandler.moc>
