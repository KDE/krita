/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007,2008 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007-2011 Pierre Ducroquet <pinaraf@pinaraf.info>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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
#include "KoParagraphStyle.h"

#include "KoList.h"
#include "KoListStyle.h"
#include "KoTextBlockData.h"
#include "KoStyleManager.h"
#include "KoListLevelProperties.h"
#include "KoTextSharedLoadingData.h"
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include "Styles_p.h"
#include "KoTextDocument.h"

#include "TextDebug.h"

#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QBuffer>

#include <KoUnit.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoBorder.h>
#include <KoShadowStyle.h>

//already defined in KoRulerController.cpp
#ifndef KDE_USE_FINAL
struct {
    bool operator()(KoText::Tab tab1, KoText::Tab tab2) const
    {
        return tab1.position < tab2.position;
    }
} compareTabs;
#endif
class Q_DECL_HIDDEN KoParagraphStyle::Private
{
public:
    Private() : parentStyle(0), defaultStyle(0), list(0), m_inUse(false) {}

    ~Private()
    {
    }

    void setProperty(int key, const QVariant &value)
    {
        stylesPrivate.add(key, value);
    }

    void ensureDefaults(QTextBlockFormat &format)
    {
        if (defaultStyle) {
            QMap<int, QVariant> props = defaultStyle->d->stylesPrivate.properties();
            QMap<int, QVariant>::const_iterator it = props.constBegin();
            while (it != props.constEnd()) {
                if (!it.value().isNull() && !format.hasProperty(it.key())) {
                    format.setProperty(it.key(), it.value());
                }
                ++it;
            }
        }
    }

    QString name;
    KoParagraphStyle *parentStyle;
    KoParagraphStyle *defaultStyle;
    KoList *list;
    StylePrivate stylesPrivate;
    bool m_inUse;
};

KoParagraphStyle::KoParagraphStyle(QObject *parent)
        : KoCharacterStyle(parent), d(new Private())
{
}

KoParagraphStyle::KoParagraphStyle(const QTextBlockFormat &blockFormat, const QTextCharFormat &blockCharFormat, QObject *parent)
        : KoCharacterStyle(blockCharFormat, parent),
        d(new Private())
{
    d->stylesPrivate = blockFormat.properties();
}

KoParagraphStyle *KoParagraphStyle::fromBlock(const QTextBlock &block, QObject *parent)
{
    QTextBlockFormat blockFormat = block.blockFormat();
    QTextCursor cursor(block);
    KoParagraphStyle *answer = new KoParagraphStyle(blockFormat, cursor.blockCharFormat(), parent);

    int listStyleId = blockFormat.intProperty(ListStyleId);
    KoStyleManager *sm = KoTextDocument(block.document()).styleManager();
    if (KoListStyle *listStyle = sm->listStyle(listStyleId)) {
        answer->setListStyle(listStyle->clone(answer));
    } else if (block.textList()) {
        KoListLevelProperties llp = KoListLevelProperties::fromTextList(block.textList());
        KoListStyle *listStyle = new KoListStyle(answer);
        listStyle->setLevelProperties(llp);
        answer->setListStyle(listStyle);
    }
    return answer;
}

KoParagraphStyle::~KoParagraphStyle()
{
    delete d;
}

KoCharacterStyle::Type KoParagraphStyle::styleType() const
{
    return KoCharacterStyle::ParagraphStyle;
}

void KoParagraphStyle::setDefaultStyle(KoParagraphStyle *defaultStyle)
{
    d->defaultStyle = defaultStyle;
    KoCharacterStyle::setDefaultStyle(defaultStyle);
}

void KoParagraphStyle::setParentStyle(KoParagraphStyle *parent)
{
    d->parentStyle = parent;
    KoCharacterStyle::setParentStyle(parent);
}

void KoParagraphStyle::setProperty(int key, const QVariant &value)
{
    if (d->parentStyle) {
        QVariant var = d->parentStyle->value(key);
        if (!var.isNull() && var == value) { // same as parent, so its actually a reset.
            d->stylesPrivate.remove(key);
            return;
        }
    }
    d->stylesPrivate.add(key, value);
}

void KoParagraphStyle::remove(int key)
{
    d->stylesPrivate.remove(key);
}

QVariant KoParagraphStyle::value(int key) const
{
    QVariant var = d->stylesPrivate.value(key);
    if (var.isNull()) {
        if (d->parentStyle)
            return d->parentStyle->value(key);
        else if (d->defaultStyle)
            return d->defaultStyle->value(key);
    }
    return var;
}

bool KoParagraphStyle::hasProperty(int key) const
{
    return d->stylesPrivate.contains(key);
}

qreal KoParagraphStyle::propertyDouble(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0.0;
    return variant.toDouble();
}

QTextLength KoParagraphStyle::propertyLength(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return QTextLength(QTextLength::FixedLength, 0.0);
    if (!variant.canConvert<QTextLength>())
    {
        // Fake support, for compatibility sake
        if (variant.canConvert<qreal>())
        {
            return QTextLength(QTextLength::FixedLength, variant.toReal());
        }

        warnText << "This should never happen : requested property can't be converted to QTextLength";
        return QTextLength(QTextLength::FixedLength, 0.0);
    }
    return variant.value<QTextLength>();
}

int KoParagraphStyle::propertyInt(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0;
    return variant.toInt();
}

bool KoParagraphStyle::propertyBoolean(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return false;
    return variant.toBool();
}

QColor KoParagraphStyle::propertyColor(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull()) {
        return QColor();
    }
    return qvariant_cast<QColor>(variant);
}

void KoParagraphStyle::applyStyle(QTextBlockFormat &format) const
{
    if (d->parentStyle) {
        d->parentStyle->applyStyle(format);
    }

    const QMap<int, QVariant> props = d->stylesPrivate.properties();
    QMap<int, QVariant>::const_iterator it = props.begin();
    while (it != props.end()) {
        if (it.key() == QTextBlockFormat::BlockLeftMargin) {
            format.setLeftMargin(leftMargin());
        } else if (it.key() == QTextBlockFormat::BlockRightMargin) {
            format.setRightMargin(rightMargin());
        } else if (it.key() == QTextBlockFormat::TextIndent) {
            format.setTextIndent(textIndent());
        } else {
            format.setProperty(it.key(), it.value());
        }
        ++it;
    }
    if ((hasProperty(DefaultOutlineLevel)) && (!format.hasProperty(OutlineLevel))) {
       format.setProperty(OutlineLevel, defaultOutlineLevel());
    }
    emit styleApplied(this);
    d->m_inUse = true;
}

void KoParagraphStyle::applyStyle(QTextBlock &block, bool applyListStyle) const
{
    QTextCursor cursor(block);
    QTextBlockFormat format = cursor.blockFormat();
    applyStyle(format);
    d->ensureDefaults(format);
    cursor.setBlockFormat(format);

    KoCharacterStyle::applyStyle(block);

    if (applyListStyle) {
        applyParagraphListStyle(block, format);
    }
}

bool KoParagraphStyle::isApplied() const
{
    return d->m_inUse;
}

void KoParagraphStyle::applyParagraphListStyle(QTextBlock &block, const QTextBlockFormat &blockFormat) const
{
    //gopalakbhat: We need to differentiate between normal styles and styles with outline(part of heading)
    //Styles part of outline: We ignore the listStyle()( even if this is a valid in ODF; even LibreOffice does the same)
    //                        since we can specify all info required in text:outline-style
    //Normal styles: we use the listStyle()
    if (blockFormat.hasProperty(OutlineLevel)) {
        if (! d->list) {
            if (! KoTextDocument(block.document()).headingList()) {
                if (KoTextDocument(block.document()).styleManager() && KoTextDocument(block.document()).styleManager()->outlineStyle()) {
                    d->list = new KoList(block.document(), KoTextDocument(block.document()).styleManager()->outlineStyle());
                    KoTextDocument(block.document()).setHeadingList(d->list);
                }
            } else {
                d->list = KoTextDocument(block.document()).headingList();
            }
        }
        if (d->list) {
            d->list->applyStyle(block, KoTextDocument(block.document()).styleManager()->outlineStyle(), blockFormat.intProperty(OutlineLevel));
        }
    } else {
        if (listStyle()) {
            if (!d->list) {
                d->list = new KoList(block.document(), listStyle());
            }
            //FIXME: Gopalakrishna Bhat A: This condition should never happen.
            // i.e. d->list->style() should always be in sync with the listStyle()
            if (d->list->style() != listStyle()) {
                d->list->setStyle(listStyle());
            }
            d->list->add(block, listLevel());
        } else {
            if (block.textList())
                block.textList()->remove(block);
            KoTextBlockData data(block);
            data.setCounterWidth(-1);
        }
    }
}

void KoParagraphStyle::unapplyStyle(QTextBlock &block) const
{
    if (d->parentStyle)
        d->parentStyle->unapplyStyle(block);

    QTextCursor cursor(block);
    QTextBlockFormat format = cursor.blockFormat();

    QList<int> keys = d->stylesPrivate.keys();
    for (int i = 0; i < keys.count(); i++) {
        QVariant variant = d->stylesPrivate.value(keys[i]);
        if (keys[i] == QTextBlockFormat::BlockLeftMargin) {
            if (leftMargin() == format.property(keys[i]))
                format.clearProperty(keys[i]);
        } else if (keys[i] == QTextBlockFormat::BlockRightMargin) {
            if (rightMargin() == format.property(keys[i]))
                format.clearProperty(keys[i]);
        } else if (keys[i] == QTextBlockFormat::TextIndent) {
            if (textIndent() == format.property(keys[i]))
                format.clearProperty(keys[i]);
        } else {
            if (variant == format.property(keys[i]))
                format.clearProperty(keys[i]);
        }
    }

    format.clearProperty(KoParagraphStyle::OutlineLevel);

    cursor.setBlockFormat(format);
    KoCharacterStyle::unapplyStyle(block);
    if (listStyle() && block.textList()) { // TODO check its the same one?
        KoList::remove(block);
    }
    if (d->list && block.textList()) { // TODO check its the same one?
        KoList::remove(block);
    }
}

void KoParagraphStyle::setLineHeightPercent(qreal lineHeight)
{
    setProperty(PercentLineHeight, lineHeight);
    setProperty(FixedLineHeight, 0.0);
    setProperty(MinimumLineHeight, QTextLength(QTextLength::FixedLength, 0.0));
    remove(NormalLineHeight);
}

qreal KoParagraphStyle::lineHeightPercent() const
{
    return propertyInt(PercentLineHeight);
}

void KoParagraphStyle::setLineHeightAbsolute(qreal height)
{
    setProperty(FixedLineHeight, height);
    setProperty(PercentLineHeight, 0);
    setProperty(MinimumLineHeight, QTextLength(QTextLength::FixedLength, 0.0));
    remove(NormalLineHeight);
}

