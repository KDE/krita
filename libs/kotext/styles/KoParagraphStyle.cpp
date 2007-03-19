/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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

#include "Styles_p.h"

#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>

#include <KoUnit.h>
#include <KoStyleStack.h>
#include <KoXmlNS.h>

KoParagraphStyle::KoParagraphStyle()
    : m_charStyle(new KoCharacterStyle(this)),
    m_listStyle(0),
    m_parent(0),
    m_next(0)
{
    m_stylesPrivate = new StylePrivate();
    setLineHeightPercent(120);
}

KoParagraphStyle::KoParagraphStyle(const KoParagraphStyle &orig)
    : QObject(0),
    m_listStyle(0),
    m_parent(0),
    m_next(0)
{
    m_stylesPrivate = new StylePrivate();
    m_stylesPrivate->copyMissing(orig.m_stylesPrivate);
    m_name = orig.name();
    m_charStyle = orig.m_charStyle;
    m_next = orig.m_next;
    if(orig.m_listStyle) {
        m_listStyle = orig.m_listStyle;
        m_listStyle->addUser();
    }
}

KoParagraphStyle::~KoParagraphStyle() {
    delete m_stylesPrivate;
    m_stylesPrivate = 0;
    m_charStyle = 0; // QObject will delete it.
    if(m_listStyle) {
        m_listStyle->removeUser();
        if(m_listStyle->userCount() == 0)
            delete m_listStyle;
        m_listStyle = 0;
    }
}

void KoParagraphStyle::setParent(KoParagraphStyle *parent) {
    Q_ASSERT(parent != this);
    if(m_parent)
        m_stylesPrivate->copyMissing(m_parent->m_stylesPrivate);
    m_parent = parent;
    if(m_parent)
        m_stylesPrivate->removeDuplicates(m_parent->m_stylesPrivate);
}

void KoParagraphStyle::setProperty(int key, const QVariant &value) {
    if(m_parent) {
        QVariant const *var = m_parent->get(key);
        if(var && (*var) == value) { // same as parent, so its actually a reset.
            m_stylesPrivate->remove(key);
            return;
        }
    }
    m_stylesPrivate->add(key, value);
}

void KoParagraphStyle::remove(int key) {
    m_stylesPrivate->remove(key);
}

QVariant const *KoParagraphStyle::get(int key) const {
    QVariant const *var = m_stylesPrivate->get(key);
    if(var == 0 && m_parent)
        var = m_parent->get(key);
    return var;
}

double KoParagraphStyle::propertyDouble(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0)
        return 0.0;
    return variant->toDouble();
}

int KoParagraphStyle::propertyInt(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0)
        return 0;
    return variant->toInt();
}

bool KoParagraphStyle::propertyBoolean(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0)
        return false;
    return variant->toBool();
}

QColor KoParagraphStyle::propertyColor(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0) {
        QColor color;
        return color;
    }
    return qvariant_cast<QColor>(*variant);
}

