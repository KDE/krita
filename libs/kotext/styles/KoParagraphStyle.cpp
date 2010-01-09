/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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
#include "KoCharacterStyle.h"
#include "KoListStyle.h"
#include "KoTextBlockData.h"
#include "KoTextDocumentLayout.h"
#include "KoStyleManager.h"
#include "KoListLevelProperties.h"
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include "Styles_p.h"
#include "KoTextDocument.h"

#include <KDebug>

#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QFontMetrics>
#include <QBuffer>

#include <KoUnit.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoBorder.h>

//already defined in KoRulerController.cpp
#ifndef KDE_USE_FINAL
static int compareTabs(KoText::Tab &tab1, KoText::Tab &tab2)
{
    return tab1.position < tab2.position;
}
#endif
class KoParagraphStyle::Private
{
public:
    Private() : charStyle(0), listStyle(0), parentStyle(0), list(0), next(0) {}

    ~Private() {
    }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }

    QString name;
    KoCharacterStyle *charStyle;
    KoListStyle *listStyle;
    KoParagraphStyle *parentStyle;
    KoList *list;
    int next;
    StylePrivate stylesPrivate;
};

KoParagraphStyle::KoParagraphStyle(QObject *parent)
        : QObject(parent), d(new Private()), normalLineHeight(false)
{
    d->charStyle = new KoCharacterStyle(this);
}