qreal KoParagraphStyle::lineHeightAbsolute() const
{
    return propertyDouble(FixedLineHeight);
}

void KoParagraphStyle::setMinimumLineHeight(const QTextLength &height)
{
    setProperty(FixedLineHeight, 0.0);
    setProperty(PercentLineHeight, 0);
    setProperty(MinimumLineHeight, height);
    remove(NormalLineHeight);
}

qreal KoParagraphStyle::minimumLineHeight() const
{
    if (parentStyle())
        return propertyLength(MinimumLineHeight).value(parentStyle()->minimumLineHeight());
    else
        return propertyLength(MinimumLineHeight).value(0);
}

void KoParagraphStyle::setLineSpacing(qreal spacing)
{
    setProperty(LineSpacing, spacing);
    remove(NormalLineHeight);
}

qreal KoParagraphStyle::lineSpacing() const
{
    return propertyDouble(LineSpacing);
}

void KoParagraphStyle::setLineSpacingFromFont(bool on)
{
    setProperty(LineSpacingFromFont, on);
    remove(NormalLineHeight);
}

bool KoParagraphStyle::lineSpacingFromFont() const
{
    return propertyBoolean(LineSpacingFromFont);
}

void KoParagraphStyle::setNormalLineHeight()
{
    setProperty(NormalLineHeight, true);
    setProperty(PercentLineHeight, 0);
    setProperty(FixedLineHeight, 0.0);
    setProperty(MinimumLineHeight, QTextLength(QTextLength::FixedLength, 0.0));
    setProperty(LineSpacing, 0.0);
}

bool KoParagraphStyle::hasNormalLineHeight() const
{
    return propertyBoolean(NormalLineHeight);
}

void KoParagraphStyle::setAlignLastLine(Qt::Alignment alignment)
{
    setProperty(AlignLastLine, (int) alignment);
}

Qt::Alignment KoParagraphStyle::alignLastLine() const
{
    if (hasProperty(AlignLastLine))
        return static_cast<Qt::Alignment>(propertyInt(AlignLastLine));
    // Hum, that doesn't sound right !
    return alignment();
}

void KoParagraphStyle::setWidowThreshold(int lines)
{
    setProperty(WidowThreshold, lines);
}

int KoParagraphStyle::widowThreshold() const
{
    return propertyInt(WidowThreshold);
}

void KoParagraphStyle::setOrphanThreshold(int lines)
{
    setProperty(OrphanThreshold, lines);
}

int KoParagraphStyle::orphanThreshold() const
{
    return propertyInt(OrphanThreshold);
}

void KoParagraphStyle::setDropCaps(bool on)
{
    setProperty(DropCaps, on);
}

bool KoParagraphStyle::dropCaps() const
{
    return propertyBoolean(DropCaps);
}

void KoParagraphStyle::setDropCapsLength(int characters)
{
    setProperty(DropCapsLength, characters);
}

int KoParagraphStyle::dropCapsLength() const
{
    return propertyInt(DropCapsLength);
}

void KoParagraphStyle::setDropCapsLines(int lines)
{
    setProperty(DropCapsLines, lines);
}

int KoParagraphStyle::dropCapsLines() const
{
    return propertyInt(DropCapsLines);
}

void KoParagraphStyle::setDropCapsDistance(qreal distance)
{
    setProperty(DropCapsDistance, distance);
}

qreal KoParagraphStyle::dropCapsDistance() const
{
    return propertyDouble(DropCapsDistance);
}

void KoParagraphStyle::setDropCapsTextStyleId(int id)
{
    setProperty(KoParagraphStyle::DropCapsTextStyle, id);
}

int KoParagraphStyle::dropCapsTextStyleId() const
{
    return propertyInt(KoParagraphStyle::DropCapsTextStyle);
}

void KoParagraphStyle::setFollowDocBaseline(bool on)
{
    setProperty(FollowDocBaseline, on);
}

bool KoParagraphStyle::followDocBaseline() const
{
    return propertyBoolean(FollowDocBaseline);
}

void KoParagraphStyle::setBreakBefore(KoText::KoTextBreakProperty value)
{
    setProperty(BreakBefore, value);
}

KoText::KoTextBreakProperty KoParagraphStyle::breakBefore() const
{
    return static_cast<KoText::KoTextBreakProperty>(propertyInt(BreakBefore));
}

void KoParagraphStyle::setBreakAfter(KoText::KoTextBreakProperty value)
{
    setProperty(BreakAfter, value);
}

KoText::KoTextBreakProperty KoParagraphStyle::breakAfter() const
{
    return static_cast<KoText::KoTextBreakProperty>(propertyInt(BreakAfter));
}

void KoParagraphStyle::setLeftPadding(qreal padding)
{
    setProperty(LeftPadding, padding);
}

qreal KoParagraphStyle::leftPadding() const
{
    return propertyDouble(LeftPadding);
}

void KoParagraphStyle::setTopPadding(qreal padding)
{
    setProperty(TopPadding, padding);
}

qreal KoParagraphStyle::topPadding() const
{
    return propertyDouble(TopPadding);
}

void KoParagraphStyle::setRightPadding(qreal padding)
{
    setProperty(RightPadding, padding);
}

qreal KoParagraphStyle::rightPadding() const
{
    return propertyDouble(RightPadding);
}

void KoParagraphStyle::setBottomPadding(qreal padding)
{
    setProperty(BottomPadding, padding);
}

qreal KoParagraphStyle::bottomPadding() const
{
    return propertyDouble(BottomPadding);
}

void KoParagraphStyle::setPadding(qreal padding)
{
    setBottomPadding(padding);
    setTopPadding(padding);
    setRightPadding(padding);
    setLeftPadding(padding);
}

void KoParagraphStyle::setLeftBorderWidth(qreal width)
{
    setProperty(LeftBorderWidth, width);
}

qreal KoParagraphStyle::leftBorderWidth() const
{
    return propertyDouble(LeftBorderWidth);
}

void KoParagraphStyle::setLeftInnerBorderWidth(qreal width)
{
    setProperty(LeftInnerBorderWidth, width);
}

qreal KoParagraphStyle::leftInnerBorderWidth() const
{
    return propertyDouble(LeftInnerBorderWidth);
}

void KoParagraphStyle::setLeftBorderSpacing(qreal width)
{
    setProperty(LeftBorderSpacing, width);
}

qreal KoParagraphStyle::leftBorderSpacing() const
{
    return propertyDouble(LeftBorderSpacing);
}

void KoParagraphStyle::setLeftBorderStyle(KoBorder::BorderStyle style)
{
    setProperty(LeftBorderStyle, style);
}

KoBorder::BorderStyle KoParagraphStyle::leftBorderStyle() const
{
    return static_cast<KoBorder::BorderStyle>(propertyInt(LeftBorderStyle));
}

void KoParagraphStyle::setLeftBorderColor(const QColor &color)
{
    setProperty(LeftBorderColor, color);
}

QColor KoParagraphStyle::leftBorderColor() const
{
    return propertyColor(LeftBorderColor);
}

void KoParagraphStyle::setTopBorderWidth(qreal width)
{
    setProperty(TopBorderWidth, width);
}

qreal KoParagraphStyle::topBorderWidth() const
{
    return propertyDouble(TopBorderWidth);
}

void KoParagraphStyle::setTopInnerBorderWidth(qreal width)
{
    setProperty(TopInnerBorderWidth, width);
}

qreal KoParagraphStyle::topInnerBorderWidth() const
{
    return propertyDouble(TopInnerBorderWidth);
}

void KoParagraphStyle::setTopBorderSpacing(qreal width)
{
    setProperty(TopBorderSpacing, width);
}

qreal KoParagraphStyle::topBorderSpacing() const
{
    return propertyDouble(TopBorderSpacing);
}

void KoParagraphStyle::setTopBorderStyle(KoBorder::BorderStyle style)
{
    setProperty(TopBorderStyle, style);
}

KoBorder::BorderStyle KoParagraphStyle::topBorderStyle() const
{
    return static_cast<KoBorder::BorderStyle>(propertyInt(TopBorderStyle));
}

void KoParagraphStyle::setTopBorderColor(const QColor &color)
{
    setProperty(TopBorderColor, color);
}

QColor KoParagraphStyle::topBorderColor() const
{
    return propertyColor(TopBorderColor);
}

void KoParagraphStyle::setRightBorderWidth(qreal width)
{
    setProperty(RightBorderWidth, width);
}

qreal KoParagraphStyle::rightBorderWidth() const
{
    return propertyDouble(RightBorderWidth);
}

void KoParagraphStyle::setRightInnerBorderWidth(qreal width)
{
    setProperty(RightInnerBorderWidth, width);
}

qreal KoParagraphStyle::rightInnerBorderWidth() const
{
    return propertyDouble(RightInnerBorderWidth);
}

void KoParagraphStyle::setRightBorderSpacing(qreal width)
{
    setProperty(RightBorderSpacing, width);
}

qreal KoParagraphStyle::rightBorderSpacing() const
{
    return propertyDouble(RightBorderSpacing);
}

void KoParagraphStyle::setRightBorderStyle(KoBorder::BorderStyle style)
{
    setProperty(RightBorderStyle, style);
}

KoBorder::BorderStyle KoParagraphStyle::rightBorderStyle() const
{
    return static_cast<KoBorder::BorderStyle>(propertyInt(RightBorderStyle));
}

void KoParagraphStyle::setRightBorderColor(const QColor &color)
{
    setProperty(RightBorderColor, color);
}

QColor KoParagraphStyle::rightBorderColor() const
{
    return propertyColor(RightBorderColor);
}

void KoParagraphStyle::setBottomBorderWidth(qreal width)
{
    setProperty(BottomBorderWidth, width);
}

qreal KoParagraphStyle::bottomBorderWidth() const
{
    return propertyDouble(BottomBorderWidth);
}

void KoParagraphStyle::setBottomInnerBorderWidth(qreal width)
{
    setProperty(BottomInnerBorderWidth, width);
}

qreal KoParagraphStyle::bottomInnerBorderWidth() const
{
    return propertyDouble(BottomInnerBorderWidth);
}

void KoParagraphStyle::setBottomBorderSpacing(qreal width)
{
    setProperty(BottomBorderSpacing, width);
}

qreal KoParagraphStyle::bottomBorderSpacing() const
{
    return propertyDouble(BottomBorderSpacing);
}

void KoParagraphStyle::setBottomBorderStyle(KoBorder::BorderStyle style)
{
    setProperty(BottomBorderStyle, style);
}

KoBorder::BorderStyle KoParagraphStyle::bottomBorderStyle() const
{
    return static_cast<KoBorder::BorderStyle>(propertyInt(BottomBorderStyle));
}

void KoParagraphStyle::setBottomBorderColor(const QColor &color)
{
    setProperty(BottomBorderColor, color);
}