void KoParagraphStyle::applyStyle(QTextBlockFormat &format) const {
    // copy all relevant properties.
    static const int properties[] = {
        QTextFormat::BlockTopMargin,
        QTextFormat::BlockBottomMargin,
        QTextFormat::BlockLeftMargin,
        QTextFormat::BlockRightMargin,
        QTextFormat::BlockAlignment,
        QTextFormat::TextIndent,
        QTextFormat::BlockIndent,
        QTextFormat::BlockNonBreakableLines,
        StyleId,
        FixedLineHeight,
        MinimumLineHeight,
        LineSpacing,
        LineSpacingFromFont,
//       AlignLastLine,
//       WidowThreshold,
//       OrphanThreshold,
//       DropCaps,
//       DropCapsLength,
//       DropCapsLines,
//       DropCapsDistance,
//       FollowDocBaseline,
        BreakBefore,
        BreakAfter,
//       HasLeftBorder,
//       HasTopBorder,
//       HasRightBorder,
//       HasBottomBorder,
//       BorderLineWidth,
//       SecondBorderLineWidth,
//       DistanceToSecondBorder,
        LeftPadding,
        TopPadding,
        RightPadding,
        BottomPadding,
        LeftBorderWidth,
        LeftInnerBorderWidth,
        LeftBorderSpacing,
        LeftBorderStyle,
        TopBorderWidth,
        TopInnerBorderWidth,
        TopBorderSpacing,
        TopBorderStyle,
        RightBorderWidth,
        RightInnerBorderWidth,
        RightBorderSpacing,
        RightBorderStyle,
        BottomBorderWidth,
        BottomInnerBorderWidth,
        BottomBorderSpacing,
        BottomBorderStyle,
        LeftBorderColor,
        TopBorderColor,
        RightBorderColor,
        BottomBorderColor,
        ExplicitListValue,
        RestartListNumbering,

        -1
    };

    int i=0;
    while(properties[i] != -1) {
        QVariant const *variant = get(properties[i]);
        if(variant)
            format.setProperty(properties[i], *variant);
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
    if(m_charStyle)
        m_charStyle->applyStyle(block);

    if(m_listStyle) {
        // make sure this block becomes a list if its not one already
        m_listStyle->applyStyle(block);
    } else if(block.textList()) {
        // remove
        block.textList()->remove(block);
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
        if(data)
            data->setCounterWidth(-1);
    }
}

void KoParagraphStyle::setListStyle(const KoListStyle &style) {
    if(m_listStyle)
        m_listStyle->apply(style);
    else {
        m_listStyle = new KoListStyle(style);
        m_listStyle->addUser();
    }
}

void KoParagraphStyle::removeListStyle() {
    delete m_listStyle; m_listStyle = 0;
}

void KoParagraphStyle::loadOasis(KoStyleStack& styleStack) {
    // in 1.6 this was defined at KoParagLayout::loadOasisParagLayout(KoParagLayout&, KoOasisContext&)

#if 0
    // code from OoWriterImport::writeLayout
    if ( context.styleStack().hasAttributeNS( KoXmlNS::fo, "text-align" ) ) {
        QCString align = context.styleStack().attributeNS( KoXmlNS::fo, "text-align" ).latin1();
        layout.alignment = loadOasisAlignment( align );
    }
#else
    if ( styleStack.hasAttributeNS( KoXmlNS::fo, "text-align" ) ) {
        QString align = styleStack.attributeNS( KoXmlNS::fo, "text-align" );
        Qt::Alignment alignment = Qt::AlignAuto;
        if(align == "left")
            alignment = Qt::AlignLeft;
        else if(align == "right")
            alignment = Qt::AlignRight;
        else if(align == "start")
            alignment = Qt::AlignLeft;
        else if(align == "end")
            alignment = Qt::AlignRight;
        else if(align == "center")
            alignment = Qt::AlignHCenter;
        else if(align == "justify")
            alignment = Qt::AlignJustify;
        setAlignment(alignment);
    }
#endif

//TODO
#if 0
    if ( context.styleStack().hasAttributeNS( KoXmlNS::style, "writing-mode" ) ) { // http://web4.w3.org/TR/xsl/slice7.html#writing-mode
        // LTR is lr-tb. RTL is rl-tb
        QString writingMode = context.styleStack().attributeNS( KoXmlNS::style, "writing-mode" );
        layout.direction = ( writingMode=="rl-tb" || writingMode=="rl" ) ? QChar::DirR : QChar::DirL;
    }
#endif

#if 0
    // Indentation (margins)
    if ( context.styleStack().hasAttributeNS( KoXmlNS::fo, "margin-left" ) || // 3.11.19
         context.styleStack().hasAttributeNS( KoXmlNS::fo, "margin-right" ) ) {
        layout.margins[QStyleSheetItem::MarginLeft] = KoUnit::parseValue( context.styleStack().attributeNS( KoXmlNS::fo, "margin-left" ) );
        layout.margins[QStyleSheetItem::MarginRight] = KoUnit::parseValue( context.styleStack().attributeNS( KoXmlNS::fo, "margin-right" ) );
        // *text-indent must always be bound to either margin-left or margin-right
        double first = 0;
        if ( context.styleStack().attributeNS( KoXmlNS::style, "auto-text-indent") == "true" ) // style:auto-text-indent takes precedence
            // ### "indented by a value that is based on the current font size"
            // ### and "requires margin-left and margin-right
            // ### but how much is the indent?
            first = 10;
        else if ( context.styleStack().hasAttributeNS( KoXmlNS::fo, "text-indent") )
            first = KoUnit::parseValue( context.styleStack().attributeNS( KoXmlNS::fo, "text-indent") );

        layout.margins[QStyleSheetItem::MarginFirstLine] = first;
    }

    // Offset before and after paragraph
    if( context.styleStack().hasAttributeNS( KoXmlNS::fo, "margin-top") || // 3.11.22
        context.styleStack().hasAttributeNS( KoXmlNS::fo, "margin-bottom")) {
        layout.margins[QStyleSheetItem::MarginTop] = KoUnit::parseValue( context.styleStack().attributeNS( KoXmlNS::fo, "margin-top" ) );
        layout.margins[QStyleSheetItem::MarginBottom] = KoUnit::parseValue( context.styleStack().attributeNS( KoXmlNS::fo, "margin-bottom" ) );
    }
#else
    // Indentation (margins)
    if ( styleStack.hasAttributeNS( KoXmlNS::fo, "margin-left" ) )
        setLeftMargin( KoUnit::parseValue( styleStack.attributeNS( KoXmlNS::fo, "margin-left" ) ) );
    if ( styleStack.hasAttributeNS( KoXmlNS::fo, "margin-right" ) )
        setRightMargin( KoUnit::parseValue( styleStack.attributeNS( KoXmlNS::fo, "margin-right" ) ) );
    if ( styleStack.hasAttributeNS( KoXmlNS::fo, "margin-top" ) )
        setTopMargin( KoUnit::parseValue( styleStack.attributeNS( KoXmlNS::fo, "margin-top" ) ) );
    if ( styleStack.hasAttributeNS( KoXmlNS::fo, "margin-bottom" ) )
        setBottomMargin( KoUnit::parseValue( styleStack.attributeNS( KoXmlNS::fo, "margin-bottom" ) ) );
#endif

//TODO
#if 0
    // Line spacing
    if( context.styleStack().hasAttributeNS( KoXmlNS::fo, "line-height") ) {  // 3.11.1
        // Fixed line height
        QString value = context.styleStack().attributeNS( KoXmlNS::fo, "line-height" );
        if ( value != "normal" ) {
            if ( value == "100%" )
                layout.lineSpacingType = KoParagLayout::LS_SINGLE;
            else if( value=="150%")
                layout.lineSpacingType = KoParagLayout::LS_ONEANDHALF;
            else if( value=="200%")
                layout.lineSpacingType = KoParagLayout::LS_DOUBLE;
            else if ( value.find('%') > -1 )
            {
                value = value.remove( '%' );
                double percent = value.toDouble();
                layout.lineSpacingType = KoParagLayout::LS_MULTIPLE;
                layout.lineSpacing = percent / 100.0;
                kdDebug(33001) << "line-height =" << percent << ", " << layout.lineSpacing << ", " << percent/100 << endl;
            }
            else // fixed value
            {
                layout.lineSpacingType = KoParagLayout::LS_FIXED;
                layout.lineSpacing = KoUnit::parseValue( value );
            }
        }
    }
    // Line-height-at-least is mutually exclusive with line-height
    else if ( context.styleStack().hasAttributeNS( KoXmlNS::style, "line-height-at-least") ) // 3.11.2
    {
        QString value = context.styleStack().attributeNS( KoXmlNS::style, "line-height-at-least" );
        // kotext has "at least" but that's for the linespacing, not for the entire line height!
        // Strange. kotext also has "at least" for the whole line height....
        // Did we make the wrong choice in kotext?
        //kdWarning() << "Unimplemented support for style:line-height-at-least: " << value << endl;
        // Well let's see if this makes a big difference.
        layout.lineSpacingType = KoParagLayout::LS_AT_LEAST;
        layout.lineSpacing = KoUnit::parseValue( value );
    }
    // Line-spacing is mutually exclusive with line-height and line-height-at-least
    else if ( context.styleStack().hasAttributeNS( KoXmlNS::style, "line-spacing") ) // 3.11.3
    {
        double value = KoUnit::parseValue( context.styleStack().attributeNS( KoXmlNS::style, "line-spacing" ) );
        if ( value != 0.0 )
        {
            layout.lineSpacingType = KoParagLayout::LS_CUSTOM;
            layout.lineSpacing = value;
        }
    }

    // Tabulators
    KoTabulatorList tabList;
    if ( context.styleStack().hasChildNodeNS( KoXmlNS::style, "tab-stops" ) ) { // 3.11.10
        QDomElement tabStops = context.styleStack().childNodeNS( KoXmlNS::style, "tab-stops" );
        //kdDebug(30519) << k_funcinfo << tabStops.childNodes().count() << " tab stops in layout." << endl;
        QDomElement tabStop;
        forEachElement( tabStop, tabStops )
        {
            Q_ASSERT( tabStop.localName() == "tab-stop" );
            const QString type = tabStop.attributeNS( KoXmlNS::style, "type", QString::null ); // left, right, center or char

            KoTabulator tab;
            tab.ptPos = KoUnit::parseValue( tabStop.attributeNS( KoXmlNS::style, "position", QString::null ) );
            // Tab stop positions in the XML are relative to the left-margin
            tab.ptPos += layout.margins[QStyleSheetItem::MarginLeft];
            if ( type == "center" )
                tab.type = T_CENTER;
            else if ( type == "right" )
                tab.type = T_RIGHT;
            else if ( type == "char" ) {
                QString delimiterChar = tabStop.attributeNS( KoXmlNS::style, "char", QString::null ); // single character
                if ( !delimiterChar.isEmpty() )
                    tab.alignChar = delimiterChar[0];
                tab.type = T_DEC_PNT; // "alignment on decimal point"
            }
            else //if ( type == "left" )
                tab.type = T_LEFT;

            tab.ptWidth = KoUnit::parseValue( tabStop.attributeNS( KoXmlNS::style, "leader-width", QString::null ), 0.5 );

            tab.filling = TF_BLANK;
            if ( tabStop.attributeNS( KoXmlNS::style, "leader-type", QString::null ) == "single" )
            {
                QString leaderStyle = tabStop.attributeNS( KoXmlNS::style, "leader-style", QString::null );
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
                QString leaderChar = tabStop.attributeNS( KoXmlNS::style, "leader-text", QString::null );
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

    layout.joinBorder = !( context.styleStack().attributeNS( KoXmlNS::style, "join-border") == "false" );

    // Borders
    if ( context.styleStack().hasAttributeNS( KoXmlNS::fo, "border","left") )
        layout.leftBorder.loadFoBorder( context.styleStack().attributeNS( KoXmlNS::fo, "border","left") );
    else
        layout.leftBorder.setPenWidth(0);
    if ( context.styleStack().hasAttributeNS( KoXmlNS::fo, "border","right") )
        layout.rightBorder.loadFoBorder( context.styleStack().attributeNS( KoXmlNS::fo, "border","right") );
    else
        layout.rightBorder.setPenWidth(0);
    if ( context.styleStack().hasAttributeNS( KoXmlNS::fo, "border","top") )
        layout.topBorder.loadFoBorder( context.styleStack().attributeNS( KoXmlNS::fo, "border","top") );
    else
        layout.topBorder.setPenWidth(0);
    if ( context.styleStack().hasAttributeNS( KoXmlNS::fo, "border","bottom") )
        layout.bottomBorder.loadFoBorder( context.styleStack().attributeNS( KoXmlNS::fo, "border","bottom") );
    else
        layout.bottomBorder.setPenWidth(0);


    // Page breaking
    int pageBreaking = 0;
    if( context.styleStack().hasAttributeNS( KoXmlNS::fo, "break-before") ||
        context.styleStack().hasAttributeNS( KoXmlNS::fo, "break-after") ||
        context.styleStack().hasAttributeNS( KoXmlNS::fo, "keep-together") ||
        context.styleStack().hasAttributeNS( KoXmlNS::style, "keep-with-next") ||
        context.styleStack().hasAttributeNS( KoXmlNS::fo, "keep-with-next") )
    {
        if ( context.styleStack().hasAttributeNS( KoXmlNS::fo, "break-before") ) { // 3.11.24
            // TODO in KWord: implement difference between "column" and "page"
            if ( context.styleStack().attributeNS( KoXmlNS::fo, "break-before" ) != "auto" )
                pageBreaking |= KoParagLayout::HardFrameBreakBefore;
        }
        else if ( context.styleStack().hasAttributeNS( KoXmlNS::fo, "break-after") ) { // 3.11.24
            // TODO in KWord: implement difference between "column" and "page"
            if ( context.styleStack().attributeNS( KoXmlNS::fo, "break-after" ) != "auto" )
                pageBreaking |= KoParagLayout::HardFrameBreakAfter;
        }

        if ( context.styleStack().hasAttributeNS( KoXmlNS::fo, "keep-together" ) ) { // was style:break-inside in OOo-1.1, renamed in OASIS
            if ( context.styleStack().attributeNS( KoXmlNS::fo, "keep-together" ) != "auto" )
                 pageBreaking |= KoParagLayout::KeepLinesTogether;
        }
        if ( context.styleStack().hasAttributeNS( KoXmlNS::fo, "keep-with-next" ) ) {
            // OASIS spec says it's "auto"/"always", not a boolean.
            QString val = context.styleStack().attributeNS( KoXmlNS::fo, "keep-with-next" );
            if ( val == "true" || val == "always" )
                pageBreaking |= KoParagLayout::KeepWithNext;
        }
    }
    layout.pageBreaking = pageBreaking;

    // Paragraph background color -  fo:background-color
    // The background color for parts of a paragraph that have no text underneath
    if ( context.styleStack().hasAttributeNS( KoXmlNS::fo, "background-color" ) ) {
        QString bgColor = context.styleStack().attributeNS( KoXmlNS::fo, "background-color");
        if (bgColor != "transparent")
            layout.backgroundColor.setNamedColor( bgColor );
    }
#endif
}

#include "KoParagraphStyle.moc"