KoParagraphStyle::KoParagraphStyle(const QTextBlockFormat &blockFormat, const QTextCharFormat &blockCharFormat, QObject *parent)
        : QObject(parent),
        d(new Private()),
        normalLineHeight(false)
{
    d->stylesPrivate = blockFormat.properties();
    d->charStyle = new KoCharacterStyle(blockCharFormat, this);
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

void KoParagraphStyle::setParentStyle(KoParagraphStyle *parent)
{
    d->parentStyle = parent;
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
    if (var.isNull() && d->parentStyle)
        return d->parentStyle->value(key);
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
    QList<int> keys = d->stylesPrivate.keys();
    for (int i = 0; i < keys.count(); i++) {
        QVariant variant = d->stylesPrivate.value(keys[i]);
        format.setProperty(keys[i], variant);
    }
}

void KoParagraphStyle::applyStyle(QTextBlock &block, bool applyListStyle) const
{
    QTextCursor cursor(block);
    QTextBlockFormat format = cursor.blockFormat();
    applyStyle(format);
    cursor.setBlockFormat(format);
    if (d->parentStyle && d->parentStyle->characterStyle())
        d->parentStyle->characterStyle()->applyStyle(block);
    if (d->charStyle) {
        d->charStyle->applyStyle(block);
    }

    if (applyListStyle) {
        if (d->listStyle) {
            if (!d->list)
                d->list = new KoList(block.document(), d->listStyle);
            d->list->add(block, listLevel());
        } else {
            if (block.textList())
                block.textList()->remove(block);
            KoTextBlockData *data = dynamic_cast<KoTextBlockData*>(block.userData());
            if (data)
                data->setCounterWidth(-1);
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
        if (variant == format.property(keys[i]))
            format.clearProperty(keys[i]);
    }
    cursor.setBlockFormat(format);
    if (d->charStyle)
        d->charStyle->unapplyStyle(block);
    if (d->listStyle && block.textList()) // TODO check its the same one?
        block.textList()->remove(block);
}

void KoParagraphStyle::setLineHeightPercent(int lineHeight)
{
    setProperty(PercentLineHeight, lineHeight);
    normalLineHeight = false;
}

int KoParagraphStyle::lineHeightPercent() const
{
    return propertyInt(PercentLineHeight);
}

void KoParagraphStyle::setLineHeightAbsolute(qreal height)
{
    setProperty(FixedLineHeight, height);
    normalLineHeight = false;
}

qreal KoParagraphStyle::lineHeightAbsolute() const
{
    return propertyDouble(FixedLineHeight);
}

void KoParagraphStyle::setMinimumLineHeight(qreal height)
{
    setProperty(MinimumLineHeight, height);
    normalLineHeight = false;
}

qreal KoParagraphStyle::minimumLineHeight() const
{
    return propertyDouble(MinimumLineHeight);
}

void KoParagraphStyle::setLineSpacing(qreal spacing)
{
    setProperty(LineSpacing, spacing);
    normalLineHeight = false;
}

qreal KoParagraphStyle::lineSpacing() const
{
    return propertyDouble(LineSpacing);
}

void KoParagraphStyle::setLineSpacingFromFont(bool on)
{
    setProperty(LineSpacingFromFont, on);
    normalLineHeight = false;
}

bool KoParagraphStyle::lineSpacingFromFont() const
{
    return propertyBoolean(LineSpacingFromFont);
}

void KoParagraphStyle::setAlignLastLine(Qt::Alignment alignment)
{
    setProperty(AlignLastLine, (int) alignment);
}

Qt::Alignment KoParagraphStyle::alignLastLine() const
{
    return static_cast<Qt::Alignment>(propertyInt(QTextFormat::BlockAlignment));
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

void KoParagraphStyle::setFollowDocBaseline(bool on)
{
    setProperty(FollowDocBaseline, on);
}

bool KoParagraphStyle::followDocBaseline() const
{
    return propertyBoolean(FollowDocBaseline);
}

void KoParagraphStyle::setBreakBefore(bool on)
{
    setProperty(BreakBefore, on);
}

bool KoParagraphStyle::breakBefore()
{
    return propertyBoolean(BreakBefore);
}

void KoParagraphStyle::setBreakAfter(bool on)
{
    setProperty(BreakAfter, on);
}

bool KoParagraphStyle::breakAfter()
{
    return propertyBoolean(BreakAfter);
}

void KoParagraphStyle::setLeftPadding(qreal padding)
{
    setProperty(LeftPadding, padding);
}

qreal KoParagraphStyle::leftPadding()
{
    return propertyDouble(LeftPadding);
}

void KoParagraphStyle::setTopPadding(qreal padding)
{
    setProperty(TopPadding, padding);
}

qreal KoParagraphStyle::topPadding()
{
    return propertyDouble(TopPadding);
}

void KoParagraphStyle::setRightPadding(qreal padding)
{
    setProperty(RightPadding, padding);
}

qreal KoParagraphStyle::rightPadding()
{
    return propertyDouble(RightPadding);
}

void KoParagraphStyle::setBottomPadding(qreal padding)
{
    setProperty(BottomPadding, padding);
}

qreal KoParagraphStyle::bottomPadding()
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

qreal KoParagraphStyle::leftBorderWidth()
{
    return propertyDouble(LeftBorderWidth);
}

void KoParagraphStyle::setLeftInnerBorderWidth(qreal width)
{
    setProperty(LeftInnerBorderWidth, width);
}

qreal KoParagraphStyle::leftInnerBorderWidth()
{
    return propertyDouble(LeftInnerBorderWidth);
}

void KoParagraphStyle::setLeftBorderSpacing(qreal width)
{
    setProperty(LeftBorderSpacing, width);
}

qreal KoParagraphStyle::leftBorderSpacing()
{
    return propertyDouble(LeftBorderSpacing);
}

void KoParagraphStyle::setLeftBorderStyle(KoBorder::BorderStyle style)
{
    setProperty(LeftBorderStyle, style);
}

KoBorder::BorderStyle KoParagraphStyle::leftBorderStyle()
{
    return static_cast<KoBorder::BorderStyle>(propertyInt(LeftBorderStyle));
}

void KoParagraphStyle::setLeftBorderColor(const QColor &color)
{
    setProperty(LeftBorderColor, color);
}

QColor KoParagraphStyle::leftBorderColor()
{
    return propertyColor(LeftBorderColor);
}

void KoParagraphStyle::setTopBorderWidth(qreal width)
{
    setProperty(TopBorderWidth, width);
}

qreal KoParagraphStyle::topBorderWidth()
{
    return propertyDouble(TopBorderWidth);
}

void KoParagraphStyle::setTopInnerBorderWidth(qreal width)
{
    setProperty(TopInnerBorderWidth, width);
}

qreal KoParagraphStyle::topInnerBorderWidth()
{
    return propertyDouble(TopInnerBorderWidth);
}

void KoParagraphStyle::setTopBorderSpacing(qreal width)
{
    setProperty(TopBorderSpacing, width);
}

qreal KoParagraphStyle::topBorderSpacing()
{
    return propertyDouble(TopBorderSpacing);
}

void KoParagraphStyle::setTopBorderStyle(KoBorder::BorderStyle style)
{
    setProperty(TopBorderStyle, style);
}

KoBorder::BorderStyle KoParagraphStyle::topBorderStyle()
{
    return static_cast<KoBorder::BorderStyle>(propertyInt(TopBorderStyle));
}

void KoParagraphStyle::setTopBorderColor(const QColor &color)
{
    setProperty(TopBorderColor, color);
}

QColor KoParagraphStyle::topBorderColor()
{
    return propertyColor(TopBorderColor);
}

void KoParagraphStyle::setRightBorderWidth(qreal width)
{
    setProperty(RightBorderWidth, width);
}

qreal KoParagraphStyle::rightBorderWidth()
{
    return propertyDouble(RightBorderWidth);
}

void KoParagraphStyle::setRightInnerBorderWidth(qreal width)
{
    setProperty(RightInnerBorderWidth, width);
}

qreal KoParagraphStyle::rightInnerBorderWidth()
{
    return propertyDouble(RightInnerBorderWidth);
}

void KoParagraphStyle::setRightBorderSpacing(qreal width)
{
    setProperty(RightBorderSpacing, width);
}

qreal KoParagraphStyle::rightBorderSpacing()
{
    return propertyDouble(RightBorderSpacing);
}

void KoParagraphStyle::setRightBorderStyle(KoBorder::BorderStyle style)
{
    setProperty(RightBorderStyle, style);
}

KoBorder::BorderStyle KoParagraphStyle::rightBorderStyle()
{
    return static_cast<KoBorder::BorderStyle>(propertyInt(RightBorderStyle));
}

void KoParagraphStyle::setRightBorderColor(const QColor &color)
{
    setProperty(RightBorderColor, color);
}

QColor KoParagraphStyle::rightBorderColor()
{
    return propertyColor(RightBorderColor);
}

void KoParagraphStyle::setBottomBorderWidth(qreal width)
{
    setProperty(BottomBorderWidth, width);
}

qreal KoParagraphStyle::bottomBorderWidth()
{
    return propertyDouble(BottomBorderWidth);
}

void KoParagraphStyle::setBottomInnerBorderWidth(qreal width)
{
    setProperty(BottomInnerBorderWidth, width);
}

qreal KoParagraphStyle::bottomInnerBorderWidth()
{
    return propertyDouble(BottomInnerBorderWidth);
}

void KoParagraphStyle::setBottomBorderSpacing(qreal width)
{
    setProperty(BottomBorderSpacing, width);
}

qreal KoParagraphStyle::bottomBorderSpacing()
{
    return propertyDouble(BottomBorderSpacing);
}

void KoParagraphStyle::setBottomBorderStyle(KoBorder::BorderStyle style)
{
    setProperty(BottomBorderStyle, style);
}

KoBorder::BorderStyle KoParagraphStyle::bottomBorderStyle()
{
    return static_cast<KoBorder::BorderStyle>(propertyInt(BottomBorderStyle));
}

void KoParagraphStyle::setBottomBorderColor(const QColor &color)
{
    setProperty(BottomBorderColor, color);
}

QColor KoParagraphStyle::bottomBorderColor()
{
    return propertyColor(BottomBorderColor);
}

void KoParagraphStyle::setTopMargin(qreal topMargin)
{
    setProperty(QTextFormat::BlockTopMargin, topMargin);
}

qreal KoParagraphStyle::topMargin() const
{
    return propertyDouble(QTextFormat::BlockTopMargin);
}

void KoParagraphStyle::setBottomMargin(qreal margin)
{
    setProperty(QTextFormat::BlockBottomMargin, margin);
}

qreal KoParagraphStyle::bottomMargin() const
{
    return propertyDouble(QTextFormat::BlockBottomMargin);
}

void KoParagraphStyle::setLeftMargin(qreal margin)
{
    setProperty(QTextFormat::BlockLeftMargin, margin);
}

qreal KoParagraphStyle::leftMargin() const
{
    return propertyDouble(QTextFormat::BlockLeftMargin);
}

void KoParagraphStyle::setRightMargin(qreal margin)
{
    setProperty(QTextFormat::BlockRightMargin, margin);
}

qreal KoParagraphStyle::rightMargin() const
{
    return propertyDouble(QTextFormat::BlockRightMargin);
}

void KoParagraphStyle::setMargin(qreal margin)
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

void KoParagraphStyle::setTextIndent(qreal margin)
{
    setProperty(QTextFormat::TextIndent, margin);
}

qreal KoParagraphStyle::textIndent() const
{
    return propertyDouble(QTextFormat::TextIndent);
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

KoParagraphStyle *KoParagraphStyle::parentStyle() const
{
    return d->parentStyle;
}

void KoParagraphStyle::setNextStyle(int next)
{
    d->next = next;
}

int KoParagraphStyle::nextStyle() const
{
    return d->next;
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
    emit nameChanged(name);
}

int KoParagraphStyle::styleId() const
{
    return propertyInt(StyleId);
}

void KoParagraphStyle::setStyleId(int id)
{
    setProperty(StyleId, id); if (d->next == 0) d->next = id;
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

void KoParagraphStyle::setIsListHeader(bool on)
{
    setProperty(IsListHeader, on);
}

bool KoParagraphStyle::isListHeader() const
{
    return propertyBoolean(IsListHeader);
}

KoCharacterStyle *KoParagraphStyle::characterStyle()
{
    return d->charStyle;
}

const KoCharacterStyle *KoParagraphStyle::characterStyle() const
{
    return d->charStyle;
}

void KoParagraphStyle::setCharacterStyle(KoCharacterStyle *style)
{
    if (d->charStyle == style)
        return;
    if (d->charStyle && d->charStyle->parent() == this)
        delete d->charStyle;
    d->charStyle = style;
}

KoListStyle *KoParagraphStyle::listStyle() const
{
    return d->listStyle;
}

void KoParagraphStyle::setListStyle(KoListStyle *style)
{
    if (d->listStyle == style)
        return;
    if (d->listStyle && d->listStyle->parent() == this)
        delete d->listStyle;
    d->listStyle = style;
}

KoText::Direction KoParagraphStyle::textProgressionDirection() const
{
    return static_cast<KoText::Direction>(propertyInt(TextProgressionDirection));
}

void KoParagraphStyle::setTextProgressionDirection(KoText::Direction dir)
{
    setProperty(TextProgressionDirection, dir);
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

void KoParagraphStyle::loadOdf(const KoXmlElement *element, KoOdfLoadingContext &context)
{
    if (element->hasAttributeNS(KoXmlNS::style, "display-name"))
        d->name = element->attributeNS(KoXmlNS::style, "display-name", QString());

    if (d->name.isEmpty()) // if no style:display-name is given us the style:name
        d->name = element->attributeNS(KoXmlNS::style, "name", QString());

#if 0 //1.6:
    // OOo hack:
    //m_bOutline = name.startsWith( "Heading" );
    // real OASIS solution:
    bool m_bOutline = element->hasAttributeNS(KoXmlNS::style, "default-outline-level");
#endif
    context.styleStack().save();
    // Load all parents - only because we don't support inheritance.
    QString family = element->attributeNS(KoXmlNS::style, "family", "paragraph");
    context.addStyles(element, family.toLocal8Bit().constData());   // Load all parents - only because we don't support inheritance.

    //setParent( d->stylemanager->defaultParagraphStyle() );

    QString masterPage = element->attributeNS(KoXmlNS::style, "master-page-name", QString());
    if (! masterPage.isEmpty()) {
        setMasterPageName(masterPage);
    }

    //1.6: KoTextFormat::load
    KoCharacterStyle *charstyle = characterStyle();
    context.styleStack().setTypeProperties("text");   // load all style attributes from "style:text-properties"
    charstyle->loadOdf(context);   // load the KoCharacterStyle from the stylestack

    //1.6: KoTextParag::loadOasis => KoParagLayout::loadOasisParagLayout
    context.styleStack().setTypeProperties("paragraph");   // load all style attributes from "style:paragraph-properties"
    loadOdfProperties(context.styleStack());   // load the KoParagraphStyle from the stylestack

    context.styleStack().restore();
}

void KoParagraphStyle::loadOdfProperties(KoStyleStack &styleStack)
{
    // in 1.6 this was defined at KoParagLayout::loadOasisParagLayout(KoParagLayout&, KoOasisContext&)

    if (styleStack.hasProperty(KoXmlNS::style, "writing-mode")) {     // http://www.w3.org/TR/2004/WD-xsl11-20041216/#writing-mode
        QString writingMode = styleStack.property(KoXmlNS::style, "writing-mode");
        setTextProgressionDirection(KoText::directionFromString(writingMode));
    }

    // Alignment
    if (styleStack.hasProperty(KoXmlNS::fo, "text-align")) {
        setAlignment(KoText::alignmentFromString(styleStack.property(KoXmlNS::fo, "text-align")));
    }

    // Spacing (padding)
    if (styleStack.hasProperty(KoXmlNS::fo, "padding-left"))
        setLeftPadding(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "padding-left")));
    if (styleStack.hasProperty(KoXmlNS::fo, "padding-right"))
        setRightPadding(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "padding-right")));
    if (styleStack.hasProperty(KoXmlNS::fo, "padding-top"))
        setTopPadding(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "padding-top")));
    if (styleStack.hasProperty(KoXmlNS::fo, "padding-bottom"))
        setBottomPadding(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "padding-bottom")));
    if (styleStack.hasProperty(KoXmlNS::fo, "padding"))
        setPadding(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "padding")));

    // Indentation (margin)
    bool hasMarginLeft = styleStack.hasProperty(KoXmlNS::fo, "margin-left");
    bool hasMarginRight = styleStack.hasProperty(KoXmlNS::fo, "margin-right");
    if (hasMarginLeft)
        setLeftMargin(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "margin-left")));
    if (hasMarginRight)
        setRightMargin(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "margin-right")));
    if (styleStack.hasProperty(KoXmlNS::fo, "margin-top"))
        setTopMargin(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "margin-top")));
    if (styleStack.hasProperty(KoXmlNS::fo, "margin-bottom"))
        setBottomMargin(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "margin-bottom")));
    if (styleStack.hasProperty(KoXmlNS::fo, "margin")) {
        setMargin(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "margin")));
        hasMarginLeft = true;
        hasMarginRight = true;
    }

    // Automatic Text indent
    // OOo is not assuming this. Commenting this line thus allow more OpenDocuments to be supported, including a
    // testcase from the ODF test suite. See §15.5.18 in the spec.
    //if ( hasMarginLeft || hasMarginRight ) {
    // style:auto-text-indent takes precedence
    if (styleStack.hasProperty(KoXmlNS::style, "auto-text-indent") &&
            styleStack.property(KoXmlNS::style, "auto-text-indent") != "false") {
        // "indented by a value that is based on the current font size"
        const QString autotextindent = styleStack.property(KoXmlNS::style, "auto-text-indent");
        if (autotextindent == "true") {
            setAutoTextIndent(true);
        }
    } else if (styleStack.hasProperty(KoXmlNS::fo, "text-indent")) {
        setTextIndent(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "text-indent")));
    }

    //}

    // Line spacing
    if (styleStack.hasProperty(KoXmlNS::fo, "line-height")) {    // 3.11.1
        // Fixed line height
        QString value = styleStack.property(KoXmlNS::fo, "line-height");
        if (value != "normal") {
            if (value.indexOf('%') > -1)   // percent value
                setLineHeightPercent(value.remove('%').toInt());
            else // fixed value
                setLineHeightAbsolute(KoUnit::parseValue(value));
        }
        else
            normalLineHeight = true;
    } // Line-spacing is mutually exclusive with line-height
    else if (styleStack.hasProperty(KoXmlNS::style, "line-spacing")) {    // 3.11.3
        setLineSpacing(KoUnit::parseValue(styleStack.property(KoXmlNS::style, "line-spacing")));
    }
    if (styleStack.hasProperty(KoXmlNS::style, "line-height-at-least") && !normalLineHeight && lineHeightAbsolute() == 0) {    // 3.11.2
        setMinimumLineHeight(KoUnit::parseValue(styleStack.property(KoXmlNS::style, "line-height-at-least")));
    }  // Line-height-at-least is mutually exclusive with absolute line-height
    if (styleStack.hasProperty(KoXmlNS::style, "font-independent-line-spacing") && !normalLineHeight && lineHeightAbsolute() == 0) {
        setLineSpacingFromFont(styleStack.property(KoXmlNS::style, "font-independent-line-spacing") == "true");
    }

    // Tabulators
    if (styleStack.hasProperty(KoXmlNS::style, "tab-stop-distance")) {
        qreal tabStopDistance = KoUnit::parseValue(styleStack.property(KoXmlNS::style, "tab-stop-distance"));
        if (tabStopDistance > 0)
            setTabStopDistance(tabStopDistance);
    }
    if (styleStack.hasChildNode(KoXmlNS::style, "tab-stops")) {     // 3.11.10
        QList<KoText::Tab> tabList;
        KoXmlElement tabStops = styleStack.childNode(KoXmlNS::style, "tab-stops");
        KoXmlElement tabStop;
        forEachElement(tabStop, tabStops) {
            Q_ASSERT(tabStop.localName() == "tab-stop");
            // Tab position
            KoText::Tab tab;
            tab.position = KoUnit::parseValue(tabStop.attributeNS(KoXmlNS::style, "position", QString()));
            kDebug(32500) << "tab position " << tab.position;
            // Tab stop positions in the XML are relative to the left-margin
            // Equivalently, relative to the left end of our textshape
            // Tab type (left/right/center/char)
            const QString type = tabStop.attributeNS(KoXmlNS::style, "type", QString());
            if (type == "center")
                tab.type = QTextOption::CenterTab;
            else if (type == "right")
                tab.type = QTextOption::RightTab;
            else if (type == "char")
                tab.type = QTextOption::DelimiterTab;
            else //if ( type == "left" )
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
                    case '_':  // TODO in KWord: differentiate --- and ___
                        tab.filling = TF_LINE; break;
                    default:
                        // KWord doesn't have support for "any char" as filling.
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
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "left")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "left");
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            // ## isn't it faster to use QStringList::split than parse it 3 times?
            QString borderwidth = border.section(' ', 0, 0);
            QString borderstyle = border.section(' ', 1, 1);
            QString bordercolor = border.section(' ', 2, 2);

            setLeftBorderWidth(KoUnit::parseValue(borderwidth, 1.0));
            //setLeftInnerBorderWidth(qreal width);
            //setLeftBorderSpacing(qreal width);
            setLeftBorderStyle(KoBorder::odfBorderStyle(borderstyle));
            setLeftBorderColor(QColor(bordercolor));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "top")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "top");
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setTopBorderWidth(KoUnit::parseValue(border.section(' ', 0, 0), 1.0));
            setTopBorderStyle(KoBorder::odfBorderStyle(border.section(' ', 1, 1)));
            setTopBorderColor(QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "right")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "right");
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setRightBorderWidth(KoUnit::parseValue(border.section(' ', 0, 0), 1.0));
            setRightBorderStyle(KoBorder::odfBorderStyle(border.section(' ', 1, 1)));
            setRightBorderColor(QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border", "bottom")) {
        QString border = styleStack.property(KoXmlNS::fo, "border", "bottom");
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setBottomBorderWidth(KoUnit::parseValue(border.section(' ', 0, 0), 1.0));
            setBottomBorderStyle(KoBorder::odfBorderStyle(border.section(' ', 1, 1)));
            setBottomBorderColor(QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width", "left")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width", "left");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setLeftInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
            setLeftBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
            setLeftBorderWidth(KoUnit::parseValue(blw[2], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width", "top")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width", "top");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setTopInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
            setTopBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
            setTopBorderWidth(KoUnit::parseValue(blw[2], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width", "right")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width", "right");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setRightInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
            setRightBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
            setRightBorderWidth(KoUnit::parseValue(blw[2], 0.1));
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "border-line-width", "bottom")) {
        QString borderLineWidth = styleStack.property(KoXmlNS::style, "border-line-width", "bottom");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setBottomInnerBorderWidth(KoUnit::parseValue(blw[0], 0.1));
            setBottomBorderSpacing(KoUnit::parseValue(blw[1], 1.0));
            setBottomBorderWidth(KoUnit::parseValue(blw[2], 0.1));
        }
    }

    // drop caps
    if (styleStack.hasChildNode(KoXmlNS::style, "drop-cap")) {
        KoXmlElement dropCap = styleStack.childNode(KoXmlNS::style, "drop-cap");
        setDropCaps(true);
        const QString length = dropCap.attributeNS(KoXmlNS::style, "length", QString("1"));
        if (length.toLower() == "word") {
            setDropCapsLength(-1); // -1 indicates drop caps of the whole first word
        } else {
            setDropCapsLength(length.toInt());
        }
        const QString lines = dropCap.attributeNS(KoXmlNS::style, "lines", QString("1"));
        setDropCapsLines(lines.toInt());
        const qreal distance = KoUnit::parseValue(dropCap.attributeNS(KoXmlNS::style, "distance", QString()));
        setDropCapsDistance(distance);
    }

    // Page breaking