QColor KoParagraphStyle::bottomBorderColor() const
{
    return propertyColor(BottomBorderColor);
}

void KoParagraphStyle::setTopMargin(QTextLength topMargin)
{
    setProperty(QTextFormat::BlockTopMargin, topMargin);
}

qreal KoParagraphStyle::topMargin() const
{
    if (parentStyle())
        return propertyLength(QTextFormat::BlockTopMargin).value(parentStyle()->topMargin());
    else
        return propertyLength(QTextFormat::BlockTopMargin).value(0);
}

void KoParagraphStyle::setBottomMargin(QTextLength margin)
{
    setProperty(QTextFormat::BlockBottomMargin, margin);
}

qreal KoParagraphStyle::bottomMargin() const
{
    if (parentStyle())
        return propertyLength(QTextFormat::BlockBottomMargin).value(parentStyle()->bottomMargin());
    else
        return propertyLength(QTextFormat::BlockBottomMargin).value(0);
}

void KoParagraphStyle::setLeftMargin(QTextLength margin)
{
    setProperty(QTextFormat::BlockLeftMargin, margin);
}

qreal KoParagraphStyle::leftMargin() const
{
    if (parentStyle())
        return propertyLength(QTextFormat::BlockLeftMargin).value(parentStyle()->leftMargin());
    else
        return propertyLength(QTextFormat::BlockLeftMargin).value(0);
}

void KoParagraphStyle::setRightMargin(QTextLength margin)
{
    setProperty(QTextFormat::BlockRightMargin, margin);
}

qreal KoParagraphStyle::rightMargin() const
{
    if (parentStyle())
        return propertyLength(QTextFormat::BlockRightMargin).value(parentStyle()->rightMargin());
    else
        return propertyLength(QTextFormat::BlockRightMargin).value(0);
}

void KoParagraphStyle::setMargin(QTextLength margin)
{
    setTopMargin(margin);
    setBottomMargin(margin);
    setLeftMargin(margin);
    setRightMargin(margin);
}

void KoParagraphStyle::setAlignment(Qt::Alignment alignment)
{
    setProperty(QTextFormat::BlockAlignment, (int) alignment);
}

Qt::Alignment KoParagraphStyle::alignment() const
{
    return static_cast<Qt::Alignment>(propertyInt(QTextFormat::BlockAlignment));
}

void KoParagraphStyle::setTextIndent(QTextLength margin)
{
    setProperty(QTextFormat::TextIndent, margin);
}

qreal KoParagraphStyle::textIndent() const
{
    if (parentStyle())
        return propertyLength(QTextFormat::TextIndent).value(parentStyle()->textIndent());
    else
        return propertyLength(QTextFormat::TextIndent).value(0);
}

void KoParagraphStyle::setAutoTextIndent(bool on)
{
    setProperty(KoParagraphStyle::AutoTextIndent, on);
}

bool KoParagraphStyle::autoTextIndent() const
{
    return propertyBoolean(KoParagraphStyle::AutoTextIndent);
}

void KoParagraphStyle::setNonBreakableLines(bool on)
{
    setProperty(QTextFormat::BlockNonBreakableLines, on);
}

bool KoParagraphStyle::nonBreakableLines() const
{
    return propertyBoolean(QTextFormat::BlockNonBreakableLines);
}

void KoParagraphStyle::setKeepWithNext(bool value)
{
    setProperty(KeepWithNext, value);
}

bool KoParagraphStyle::keepWithNext() const
{
    if (hasProperty(KeepWithNext))
        return propertyBoolean(KeepWithNext);
    return false;
}

bool KoParagraphStyle::punctuationWrap() const
{
    if (hasProperty(PunctuationWrap))
        return propertyBoolean(PunctuationWrap);
    return false;
}

void KoParagraphStyle::setPunctuationWrap(bool value)
{
    setProperty(PunctuationWrap, value);
}

KoParagraphStyle *KoParagraphStyle::parentStyle() const
{
    return d->parentStyle;
}

void KoParagraphStyle::setNextStyle(int next)
{
    setProperty(NextStyle, next);
}

int KoParagraphStyle::nextStyle() const
{
    return propertyInt(NextStyle);
}

QString KoParagraphStyle::name() const
{
    return d->name;
}

void KoParagraphStyle::setName(const QString &name)
{
    if (name == d->name)
        return;
    d->name = name;
    KoCharacterStyle::setName(name);
    emit nameChanged(name);
}

int KoParagraphStyle::styleId() const
{
    // duplicate some code to avoid getting the parents style id
    QVariant variant = d->stylesPrivate.value(StyleId);
    if (variant.isNull())
        return 0;
    return variant.toInt();
}

void KoParagraphStyle::setStyleId(int id)
{
    setProperty(StyleId, id); if (nextStyle() == 0) setNextStyle(id);
    KoCharacterStyle::setStyleId(id);
}

QString KoParagraphStyle::masterPageName() const
{
    return value(MasterPageName).toString();
}

void KoParagraphStyle::setMasterPageName(const QString &name)
{
    setProperty(MasterPageName, name);
}

void KoParagraphStyle::setListStartValue(int value)
{
    setProperty(ListStartValue, value);
}

int KoParagraphStyle::listStartValue() const
{
    return propertyInt(ListStartValue);
}

void KoParagraphStyle::setRestartListNumbering(bool on)
{
    setProperty(RestartListNumbering, on);
}

bool KoParagraphStyle::restartListNumbering()
{
    return propertyBoolean(RestartListNumbering);
}

void KoParagraphStyle::setListLevel(int value)
{
    setProperty(ListLevel, value);
}

int KoParagraphStyle::listLevel() const
{
    return propertyInt(ListLevel);
}

void KoParagraphStyle::setOutlineLevel(int outline)
{
    setProperty(OutlineLevel, outline);
}

int KoParagraphStyle::outlineLevel() const
{
    return propertyInt(OutlineLevel);
}

void KoParagraphStyle::setDefaultOutlineLevel(int outline)
{
    setProperty(DefaultOutlineLevel, outline);
}

int KoParagraphStyle::defaultOutlineLevel() const
{
    return propertyInt(DefaultOutlineLevel);
}

bool KoParagraphStyle::lineNumbering() const
{
    return propertyBoolean(LineNumbering);
}

void KoParagraphStyle::setLineNumbering(bool lineNumbering)
{
    setProperty(LineNumbering, lineNumbering);
}

int KoParagraphStyle::lineNumberStartValue() const
{
    return propertyInt(LineNumberStartValue);
}

void KoParagraphStyle::setLineNumberStartValue(int lineNumberStartValue)
{
    setProperty(LineNumberStartValue, lineNumberStartValue);
}

void KoParagraphStyle::setIsListHeader(bool on)
{
    setProperty(IsListHeader, on);
}

bool KoParagraphStyle::isListHeader() const
{
    return propertyBoolean(IsListHeader);
}

KoListStyle *KoParagraphStyle::listStyle() const
{
    QVariant variant = value(ParagraphListStyleId);
    if (variant.isNull())
        return 0;
    return variant.value<KoListStyle *>();
}

void KoParagraphStyle::setListStyle(KoListStyle *style)
{
    if (listStyle() == style)
        return;
    if (listStyle() && listStyle()->parent() == this)
        delete listStyle();
    QVariant variant;
    KoListStyle *cloneStyle = 0;
    if (style) {
        cloneStyle = style->clone();
        variant.setValue(cloneStyle);

        setProperty(ParagraphListStyleId, variant);
    } else {
        d->stylesPrivate.remove(ParagraphListStyleId);
    }
}

KoText::Direction KoParagraphStyle::textProgressionDirection() const
{
    return static_cast<KoText::Direction>(propertyInt(TextProgressionDirection));
}

void KoParagraphStyle::setTextProgressionDirection(KoText::Direction dir)
{
    setProperty(TextProgressionDirection, dir);
}

bool KoParagraphStyle::keepHyphenation() const
{
    if (hasProperty(KeepHyphenation))
        return propertyBoolean(KeepHyphenation);
    return false;
}

void KoParagraphStyle::setKeepHyphenation(bool value)
{
    setProperty(KeepHyphenation, value);
}

int KoParagraphStyle::hyphenationLadderCount() const
{
    if (hasProperty(HyphenationLadderCount))
        return propertyInt(HyphenationLadderCount);
    return 0;
}

void KoParagraphStyle::setHyphenationLadderCount(int value)
{
    setProperty(HyphenationLadderCount, value);
}

void KoParagraphStyle::setBackground(const QBrush &brush)
{
    d->setProperty(QTextFormat::BackgroundBrush, brush);
}

void KoParagraphStyle::clearBackground()
{
    d->stylesPrivate.remove(QTextCharFormat::BackgroundBrush);
}

QBrush KoParagraphStyle::background() const
{
    QVariant variant = d->stylesPrivate.value(QTextFormat::BackgroundBrush);

    if (variant.isNull()) {
        return QBrush();
    }
    return qvariant_cast<QBrush>(variant);
}

qreal KoParagraphStyle::backgroundTransparency() const
{
    if (hasProperty(BackgroundTransparency))
        return propertyDouble(BackgroundTransparency);
    return 0.0;
}

void KoParagraphStyle::setBackgroundTransparency(qreal transparency)
{
    setProperty(BackgroundTransparency, transparency);
}

void KoParagraphStyle::setSnapToLayoutGrid(bool value)
{
    setProperty(SnapToLayoutGrid, value);
}

bool KoParagraphStyle::snapToLayoutGrid() const
{
    if (hasProperty(SnapToLayoutGrid))
        return propertyBoolean(SnapToLayoutGrid);
    return false;
}

bool KoParagraphStyle::joinBorder() const
{
    if (hasProperty(JoinBorder))
        return propertyBoolean(JoinBorder);
    return true; //default is true
}

void KoParagraphStyle::setJoinBorder(bool value)
{
    setProperty(JoinBorder, value);
}

int KoParagraphStyle::pageNumber() const
{
    return propertyInt(PageNumber);
}

void KoParagraphStyle::setPageNumber(int pageNumber)
{
    if (pageNumber >= 0)
        setProperty(PageNumber, pageNumber);
}

bool KoParagraphStyle::automaticWritingMode() const
{
    if (hasProperty(AutomaticWritingMode))
        return propertyBoolean(AutomaticWritingMode);
    return true;
}

void KoParagraphStyle::setAutomaticWritingMode(bool value)
{
    setProperty(AutomaticWritingMode, value);
}

void KoParagraphStyle::setVerticalAlignment(KoParagraphStyle::VerticalAlign value)
{
    setProperty(VerticalAlignment, value);
}

KoParagraphStyle::VerticalAlign KoParagraphStyle::verticalAlignment() const
{
    if (hasProperty(VerticalAlignment))
        return (VerticalAlign) propertyInt(VerticalAlignment);
    return VAlignAuto;
}

