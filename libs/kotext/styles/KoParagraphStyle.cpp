/* This file is part of the KDE project
* Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "Styles_p.h"

#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>

#include <KoUnit.h>
#include <KoStyleStack.h>
#include <KoXmlNS.h>

class KoParagraphStyle::Private {
public:
    Private() : charStyle(0), listStyle(0), parent(0), next(0), stylesPrivate(0) {}

    ~Private() {
        delete stylesPrivate;
        stylesPrivate = 0;
        charStyle = 0; // QObject will delete it.
        delete listStyle;
    }

    QString name;
    KoCharacterStyle *charStyle;
    KoListStyle *listStyle;
    KoParagraphStyle *parent;
    int next;
    StylePrivate *stylesPrivate;
};

// all relevant properties.
static const int properties[] = {
    QTextFormat::BlockTopMargin,
    QTextFormat::BlockBottomMargin,
    QTextFormat::BlockLeftMargin,
    QTextFormat::BlockRightMargin,
    QTextFormat::BlockAlignment,
    QTextFormat::TextIndent,
    QTextFormat::BlockIndent,
    QTextFormat::BlockNonBreakableLines,
    KoParagraphStyle::StyleId,
    KoParagraphStyle::FixedLineHeight,
    KoParagraphStyle::MinimumLineHeight,
    KoParagraphStyle::LineSpacing,
    KoParagraphStyle::PercentLineHeight,
    KoParagraphStyle::LineSpacingFromFont,
//      KoParagraphStyle::AlignLastLine,
//      KoParagraphStyle::WidowThreshold,
//      KoParagraphStyle::OrphanThreshold,
//      KoParagraphStyle::DropCaps,
//      KoParagraphStyle::DropCapsLength,
//      KoParagraphStyle::DropCapsLines,
//      KoParagraphStyle::DropCapsDistance,
//      KoParagraphStyle::FollowDocBaseline,
    KoParagraphStyle::BreakBefore,
    KoParagraphStyle::BreakAfter,
//      KoParagraphStyle::HasLeftBorder,
//      KoParagraphStyle::HasTopBorder,
//      KoParagraphStyle::HasRightBorder,
//      KoParagraphStyle::HasBottomBorder,
//      KoParagraphStyle::BorderLineWidth,
//      KoParagraphStyle::SecondBorderLineWidth,
//      KoParagraphStyle::DistanceToSecondBorder,
    KoParagraphStyle::LeftPadding,
    KoParagraphStyle::TopPadding,
    KoParagraphStyle::RightPadding,
    KoParagraphStyle::BottomPadding,
    KoParagraphStyle::LeftBorderWidth,
    KoParagraphStyle::LeftInnerBorderWidth,
    KoParagraphStyle::LeftBorderSpacing,
    KoParagraphStyle::LeftBorderStyle,
    KoParagraphStyle::TopBorderWidth,
    KoParagraphStyle::TopInnerBorderWidth,
    KoParagraphStyle::TopBorderSpacing,
    KoParagraphStyle::TopBorderStyle,
    KoParagraphStyle::RightBorderWidth,
    KoParagraphStyle::RightInnerBorderWidth,
    KoParagraphStyle::RightBorderSpacing,
    KoParagraphStyle::RightBorderStyle,
    KoParagraphStyle::BottomBorderWidth,
    KoParagraphStyle::BottomInnerBorderWidth,
    KoParagraphStyle::BottomBorderSpacing,
    KoParagraphStyle::BottomBorderStyle,
    KoParagraphStyle::LeftBorderColor,
    KoParagraphStyle::TopBorderColor,
    KoParagraphStyle::RightBorderColor,
    KoParagraphStyle::BottomBorderColor,
    KoParagraphStyle::ExplicitListValue,
    KoParagraphStyle::RestartListNumbering,
    KoParagraphStyle::TabPositions,

    -1
};

KoParagraphStyle::KoParagraphStyle()
: d(new Private())
{
d->charStyle = new KoCharacterStyle(this),
d->stylesPrivate = new StylePrivate();
setLineHeightPercent(120);
}

KoParagraphStyle::KoParagraphStyle(const KoParagraphStyle &orig)
    : QObject(),
    d(new Private())
{
    d->stylesPrivate = new StylePrivate();
    d->stylesPrivate->copyMissing(orig.d->stylesPrivate);
    d->name = orig.name();
    d->charStyle = orig.d->charStyle;
    d->next = orig.d->next;
    if(orig.d->listStyle)
        setListStyle(*orig.d->listStyle);
}

KoParagraphStyle::~KoParagraphStyle() {
    delete d;
}

void KoParagraphStyle::setParent(KoParagraphStyle *parent) {
    Q_ASSERT(parent != this);
    const int id = styleId();
    if(d->parent)
        d->stylesPrivate->copyMissing(d->parent->d->stylesPrivate);
    d->parent = parent;
    if(d->parent)
        d->stylesPrivate->removeDuplicates(d->parent->d->stylesPrivate);
    setStyleId(id);
}

void KoParagraphStyle::setProperty(int key, const QVariant &value) {
    if(d->parent) {
        QVariant var = d->parent->value(key);
        if(!var.isNull() && var == value) { // same as parent, so its actually a reset.
            d->stylesPrivate->remove(key);
            return;
        }
    }
    d->stylesPrivate->add(key, value);
}

void KoParagraphStyle::remove(int key) {
    d->stylesPrivate->remove(key);
}

QVariant KoParagraphStyle::value(int key) const {
    QVariant var = d->stylesPrivate->value(key);
    if(var.isNull() && d->parent)
        var = d->parent->value(key);
    return var;
}

double KoParagraphStyle::propertyDouble(int key) const {
    QVariant variant = value(key);
    if(variant.isNull())
        return 0.0;
    return variant.toDouble();
}

int KoParagraphStyle::propertyInt(int key) const {
    QVariant variant = value(key);
    if(variant.isNull())
        return 0;
    return variant.toInt();
}

bool KoParagraphStyle::propertyBoolean(int key) const {
    QVariant variant = value(key);
    if(variant.isNull())
        return false;
    return variant.toBool();
}

QColor KoParagraphStyle::propertyColor(int key) const {
    QVariant variant = value(key);
    if(variant.isNull()) {
        QColor color;
        return color;
    }
    return qvariant_cast<QColor>(variant);
}

void KoParagraphStyle::applyStyle(QTextBlockFormat &format) const {
    int i=0;
    while(properties[i] != -1) {
        QVariant variant = value(properties[i]);
        if(! variant.isNull())
            format.setProperty(properties[i], variant);
        else
            format.clearProperty(properties[i]);
        i++;
    }
}

void KoParagraphStyle::applyStyle(QTextBlock &block) const {
    QTextCursor cursor(block);
    QTextBlockFormat format = cursor.blockFormat();
    applyStyle(format);
    cursor.setBlockFormat(format);
    if(d->charStyle)
        d->charStyle->applyStyle(block);

    if(d->listStyle) {
        // make sure this block becomes a list if its not one already
        d->listStyle->applyStyle(block, listLevel());
    } else if(block.textList()) {
        // remove
        block.textList()->remove(block);
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
        if(data)
            data->setCounterWidth(-1);
    }
}

void KoParagraphStyle::setListStyle(const KoListStyle &style) {
    if(d->listStyle)
        delete d->listStyle;
    d->listStyle = new KoListStyle(style);
}

void KoParagraphStyle::removeListStyle() {
    delete d->listStyle;
    d->listStyle = 0;
}

static KoParagraphStyle::BorderStyle oasisBorderStyle(const QString& borderstyle) {
    if ( borderstyle == "none" )
        return KoParagraphStyle::BorderNone;
    if ( borderstyle == "dashed" )
        return KoParagraphStyle::BorderDashed;
    if ( borderstyle == "dotted" )
        return KoParagraphStyle::BorderDotted;
    if ( borderstyle == "dot-dash" )
        return KoParagraphStyle::BorderDashDotPattern;
    if ( borderstyle == "dot-dot-dash" )
        return KoParagraphStyle::BorderDashDotDotPattern;
    if ( borderstyle == "double" )
        return KoParagraphStyle::BorderDouble;
    if ( borderstyle == "groove" ) // NOT OASIS COMPATIBLE
        return KoParagraphStyle::BorderGroove;
    if ( borderstyle == "ridge" ) // NOT OASIS COMPATIBLE
        return KoParagraphStyle::BorderRidge;
    if ( borderstyle == "inset" ) // NOT OASIS COMPATIBLE
        return KoParagraphStyle::BorderInset;
    if ( borderstyle == "outset" ) // NOT OASIS COMPATIBLE
        return KoParagraphStyle::BorderOutset;
    return KoParagraphStyle::BorderSolid; // not needed to handle "solid" since it's the default
}

void KoParagraphStyle::setLineHeightPercent(int lineHeight) {
    setProperty(PercentLineHeight, lineHeight);
    remove(LineSpacing);
}

int KoParagraphStyle::lineHeightPercent() const {
    return propertyInt(PercentLineHeight);
}

void KoParagraphStyle::setLineHeightAbsolute(double height) {
    setProperty(FixedLineHeight, height);
    remove(LineSpacing);
    remove(PercentLineHeight);
}

double KoParagraphStyle::lineHeightAbsolute() const {
    return propertyDouble(FixedLineHeight);
}

void KoParagraphStyle::setMinimumLineHeight(double height) {
    setProperty(MinimumLineHeight, height);
    remove(FixedLineHeight);
    remove(PercentLineHeight);
}

double KoParagraphStyle::minimumLineHeight() const {
    return propertyDouble(MinimumLineHeight);
}

void KoParagraphStyle::setLineSpacing(double spacing) {
    setProperty(LineSpacing, spacing);
    remove(FixedLineHeight);
    remove(PercentLineHeight);
}

double KoParagraphStyle::lineSpacing() const {
    return propertyDouble(LineSpacing);
}

void KoParagraphStyle::setLineSpacingFromFont(bool on) {
    setProperty(LineSpacingFromFont, on);
}

bool KoParagraphStyle::lineSpacingFromFont() const {
    return propertyBoolean(LineSpacingFromFont);
}

void KoParagraphStyle::setAlignLastLine(Qt::Alignment alignment ) {
    setProperty(AlignLastLine, (int) alignment);
}

Qt::Alignment KoParagraphStyle::alignLastLine() const {
    return static_cast<Qt::Alignment> (propertyInt(QTextFormat::BlockAlignment));
}

void KoParagraphStyle::setWidowThreshold(int lines) {
    setProperty(WidowThreshold, lines);
}

int KoParagraphStyle::widowThreshold() const {
    return propertyInt(WidowThreshold);
}

void KoParagraphStyle::setOrphanThreshold(int lines) {
    setProperty(OrphanThreshold, lines);
}

int KoParagraphStyle::orphanThreshold() const {
    return propertyInt(OrphanThreshold);
}

void KoParagraphStyle::setDropCaps(bool on) {
    setProperty(DropCaps, on);
}

bool KoParagraphStyle::dropCaps() const {
    return propertyBoolean(DropCaps);
}

void KoParagraphStyle::setDropCapsLength(int characters) {
    setProperty(DropCapsLength, characters);
}

int KoParagraphStyle::dropCapsLength() const {
    return propertyInt(DropCapsLength);
}

void KoParagraphStyle::setDropCapsLines(int lines) {
    setProperty(DropCapsLines, lines);
}

int KoParagraphStyle::dropCapsLines() const {
    return propertyInt(DropCapsLines);
}

void KoParagraphStyle::setDropCapsDistance(double distance) {
    setProperty(DropCapsDistance, distance);
}

double KoParagraphStyle::dropCapsDistance() const {
    return propertyDouble(DropCapsDistance);
}

void KoParagraphStyle::setFollowDocBaseline(bool on) {
    setProperty(FollowDocBaseline, on);
}

bool KoParagraphStyle::followDocBaseline() const {
    return propertyBoolean(FollowDocBaseline);
}

void KoParagraphStyle::setBreakBefore(bool on) {
    setProperty(BreakBefore, on);
}

bool KoParagraphStyle::breakBefore() {
    return propertyBoolean(BreakBefore);
}

void KoParagraphStyle::setBreakAfter(bool on) {
    setProperty(BreakAfter, on);
}

bool KoParagraphStyle::breakAfter() {
    return propertyBoolean(BreakAfter);
}

void KoParagraphStyle::setLeftPadding(double padding) {
    setProperty(LeftPadding, padding);
}

double KoParagraphStyle::leftPadding() {
    return propertyDouble(LeftPadding);
}

void KoParagraphStyle::setTopPadding(double padding) {
    setProperty(TopPadding, padding);
}

double KoParagraphStyle::topPadding() {
    return propertyDouble(TopPadding);
}

void KoParagraphStyle::setRightPadding(double padding) {
    setProperty(RightPadding, padding);
}

double KoParagraphStyle::rightPadding() {
    return propertyDouble(RightPadding);
}

void KoParagraphStyle::setBottomPadding(double padding) {
    setProperty(BottomPadding, padding);
}

double KoParagraphStyle::bottomPadding() {
    return propertyDouble(BottomPadding);
}

void KoParagraphStyle::setPadding(double padding) {
    setBottomPadding(padding);
    setTopPadding(padding);
    setRightPadding(padding);
    setLeftPadding(padding);
}

void KoParagraphStyle::setLeftBorderWidth(double width) {
    setProperty(LeftBorderWidth, width);
}

double KoParagraphStyle::leftBorderWidth() {
    return propertyDouble(LeftBorderWidth);
}

void KoParagraphStyle::setLeftInnerBorderWidth(double width) {
    setProperty(LeftInnerBorderWidth, width);
}

double KoParagraphStyle::leftInnerBorderWidth() {
    return propertyDouble(LeftInnerBorderWidth);
}

void KoParagraphStyle::setLeftBorderSpacing(double width) {
    setProperty(LeftBorderSpacing, width);
}

double KoParagraphStyle::leftBorderSpacing() {
    return propertyDouble(LeftBorderSpacing);
}

void KoParagraphStyle::setLeftBorderStyle(KoParagraphStyle::BorderStyle style) {
    setProperty(LeftBorderStyle, style);
}

KoParagraphStyle::BorderStyle KoParagraphStyle::leftBorderStyle() {
    return static_cast<KoParagraphStyle::BorderStyle> (propertyInt(LeftBorderStyle));
}

void KoParagraphStyle::setLeftBorderColor(const QColor& color) {
    setProperty(LeftBorderColor, color);
}

QColor KoParagraphStyle::leftBorderColor() {
    return propertyColor(LeftBorderColor);
}

void KoParagraphStyle::setTopBorderWidth(double width) {
    setProperty(TopBorderWidth, width);
}

double KoParagraphStyle::topBorderWidth() {
    return propertyDouble(TopBorderWidth);
}

void KoParagraphStyle::setTopInnerBorderWidth(double width) {
    setProperty(TopInnerBorderWidth, width);
}

double KoParagraphStyle::topInnerBorderWidth() {
    return propertyDouble(TopInnerBorderWidth);
}

void KoParagraphStyle::setTopBorderSpacing(double width) {
    setProperty(TopBorderSpacing, width);
}

double KoParagraphStyle::topBorderSpacing() {
    return propertyDouble(TopBorderSpacing);
}

void KoParagraphStyle::setTopBorderStyle(KoParagraphStyle::BorderStyle style) {
    setProperty(TopBorderStyle, style);
}

KoParagraphStyle::BorderStyle KoParagraphStyle::topBorderStyle() {
    return static_cast<KoParagraphStyle::BorderStyle> (propertyInt(TopBorderStyle));
}

void KoParagraphStyle::setTopBorderColor(const QColor& color) {
    setProperty(TopBorderColor, color);
}

QColor KoParagraphStyle::topBorderColor() {
    return propertyColor(TopBorderColor);
}

void KoParagraphStyle::setRightBorderWidth(double width) {
    setProperty(RightBorderWidth, width);
}

double KoParagraphStyle::rightBorderWidth() {
    return propertyDouble(RightBorderWidth);
}

void KoParagraphStyle::setRightInnerBorderWidth(double width) {
    setProperty(RightInnerBorderWidth, width);
}

double KoParagraphStyle::rightInnerBorderWidth() {
    return propertyDouble(RightInnerBorderWidth);
}

void KoParagraphStyle::setRightBorderSpacing(double width) {
    setProperty(RightBorderSpacing, width);
}

double KoParagraphStyle::rightBorderSpacing() {
    return propertyDouble(RightBorderSpacing);
}

void KoParagraphStyle::setRightBorderStyle(KoParagraphStyle::BorderStyle style) {
    setProperty(RightBorderStyle, style);
}

KoParagraphStyle::BorderStyle KoParagraphStyle::rightBorderStyle() {
    return static_cast<KoParagraphStyle::BorderStyle> (propertyInt(RightBorderStyle));
}

void KoParagraphStyle::setRightBorderColor(const QColor& color) {
    setProperty(RightBorderColor, color);
}

QColor KoParagraphStyle::rightBorderColor() {
    return propertyColor(RightBorderColor);
}

void KoParagraphStyle::setBottomBorderWidth(double width) {
    setProperty(BottomBorderWidth, width);
}

double KoParagraphStyle::bottomBorderWidth() {
    return propertyDouble(BottomBorderWidth);
}

void KoParagraphStyle::setBottomInnerBorderWidth(double width) {
    setProperty(BottomInnerBorderWidth, width);
}

double KoParagraphStyle::bottomInnerBorderWidth() {
    return propertyDouble(BottomInnerBorderWidth);
}

void KoParagraphStyle::setBottomBorderSpacing(double width) {
    setProperty(BottomBorderSpacing, width);
}

double KoParagraphStyle::bottomBorderSpacing() {
    return propertyDouble(BottomBorderSpacing);
}

void KoParagraphStyle::setBottomBorderStyle(KoParagraphStyle::BorderStyle style) {
    setProperty(BottomBorderStyle, style);
}

KoParagraphStyle::BorderStyle KoParagraphStyle::bottomBorderStyle() {
    return static_cast<KoParagraphStyle::BorderStyle> (propertyInt(BottomBorderStyle));
}

void KoParagraphStyle::setBottomBorderColor(const QColor& color) {
    setProperty(BottomBorderColor, color);
}

QColor KoParagraphStyle::bottomBorderColor() {
    return propertyColor(BottomBorderColor);
}

void KoParagraphStyle::setTopMargin(double topMargin) {
    setProperty(QTextFormat::BlockTopMargin, topMargin);
}

double KoParagraphStyle::topMargin() const {
    return propertyDouble(QTextFormat::BlockTopMargin);
}

void KoParagraphStyle::setBottomMargin (double margin) {
    setProperty(QTextFormat::BlockBottomMargin, margin);
}

double KoParagraphStyle::bottomMargin () const {
    return propertyDouble(QTextFormat::BlockBottomMargin);
}

void KoParagraphStyle::setLeftMargin (double margin) {
    setProperty(QTextFormat::BlockLeftMargin, margin);
}

double KoParagraphStyle::leftMargin () const {
    return propertyDouble(QTextFormat::BlockLeftMargin);
}

void KoParagraphStyle::setRightMargin (double margin) {
    setProperty(QTextFormat::BlockRightMargin, margin);
}

double KoParagraphStyle::rightMargin () const {
    return propertyDouble(QTextFormat::BlockRightMargin);
}

void KoParagraphStyle::setMargin (double margin) {
    setTopMargin(margin);
    setBottomMargin(margin);
    setLeftMargin(margin);
    setRightMargin(margin);
}

void KoParagraphStyle::setAlignment (Qt::Alignment alignment) {

    setProperty(QTextFormat::BlockAlignment, (int) alignment);

}

Qt::Alignment KoParagraphStyle::alignment () const {
    return static_cast<Qt::Alignment> (propertyInt(QTextFormat::BlockAlignment));
}

void KoParagraphStyle::setTextIndent (double margin) {
    setProperty(QTextFormat::TextIndent, margin);
}

double KoParagraphStyle::textIndent () const {
    return propertyDouble(QTextFormat::TextIndent);
}

void KoParagraphStyle::setNonBreakableLines(bool on) {
    setProperty(QTextFormat::BlockNonBreakableLines, on);
}

bool KoParagraphStyle::nonBreakableLines() const {
    return propertyBoolean(QTextFormat::BlockNonBreakableLines);
}

KoParagraphStyle *KoParagraphStyle::parent() const {
    return d->parent;
}

void KoParagraphStyle::setNextStyle(int next) {
    d->next = next;
}

int KoParagraphStyle::nextStyle() const {
    return d->next;
}

const QString& KoParagraphStyle::name() const {
    return d->name;
}

void KoParagraphStyle::setName(const QString &name) {
    d->name = name;
}

int KoParagraphStyle::styleId() const {
    return propertyInt(StyleId);
}

void KoParagraphStyle::setStyleId(int id) {
    setProperty(StyleId, id); if(d->next == 0) d->next=id;
}

void KoParagraphStyle::setRestartListNumbering(bool on) {
    setProperty(RestartListNumbering, on);
}

bool KoParagraphStyle::restartListNumbering() {
    return propertyBoolean(RestartListNumbering);
}

void KoParagraphStyle::setListLevel(int value) {
    setProperty(ListLevel, value);
}

int KoParagraphStyle::listLevel() const {
    return propertyInt(ListLevel);
}

KoCharacterStyle *KoParagraphStyle::characterStyle() {
    return d->charStyle;
}

const KoCharacterStyle *KoParagraphStyle::characterStyle() const {
    return d->charStyle;
}

KoListStyle KoParagraphStyle::listStyle() const {
    if(d->listStyle)
        return KoListStyle(*d->listStyle);
    return KoListStyle(0); // an invalid one
}

void KoParagraphStyle::loadOasis(KoStyleStack& styleStack) {
    // in 1.6 this was defined at KoParagLayout::loadOasisParagLayout(KoParagLayout&, KoOasisContext&)

    // Alignment
    if ( styleStack.hasProperty( KoXmlNS::fo, "text-align" ) ) {
        QString align = styleStack.property( KoXmlNS::fo, "text-align" );
        Qt::Alignment alignment = Qt::AlignAuto;
        if ( align == "left" )
            alignment = Qt::AlignLeft;
        else if ( align == "right" )
            alignment = Qt::AlignRight;
        else if ( align == "start" )
            alignment = Qt::AlignLeft;
        else if ( align == "end" )
            alignment = Qt::AlignRight;
        else if ( align == "center" )
            alignment = Qt::AlignHCenter;
        else if ( align == "justify" )
            alignment = Qt::AlignJustify;
        setAlignment(alignment);
    }

    //TODO
#if 0
    if ( styleStack.hasProperty( KoXmlNS::style, "writing-mode" ) ) { // http://web4.w3.org/TR/xsl/slice7.html#writing-mode
        // LTR is lr-tb. RTL is rl-tb
        QString writingMode = styleStack.property( KoXmlNS::style, "writing-mode" );
        layout.direction = ( writingMode=="rl-tb" || writingMode=="rl" ) ? QChar::DirR : QChar::DirL;
    }
#endif

    // Spacing (padding)
    if ( styleStack.hasProperty( KoXmlNS::fo, "padding-left" ) )
        setLeftPadding( KoUnit::parseValue( styleStack.property( KoXmlNS::fo, "padding-left" ) ) );
    if ( styleStack.hasProperty( KoXmlNS::fo, "padding-right" ) )
        setRightPadding( KoUnit::parseValue( styleStack.property( KoXmlNS::fo, "padding-right" ) ) );
    if ( styleStack.hasProperty( KoXmlNS::fo, "padding-top" ) )
        setTopPadding( KoUnit::parseValue( styleStack.property( KoXmlNS::fo, "padding-top" ) ) );
    if ( styleStack.hasProperty( KoXmlNS::fo, "padding-bottom" ) )
        setBottomPadding( KoUnit::parseValue( styleStack.property( KoXmlNS::fo, "padding-bottom" ) ) );
    if ( styleStack.hasProperty( KoXmlNS::fo, "padding" ) )
        setPadding( KoUnit::parseValue( styleStack.property( KoXmlNS::fo, "padding" ) ) );

    // Indentation (margin)
    bool hasMarginLeft = styleStack.hasProperty( KoXmlNS::fo, "margin-left" );
    bool hasMarginRight = styleStack.hasProperty( KoXmlNS::fo, "margin-right" );
    if ( hasMarginLeft )
        setLeftMargin( KoUnit::parseValue( styleStack.property( KoXmlNS::fo, "margin-left" ) ) );
    if ( hasMarginRight )
        setRightMargin( KoUnit::parseValue( styleStack.property( KoXmlNS::fo, "margin-right" ) ) );
    if ( styleStack.hasProperty( KoXmlNS::fo, "margin-top" ) )
        setTopMargin( KoUnit::parseValue( styleStack.property( KoXmlNS::fo, "margin-top" ) ) );
    if ( styleStack.hasProperty( KoXmlNS::fo, "margin-bottom" ) )
        setBottomMargin( KoUnit::parseValue( styleStack.property( KoXmlNS::fo, "margin-bottom" ) ) );
    if ( styleStack.hasProperty( KoXmlNS::fo, "margin" ) ) {
        setMargin( KoUnit::parseValue( styleStack.property( KoXmlNS::fo, "margin" ) ) );
        hasMarginLeft = true;
        hasMarginRight = true;
    }

    // Automatic Text indent
    // OOo is not assuming this. Commenting this line thus allow more OpenDocuments to be supported, including a 
    // testcase from the ODF test suite. See §15.5.18 in the spec.
    //if ( hasMarginLeft || hasMarginRight ) {
        if ( styleStack.hasProperty(KoXmlNS::fo, "auto-text-indent") ) { // style:auto-text-indent takes precedence
            // "indented by a value that is based on the current font size"
            const QString autotextindent = styleStack.property(KoXmlNS::style, "auto-text-indent");
            if ( styleStack.property(KoXmlNS::style, "auto-text-indent") == "true" )
                setTextIndent( 10.0 ); //hmmm, this was "10" on 1.6...
            else
                setTextIndent( KoUnit::parseValue(autotextindent) );
        }
        else if ( styleStack.hasProperty(KoXmlNS::fo, "text-indent") ) {
            setTextIndent( KoUnit::parseValue( styleStack.property( KoXmlNS::fo, "text-indent") ) );
        }
    //}

    // Line spacing
    if( styleStack.hasProperty( KoXmlNS::fo, "line-height") ) {  // 3.11.1
        // Fixed line height
        QString value = styleStack.property( KoXmlNS::fo, "line-height" );
        if ( value != "normal" ) {
            if ( value.indexOf('%') > -1 ) // percent value
                setLineHeightPercent( value.remove( '%' ).toInt() );
            else // fixed value
                setLineHeightAbsolute( KoUnit::parseValue( value ) );
        }
    } // Line-height-at-least is mutually exclusive with line-height
    else if ( styleStack.hasProperty( KoXmlNS::style, "line-height-at-least") ) { // 3.11.2
        setMinimumLineHeight( KoUnit::parseValue( styleStack.property( KoXmlNS::style, "line-height-at-least" ) ) );
    } // Line-spacing is mutually exclusive with line-height and line-height-at-least
    else if ( styleStack.hasProperty( KoXmlNS::style, "line-spacing") ) { // 3.11.3
        setLineSpacing( KoUnit::parseValue( styleStack.property( KoXmlNS::style, "line-spacing" ) ) );
    }

    //TODO
#if 0
    // Tabulators
    KoTabulatorList tabList;
    if ( styleStack.hasChildNode( KoXmlNS::style, "tab-stops" ) ) { // 3.11.10
        QDomElement tabStops = styleStack.childNode( KoXmlNS::style, "tab-stops" );
        //kDebug(30519) << k_funcinfo << tabStops.childNodes().count() << " tab stops in layout." << endl;
        QDomElement tabStop;
        forEachElement( tabStop, tabStops )
        {
            Q_ASSERT( tabStop.localName() == "tab-stop" );
            const QString type = tabStop.attributeNS( KoXmlNS::style, "type", QString() ); // left, right, center or char

            KoTabulator tab;
            tab.ptPos = KoUnit::parseValue( tabStop.attributeNS( KoXmlNS::style, "position", QString() ) );
            // Tab stop positions in the XML are relative to the left-margin
            tab.ptPos += layout.margins[QStyleSheetItem::MarginLeft];
            if ( type == "center" )
                tab.type = T_CENTER;
            else if ( type == "right" )
                tab.type = T_RIGHT;
            else if ( type == "char" ) {
                QString delimiterChar = tabStop.attributeNS( KoXmlNS::style, "char", QString() ); // single character
                if ( !delimiterChar.isEmpty() )
                    tab.alignChar = delimiterChar[0];
                tab.type = T_DEC_PNT; // "alignment on decimal point"
            }
            else //if ( type == "left" )
                tab.type = T_LEFT;

            tab.ptWidth = KoUnit::parseValue( tabStop.attributeNS( KoXmlNS::style, "leader-width", QString() ), 0.5 );

            tab.filling = TF_BLANK;
            if ( tabStop.attributeNS( KoXmlNS::style, "leader-type", QString() ) == "single" )
            {
                QString leaderStyle = tabStop.attributeNS( KoXmlNS::style, "leader-style", QString() );
                if ( leaderStyle == "solid" )
                    tab.filling = TF_LINE;
                else if ( leaderStyle == "dotted" )
                    tab.filling = TF_DOTS;
                else if ( leaderStyle == "dash" )
                    tab.filling = TF_DASH;
                else if ( leaderStyle == "dot-dash" )
                    tab.filling = TF_DASH_DOT;
                else if ( leaderStyle == "dot-dot-dash" )
                    tab.filling = TF_DASH_DOT_DOT;
            }
            else
            {
                // Fallback: convert leaderChar's unicode value
                QString leaderChar = tabStop.attributeNS( KoXmlNS::style, "leader-text", QString() );
                if ( !leaderChar.isEmpty() )
                {
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
            tabList.append( tab );
        } //for
    }
    qHeapSort( tabList );
    layout.setTabList( tabList );
#endif

#if 0
    layout.joinBorder = !( styleStack.property( KoXmlNS::style, "join-border") == "false" );
#endif

    // Borders
    if ( styleStack.hasProperty( KoXmlNS::fo, "border", "left") ) {
        QString border = styleStack.property( KoXmlNS::fo, "border", "left" );
        if ( !border.isEmpty() && border!="none" && border!="hidden") {
            // ## isn't it faster to use QStringList::split than parse it 3 times?
            QString borderwidth = border.section(' ', 0, 0);
            QString borderstyle = border.section(' ', 1, 1);
            QString bordercolor = border.section(' ', 2, 2);

            setLeftBorderWidth( KoUnit::parseValue( borderwidth, 1.0 ) );
            //setLeftInnerBorderWidth(double width);
            //setLeftBorderSpacing(double width);
            setLeftBorderStyle( oasisBorderStyle(borderstyle) );
            setLeftBorderColor( QColor(bordercolor) );
        }
    }
    if ( styleStack.hasProperty( KoXmlNS::fo, "border","top") ) {
        QString border = styleStack.property( KoXmlNS::fo, "border", "top" );
        if ( !border.isEmpty() && border!="none" && border!="hidden" ) {
            setTopBorderWidth( KoUnit::parseValue( border.section(' ',0,0), 1.0 ) );
            setTopBorderStyle( oasisBorderStyle(border.section(' ',1,1)) );
            setTopBorderColor( QColor(border.section(' ',2,2)) );
        }
    }
    if ( styleStack.hasProperty( KoXmlNS::fo, "border","right") ) {
        QString border = styleStack.property( KoXmlNS::fo, "border", "right" );
        if ( !border.isEmpty() && border!="none" && border!="hidden") {
            setRightBorderWidth( KoUnit::parseValue( border.section(' ',0,0), 1.0 ) );
            setRightBorderStyle( oasisBorderStyle(border.section(' ',1,1)) );
            setRightBorderColor( QColor(border.section(' ',2,2)) );
        }
    }
    if ( styleStack.hasProperty( KoXmlNS::fo, "border", "bottom") ) {
        QString border = styleStack.property( KoXmlNS::fo, "border", "bottom" );
        if ( !border.isEmpty() && border!="none" && border!="hidden") {
            setBottomBorderWidth( KoUnit::parseValue( border.section(' ',0,0), 1.0 ) );
            setBottomBorderStyle( oasisBorderStyle(border.section(' ',1,1)) );
            setBottomBorderColor( QColor(border.section(' ',2,2)) );
        }
    }

    // Page breaking
#if 0
    int pageBreaking = 0;
    if( styleStack.hasProperty( KoXmlNS::fo, "break-before") ||
            styleStack.hasProperty( KoXmlNS::fo, "break-after") ||
            styleStack.hasProperty( KoXmlNS::fo, "keep-together") ||
            styleStack.hasProperty( KoXmlNS::style, "keep-with-next") ||
            styleStack.hasProperty( KoXmlNS::fo, "keep-with-next") )
    {
        if ( styleStack.hasProperty( KoXmlNS::fo, "break-before") ) { // 3.11.24
            // TODO in KWord: implement difference between "column" and "page"
            if ( styleStack.property( KoXmlNS::fo, "break-before" ) != "auto" )
                pageBreaking |= KoParagLayout::HardFrameBreakBefore;
        }
        else if ( styleStack.hasProperty( KoXmlNS::fo, "break-after") ) { // 3.11.24
            // TODO in KWord: implement difference between "column" and "page"
            if ( styleStack.property( KoXmlNS::fo, "break-after" ) != "auto" )
                pageBreaking |= KoParagLayout::HardFrameBreakAfter;
        }

        if ( styleStack.hasProperty( KoXmlNS::fo, "keep-together" ) ) { // was style:break-inside in OOo-1.1, renamed in OASIS
            if ( styleStack.property( KoXmlNS::fo, "keep-together" ) != "auto" )
                pageBreaking |= KoParagLayout::KeepLinesTogether;
        }
        if ( styleStack.hasProperty( KoXmlNS::fo, "keep-with-next" ) ) {
            // OASIS spec says it's "auto"/"always", not a boolean.
            QString val = styleStack.property( KoXmlNS::fo, "keep-with-next" );
            if ( val == "true" || val == "always" )
                pageBreaking |= KoParagLayout::KeepWithNext;
        }
    }
    layout.pageBreaking = pageBreaking;
#else

    // The fo:break-before and fo:break-after attributes insert a page or column break before or after a paragraph.
    if( styleStack.hasProperty( KoXmlNS::fo, "break-before") ) {
        if ( styleStack.property( KoXmlNS::fo, "break-before" ) != "auto" )
            setBreakBefore(true);
    }
    if( styleStack.hasProperty( KoXmlNS::fo, "break-after") ) {
        if ( styleStack.property( KoXmlNS::fo, "break-after" ) != "auto" )
            setBreakAfter(true);
    }

#endif

#if 0
    // Paragraph background color -  fo:background-color
    // The background color for parts of a paragraph that have no text underneath
    if ( styleStack.hasProperty( KoXmlNS::fo, "background-color" ) ) {
        QString bgColor = styleStack.property( KoXmlNS::fo, "background-color");
        if (bgColor != "transparent")
            layout.backgroundColor.setNamedColor( bgColor );
    }
#endif

    // The fo:background-color attribute specifies the background color of a paragraph.
    if ( styleStack.hasProperty( KoXmlNS::fo, "background-color") ) {
        const QString bgcolor = styleStack.property( KoXmlNS::fo, "background-color");
        QBrush brush = d->charStyle->background();
        if (bgcolor == "transparent")
            brush.setStyle(Qt::NoBrush);
        else {
            if (brush.style() == Qt::NoBrush)
                brush.setStyle(Qt::SolidPattern);
            brush.setColor(bgcolor); // #rrggbb format
        }
        d->charStyle->setBackground(brush);
    }
    //following properties KoParagraphStyle provides us are not handled now;
    // LineSpacingFromFont,
    // AlignLastLine,
    // WidowThreshold,
    // OrphanThreshold,
    // DropCaps,
    // DropCapsLength,
    // DropCapsLines,
    // DropCapsDistance,
    // FollowDocBaseline,

}

void KoParagraphStyle::setTabPositions(const QList<KoText::Tab> &tabs) {
    // TODO sort on position
    QList<QVariant> list;
    foreach(KoText::Tab tab, tabs) {
        QVariant v;
        v.setValue(tab);
        list.append(v);
    }
    setProperty(TabPositions, list);
}

QList<KoText::Tab> KoParagraphStyle::tabPositions() const {
    QVariant variant = value(TabPositions);
    if(variant.isNull())
        return QList<KoText::Tab>();
    QList<KoText::Tab> answer;
    foreach(QVariant tab, qvariant_cast<QList<QVariant> >(variant)) {
        answer.append(tab.value<KoText::Tab>());
    }
    return answer;
}


// static
KoParagraphStyle *KoParagraphStyle::fromBlock(const QTextBlock &block) {
    QTextBlockFormat format = block.blockFormat();
    KoParagraphStyle *answer = 0;
    KoCharacterStyle *charStyle = 0;
    int styleId = format.intProperty(StyleId);
    if(styleId > 0) {
        KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*> (block.document()->documentLayout());
        if(layout) {
            KoStyleManager *sm = layout->styleManager();
            if(sm) {
                KoParagraphStyle *style = sm->paragraphStyle(styleId);
                if(style)
                    answer = new KoParagraphStyle(*style);
                charStyle = sm->characterStyle(format.intProperty(KoCharacterStyle::StyleId));
            }
        }
    }
    if(answer == 0) {
        answer = new KoParagraphStyle();
        answer->remove(PercentLineHeight);
        delete answer->characterStyle();
    }
    answer->d->charStyle = charStyle;

    if(block.textList())
        answer->d->listStyle = KoListStyle::fromTextList(block.textList());

    int i=0;
    while(properties[i] != -1) {
        int key = properties[i];
        if(format.hasProperty(key))
            answer->setProperty(key, format.property(key));
        i++;
    }

    return answer;
}

#include "KoParagraphStyle.moc"