#if 0
    int pageBreaking = 0;
    if (styleStack.hasProperty(KoXmlNS::fo, "break-before") ||
            styleStack.hasProperty(KoXmlNS::fo, "break-after") ||
            styleStack.hasProperty(KoXmlNS::fo, "keep-together") ||
            styleStack.hasProperty(KoXmlNS::style, "keep-with-next") ||
            styleStack.hasProperty(KoXmlNS::fo, "keep-with-next")) {
        if (styleStack.hasProperty(KoXmlNS::fo, "break-before")) {    // 3.11.24
            // TODO in KWord: implement difference between "column" and "page"
            if (styleStack.property(KoXmlNS::fo, "break-before") != "auto")
                pageBreaking |= KoParagLayout::HardFrameBreakBefore;
        } else if (styleStack.hasProperty(KoXmlNS::fo, "break-after")) {   // 3.11.24
            // TODO in KWord: implement difference between "column" and "page"
            if (styleStack.property(KoXmlNS::fo, "break-after") != "auto")
                pageBreaking |= KoParagLayout::HardFrameBreakAfter;
        }

        if (styleStack.hasProperty(KoXmlNS::fo, "keep-together")) {     // was style:break-inside in OOo-1.1, renamed in OASIS
            if (styleStack.property(KoXmlNS::fo, "keep-together") != "auto")
                pageBreaking |= KoParagLayout::KeepLinesTogether;
        }
        if (styleStack.hasProperty(KoXmlNS::fo, "keep-with-next")) {
            // OASIS spec says it's "auto"/"always", not a boolean.
            QString val = styleStack.property(KoXmlNS::fo, "keep-with-next");
            if (val == "true" || val == "always")
                pageBreaking |= KoParagLayout::KeepWithNext;
        }
    }
    layout.pageBreaking = pageBreaking;