void KoParagraphStyle::setShadow(const KoShadowStyle &shadow)
{
    d->setProperty(Shadow, QVariant::fromValue<KoShadowStyle>(shadow));
}

KoShadowStyle KoParagraphStyle::shadow() const
{
    if (hasProperty(Shadow))
        return value(Shadow).value<KoShadowStyle>();
    return KoShadowStyle();
}

void KoParagraphStyle::loadOdf(const KoXmlElement *element, KoShapeLoadingContext &scontext,
    bool loadParents)
{
    KoOdfLoadingContext &context = scontext.odfLoadingContext();
    const QString name(element->attributeNS(KoXmlNS::style, "display-name", QString()));
    if (!name.isEmpty()) {
        setName(name);
    }
    else {
        setName(element->attributeNS(KoXmlNS::style, "name", QString()));
    }

    QString family = element->attributeNS(KoXmlNS::style, "family", "paragraph");

    context.styleStack().save();
    if (loadParents) {
        context.addStyles(element, family.toLocal8Bit().constData());   // Load all parent
    } else {
        context.styleStack().push(*element);
    }
    context.styleStack().setTypeProperties("text");  // load the style:text-properties
    KoCharacterStyle::loadOdfProperties(scontext);

    QString masterPage = element->attributeNS(KoXmlNS::style, "master-page-name", QString());
    if (! masterPage.isEmpty()) {
        setMasterPageName(masterPage);
    }

    if (element->hasAttributeNS(KoXmlNS::style, "default-outline-level")) {
        bool ok = false;
        int level = element->attributeNS(KoXmlNS::style, "default-outline-level").toInt(&ok);
        if (ok)
            setDefaultOutlineLevel(level);
    }

    context.styleStack().setTypeProperties("paragraph");   // load all style attributes from "style:paragraph-properties"

    loadOdfProperties(scontext);   // load the KoParagraphStyle from the stylestack

    context.styleStack().restore();
}

struct ParagraphBorderData {
    enum Values {Style = 1, Color = 2, Width = 4};

    ParagraphBorderData()
    : values(0) {}

    ParagraphBorderData(const ParagraphBorderData &other)
    : values(other.values), style(other.style), color(other.color), width(other.width) {}

    // flag defining which data is set
    int values;

    KoBorder::BorderStyle style;
    QColor color;
    qreal width; ///< in pt
};


/// Parses the @p dataString as value defined by CSS2 §7.29.3 "border"
/// Adds parsed data to the data as set for @p defaultParagraphBorderData.
/// Returns the enriched border data on success, the original @p defaultParagraphBorderData on a parsing error
static ParagraphBorderData parseParagraphBorderData(const QString &dataString, const ParagraphBorderData &defaultParagraphBorderData)
{
    const QStringList bv = dataString.split(QLatin1Char(' '), QString::SkipEmptyParts);
    // too many items? ignore complete value
    if (bv.count() > 3) {
        return defaultParagraphBorderData;
    }

    ParagraphBorderData borderData = defaultParagraphBorderData;
    int parsedValues = 0; ///< used to track what is read from the given string

    Q_FOREACH (const QString &v, bv) {
        // try style
        if (! (parsedValues & ParagraphBorderData::Style)) {
            bool success = false;
            KoBorder::BorderStyle style = KoBorder::odfBorderStyle(v, &success);
            // workaround for not yet supported "hidden"
            if (! success && (v == QLatin1String("hidden"))) {
                // map to "none" for now TODO: KoBorder needs to support "hidden"
                style = KoBorder::BorderNone;
                success = true;
            }
            if (success) {
                borderData.style = style;
                borderData.values |= ParagraphBorderData::Style;
                parsedValues |= ParagraphBorderData::Style;
                continue;
            }
        }
        // try color
        if (! (parsedValues & ParagraphBorderData::Color)) {
            const QColor color(v);
            if (color.isValid()) {
                borderData.color = color;
                borderData.values |= ParagraphBorderData::Color;
                parsedValues |= ParagraphBorderData::Color;
                continue;
            }
        }
        // try width
        if (! (parsedValues & ParagraphBorderData::Width)) {
            const qreal width = KoUnit::parseValue(v);
            if (width >= 0.0) {
                borderData.width = width;
                borderData.values |= ParagraphBorderData::Width;
                parsedValues |= ParagraphBorderData::Width;
                continue;
            }
        }
        // still here? found a value which cannot be parsed
        return defaultParagraphBorderData;
    }
    return borderData;
}

