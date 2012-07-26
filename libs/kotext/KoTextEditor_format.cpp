/* This file is part of the KDE project
 * Copyright (C) 2009-2012 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (c) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2011-2012 C. Boemann <cbo@boemann.dk>
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

#include "changetracker/KoChangeTracker.h"
#include "changetracker/KoChangeTrackerElement.h"
#include "changetracker/KoDeleteChangeMarker.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoStyleManager.h"
#include "commands/TextPasteCommand.h"

#include <KLocale>
#include <kundo2stack.h>

#include <QApplication>
#include <QFontDatabase>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextFormat>
#include <QTextTable>
#include <QTextTableCell>
#include <QTimer>
#include <QString>
#include <kundo2command.h>

#include <kdebug.h>
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
    CharFormatVisitor::visitSelection(q, wiper,QString(), false);
}

void KoTextEditor::bold(bool bold)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, i18nc("(qtundo-format)", "Bold"));
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);

    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KoGenChange::FormatChange, i18nc("(qtundo-format)", "Bold"), format, prevFormat, false);
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

void KoTextEditor::italic(bool italic)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, i18nc("(qtundo-format)", "Italic"));
    QTextCharFormat format;
    format.setFontItalic(italic);

    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KoGenChange::FormatChange, i18nc("(qtundo-format)", "Italic"), format, prevFormat, false);
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

void KoTextEditor::underline(bool underline)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, i18nc("(qtundo-format)", "Underline"));
    QTextCharFormat format;
    if (underline) {
        format.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::SingleLine);
        format.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::SolidLine);
    } else {
        format.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::NoLineType);
        format.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::NoLineStyle);
    }

    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KoGenChange::FormatChange, i18nc("(qtundo-format)", "Underline"), format, prevFormat, false);
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

void KoTextEditor::strikeOut(bool strikeout)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, i18nc("(qtundo-format)", "Strike Out"));
    QTextCharFormat format;
    if (strikeout) {
        format.setProperty(KoCharacterStyle::StrikeOutType, KoCharacterStyle::SingleLine);
        format.setProperty(KoCharacterStyle::StrikeOutStyle, KoCharacterStyle::SolidLine);
    } else {
        format.setProperty(KoCharacterStyle::StrikeOutType, KoCharacterStyle::NoLineType);
        format.setProperty(KoCharacterStyle::StrikeOutStyle, KoCharacterStyle::NoLineStyle);
    }
    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KoGenChange::FormatChange, i18nc("(qtundo-format)", "Strike Out"), format, prevFormat, false);
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
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
    d->updateState(KoTextEditor::Private::Format, i18nc("(qtundo-format)", "Change Alignment"));
    BlockFormatVisitor::visitSelection(this, aligner, i18nc("(qtundo-format)", "Change Alignment"));
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

    d->updateState(KoTextEditor::Private::Format, i18n("Set Vertical Alignment"));
    QTextCharFormat format;
    format.setVerticalAlignment(charAlign);
    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KoGenChange::FormatChange, i18n("Set Vertical Alignment"), format, prevFormat, false);
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
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
    d->updateState(KoTextEditor::Private::Format, i18n("Decrease Indent"));
    BlockFormatVisitor::visitSelection(this, indenter, i18n("Decrease Indent"));
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
    d->updateState(KoTextEditor::Private::Format, i18n("Increase Indent"));
    BlockFormatVisitor::visitSelection(this, indenter, i18n("Increase Indent"));
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
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, i18n("Decrease font size"));
    FontResizer sizer(FontResizer::Shrink);
    CharFormatVisitor::visitSelection(this, sizer, i18n("Decrease font size"));
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

void KoTextEditor::increaseFontSize()
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, i18n("Increase font size"));
    FontResizer sizer(FontResizer::Grow);
    CharFormatVisitor::visitSelection(this, sizer, i18n("Increase font size"));
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

void KoTextEditor::setFontFamily(const QString &font)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, i18n("Set Font"));
    QTextCharFormat format;
    format.setFontFamily(font);
    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KoGenChange::FormatChange, i18n("Set Font"), format, prevFormat, false);
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

void KoTextEditor::setFontSize(qreal size)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, i18n("Set Font Size"));
    QTextCharFormat format;
    format.setFontPointSize(size);
    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KoGenChange::FormatChange, i18n("Set Font Size"), format, prevFormat, false);
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

void KoTextEditor::setTextBackgroundColor(const QColor &color)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, i18n("Set Background Color"));
    QTextCharFormat format;
    format.setBackground(QBrush(color));
    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KoGenChange::FormatChange, i18n("Set Background Color"), format, prevFormat, false);
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

void KoTextEditor::setTextColor(const QColor &color)
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Format, i18n("Set Text Color"));
    QTextCharFormat format;
    format.setForeground(QBrush(color));
    QTextCharFormat prevFormat(d->caret.charFormat());
    d->caret.mergeCharFormat(format);
    registerTrackedChange(d->caret, KoGenChange::FormatChange, i18n("Set Text Color"), format, prevFormat, false);
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

class SetCharacterStyleVisitor : public KoTextVisitor
{
public:
    SetCharacterStyleVisitor(KoTextEditor *editor, KoCharacterStyle *style)
        : KoTextVisitor(editor)
        , m_style(style)
    {
    }

    virtual void visitBlock(QTextBlock block, const QTextCursor &caret)
    {
        m_newFormat = block.charFormat();
        m_style->applyStyle(m_newFormat);
        m_style->ensureMinimalProperties(m_newFormat);

        KoTextVisitor::visitBlock(block, caret);

        QList<QTextCharFormat>::Iterator it = m_formats.begin();
        foreach(QTextCursor cursor, m_cursors) {
            QTextFormat prevFormat(cursor.charFormat());
            cursor.setCharFormat(*it);
            editor()->registerTrackedChange(cursor, KoGenChange::FormatChange, i18n("Set Character Style"), *it, prevFormat, false);
            ++it;
        }
    }

    virtual void visitFragmentSelection(QTextCursor fragmentSelection)
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
    d->updateState(KoTextEditor::Private::Custom, i18n("Set Character Style"));

    SetCharacterStyleVisitor visitor(this, style);

    recursivelyVisitSelection(d->document->rootFrame()->begin(), visitor);
    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
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

    virtual void visitBlock(QTextBlock block, const QTextCursor &)
    {
        if (m_styleManager) {
            QTextBlockFormat bf = block.blockFormat();
            KoParagraphStyle *old = m_styleManager->paragraphStyle(bf.intProperty(KoParagraphStyle::StyleId));
            if (old)
                old->unapplyStyle(block);
        }
        // above should unaaply the style and it's lists part, but we want to clear everything
        QTextCursor cursor(block);
        cursor.setBlockFormat(QTextBlockFormat());
        m_style->applyStyle(block);
    }

    KoStyleManager *m_styleManager;
    KoParagraphStyle *m_style;
};

void KoTextEditor::setStyle(KoParagraphStyle *style)
{
    d->updateState(KoTextEditor::Private::Custom, i18n("Set Paragraph Style"));

    KoStyleManager *styleManager = KoTextDocument(d->document).styleManager();
    SetParagraphStyleVisitor visitor(this, styleManager, style);

    recursivelyVisitSelection(d->document->rootFrame()->begin(), visitor);

    d->updateState(KoTextEditor::Private::NoOp);
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

    virtual void visitBlock(QTextBlock block, const QTextCursor &caret)
    {
        KoTextVisitor::visitBlock(block, caret);

        QList<QTextCharFormat>::Iterator it = m_formats.begin();
        foreach(QTextCursor cursor, m_cursors) {
            QTextFormat prevFormat(cursor.charFormat());
            cursor.setCharFormat(*it);
            editor()->registerTrackedChange(cursor, KoGenChange::FormatChange, i18n("Formatting"), *it, prevFormat, false);
            ++it;
        }
    }

    virtual void visitFragmentSelection(QTextCursor fragmentSelection)
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

void KoTextEditor::mergeAutoStyle(QTextCharFormat deltaCharFormat)
{
    d->updateState(KoTextEditor::Private::Custom, "Formatting");

    MergeAutoCharacterStyleVisitor visitor(this, deltaCharFormat);

    recursivelyVisitSelection(d->document->rootFrame()->begin(), visitor);

    d->updateState(KoTextEditor::Private::NoOp);
    emit textFormatChanged();
}

class MergeAutoParagraphStyleVisitor : public KoTextVisitor
{
public:
    MergeAutoParagraphStyleVisitor(KoTextEditor *editor, QTextCharFormat deltaCharFormat, QTextBlockFormat deltaBlockFormat)
        : KoTextVisitor(editor)
        , m_deltaCharFormat(deltaCharFormat)
        , m_deltaBlockFormat(deltaBlockFormat)
    {
    }

    virtual void visitBlock(QTextBlock block, const QTextCursor &caret)
    {
        for (QTextBlock::iterator it = block.begin(); it != block.end(); ++it) {
            QTextCursor fragmentSelection(caret);
            fragmentSelection.setPosition(it.fragment().position());
            fragmentSelection.setPosition(it.fragment().position() + it.fragment().length(), QTextCursor::KeepAnchor);

            if (fragmentSelection.anchor() >= fragmentSelection.position()) {
                continue;
            }

            visitFragmentSelection(fragmentSelection);
        }

        QList<QTextCharFormat>::Iterator it = m_formats.begin();
        foreach(QTextCursor cursor, m_cursors) {
            QTextFormat prevFormat(cursor.charFormat());
            cursor.setCharFormat(*it);
            editor()->registerTrackedChange(cursor, KoGenChange::FormatChange, i18n("Formatting"), *it, prevFormat, false);
            ++it;
        }
        QTextCursor cursor(caret);
        cursor.mergeBlockFormat(m_deltaBlockFormat);
        cursor.mergeBlockCharFormat(m_deltaCharFormat);
    }

    virtual void visitFragmentSelection(QTextCursor fragmentSelection)
    {
        QTextCharFormat format = fragmentSelection.charFormat();
        format.merge(m_deltaCharFormat);

        m_formats.append(format);
        m_cursors.append(fragmentSelection);
    }

    QTextCharFormat m_deltaCharFormat;
    QTextBlockFormat m_deltaBlockFormat;
    QList<QTextCharFormat> m_formats;
    QList<QTextCursor> m_cursors;
};

void KoTextEditor::mergeAutoStyle(QTextCharFormat deltaCharFormat, QTextBlockFormat deltaBlockFormat)
{
    d->updateState(KoTextEditor::Private::Custom, "Formatting");

    MergeAutoParagraphStyleVisitor visitor(this, deltaCharFormat, deltaBlockFormat);

    recursivelyVisitSelection(d->document->rootFrame()->begin(), visitor);

    d->updateState(KoTextEditor::Private::NoOp);
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
