/* This file is part of the KDE project
 * Copyright (C) 2009-2012 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (c) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2011-2012 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2014 Denis Kuplyakov <dener.kup@gmail.com>
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
#include "KoTextEditor_p.h"

#include "styles/KoCharacterStyle.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoStyleManager.h"
#include "commands/ParagraphFormattingCommand.h"

#include <klocalizedstring.h>

#include <QFontDatabase>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextFormat>
#include <QTextList>

#include "TextDebug.h"
#include "KoTextDebug.h"


void KoTextEditor::Private::clearCharFormatProperty(int property)
{
    class PropertyWiper : public CharFormatVisitor
    {
    public:
        PropertyWiper(int propertyId) : propertyId(propertyId) {}
        void visit(QTextCharFormat &format) const {
            format.clearProperty(propertyId);
        }

        int propertyId;
    };
    PropertyWiper wiper(property);
    CharFormatVisitor::visitSelection(q, wiper, KUndo2MagicString(), false);
}

void KoTextEditor::bold(bool bold)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Bold"));
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    mergeAutoStyle(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::italic(bool italic)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Italic"));
    QTextCharFormat format;
    format.setFontItalic(italic);
    mergeAutoStyle(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::underline(bool underline)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Underline"));
    QTextCharFormat format;
    if (underline) {
        format.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::SingleLine);
        format.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::SolidLine);
    } else {
        format.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::NoLineType);
        format.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::NoLineStyle);
    }
    mergeAutoStyle(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::strikeOut(bool strikeout)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Strike Out"));
    QTextCharFormat format;
    if (strikeout) {
        format.setProperty(KoCharacterStyle::StrikeOutType, KoCharacterStyle::SingleLine);
        format.setProperty(KoCharacterStyle::StrikeOutStyle, KoCharacterStyle::SolidLine);
    } else {
        format.setProperty(KoCharacterStyle::StrikeOutType, KoCharacterStyle::NoLineType);
        format.setProperty(KoCharacterStyle::StrikeOutStyle, KoCharacterStyle::NoLineStyle);
    }
    mergeAutoStyle(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setHorizontalTextAlignment(Qt::Alignment align)
{
    if (isEditProtected()) {
        return;
    }

    class Aligner : public BlockFormatVisitor
    {
    public:
        Aligner(Qt::Alignment align) : alignment(align) {}
        void visit(QTextBlock &block) const {
            QTextBlockFormat format = block.blockFormat();
            format.setAlignment(alignment);
            QTextCursor cursor(block);
            cursor.setBlockFormat(format);
        }
        Qt::Alignment alignment;
    };

    Aligner aligner(align);
    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Change Alignment"));
    BlockFormatVisitor::visitSelection(this, aligner, kundo2_i18n("Change Alignment"));
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

void KoTextEditor::setVerticalTextAlignment(Qt::Alignment align)
{
    if (isEditProtected()) {
        return;
    }

    QTextCharFormat::VerticalAlignment charAlign = QTextCharFormat::AlignNormal;
    if (align == Qt::AlignTop)
        charAlign = QTextCharFormat::AlignSuperScript;
    else if (align == Qt::AlignBottom)
        charAlign = QTextCharFormat::AlignSubScript;

    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Set Vertical Alignment"));
    QTextCharFormat format;
    format.setVerticalAlignment(charAlign);
    mergeAutoStyle(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::decreaseIndent()
{
    if (isEditProtected()) {
        return;
    }

    class Indenter : public BlockFormatVisitor
    {
    public:
        void visit(QTextBlock &block) const {
            QTextBlockFormat format = block.blockFormat();
            // TODO make the 10 configurable.
            format.setLeftMargin(qMax(qreal(0.0), format.leftMargin() - 10));

            if (block.textList()) {
                const QTextListFormat listFormat = block.textList()->format();
                if (format.leftMargin() < listFormat.doubleProperty(KoListStyle::Margin)) {
                    format.setLeftMargin(listFormat.doubleProperty(KoListStyle::Margin));
                }
            }
            QTextCursor cursor(block);
            cursor.setBlockFormat(format);
        }
        Qt::Alignment alignment;
    };

    Indenter indenter;
    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Decrease Indent"));
    BlockFormatVisitor::visitSelection(this, indenter, kundo2_i18n("Decrease Indent"));
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

void KoTextEditor::increaseIndent()
{
    if (isEditProtected()) {
        return;
    }

    class Indenter : public BlockFormatVisitor
    {
    public:
        void visit(QTextBlock &block) const {
            QTextBlockFormat format = block.blockFormat();
            // TODO make the 10 configurable.

            if (!block.textList()) {
                format.setLeftMargin(format.leftMargin() + 10);
            } else {
                const QTextListFormat listFormat = block.textList()->format();
                if (format.leftMargin() == 0) {
                    format.setLeftMargin(listFormat.doubleProperty(KoListStyle::Margin) + 10);
                } else {
                    format.setLeftMargin(format.leftMargin() + 10);
                }
            }
            QTextCursor cursor(block);
            cursor.setBlockFormat(format);
        }
        Qt::Alignment alignment;
    };

    Indenter indenter;
    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Increase Indent"));
    BlockFormatVisitor::visitSelection(this, indenter, kundo2_i18n("Increase Indent"));
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
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
        Q_FOREACH (int pt, defaultSizes) {
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
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Decrease font size"));
    FontResizer sizer(FontResizer::Shrink);
    CharFormatVisitor::visitSelection(this, sizer, kundo2_i18n("Decrease font size"));
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

void KoTextEditor::increaseFontSize()
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Increase font size"));
    FontResizer sizer(FontResizer::Grow);
    CharFormatVisitor::visitSelection(this, sizer, kundo2_i18n("Increase font size"));
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

void KoTextEditor::setFontFamily(const QString &font)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Set Font"));
    QTextCharFormat format;
    format.setFontFamily(font);
    mergeAutoStyle(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setFontSize(qreal size)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Set Font Size"));
    QTextCharFormat format;
    format.setFontPointSize(size);
    mergeAutoStyle(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setTextBackgroundColor(const QColor &color)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Set Background Color"));
    QTextCharFormat format;
    format.setBackground(QBrush(color));
    mergeAutoStyle(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setTextColor(const QColor &color)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, kundo2_i18n("Set Text Color"));
    QTextCharFormat format;
    format.setForeground(QBrush(color));
    mergeAutoStyle(format);
    d->updateState(KoTextEditor::Private::NoOp);
}

class SetCharacterStyleVisitor : public KoTextVisitor
{
public:
    SetCharacterStyleVisitor(KoTextEditor *editor, KoCharacterStyle *style)
        : KoTextVisitor(editor)
        , m_style(style)
    {
    }

    virtual void visitBlock(QTextBlock &block, const QTextCursor &caret)
    {
        m_newFormat = block.charFormat();
        m_style->applyStyle(m_newFormat);
        m_style->ensureMinimalProperties(m_newFormat);

        KoTextVisitor::visitBlock(block, caret);

        QList<QTextCharFormat>::Iterator it = m_formats.begin();
        Q_FOREACH (QTextCursor cursor, m_cursors) {
            QTextFormat prevFormat(cursor.charFormat());
            cursor.setCharFormat(*it);
            editor()->registerTrackedChange(cursor, KoGenChange::FormatChange, kundo2_i18n("Set Character Style"), *it, prevFormat, false);
            ++it;
        }
    }

    virtual void visitFragmentSelection(QTextCursor &fragmentSelection)
    {
        QTextCharFormat format = m_newFormat;

        QVariant v;
        v = fragmentSelection.charFormat().property(KoCharacterStyle::InlineInstanceId);
        if (!v.isNull()) {
            format.setProperty(KoCharacterStyle::InlineInstanceId, v);
        }

        v = fragmentSelection.charFormat().property(KoCharacterStyle::ChangeTrackerId);
        if (!v.isNull()) {
            format.setProperty(KoCharacterStyle::ChangeTrackerId, v);
        }

        if (fragmentSelection.charFormat().isAnchor()) {
            format.setAnchor(true);
            format.setProperty(KoCharacterStyle::AnchorType, fragmentSelection.charFormat().intProperty(KoCharacterStyle::AnchorType));
            format.setAnchorHref(fragmentSelection.charFormat().anchorHref());
        }

        m_formats.append(format);
        m_cursors.append(fragmentSelection);
    }

    KoCharacterStyle *m_style;
    QTextCharFormat m_newFormat;
    QList<QTextCharFormat> m_formats;
    QList<QTextCursor> m_cursors;
};

void KoTextEditor::setStyle(KoCharacterStyle *style)
{
    Q_ASSERT(style);
    d->updateState(KoTextEditor::Private::Custom, kundo2_i18n("Set Character Style"));

    int caretAnchor = d->caret.anchor();
    int caretPosition = d->caret.position();

    SetCharacterStyleVisitor visitor(this, style);

    recursivelyVisitSelection(d->document->rootFrame()->begin(), visitor);

    if (!isEditProtected() && caretAnchor == caretPosition) { //if there is no selection, it can happen that the caret does not get the proper style applied (beginning of a block). We need to force it.
         //applying a style is absolute, so first initialise the caret with the frame's style, then apply the paragraph's. Finally apply the character style
        QTextCharFormat charFormat = KoTextDocument(d->document).frameCharFormat();
        KoStyleManager *styleManager = KoTextDocument(d->document).styleManager();
        KoParagraphStyle *paragraphStyle = styleManager->paragraphStyle(d->caret.charFormat().intProperty(KoParagraphStyle::StyleId));
        if (paragraphStyle) {
            paragraphStyle->KoCharacterStyle::applyStyle(charFormat);
        }
        d->caret.setCharFormat(charFormat);
        style->applyStyle(&(d->caret));
    }
    else { //if the caret has a selection, the visitor has already applied the style, reset the caret's position so it picks the proper style.
        d->caret.setPosition(caretAnchor);
        d->caret.setPosition(caretPosition, QTextCursor::KeepAnchor);
    }

    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
    emit characterStyleApplied(style);
}


class SetParagraphStyleVisitor : public KoTextVisitor
{
public:
    SetParagraphStyleVisitor(KoTextEditor *editor, KoStyleManager *styleManager, KoParagraphStyle *style)
        : KoTextVisitor(editor)
        , m_styleManager(styleManager)
        , m_style(style)
    {
    }

    virtual void visitBlock(QTextBlock &block, const QTextCursor &)
    {
        if (m_styleManager) {
            QTextBlockFormat bf = block.blockFormat();
            KoParagraphStyle *old = m_styleManager->paragraphStyle(bf.intProperty(KoParagraphStyle::StyleId));
            if (old)
                old->unapplyStyle(block);
        }
        // The above should unapply the style and it's lists part, but we want to clear everything
        // except section info.
        QTextCursor cursor(block);
        QVariant sectionStartings = cursor.blockFormat().property(KoParagraphStyle::SectionStartings);
        QVariant sectionEndings = cursor.blockFormat().property(KoParagraphStyle::SectionEndings);
        QTextBlockFormat fmt;
        fmt.setProperty(KoParagraphStyle::SectionStartings, sectionStartings);
        fmt.setProperty(KoParagraphStyle::SectionEndings, sectionEndings);
        cursor.setBlockFormat(fmt);
        m_style->applyStyle(block);
    }

    KoStyleManager *m_styleManager;
    KoParagraphStyle *m_style;
};

void KoTextEditor::setStyle(KoParagraphStyle *style)
{
    d->updateState(KoTextEditor::Private::Custom, kundo2_i18n("Set Paragraph Style"));

    int caretAnchor = d->caret.anchor();
    int caretPosition = d->caret.position();
    KoStyleManager *styleManager = KoTextDocument(d->document).styleManager();
    SetParagraphStyleVisitor visitor(this, styleManager, style);

    recursivelyVisitSelection(d->document->rootFrame()->begin(), visitor);

    if (!isEditProtected() && caretAnchor == caretPosition) { //if there is no selection, it can happen that the caret does not get the proper style applied (beginning of a block). We need to force it.
        //applying a style is absolute, so first initialise the caret with the frame's style, then apply the paragraph style
        QTextCharFormat charFormat = KoTextDocument(d->document).frameCharFormat();
        d->caret.setCharFormat(charFormat);
        style->KoCharacterStyle::applyStyle(&(d->caret));
    }
    else {
        d->caret.setPosition(caretAnchor);
        d->caret.setPosition(caretPosition, QTextCursor::KeepAnchor);
    }

    d->updateState(KoTextEditor::Private::NoOp);
    emit paragraphStyleApplied(style);
    emit textFormatChanged();
}

class MergeAutoCharacterStyleVisitor : public KoTextVisitor
{
public:
    MergeAutoCharacterStyleVisitor(KoTextEditor *editor, QTextCharFormat deltaCharFormat)
        : KoTextVisitor(editor)
        , m_deltaCharFormat(deltaCharFormat)
    {
    }

    virtual void visitBlock(QTextBlock &block, const QTextCursor &caret)
    {
        KoTextVisitor::visitBlock(block, caret);

        QList<QTextCharFormat>::Iterator it = m_formats.begin();
        Q_FOREACH (QTextCursor cursor, m_cursors) {
            QTextFormat prevFormat(cursor.charFormat());
            cursor.setCharFormat(*it);
            editor()->registerTrackedChange(cursor, KoGenChange::FormatChange, kundo2_i18n("Formatting"), *it, prevFormat, false);
            ++it;
        }
    }

    virtual void visitFragmentSelection(QTextCursor &fragmentSelection)
    {
        QTextCharFormat format = fragmentSelection.charFormat();
        format.merge(m_deltaCharFormat);

        m_formats.append(format);
        m_cursors.append(fragmentSelection);
    }

    QTextCharFormat m_deltaCharFormat;
    QList<QTextCharFormat> m_formats;
    QList<QTextCursor> m_cursors;
};

void KoTextEditor::mergeAutoStyle(const QTextCharFormat &deltaCharFormat)
{
    d->updateState(KoTextEditor::Private::Custom, kundo2_i18n("Formatting"));

    int caretAnchor = d->caret.anchor();
    int caretPosition = d->caret.position();
    MergeAutoCharacterStyleVisitor visitor(this, deltaCharFormat);

    recursivelyVisitSelection(d->document->rootFrame()->begin(), visitor);

    if (!isEditProtected() && caretAnchor == caretPosition) { //if there is no selection, it can happen that the caret does not get the proper style applied (beginning of a block). We need to force it.
        d->caret.mergeCharFormat(deltaCharFormat);
    }
    else {
        d->caret.setPosition(caretAnchor);
        d->caret.setPosition(caretPosition, QTextCursor::KeepAnchor);
    }

    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}


void KoTextEditor::applyDirectFormatting(const QTextCharFormat &deltaCharFormat,
                                  const QTextBlockFormat &deltaBlockFormat,
                                  const KoListLevelProperties &llp)
{
    addCommand(new ParagraphFormattingCommand(this, deltaCharFormat, deltaBlockFormat, llp));
    emit textFormatChanged();
}

QTextCharFormat KoTextEditor::blockCharFormat() const
{
    return d->caret.blockCharFormat();
}

QTextBlockFormat KoTextEditor::blockFormat() const
{
     return d->caret.blockFormat();
}

QTextCharFormat KoTextEditor::charFormat() const
{
    return d->caret.charFormat();
}


void KoTextEditor::mergeBlockFormat(const QTextBlockFormat &modifier)
{
    if (isEditProtected()) {
        return;
    }
    d->caret.mergeBlockFormat(modifier);
    emit textFormatChanged();
}


void KoTextEditor::setBlockFormat(const QTextBlockFormat &format)
{
    if (isEditProtected()) {
        return;
    }

    Q_UNUSED(format)
    d->caret.setBlockFormat(format);
    emit textFormatChanged();
}

void KoTextEditor::setCharFormat(const QTextCharFormat &format)
{
    if (isEditProtected()) {
        return;
    }

    d->caret.setCharFormat(format);
    emit textFormatChanged();
}