void KoParagraphStyle::loadOdfProperties(KoShapeLoadingContext &scontext)
{
    KoStyleStack &styleStack = scontext.odfLoadingContext().styleStack();

    // in 1.6 this was defined at KoParagLayout::loadOasisParagLayout(KoParagLayout&, KoOasisContext&)
    const QString writingMode(styleStack.property(KoXmlNS::style, "writing-mode"));
    if (!writingMode.isEmpty()) {
        setTextProgressionDirection(KoText::directionFromString(writingMode));
    }

    // Alignment
    const QString textAlign(styleStack.property(KoXmlNS::fo, "text-align"));
    if (!textAlign.isEmpty()) {
        setAlignment(KoText::alignmentFromString(textAlign));
    }

    // Spacing (padding)
    const QString padding(styleStack.property(KoXmlNS::fo, "padding"));
    if (!padding.isEmpty()) {
        setPadding(KoUnit::parseValue(padding));
    }
    const QString paddingLeft(styleStack.property(KoXmlNS::fo, "padding-left" ));
    if (!paddingLeft.isEmpty()) {
        setLeftPadding(KoUnit::parseValue(paddingLeft));
    }
    const QString paddingRight(styleStack.property(KoXmlNS::fo, "padding-right" ));
    if (!paddingRight.isEmpty()) {
        setRightPadding(KoUnit::parseValue(paddingRight));
    }
    const QString paddingTop(styleStack.property(KoXmlNS::fo, "padding-top" ));
    if (!paddingTop.isEmpty()) {
        setTopPadding(KoUnit::parseValue(paddingTop));
    }
    const QString paddingBottom(styleStack.property(KoXmlNS::fo, "padding-bottom" ));
    if (!paddingBottom.isEmpty()) {
        setBottomPadding(KoUnit::parseValue(paddingBottom));
    }

    // Indentation (margin)
    const QString margin(styleStack.property(KoXmlNS::fo, "margin"));
    if (!margin.isEmpty()) {
        setMargin(KoText::parseLength(margin));
    }
    const QString marginLeft(styleStack.property(KoXmlNS::fo, "margin-left" ));
    if (!marginLeft.isEmpty()) {
        setLeftMargin(KoText::parseLength(marginLeft));
    }
    const QString marginRight(styleStack.property(KoXmlNS::fo, "margin-right" ));
    if (!marginRight.isEmpty()) {
        setRightMargin(KoText::parseLength(marginRight));
    }
    const QString marginTop(styleStack.property(KoXmlNS::fo, "margin-top"));
    if (!marginTop.isEmpty()) {
        setTopMargin(KoText::parseLength(marginTop));
    }
    const QString marginBottom(styleStack.property(KoXmlNS::fo, "margin-bottom"));
    if (!marginBottom.isEmpty()) {
        setBottomMargin(KoText::parseLength(marginBottom));
    }

    // Automatic Text indent
    // OOo is not assuming this. Commenting this line thus allow more OpenDocuments to be supported, including a
    // testcase from the ODF test suite. See §15.5.18 in the spec.
    //if ( hasMarginLeft || hasMarginRight ) {
    // style:auto-text-indent takes precedence
    const QString autoTextIndent(styleStack.property(KoXmlNS::style, "auto-text-indent"));
    if (!autoTextIndent.isEmpty()) {
        setAutoTextIndent(autoTextIndent == "true");
    }

    if (autoTextIndent != "true" || autoTextIndent.isEmpty()) {
        const QString textIndent(styleStack.property(KoXmlNS::fo, "text-indent"));
        if (!textIndent.isEmpty()) {
            setTextIndent(KoText::parseLength(textIndent));
        }
    }

    //}

    // Line spacing
    QString lineHeight(styleStack.property(KoXmlNS::fo, "line-height"));
    if (!lineHeight.isEmpty()) {
        if (lineHeight != "normal") {
            if (lineHeight.indexOf('%') > -1) {
                bool ok;
                const qreal percent = lineHeight.remove('%').toDouble(&ok);
                if (ok) {
                    setLineHeightPercent(percent);
                }
            }
            else  { // fixed value is between 0.0201in and 3.9402in
                const qreal value = KoUnit::parseValue(lineHeight, -1.0);
                if (value >= 0.0) {
                    setLineHeightAbsolute(value);
                }
            }
        }
        else {
            setNormalLineHeight();
        }
    }
    else {
        const QString lineSpacing(styleStack.property(KoXmlNS::style, "line-spacing"));
        if (!lineSpacing.isEmpty()) {    // 3.11.3
            setLineSpacing(KoUnit::parseValue(lineSpacing));
        }
    }

    // 15.5.30 - 31
    if (styleStack.hasProperty(KoXmlNS::text, "number-lines")) {
        setLineNumbering(styleStack.property(KoXmlNS::text, "number-lines", "false") == "true");
    }
    if (styleStack.hasProperty(KoXmlNS::text, "line-number")) {
        bool ok;
        int startValue = styleStack.property(KoXmlNS::text, "line-number").toInt(&ok);
        if (ok) {
            setLineNumberStartValue(startValue);
        }
    }

    const QString lineHeightAtLeast(styleStack.property(KoXmlNS::style, "line-height-at-least"));
    if (!lineHeightAtLeast.isEmpty() && !propertyBoolean(NormalLineHeight) && lineHeightAbsolute() == 0) {    // 3.11.2
        setMinimumLineHeight(KoText::parseLength(lineHeightAtLeast));
    }  // Line-height-at-least is mutually exclusive with absolute line-height
    const QString fontIndependentLineSpacing(styleStack.property(KoXmlNS::style, "font-independent-line-spacing"));
    if (!fontIndependentLineSpacing.isEmpty() && !propertyBoolean(NormalLineHeight) && lineHeightAbsolute() == 0) {
        setLineSpacingFromFont(fontIndependentLineSpacing == "true");
    }

    // Tabulators
    const QString tabStopDistance(styleStack.property(KoXmlNS::style, "tab-stop-distance"));
    if (!tabStopDistance.isEmpty()) {
        qreal stopDistance = KoUnit::parseValue(tabStopDistance);
        if (stopDistance >= 0)
            setTabStopDistance(stopDistance);
    }
    KoXmlElement tabStops(styleStack.childNode(KoXmlNS::style, "tab-stops"));
    if (!tabStops.isNull()) {     // 3.11.10
        QList<KoText::Tab> tabList;
        KoXmlElement tabStop;
        forEachElement(tabStop, tabStops) {
            if(tabStop.localName() != "tab-stop")
                continue;

            // Tab position
            KoText::Tab tab;
            tab.position = KoUnit::parseValue(tabStop.attributeNS(KoXmlNS::style, "position", QString()));
            //debugText << "tab position " << tab.position;

            // Tab stop positions in the XML are relative to the left-margin
            // Equivalently, relative to the left end of our textshape
            // Tab type (left/right/center/char)
            const QString type = tabStop.attributeNS(KoXmlNS::style, "type", QString());
            if (type == "center")
                tab.type = QTextOption::CenterTab;
            else if (type == "right")
                tab.type = QTextOption::RightTab;
            else if (type == "char") {
                tab.type = QTextOption::DelimiterTab;
                tab.delimiter = QChar('.');
            } else //if ( type == "left" )
                tab.type = QTextOption::LeftTab;

            // Tab delimiter char
            if (tab.type == QTextOption::DelimiterTab) {
                QString delimiterChar = tabStop.attributeNS(KoXmlNS::style, "char", QString());   // single character
                if (!delimiterChar.isEmpty()) {
                    tab.delimiter = delimiterChar[0];
                } else {
                    // this is invalid. fallback to left-tabbing.
                    tab.type = QTextOption::LeftTab;
                }
            }

            QString leaderType = tabStop.attributeNS(KoXmlNS::style, "leader-type", QString());
            if (leaderType.isEmpty() || leaderType == "none") {
                tab.leaderType = KoCharacterStyle::NoLineType;
            } else {
                if (leaderType == "single")
                    tab.leaderType = KoCharacterStyle::SingleLine;
                else if (leaderType == "double")
                    tab.leaderType = KoCharacterStyle::DoubleLine;
                // change default leaderStyle
                tab.leaderStyle = KoCharacterStyle::SolidLine;
            }

            QString leaderStyle = tabStop.attributeNS(KoXmlNS::style, "leader-style", QString());
            if (leaderStyle == "none")
                tab.leaderStyle = KoCharacterStyle::NoLineStyle;
            else if (leaderStyle == "solid")
                tab.leaderStyle = KoCharacterStyle::SolidLine;
            else if (leaderStyle == "dotted")
                tab.leaderStyle = KoCharacterStyle::DottedLine;
            else if (leaderStyle == "dash")
                tab.leaderStyle = KoCharacterStyle::DashLine;
            else if (leaderStyle == "long-dash")
                tab.leaderStyle = KoCharacterStyle::LongDashLine;
            else if (leaderStyle == "dot-dash")
                tab.leaderStyle = KoCharacterStyle::DotDashLine;
            else if (leaderStyle == "dot-dot-dash")
                tab.leaderStyle = KoCharacterStyle::DotDotDashLine;
            else if (leaderStyle == "wave")
                tab.leaderStyle = KoCharacterStyle::WaveLine;

            if (tab.leaderType == KoCharacterStyle::NoLineType && tab.leaderStyle != KoCharacterStyle::NoLineStyle) {
                if (leaderType == "none")
                    // if leaderType was explicitly specified as none, but style was not none,
                    // make leaderType override (ODF1.1 §15.5.11)
                    tab.leaderStyle = KoCharacterStyle::NoLineStyle;
                else
                    // if leaderType was implicitly assumed none, but style was not none,
                    // make leaderStyle override
                    tab.leaderType = KoCharacterStyle::SingleLine;
            }

            QString leaderColor = tabStop.attributeNS(KoXmlNS::style, "leader-color", QString());
            if (leaderColor != "font-color")
                tab.leaderColor = QColor(leaderColor); // if invalid color (the default), will use text color

            QString width = tabStop.attributeNS(KoXmlNS::style, "leader-width", QString());
            if (width.isEmpty() || width == "auto")
                tab.leaderWeight = KoCharacterStyle::AutoLineWeight;
            else if (width == "normal")
                tab.leaderWeight = KoCharacterStyle::NormalLineWeight;
            else if (width == "bold")
                tab.leaderWeight = KoCharacterStyle::BoldLineWeight;
            else if (width == "thin")
                tab.leaderWeight = KoCharacterStyle::ThinLineWeight;
            else if (width == "dash")
                tab.leaderWeight = KoCharacterStyle::DashLineWeight;
            else if (width == "medium")
                tab.leaderWeight = KoCharacterStyle::MediumLineWeight;
            else if (width == "thick")
                tab.leaderWeight = KoCharacterStyle::ThickLineWeight;
            else if (width.endsWith('%')) {
                tab.leaderWeight = KoCharacterStyle::PercentLineWeight;
                tab.leaderWidth = width.mid(0, width.length() - 1).toDouble();
            } else if (width[width.length()-1].isNumber()) {
                tab.leaderWeight = KoCharacterStyle::PercentLineWeight;
                tab.leaderWidth = 100 * width.toDouble();
            } else {
                tab.leaderWeight = KoCharacterStyle::LengthLineWeight;
                tab.leaderWidth = KoUnit::parseValue(width);
            }

            tab.leaderText = tabStop.attributeNS(KoXmlNS::style, "leader-text", QString());

#if 0
            else {
                // Fallback: convert leaderChar's unicode value
                QString leaderChar = tabStop.attributeNS(KoXmlNS::style, "leader-text", QString());
                if (!leaderChar.isEmpty()) {
                    QChar ch = leaderChar[0];
                    switch (ch.latin1()) {
                    case '.':
                        tab.filling = TF_DOTS; break;
                    case '-':
                    case '_':  // TODO in Words: differentiate --- and ___
                        tab.filling = TF_LINE; break;
                    default:
                        // Words doesn't have support for "any char" as filling.
                        break;
                    }
                }
            }
#endif
            tabList.append(tab);
        } //for
        setTabPositions(tabList);
    }

#if 0
    layout.joinBorder = !(styleStack.property(KoXmlNS::style, "join-border") == "false");
#endif

    // Borders
    // The border attribute is actually three attributes in one string, all optional
    // and with no given order. Also there is a hierarchy, first the common for all
    // sides and then overwrites per side, while in the code only the sides are stored.
    // So first the common data border is fetched, then this is overwritten per
    // side and the result stored.
    const QString border(styleStack.property(KoXmlNS::fo, "border"));
    const ParagraphBorderData borderData = parseParagraphBorderData(border, ParagraphBorderData());

    const QString borderLeft(styleStack.property(KoXmlNS::fo, "border-left"));
    const ParagraphBorderData leftParagraphBorderData = parseParagraphBorderData(borderLeft, borderData);
    if (leftParagraphBorderData.values & ParagraphBorderData::Width) {
        setLeftBorderWidth(leftParagraphBorderData.width);
    }
    if (leftParagraphBorderData.values & ParagraphBorderData::Style) {
        setLeftBorderStyle(leftParagraphBorderData.style);
    }
    if (leftParagraphBorderData.values & ParagraphBorderData::Color) {
        setLeftBorderColor(leftParagraphBorderData.color);
    }

    const QString borderTop(styleStack.property(KoXmlNS::fo, "border-top"));
    const ParagraphBorderData topParagraphBorderData = parseParagraphBorderData(borderTop, borderData);
    if (topParagraphBorderData.values & ParagraphBorderData::Width) {
        setTopBorderWidth(topParagraphBorderData.width);
    }
    if (topParagraphBorderData.values & ParagraphBorderData::Style) {
        setTopBorderStyle(topParagraphBorderData.style);
    }
    if (topParagraphBorderData.values & ParagraphBorderData::Color) {
        setTopBorderColor(topParagraphBorderData.color);
    }

    const QString borderRight(styleStack.property(KoXmlNS::fo, "border-right"));
    const ParagraphBorderData rightParagraphBorderData = parseParagraphBorderData(borderRight, borderData);
    if (rightParagraphBorderData.values & ParagraphBorderData::Width) {
        setRightBorderWidth(rightParagraphBorderData.width);
    }
    if (rightParagraphBorderData.values & ParagraphBorderData::Style) {
        setRightBorderStyle(rightParagraphBorderData.style);
    }
    if (rightParagraphBorderData.values & ParagraphBorderData::Color) {
        setRightBorderColor(rightParagraphBorderData.color);
    }

    const QString borderBottom(styleStack.property(KoXmlNS::fo, "border-bottom"));
    const ParagraphBorderData bottomParagraphBorderData = parseParagraphBorderData(borderBottom, borderData);
    if (bottomParagraphBorderData.values & ParagraphBorderData::Width) {
        setBottomBorderWidth(bottomParagraphBorderData.width);
    }
    if (bottomParagraphBorderData.values & ParagraphBorderData::Style) {
        setBottomBorderStyle(bottomParagraphBorderData.style);
    }
    if (bottomParagraphBorderData.values & ParagraphBorderData::Color) {
        setBottomBorderColor(bottomParagraphBorderData.color);
    }

    const QString borderLineWidthLeft(styleStack.property(KoXmlNS::style, "border-line-width", "left"));
    if (!borderLineWidthLeft.isEmpty()) {
        QStringList blw = borderLineWidthLeft.split(' ', QString::SkipEmptyParts);
        setLeftInnerBorderWidth(KoUnit::parseValue(blw.value(0), 0.1));
        setLeftBorderSpacing(KoUnit::parseValue(blw.value(1), 1.0));
        setLeftBorderWidth(KoUnit::parseValue(blw.value(2), 0.1));
    }
    const QString borderLineWidthTop(styleStack.property(KoXmlNS::style, "border-line-width", "top"));
    if (!borderLineWidthTop.isEmpty()) {
        QStringList blw = borderLineWidthTop.split(' ', QString::SkipEmptyParts);
        setTopInnerBorderWidth(KoUnit::parseValue(blw.value(0), 0.1));
        setTopBorderSpacing(KoUnit::parseValue(blw.value(1), 1.0));
        setTopBorderWidth(KoUnit::parseValue(blw.value(2), 0.1));
    }
    const QString borderLineWidthRight(styleStack.property(KoXmlNS::style, "border-line-width", "right"));
    if (!borderLineWidthRight.isEmpty()) {
        QStringList blw = borderLineWidthRight.split(' ', QString::SkipEmptyParts);
        setRightInnerBorderWidth(KoUnit::parseValue(blw.value(0), 0.1));
        setRightBorderSpacing(KoUnit::parseValue(blw.value(1), 1.0));
        setRightBorderWidth(KoUnit::parseValue(blw.value(2), 0.1));
    }
    const QString borderLineWidthBottom(styleStack.property(KoXmlNS::style, "border-line-width", "bottom"));
    if (!borderLineWidthBottom.isEmpty()) {
        QStringList blw = borderLineWidthBottom.split(' ', QString::SkipEmptyParts);
        setBottomInnerBorderWidth(KoUnit::parseValue(blw.value(0), 0.1));
        setBottomBorderSpacing(KoUnit::parseValue(blw.value(1), 1.0));
        setBottomBorderWidth(KoUnit::parseValue(blw.value(2), 0.1));
    }

    // drop caps
    KoXmlElement dropCap(styleStack.childNode(KoXmlNS::style, "drop-cap"));
    if (!dropCap.isNull()) {
        setDropCaps(true);
        const QString length = dropCap.attributeNS(KoXmlNS::style, "length", QString("1"));
        if (length.toLower() == "word") {
            setDropCapsLength(0); // 0 indicates drop caps of the whole first word
        } else {
            int l = length.toInt();
            if (l > 0) // somefiles may use this to turn dropcaps off
                setDropCapsLength(length.toInt());
            else
                setDropCaps(false);
        }
        const QString lines = dropCap.attributeNS(KoXmlNS::style, "lines", QString("1"));
        setDropCapsLines(lines.toInt());
        const qreal distance = KoUnit::parseValue(dropCap.attributeNS(KoXmlNS::style, "distance", QString()));
        setDropCapsDistance(distance);

        const QString dropstyle = dropCap.attributeNS(KoXmlNS::style, "style-name");
        if (! dropstyle.isEmpty()) {
            KoSharedLoadingData *sharedData = scontext.sharedData(KOTEXT_SHARED_LOADING_ID);
            KoTextSharedLoadingData *textSharedData = 0;
            textSharedData = dynamic_cast<KoTextSharedLoadingData *>(sharedData);
            if (textSharedData) {
                KoCharacterStyle *cs = textSharedData->characterStyle(dropstyle, true);
                if (cs)
                    setDropCapsTextStyleId(cs->styleId());
            }
        }
    }

    // The fo:break-before and fo:break-after attributes insert a page or column break before or after a paragraph.
    const QString breakBefore(styleStack.property(KoXmlNS::fo, "break-before"));
    if (!breakBefore.isEmpty()) {
        setBreakBefore(KoText::textBreakFromString(breakBefore));
    }
    const QString breakAfter(styleStack.property(KoXmlNS::fo, "break-after"));
    if (!breakAfter.isEmpty()) {
        setBreakAfter(KoText::textBreakFromString(breakAfter));
    }
    const QString keepTogether(styleStack.property(KoXmlNS::fo, "keep-together"));
    if (!keepTogether.isEmpty()) {
        setNonBreakableLines(keepTogether == "always");
    }

    const QString rawPageNumber(styleStack.property(KoXmlNS::style, "page-number"));
    if (!rawPageNumber.isEmpty()) {
        if (rawPageNumber == "auto") {
            setPageNumber(0);
        } else {
            bool ok;
            int number = rawPageNumber.toInt(&ok);
            if (ok)
                setPageNumber(number);
        }
    }
    // The fo:background-color attribute specifies the background color of a paragraph.
    const QString bgcolor(styleStack.property(KoXmlNS::fo, "background-color"));
    if (!bgcolor.isEmpty()) {
        const QString bgcolor = styleStack.property(KoXmlNS::fo, "background-color");
        QBrush brush = background();
        if (bgcolor == "transparent")
            brush.setStyle(Qt::NoBrush);
        else {
            if (brush.style() == Qt::NoBrush)
                brush.setStyle(Qt::SolidPattern);
            brush.setColor(bgcolor); // #rrggbb format
        }
        setBackground(brush);
    }
    if (styleStack.hasProperty(KoXmlNS::style, "background-transparency"))
    {
        QString transparency = styleStack.property(KoXmlNS::style, "background-transparency");
        bool ok = false;
        qreal transparencyValue  = transparency.remove('%').toDouble(&ok);
        if (ok) {
            setBackgroundTransparency(transparencyValue/100);
        }
    }

    if (styleStack.hasProperty(KoXmlNS::style, "snap-to-layout-grid"))
    {
        setSnapToLayoutGrid(styleStack.property(KoXmlNS::style, "snap-to-layout-grid") == "true");
    }

    if (styleStack.hasProperty(KoXmlNS::style, "register-true"))
    {
        setRegisterTrue(styleStack.property(KoXmlNS::style, "register-true") == "true");
    }

    if (styleStack.hasProperty(KoXmlNS::style, "join-border"))
    {
        setJoinBorder(styleStack.property(KoXmlNS::style, "join-border") == "true");
    }

    if (styleStack.hasProperty(KoXmlNS::style, "line-break"))
    {
        setStrictLineBreak(styleStack.property(KoXmlNS::style, "line-break") == "strict");
    }

    // Support for an old non-standard OpenOffice attribute that we still find in too many documents...
    if (styleStack.hasProperty(KoXmlNS::text, "enable-numbering")) {
        setProperty(ForceDisablingList, styleStack.property(KoXmlNS::text, "enable-numbering") == "false");
    }

    if (styleStack.hasProperty(KoXmlNS::fo, "orphans")) {
        bool ok = false;
        int orphans = styleStack.property(KoXmlNS::fo, "orphans").toInt(&ok);
        if (ok)
            setOrphanThreshold(orphans);
    }

    if (styleStack.hasProperty(KoXmlNS::fo, "widows")) {
        bool ok = false;
        int widows = styleStack.property(KoXmlNS::fo, "widows").toInt(&ok);
        if (ok)
            setWidowThreshold(widows);
    }

    if (styleStack.hasProperty(KoXmlNS::style, "justify-single-word")) {
        setJustifySingleWord(styleStack.property(KoXmlNS::style, "justify-single-word") == "true");
    }

    if (styleStack.hasProperty(KoXmlNS::style, "writing-mode-automatic")) {
        setAutomaticWritingMode(styleStack.property(KoXmlNS::style, "writing-mode-automatic") == "true");
    }

    if (styleStack.hasProperty(KoXmlNS::fo, "text-align-last")) {
        setAlignLastLine(KoText::alignmentFromString(styleStack.property(KoXmlNS::fo, "text-align-last")));
    }

    if (styleStack.hasProperty(KoXmlNS::fo, "keep-with-next")) {
        setKeepWithNext(styleStack.property(KoXmlNS::fo, "keep-with-next") == "always");
    }

    if (styleStack.hasProperty(KoXmlNS::style, "text-autospace")) {
        const QString autoSpace = styleStack.property(KoXmlNS::style, "text-autospace");
        if (autoSpace == "none")
            setTextAutoSpace(NoAutoSpace);
        else if (autoSpace == "ideograph-alpha")
            setTextAutoSpace(IdeographAlpha);
    }

    if (styleStack.hasProperty(KoXmlNS::fo, "hyphenation-keep")) {
        setKeepHyphenation(styleStack.property(KoXmlNS::fo, "hyphenation-keep") == "page");
    }

    if (styleStack.hasProperty(KoXmlNS::fo, "hyphenation-ladder-count")) {
        QString ladderCount = styleStack.property(KoXmlNS::fo, "hyphenation-ladder-count");
        if (ladderCount == "no-limit")
            setHyphenationLadderCount(0);
        else {
            bool ok;
            int value = ladderCount.toInt(&ok);
            if ((ok) && (value > 0))
                setHyphenationLadderCount(value);
        }
    }

    if (styleStack.hasProperty(KoXmlNS::style, "punctuation-wrap")) {
        setPunctuationWrap(styleStack.property(KoXmlNS::style, "punctuation-wrap") == "simple");
    }

    if (styleStack.hasProperty(KoXmlNS::style, "vertical-align")) {
        const QString valign = styleStack.property(KoXmlNS::style, "vertical-align");
        if (valign == "auto")
            setVerticalAlignment(VAlignAuto);
        else if (valign == "baseline")
            setVerticalAlignment(VAlignBaseline);
        else if (valign == "bottom")
            setVerticalAlignment(VAlignBottom);
        else if (valign == "middle")
            setVerticalAlignment(VAlignMiddle);
        else if (valign == "top")
            setVerticalAlignment(VAlignTop);
    }


    if (styleStack.hasProperty(KoXmlNS::style, "shadow")) {
        KoShadowStyle shadow;
        if (shadow.loadOdf(styleStack.property(KoXmlNS::style, "shadow")))
            setShadow(shadow);
    }

    //following properties KoParagraphStyle provides us are not handled now;
    // LineSpacingFromFont,
    // FollowDocBaseline,

}