#else

    // The fo:break-before and fo:break-after attributes insert a page or column break before or after a paragraph.
    if (styleStack.hasProperty(KoXmlNS::fo, "break-before")) {
        if (styleStack.property(KoXmlNS::fo, "break-before") != "auto")
            setBreakBefore(true);
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "break-after")) {
        if (styleStack.property(KoXmlNS::fo, "break-after") != "auto")
            setBreakAfter(true);
    }

#endif

#if 0
    // Paragraph background color -  fo:background-color
    // The background color for parts of a paragraph that have no text underneath
    if (styleStack.hasProperty(KoXmlNS::fo, "background-color")) {
        QString bgColor = styleStack.property(KoXmlNS::fo, "background-color");
        if (bgColor != "transparent")
            layout.backgroundColor.setNamedColor(bgColor);
    }
#endif

    // The fo:background-color attribute specifies the background color of a paragraph.
    if (styleStack.hasProperty(KoXmlNS::fo, "background-color")) {
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
    //following properties KoParagraphStyle provides us are not handled now;
    // LineSpacingFromFont,
    // AlignLastLine,
    // WidowThreshold,
    // OrphanThreshold,
    // FollowDocBaseline,

}

void KoParagraphStyle::setTabPositions(const QList<KoText::Tab> &tabs)
{
    QList<KoText::Tab> newTabs = tabs;
    qSort(newTabs.begin(), newTabs.end(), compareTabs);
    QList<QVariant> list;
    foreach(const KoText::Tab &tab, tabs) {
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
    foreach(const QVariant &tab, qvariant_cast<QList<QVariant> >(variant)) {
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

void KoParagraphStyle::copyProperties(const KoParagraphStyle *style)
{
    d->stylesPrivate = style->d->stylesPrivate;
    setName(style->name()); // make sure we emit property change
    if (d->charStyle && d->charStyle->parent() == this)
        delete d->charStyle;
    d->charStyle = style->d->charStyle->clone(this);
    d->next = style->d->next;
    d->parentStyle = style->d->parentStyle;
    d->listStyle = style->d->listStyle;
}

KoParagraphStyle *KoParagraphStyle::clone(QObject *parent)
{
    KoParagraphStyle *newStyle = new KoParagraphStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}

bool KoParagraphStyle::compareParagraphProperties(const KoParagraphStyle &other) const
{
    return other.d->stylesPrivate == d->stylesPrivate;
}

bool KoParagraphStyle::compareCharacterProperties(const KoParagraphStyle &other) const
{
    if (d->charStyle == 0 && other.d->charStyle == 0)
        return true;
    if (!d->charStyle || !other.d->charStyle)
        return false;
    return *d->charStyle == *other.d->charStyle;
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
    if (d->charStyle && other.d->charStyle)
        d->charStyle->removeDuplicates(*other.d->charStyle);
}

void KoParagraphStyle::saveOdf(KoGenStyle &style, KoGenStyles &mainStyles)
{
    bool writtenLineSpacing = false;
    if (d->charStyle) {
        d->charStyle->saveOdf(style);
    }
    if (d->listStyle) {
        KoGenStyle liststyle(KoGenStyle::StyleList);
        d->listStyle->saveOdf(liststyle);
        QString name(QString(QUrl::toPercentEncoding(d->listStyle->name(), "", " ")).replace('%', '_'));
        if (name.isEmpty())
            name = "L";
        style.addAttribute("style:list-style-name", mainStyles.lookup(liststyle, name, KoGenStyles::DontForceNumbering));
    }
    // only custom style have a displayname. automatic styles don't have a name set.
    if (!d->name.isEmpty() && !style.isDefaultStyle()) {
        style.addAttribute("style:display-name", d->name);
    }

    QList<int> keys = d->stylesPrivate.keys();
    foreach(int key, keys) {
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
        } else if (key == KoParagraphStyle::TextProgressionDirection) {
            int directionValue = 0;
            bool ok = false;
            directionValue = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                QString direction;
                if (directionValue == KoText::LeftRightTopBottom)
                    direction = "lr-tb";
                else if (directionValue == KoText::RightLeftTopBottom)
                    direction = "rl-tb";
                else if (directionValue == KoText::TopBottomRightLeft)
                    direction = "tb-lr";
                else if (directionValue == KoText::InheritDirection)
                    direction = "page";
                if (!direction.isEmpty())
                    style.addProperty("style:writing-mode", direction, KoGenStyle::ParagraphType);
            }
        } else if (key == QTextFormat::PageBreakPolicy) {
            QTextFormat::PageBreakFlag pageBreak = static_cast<QTextFormat::PageBreakFlag>(d->stylesPrivate.value(key).toInt());
            if (pageBreak == QTextFormat::PageBreak_AlwaysBefore)
                style.addProperty("fo:break-before", "page", KoGenStyle::ParagraphType);
            else if (pageBreak == QTextFormat::PageBreak_AlwaysAfter)
                style.addProperty("fo:break-after", "page", KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::BreakBefore) {
            if (breakBefore())
                style.addProperty("fo:break-before", "page", KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::BreakAfter) {
            if (breakAfter())
                style.addProperty("fo:break-after", "page", KoGenStyle::ParagraphType);
        } else if (key == QTextFormat::BackgroundBrush) {
            QBrush backBrush = background();
            if (backBrush.style() != Qt::NoBrush)
                style.addProperty("fo:background-color", backBrush.color().name(), KoGenStyle::ParagraphType);
            else
                style.addProperty("fo:background-color", "transparent", KoGenStyle::ParagraphType);
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
            style.addPropertyPt("fo:margin-left", leftMargin(), KoGenStyle::ParagraphType);
        } else if (key == QTextFormat::BlockRightMargin) {
            style.addPropertyPt("fo:margin-right", rightMargin(), KoGenStyle::ParagraphType);
        } else if (key == QTextFormat::BlockTopMargin) {
            style.addPropertyPt("fo:margin-top", topMargin(), KoGenStyle::ParagraphType);
        } else if (key == QTextFormat::BlockBottomMargin) {
            style.addPropertyPt("fo:margin-bottom", bottomMargin(), KoGenStyle::ParagraphType);
    // Line spacing
        } else if ( key == KoParagraphStyle::MinimumLineHeight ||
                    key == KoParagraphStyle::LineSpacing ||
                    key == KoParagraphStyle::PercentLineHeight ||
                    key == KoParagraphStyle::FixedLineHeight ||
                    key == KoParagraphStyle::LineSpacingFromFont) {

            if (key == KoParagraphStyle::MinimumLineHeight && minimumLineHeight() > 0) {
                style.addPropertyPt("style:line-height-at-least", minimumLineHeight(), KoGenStyle::ParagraphType);
                writtenLineSpacing = true;
            } else if (key == KoParagraphStyle::LineSpacing && lineSpacing() > 0) {
                style.addPropertyPt("style:line-spacing", lineSpacing(), KoGenStyle::ParagraphType);
                writtenLineSpacing = true;
            } else if (key == KoParagraphStyle::PercentLineHeight && lineHeightPercent() > 0) {
                style.addProperty("fo:line-height", QString("%1%").arg(lineHeightPercent()), KoGenStyle::ParagraphType);
                writtenLineSpacing = true;
            } else if (key == KoParagraphStyle::FixedLineHeight && lineHeightAbsolute() > 0) {
                style.addPropertyPt("fo:line-height", lineHeightAbsolute(), KoGenStyle::ParagraphType);
                writtenLineSpacing = true;
            } else if (key == KoParagraphStyle::LineSpacingFromFont && lineHeightAbsolute() == 0) {
                style.addProperty("style:font-independent-line-spacing", lineSpacingFromFont(), KoGenStyle::ParagraphType);
                writtenLineSpacing = true;
            }
    //
        } else if (key == QTextFormat::TextIndent) {
            style.addPropertyPt("fo:text-indent", textIndent(), KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::AutoTextIndent) {
            style.addProperty("style:auto-text-indent", autoTextIndent(), KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::TabStopDistance) {
            style.addPropertyPt("style:tab-stop-distance", tabStopDistance(), KoGenStyle::ParagraphType);
        } else if (key == KoParagraphStyle::MasterPageName) {
            style.addProperty("style:master-page-name", masterPageName());
        }
    }
    if (!writtenLineSpacing && normalLineHeight)
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
    QString leftBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(leftInnerBorderWidth()),
                                  QString::number(leftBorderSpacing()),
                                  QString::number(leftBorderWidth()));
    QString rightBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(rightInnerBorderWidth()),
                                   QString::number(rightBorderSpacing()),
                                   QString::number(rightBorderWidth()));
    QString topBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(topInnerBorderWidth()),
                                 QString::number(topBorderSpacing()),
                                 QString::number(topBorderWidth()));
    QString bottomBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(bottomInnerBorderWidth()),
                                    QString::number(bottomBorderSpacing()),
                                    QString::number(bottomBorderWidth()));
    if (leftBorderLineWidth == rightBorderLineWidth &&
            leftBorderLineWidth == topBorderLineWidth &&
            leftBorderLineWidth == bottomBorderLineWidth &&
            leftBorderStyle() == KoBorder::BorderDouble &&
            rightBorderStyle() == KoBorder::BorderDouble &&
            topBorderStyle() == KoBorder::BorderDouble &&
            bottomBorderStyle() == KoBorder::BorderDouble) {
        style.addProperty("style:border-line-width", leftBorderLineWidth, KoGenStyle::ParagraphType);
    } else {
        if (leftBorderStyle() == KoBorder::BorderDouble)
            style.addProperty("style:border-line-width-left", leftBorderLineWidth, KoGenStyle::ParagraphType);
        if (rightBorderStyle() == KoBorder::BorderDouble)
            style.addProperty("style:border-line-width-right", rightBorderLineWidth, KoGenStyle::ParagraphType);
        if (topBorderStyle() == KoBorder::BorderDouble)
            style.addProperty("style:border-line-width-top", topBorderLineWidth, KoGenStyle::ParagraphType);
        if (bottomBorderStyle() == KoBorder::BorderDouble)
            style.addProperty("style:border-line-width-bottom", bottomBorderLineWidth, KoGenStyle::ParagraphType);
    }
    // drop-caps
    if (dropCaps()) {
        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        KoXmlWriter elementWriter(&buf);
        elementWriter.startElement("style:drop-cap");
        elementWriter.addAttribute("style:lines", QString::number(dropCapsLines()));
        elementWriter.addAttribute("style:length", dropCapsLength() < 0 ? "word" : QString::number(dropCapsLength()));
        if (dropCapsDistance())
            elementWriter.addAttributePt("style:distance", dropCapsDistance());
        elementWriter.endElement();
        QString elementContents = QString::fromUtf8(buf.buffer(), buf.buffer().size());
        style.addChildElement("style:drop-cap", elementContents);
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
        KoXmlWriter elementWriter(&buf);
        elementWriter.startElement("style:tab-stops");
        foreach(const KoText::Tab &tab, tabPositions()) {
            elementWriter.startElement("style:tab-stop");
            elementWriter.addAttributePt("style:position", tab.position);
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
                elementWriter.addAttributePt("style:leader-width", tab.leaderWidth);
            if (tab.leaderColor.isValid())
                elementWriter.addAttribute("style:leader-color", tab.leaderColor.name());
            else
                elementWriter.addAttribute("style:leader-color", "font-color");
            if (!tab.leaderText.isEmpty())
                elementWriter.addAttribute("style:leader-text", tab.leaderText);
            elementWriter.endElement();
        }
        elementWriter.endElement();
        QString elementContents = QString::fromUtf8(buf.buffer(), buf.buffer().size());
        style.addChildElement("style:tab-stops", elementContents);

    }
}

#include <KoParagraphStyle.moc>