void KoParagraphStyle::setTabPositions(const QList<KoText::Tab> &tabs)
{
    QList<KoText::Tab> newTabs = tabs;
    std::sort(newTabs.begin(), newTabs.end(), compareTabs);
    QList<QVariant> list;
    Q_FOREACH (const KoText::Tab &tab, tabs) {
        QVariant v;
        v.setValue(tab);
        list.append(v);
    }
    setProperty(TabPositions, list);
}

QList<KoText::Tab> KoParagraphStyle::tabPositions() const
{
    QVariant variant = value(TabPositions);
    if (variant.isNull())
        return QList<KoText::Tab>();
    QList<KoText::Tab> answer;
    Q_FOREACH (const QVariant &tab, qvariant_cast<QList<QVariant> >(variant)) {
        answer.append(tab.value<KoText::Tab>());
    }
    return answer;
}

void KoParagraphStyle::setTabStopDistance(qreal value)
{
    setProperty(TabStopDistance, value);
}

qreal KoParagraphStyle::tabStopDistance() const
{
    return propertyDouble(TabStopDistance);
}

bool KoParagraphStyle::registerTrue() const
{
    if (hasProperty(RegisterTrue))
        return propertyBoolean(RegisterTrue);
    return false;
}

void KoParagraphStyle::setRegisterTrue(bool value)
{
    setProperty(RegisterTrue, value);
}

bool KoParagraphStyle::strictLineBreak() const
{
    if (hasProperty(StrictLineBreak))
        return propertyBoolean(StrictLineBreak);
    return false;
}

void KoParagraphStyle::setStrictLineBreak(bool value)
{
    setProperty(StrictLineBreak, value);
}

bool KoParagraphStyle::justifySingleWord() const
{
    if (hasProperty(JustifySingleWord))
        return propertyBoolean(JustifySingleWord);
    return false;
}

void KoParagraphStyle::setJustifySingleWord(bool value)
{
    setProperty(JustifySingleWord, value);
}

void KoParagraphStyle::setTextAutoSpace(KoParagraphStyle::AutoSpace value)
{
    setProperty(TextAutoSpace, value);
}

KoParagraphStyle::AutoSpace KoParagraphStyle::textAutoSpace() const
{
    if (hasProperty(TextAutoSpace))
        return static_cast<AutoSpace>(propertyInt(TextAutoSpace));
    return NoAutoSpace;
}

void KoParagraphStyle::copyProperties(const KoParagraphStyle *style)
{
    d->stylesPrivate = style->d->stylesPrivate;
    setName(style->name()); // make sure we emit property change
    KoCharacterStyle::copyProperties(style);
    d->parentStyle = style->d->parentStyle;
    d->defaultStyle = style->d->defaultStyle;
}

KoParagraphStyle *KoParagraphStyle::clone(QObject *parent) const
{
    KoParagraphStyle *newStyle = new KoParagraphStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}

bool KoParagraphStyle::compareParagraphProperties(const KoParagraphStyle &other) const
{
    return other.d->stylesPrivate == d->stylesPrivate;
}

bool KoParagraphStyle::operator==(const KoParagraphStyle &other) const
{
    if (!compareParagraphProperties(other))
        return false;
    if (!compareCharacterProperties(other))
        return false;
    return true;
}

void KoParagraphStyle::removeDuplicates(const KoParagraphStyle &other)
{
    d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
    KoCharacterStyle::removeDuplicates(other);
}

void KoParagraphStyle::saveOdf(KoGenStyle &style, KoShapeSavingContext &context) const
{
    bool writtenLineSpacing = false;
    KoCharacterStyle::saveOdf(style);

    if (listStyle()) {
        KoGenStyle liststyle(KoGenStyle::ListStyle);
        listStyle()->saveOdf(liststyle, context);
        QString name(QString(QUrl::toPercentEncoding(listStyle()->name(), "", " ")).replace('%', '_'));
        if (name.isEmpty())
            name = 'L';
        style.addAttribute("style:list-style-name", context.mainStyles().insert(liststyle, name, KoGenStyles::DontAddNumberToName));
    }
    // only custom style have a displayname. automatic styles don't have a name set.
    if (!d->name.isEmpty() && !style.isDefaultStyle()) {
        style.addAttribute("style:display-name", d->name);
    }

    QList<int> keys = d->stylesPrivate.keys();
    if (keys.contains(KoParagraphStyle::LeftPadding) && keys.contains(KoParagraphStyle::RightPadding)
            && keys.contains(KoParagraphStyle::TopPadding) && keys.contains(KoParagraphStyle::BottomPadding))
    {
        if ((leftPadding() == rightPadding()) && (topPadding() == bottomPadding()) && (rightPadding() == topPadding())) {
            style.addPropertyPt("fo:padding", leftPadding(), KoGenStyle::ParagraphType);
            keys.removeOne(KoParagraphStyle::LeftPadding);
            keys.removeOne(KoParagraphStyle::RightPadding);
            keys.removeOne(KoParagraphStyle::TopPadding);
            keys.removeOne(KoParagraphStyle::BottomPadding);
        }
    }

    if (keys.contains(QTextFormat::BlockLeftMargin) && keys.contains(QTextFormat::BlockRightMargin)
            && keys.contains(QTextFormat::BlockBottomMargin) && keys.contains(QTextFormat::BlockTopMargin))
    {
        if ((leftMargin() == rightMargin()) && (topMargin() == bottomMargin()) && (rightMargin() == topMargin())) {
            style.addPropertyLength("fo:margin", propertyLength(QTextFormat::BlockLeftMargin), KoGenStyle::ParagraphType);
            keys.removeOne(QTextFormat::BlockLeftMargin);
            keys.removeOne(QTextFormat::BlockRightMargin);
            keys.removeOne(QTextFormat::BlockTopMargin);
            keys.removeOne(QTextFormat::BlockBottomMargin);
        }
    }


    foreach (int key, keys) {
        if (key == QTextFormat::BlockAlignment) {
            int alignValue = 0;
            bool ok = false;
            alignValue = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                Qt::Alignment alignment = (Qt::Alignment) alignValue;
                QString align = KoText::alignmentToString(alignment);
                if (!align.isEmpty())
                    style.addProperty("fo:text-align", align, KoGenStyle::ParagraphType);
            }
        } else if (key == KoParagraphStyle::AlignLastLine) {
            QString align = KoText::alignmentToString(alignLastLine());
            if (!align.isEmpty())
                style.addProperty("fo:text-align-last", align, KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::TextProgressionDirection) {
            style.addProperty("style:writing-mode", KoText::directionToString(textProgressionDirection()), KoGenStyle::ParagraphType);
        } else if (key == LineNumbering) {
            style.addProperty("text:number-lines", lineNumbering());
        } else if (key == PageNumber) {
            if (pageNumber() == 0)
                style.addProperty("style:page-number", "auto", KoGenStyle::ParagraphType);
            else
                style.addProperty("style:page-number", pageNumber(), KoGenStyle::ParagraphType);
        } else if (key == LineNumberStartValue) {
            style.addProperty("text:line-number", lineNumberStartValue());
        } else if (key == BreakAfter) {
            style.addProperty("fo:break-after", KoText::textBreakToString(breakAfter()), KoGenStyle::ParagraphType);
        } else if (key == BreakBefore) {
            style.addProperty("fo:break-before", KoText::textBreakToString(breakBefore()), KoGenStyle::ParagraphType);
        } else if (key == QTextFormat::BlockNonBreakableLines) {
            if (nonBreakableLines()) {
                style.addProperty("fo:keep-together", "always", KoGenStyle::ParagraphType);
            } else {
                style.addProperty("fo:keep-together", "auto", KoGenStyle::ParagraphType);
            }
        } else if (key == QTextFormat::BackgroundBrush) {
            QBrush backBrush = background();
            if (backBrush.style() != Qt::NoBrush)
                style.addProperty("fo:background-color", backBrush.color().name(), KoGenStyle::ParagraphType);
            else
                style.addProperty("fo:background-color", "transparent", KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::BackgroundTransparency) {
            style.addProperty("style:background-transparency", QString("%1%").arg(backgroundTransparency() * 100), KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::SnapToLayoutGrid) {
            style.addProperty("style:snap-to-layout-grid", snapToLayoutGrid(), KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::JustifySingleWord) {
            style.addProperty("style:justify-single-word", justifySingleWord(), KoGenStyle::ParagraphType);
        } else if (key == RegisterTrue) {
            style.addProperty("style:register-true", registerTrue(), KoGenStyle::ParagraphType);
        } else if (key == StrictLineBreak) {
            if (strictLineBreak())
                style.addProperty("style:line-break", "strict", KoGenStyle::ParagraphType);
            else
                style.addProperty("style:line-break", "normal", KoGenStyle::ParagraphType);
        } else if (key == JoinBorder) {
            style.addProperty("style:join-border", joinBorder(), KoGenStyle::ParagraphType);
        } else if (key == OrphanThreshold) {
            style.addProperty("fo:orphans", orphanThreshold(), KoGenStyle::ParagraphType);
        } else if (key == WidowThreshold) {
            style.addProperty("fo:widows", widowThreshold(), KoGenStyle::ParagraphType);

        // Padding
        } else if (key == KoParagraphStyle::LeftPadding) {
            style.addPropertyPt("fo:padding-left", leftPadding(), KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::RightPadding) {
            style.addPropertyPt("fo:padding-right", rightPadding(), KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::TopPadding) {
            style.addPropertyPt("fo:padding-top", topPadding(), KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::BottomPadding) {
            style.addPropertyPt("fo:padding-bottom", bottomPadding(), KoGenStyle::ParagraphType);

        // Margin
        } else if (key == QTextFormat::BlockLeftMargin) {
            style.addPropertyLength("fo:margin-left", propertyLength(QTextFormat::BlockLeftMargin), KoGenStyle::ParagraphType);
        } else if (key == QTextFormat::BlockRightMargin) {
            style.addPropertyLength("fo:margin-right", propertyLength(QTextFormat::BlockRightMargin), KoGenStyle::ParagraphType);
        } else if (key == QTextFormat::BlockTopMargin) {
            style.addPropertyLength("fo:margin-top", propertyLength(QTextFormat::BlockTopMargin), KoGenStyle::ParagraphType);
        } else if (key == QTextFormat::BlockBottomMargin) {
            style.addPropertyLength("fo:margin-bottom", propertyLength(QTextFormat::BlockBottomMargin), KoGenStyle::ParagraphType);

        // Line spacing
        } else if ( key == KoParagraphStyle::MinimumLineHeight ||
                    key == KoParagraphStyle::LineSpacing ||
                    key == KoParagraphStyle::PercentLineHeight ||
                    key == KoParagraphStyle::FixedLineHeight ||
                    key == KoParagraphStyle::LineSpacingFromFont) {

            if (key == KoParagraphStyle::MinimumLineHeight && propertyLength(MinimumLineHeight).rawValue() != 0) {
                style.addPropertyLength("style:line-height-at-least", propertyLength(MinimumLineHeight), KoGenStyle::ParagraphType);
                writtenLineSpacing = true;
            } else if (key == KoParagraphStyle::LineSpacing && lineSpacing() != 0) {
                style.addPropertyPt("style:line-spacing", lineSpacing(), KoGenStyle::ParagraphType);
                writtenLineSpacing = true;
            } else if (key == KoParagraphStyle::PercentLineHeight && lineHeightPercent() != 0) {
                style.addProperty("fo:line-height", QString("%1%").arg(lineHeightPercent()), KoGenStyle::ParagraphType);
                writtenLineSpacing = true;
            } else if (key == KoParagraphStyle::FixedLineHeight && lineHeightAbsolute() != 0) {
                style.addPropertyPt("fo:line-height", lineHeightAbsolute(), KoGenStyle::ParagraphType);
                writtenLineSpacing = true;
            } else if (key == KoParagraphStyle::LineSpacingFromFont && lineHeightAbsolute() == 0) {
                style.addProperty("style:font-independent-line-spacing", lineSpacingFromFont(), KoGenStyle::ParagraphType);
                writtenLineSpacing = true;
            }
    //
        } else if (key == QTextFormat::TextIndent) {
            style.addPropertyLength("fo:text-indent", propertyLength(QTextFormat::TextIndent), KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::AutoTextIndent) {
            style.addProperty("style:auto-text-indent", autoTextIndent(), KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::TabStopDistance) {
            style.addPropertyPt("style:tab-stop-distance", tabStopDistance(), KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::MasterPageName) {
            style.addAttribute("style:master-page-name", masterPageName());
        } else if (key == KoParagraphStyle::DefaultOutlineLevel) {
            style.addAttribute("style:default-outline-level", defaultOutlineLevel());
        } else if (key == KoParagraphStyle::AutomaticWritingMode) {
            style.addProperty("style:writing-mode-automatic", automaticWritingMode(), KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::TextAutoSpace) {
            if (textAutoSpace() == NoAutoSpace)
                style.addProperty("style:text-autospace", "none", KoGenStyle::ParagraphType);
            else if (textAutoSpace() == IdeographAlpha)
                style.addProperty("style:text-autospace", "ideograph-alpha", KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::KeepWithNext) {
            if (keepWithNext())
                style.addProperty("fo:keep-with-next", "always", KoGenStyle::ParagraphType);
            else
                style.addProperty("fo:keep-with-next", "auto", KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::KeepHyphenation) {
            if (keepHyphenation())
                style.addProperty("fo:hyphenation-keep", "page", KoGenStyle::ParagraphType);
            else
                style.addProperty("fo:hyphenation-keep", "auto", KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::HyphenationLadderCount) {
            int value = hyphenationLadderCount();
            if (value == 0)
                style.addProperty("fo:hyphenation-ladder-count", "no-limit", KoGenStyle::ParagraphType);
            else
                style.addProperty("fo:hyphenation-ladder-count", value, KoGenStyle::ParagraphType);
        } else if (key == PunctuationWrap) {
            if (punctuationWrap())
                style.addProperty("style:punctuation-wrap", "simple", KoGenStyle::ParagraphType);
            else
                style.addProperty("style:punctuation-wrap", "hanging", KoGenStyle::ParagraphType);
        } else if (key == VerticalAlignment) {
            VerticalAlign valign = verticalAlignment();
            if (valign == VAlignAuto)
                style.addProperty("style:vertical-align", "auto", KoGenStyle::ParagraphType);
            else if (valign == VAlignBaseline)
                style.addProperty("style:vertical-align", "baseline", KoGenStyle::ParagraphType);
            else if (valign == VAlignBottom)
                style.addProperty("style:vertical-align", "bottom", KoGenStyle::ParagraphType);
            else if (valign == VAlignMiddle)
                style.addProperty("style:vertical-align", "middle", KoGenStyle::ParagraphType);
            else if (valign == VAlignTop)
                style.addProperty("style:vertical-align", "top", KoGenStyle::ParagraphType);
        } else if (key == Shadow) {
            style.addProperty("style:shadow", shadow().saveOdf());
        }
    }
    if (!writtenLineSpacing && propertyBoolean(NormalLineHeight))
        style.addProperty("fo:line-height", QString("normal"), KoGenStyle::ParagraphType);
    // save border stuff
    QString leftBorder = QString("%1pt %2 %3").arg(QString::number(leftBorderWidth()),
                                                   KoBorder::odfBorderStyleString(leftBorderStyle()),
                         leftBorderColor().name());
    QString rightBorder = QString("%1pt %2 %3").arg(QString::number(rightBorderWidth()),
                          KoBorder::odfBorderStyleString(rightBorderStyle()),
                          rightBorderColor().name());
    QString topBorder = QString("%1pt %2 %3").arg(QString::number(topBorderWidth()),
                        KoBorder::odfBorderStyleString(topBorderStyle()),
                        topBorderColor().name());
    QString bottomBorder = QString("%1pt %2 %3").arg(QString::number(bottomBorderWidth()),
                           KoBorder::odfBorderStyleString(bottomBorderStyle()),
                           bottomBorderColor().name());
    if (leftBorder == rightBorder && leftBorder == topBorder && leftBorder == bottomBorder) {
        if (leftBorderWidth() > 0 && leftBorderStyle() != KoBorder::BorderNone)
            style.addProperty("fo:border", leftBorder, KoGenStyle::ParagraphType);
    } else {
        if (leftBorderWidth() > 0 && leftBorderStyle() != KoBorder::BorderNone)
            style.addProperty("fo:border-left", leftBorder, KoGenStyle::ParagraphType);
        if (rightBorderWidth() > 0 && rightBorderStyle() != KoBorder::BorderNone)
            style.addProperty("fo:border-right", rightBorder, KoGenStyle::ParagraphType);
        if (topBorderWidth() > 0 && topBorderStyle() != KoBorder::BorderNone)
            style.addProperty("fo:border-top", topBorder, KoGenStyle::ParagraphType);
        if (bottomBorderWidth() > 0 && bottomBorderStyle() != KoBorder::BorderNone)
            style.addProperty("fo:border-bottom", bottomBorder, KoGenStyle::ParagraphType);
    }
    QString leftBorderLineWidth, rightBorderLineWidth, topBorderLineWidth, bottomBorderLineWidth;
    if (leftBorderStyle() == KoBorder::BorderDouble)
        leftBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(leftInnerBorderWidth()),
                                  QString::number(leftBorderSpacing()),
                                  QString::number(leftBorderWidth()));
    if (rightBorderStyle() == KoBorder::BorderDouble)
        rightBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(rightInnerBorderWidth()),
                                   QString::number(rightBorderSpacing()),
                                   QString::number(rightBorderWidth()));
    if (topBorderStyle() == KoBorder::BorderDouble)
        topBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(topInnerBorderWidth()),
                                 QString::number(topBorderSpacing()),
                                 QString::number(topBorderWidth()));
    if (bottomBorderStyle() == KoBorder::BorderDouble)
        bottomBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(bottomInnerBorderWidth()),
                                    QString::number(bottomBorderSpacing()),
                                    QString::number(bottomBorderWidth()));
    if (leftBorderLineWidth == rightBorderLineWidth &&
            leftBorderLineWidth == topBorderLineWidth &&
            leftBorderLineWidth == bottomBorderLineWidth && !leftBorderLineWidth.isEmpty()) {
        style.addProperty("style:border-line-width", leftBorderLineWidth, KoGenStyle::ParagraphType);
    } else {
        if (!leftBorderLineWidth.isEmpty())
            style.addProperty("style:border-line-width-left", leftBorderLineWidth, KoGenStyle::ParagraphType);
        if (!rightBorderLineWidth.isEmpty())
            style.addProperty("style:border-line-width-right", rightBorderLineWidth, KoGenStyle::ParagraphType);
        if (!topBorderLineWidth.isEmpty())
            style.addProperty("style:border-line-width-top", topBorderLineWidth, KoGenStyle::ParagraphType);
        if (!bottomBorderLineWidth.isEmpty())
            style.addProperty("style:border-line-width-bottom", bottomBorderLineWidth, KoGenStyle::ParagraphType);
    }
    const int indentation = 4; // indentation for children of office:styles/style:style/style:paragraph-properties
    // drop-caps
    if (dropCaps()) {
        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        KoXmlWriter elementWriter(&buf, indentation);
        elementWriter.startElement("style:drop-cap");
        elementWriter.addAttribute("style:lines", QString::number(dropCapsLines()));
        elementWriter.addAttribute("style:length", dropCapsLength() == 0 ? "word" : QString::number(dropCapsLength()));
        if (dropCapsDistance())
            elementWriter.addAttribute("style:distance", dropCapsDistance());
        elementWriter.endElement();
        QString elementContents = QString::fromUtf8(buf.buffer(), buf.buffer().size());
        style.addChildElement("style:drop-cap", elementContents, KoGenStyle::ParagraphType);
    }
    if (tabPositions().count() > 0) {
        QMap<int, QString> tabTypeMap, leaderTypeMap, leaderStyleMap, leaderWeightMap;
        tabTypeMap[QTextOption::LeftTab] = "left";
        tabTypeMap[QTextOption::RightTab] = "right";
        tabTypeMap[QTextOption::CenterTab] = "center";
        tabTypeMap[QTextOption::DelimiterTab] = "char";
        leaderTypeMap[KoCharacterStyle::NoLineType] = "none";
        leaderTypeMap[KoCharacterStyle::SingleLine] = "single";
        leaderTypeMap[KoCharacterStyle::DoubleLine] = "double";
        leaderStyleMap[KoCharacterStyle::NoLineStyle] = "none";
        leaderStyleMap[KoCharacterStyle::SolidLine] = "solid";
        leaderStyleMap[KoCharacterStyle::DottedLine] = "dotted";
        leaderStyleMap[KoCharacterStyle::DashLine] = "dash";
        leaderStyleMap[KoCharacterStyle::LongDashLine] = "long-dash";
        leaderStyleMap[KoCharacterStyle::DotDashLine] = "dot-dash";
        leaderStyleMap[KoCharacterStyle::DotDotDashLine] = "dot-dot-dash";
        leaderStyleMap[KoCharacterStyle::WaveLine] = "wave";
        leaderWeightMap[KoCharacterStyle::AutoLineWeight] = "auto";
        leaderWeightMap[KoCharacterStyle::NormalLineWeight] = "normal";
        leaderWeightMap[KoCharacterStyle::BoldLineWeight] = "bold";
        leaderWeightMap[KoCharacterStyle::ThinLineWeight] = "thin";
        leaderWeightMap[KoCharacterStyle::DashLineWeight] = "dash";
        leaderWeightMap[KoCharacterStyle::MediumLineWeight] = "medium";
        leaderWeightMap[KoCharacterStyle::ThickLineWeight] = "thick";

        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        KoXmlWriter elementWriter(&buf, indentation);
        elementWriter.startElement("style:tab-stops");
        Q_FOREACH (const KoText::Tab &tab, tabPositions()) {
            elementWriter.startElement("style:tab-stop");
            elementWriter.addAttribute("style:position", tab.position);
            if (!tabTypeMap[tab.type].isEmpty())
                elementWriter.addAttribute("style:type", tabTypeMap[tab.type]);
            if (tab.type == QTextOption::DelimiterTab && !tab.delimiter.isNull())
                elementWriter.addAttribute("style:char", tab.delimiter);
            if (!leaderTypeMap[tab.leaderType].isEmpty())
                elementWriter.addAttribute("style:leader-type", leaderTypeMap[tab.leaderType]);
            if (!leaderStyleMap[tab.leaderStyle].isEmpty())
                elementWriter.addAttribute("style:leader-style", leaderStyleMap[tab.leaderStyle]);
            if (!leaderWeightMap[tab.leaderWeight].isEmpty())
                elementWriter.addAttribute("style:leader-width", leaderWeightMap[tab.leaderWeight]);
            else if (tab.leaderWeight == KoCharacterStyle::PercentLineWeight)
                elementWriter.addAttribute("style:leader-width", QString("%1%").arg(QString::number(tab.leaderWidth)));
            else if (tab.leaderWeight == KoCharacterStyle::LengthLineWeight)
                elementWriter.addAttribute("style:leader-width", tab.leaderWidth);
            if (tab.leaderColor.isValid())
                elementWriter.addAttribute("style:leader-color", tab.leaderColor.name());
            else
                elementWriter.addAttribute("style:leader-color", "font-color");
            if (!tab.leaderText.isEmpty())
                elementWriter.addAttribute("style:leader-text", tab.leaderText);
            elementWriter.endElement();
        }
        elementWriter.endElement();
        buf.close();
        QString elementContents = QString::fromUtf8(buf.buffer(), buf.buffer().size());
        style.addChildElement("style:tab-stops", elementContents, KoGenStyle::ParagraphType);
    }
}

bool KoParagraphStyle::hasDefaults() const
{
    int size=d->stylesPrivate.properties().size();
    if ((size == 0) || (size==1 && d->stylesPrivate.properties().contains(StyleId))) {
        return true;
    }
    return false;
}

KoList *KoParagraphStyle::list() const
{
    return d->list;
}
