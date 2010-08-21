/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.com>
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

#include "KoTextDebug.h"

#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextFrame>
#include <QTextBlock>
#include <QTextTable>
#include <QTextFragment>
#include <QTextList>
#include <QDebug>

#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoTableStyle.h"
#include "styles/KoTableCellStyle.h"
#include "styles/KoStyleManager.h"
#include "KoTextDocument.h"
#include "KoTextBlockData.h"
#include <KoTextDocumentLayout.h>
#include <KoInlineTextObjectManager.h>
#include <KoBookmark.h>
#include <KoInlineNote.h>

#define PARAGRAPH_BORDER_DEBUG

int KoTextDebug::depth = 0;
const int KoTextDebug::INDENT = 2;
const QTextDocument *KoTextDebug::document = 0;

Q_DECLARE_METATYPE(QList<KoText::Tab>)

static QString fontProperties(const QTextCharFormat &textFormat)
{
    QMap<int, QVariant> properties = textFormat.properties();
    QStringList fontProps;
    // add only font properties here
    foreach(int id, properties.keys()) {
        QString key, value;
        switch (id) {
        case QTextFormat::FontFamily:
            fontProps.append(properties[id].toString());
            break;
        case QTextFormat::FontPointSize:
            fontProps.append(QString("%1pt").arg(properties[id].toDouble()));
            break;
        case QTextFormat::FontSizeAdjustment:
            fontProps.append(QString("%1adj").arg(properties[id].toDouble()));
            break;
        case QTextFormat::FontWeight:
            fontProps.append(QString("weight %1").arg(properties[id].toInt()));
            break;
        case QTextFormat::FontItalic:
            fontProps.append(properties[id].toBool() ? "italic" : "non-italic");
            break;
        case QTextFormat::FontPixelSize:
            fontProps.append(QString("%1px").arg(properties[id].toDouble()));
            break;
        case QTextFormat::FontFixedPitch:
            fontProps.append(properties[id].toBool() ? "fixedpitch" : "varpitch");
            break;
        case QTextFormat::FontCapitalization:
            fontProps.append(QString("caps %1").arg(properties[id].toInt()));
            break;
        case KoCharacterStyle::FontCharset:
            fontProps.append(properties[id].toString());
            break;
        case QTextFormat::FontStyleHint:
            fontProps.append(QString::number(properties[id].toInt()));
            break;
        case QTextFormat::FontKerning:
            fontProps.append(QString("kerning %1").arg(properties[id].toInt()));
            break;
        default:
            break;
        }
    }
    return fontProps.join(",");
}

void KoTextDebug::dumpDocument(const QTextDocument *doc)
{
    Q_ASSERT(doc);
    document = doc;
    qDebug() << qPrintable(QString("<document defaultfont=\"%1\">").arg(doc->defaultFont().toString()));
    dumpFrame(document->rootFrame());
    qDebug() << "</document>";
    document = 0;
}

QString KoTextDebug::textAttributes(const KoCharacterStyle &style)
{
    QTextCharFormat format;
    style.applyStyle(format);
    return textAttributes(format);
}

QString KoTextDebug::inlineObjectAttributes(const QTextCharFormat &textFormat)
{
    QString attrs;

    if (textFormat.objectType() == QTextFormat::UserObject + 1) {
        KoTextDocumentLayout *lay = document ? qobject_cast<KoTextDocumentLayout *>(document->documentLayout()) : 0;
        KoInlineTextObjectManager *inlineObjectManager = lay ? lay->inlineTextObjectManager() : 0;
        KoInlineObject *inlineObject = inlineObjectManager->inlineTextObject(textFormat);
        if (KoBookmark *bookmark = dynamic_cast<KoBookmark *>(inlineObject)) {
            if (bookmark->type() == KoBookmark::SinglePosition) {
                attrs.append(" type=\"bookmark\"");
            } else if (bookmark->type() == KoBookmark::StartBookmark) {
                attrs.append(" type=\"bookmark-start\"");
            } else if (bookmark->type() == KoBookmark::EndBookmark) {
                attrs.append(" type=\"bookmark-end\"");
            } else {
                attrs.append(" type=\"bookmark-unknown\"");
            }
            attrs.append(QString(" name=\"%1\"").arg(bookmark->name()));
        } else if (KoInlineNote *note = dynamic_cast<KoInlineNote *>(inlineObject)) {
            attrs.append(QString(" id=\"%1\"").arg(note->id()));
            if (note->type() == KoInlineNote::Footnote) {
                attrs.append(" type=\"footnote\"");
            } else if (note->type() == KoInlineNote::Endnote) {
                attrs.append(" type=\"endnote\"");
            }
            attrs.append(QString(" label=\"%1\"").arg(note->label()));
            attrs.append(QString(" text=\"%1\"").arg(note->text().toPlainText()));
        } else {
            attrs.append(" type=\"inlineobject\">");
        }
    }

    return attrs;
}

QString KoTextDebug::textAttributes(const QTextCharFormat &textFormat)
{
    QString attrs;

    QTextImageFormat imageFormat = textFormat.toImageFormat();

    if (imageFormat.isValid()) {
        attrs.append(" type=\"image\">");
        return attrs;
    }

    KoStyleManager *styleManager = document ? KoTextDocument(document).styleManager() : 0;
    if (styleManager && textFormat.hasProperty(KoCharacterStyle::StyleId)) {
        int id = textFormat.intProperty(KoCharacterStyle::StyleId);
        KoCharacterStyle *characterStyle = styleManager->characterStyle(id);
        attrs.append(" characterStyle=\"id:").append(QString::number(id));
        if (characterStyle)
            attrs.append(" name:").append(characterStyle->name());
        attrs.append("\"");
    }

    QMap<int, QVariant> properties = textFormat.properties();
    attrs.append(" type=\"char\"");
    QString fontProps = fontProperties(textFormat);
    if (!fontProps.isEmpty())
        attrs.append(QString(" font=\"%1\"").arg(fontProps));

    if (textFormat.isAnchor()) {
        attrs.append(QString(" achorHref=\"%1\"").arg(textFormat.anchorHref()));
        attrs.append(QString(" achorName=\"%1\"").arg(textFormat.anchorName()));
    }

    foreach(int id, properties.keys()) {
        QString key, value;
        switch (id) {
        case QTextFormat::TextOutline: {
            key = "outline";
            QPen pen = qvariant_cast<QPen>(properties[id]);
            if (pen.style() == Qt::NoPen)
                value = "false";
            else
                value = pen.color().name();
            break;
        }
        case KoCharacterStyle::UnderlineStyle:
            key = "underlinestyle";
            value = QString::number(properties[id].toInt());
            break;
        case QTextFormat::TextUnderlineColor:
            key = "underlinecolor";
            value = qvariant_cast<QColor>(properties[id]).name();
            break;
        case KoCharacterStyle::UnderlineType:
            key = "underlinetype";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::UnderlineMode:
            key = "underlinemode";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::UnderlineWeight:
            key = "underlineweight";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::UnderlineWidth:
            key = "underlinewidth";
            value = QString::number(properties[id].toDouble());
            break;
        case KoCharacterStyle::StrikeOutStyle:
            key = "strikeoutstyle";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::StrikeOutColor:
            key = "strikeoutcolor";
            value = qvariant_cast<QColor>(properties[id]).name();
            break;
        case KoCharacterStyle::StrikeOutType:
            key = "strikeouttype";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::StrikeOutMode:
            key = "strikeoutmode";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::StrikeOutWeight:
            key = "strikeoutweight";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::StrikeOutWidth:
            key = "strikeoutwidth";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFormat::ForegroundBrush:
            key = "foreground";
            value = qvariant_cast<QBrush>(properties[id]).color().name(); // beware!
            break;
        case QTextFormat::BackgroundBrush:
            key = "background";
            value = qvariant_cast<QBrush>(properties[id]).color().name(); // beware!
            break;
        case QTextFormat::BlockAlignment:
            key = "align";
            value = QString::number(properties[id].toInt());
            break;
        case QTextFormat::TextIndent:
            key = "textindent";
            value = QString::number(properties[id].toInt());
            break;
        case QTextFormat::BlockIndent:
            key = "indent";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::Country:
            key = "country";
            value = properties[id].toString();
            break;
        case KoCharacterStyle::Language:
            key = "language";
            value = properties[id].toString();
            break;
        case KoCharacterStyle::HasHyphenation:
            key = "hypenation";
            value = properties[id].toBool();
            break;
        case KoCharacterStyle::StrikeOutText:
            key = "strikeout-text";
            value = properties[id].toString();
            break;
        case KoCharacterStyle::FontCharset:
            key = "font-charset";
            value = properties[id].toString();
            break;
        case KoCharacterStyle::TextRotationAngle:
            key = "rotation-angle";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::TextRotationScale:
            key = "text-rotation-scale";
            value = properties[id].toInt() == KoCharacterStyle::Fixed ? "Fixed" : "LineHeight";
            break;
        case KoCharacterStyle::TextScale:
            key = "text-scale";
            value = QString::number(properties[id].toInt());
            break;
        case KoCharacterStyle::InlineRdf:
            key = "inline-rdf";
            value = QString::number(properties[id].toInt());
            break;
        default:
            break;
        }
        if (!key.isEmpty())
            attrs.append(" ").append(key).append("=\"").append(value).append("\"");
    }
    return attrs;
}

QString KoTextDebug::paraAttributes(const KoParagraphStyle &style)
{
    QTextBlockFormat format;
    style.applyStyle(format);
    return paraAttributes(format);
}

QString KoTextDebug::paraAttributes(const QTextBlockFormat &blockFormat)
{
    QString attrs;
    KoStyleManager *styleManager = document ? KoTextDocument(document).styleManager() : 0;
    if (styleManager && blockFormat.hasProperty(KoParagraphStyle::StyleId)) {
        int id = blockFormat.intProperty(KoParagraphStyle::StyleId);
        KoParagraphStyle *paragraphStyle = styleManager->paragraphStyle(id);
        attrs.append(" paragraphStyle=\"id:").append(QString::number(id));
        if (paragraphStyle)
            attrs.append(" name:").append(paragraphStyle->name());
        attrs.append("\"");
    }

    QMap<int, QVariant> properties = blockFormat.properties();
    foreach(int id, properties.keys()) {
        QString key, value;
        switch (id) {
        // the following are 'todo'
        case KoParagraphStyle::PercentLineHeight:
        case KoParagraphStyle::FixedLineHeight:
        case KoParagraphStyle::MinimumLineHeight:
        case KoParagraphStyle::LineSpacing:
        case KoParagraphStyle::LineSpacingFromFont:
        case KoParagraphStyle::AlignLastLine:
        case KoParagraphStyle::WidowThreshold:
        case KoParagraphStyle::OrphanThreshold:
        case KoParagraphStyle::DropCapsTextStyle:
        case KoParagraphStyle::FollowDocBaseline:
        case KoParagraphStyle::HasLeftBorder:
        case KoParagraphStyle::HasTopBorder:
        case KoParagraphStyle::HasRightBorder:
        case KoParagraphStyle::HasBottomBorder:
        case KoParagraphStyle::BorderLineWidth:
        case KoParagraphStyle::SecondBorderLineWidth:
        case KoParagraphStyle::DistanceToSecondBorder:
        case KoParagraphStyle::LeftPadding:
        case KoParagraphStyle::TopPadding:
        case KoParagraphStyle::RightPadding:
        case KoParagraphStyle::BottomPadding:
        case KoParagraphStyle::LeftBorderColor:
        case KoParagraphStyle::TopInnerBorderWidth:
        case KoParagraphStyle::TopBorderSpacing:
        case KoParagraphStyle::TopBorderStyle:
        case KoParagraphStyle::TopBorderColor:
        case KoParagraphStyle::RightInnerBorderWidth:
        case KoParagraphStyle::RightBorderSpacing:
        case KoParagraphStyle::RightBorderStyle:
        case KoParagraphStyle::RightBorderColor:
        case KoParagraphStyle::BottomInnerBorderWidth:
        case KoParagraphStyle::BottomBorderSpacing:
        case KoParagraphStyle::BottomBorderStyle:
        case KoParagraphStyle::BottomBorderColor:
        case KoParagraphStyle::ListStyleId:
        case KoParagraphStyle::ListStartValue:
        case KoParagraphStyle::RestartListNumbering:
        case KoParagraphStyle::TextProgressionDirection:
        case KoParagraphStyle::MasterPageName:
        case KoParagraphStyle::OutlineLevel:
            break;
        case KoParagraphStyle::AutoTextIndent:
            key = "autotextindent";
            value = properties[id].toBool() ? "true" : "false" ;
            break;
#ifdef PARAGRAPH_BORDER_DEBUG // because it tends to get annoyingly long :)
        case KoParagraphStyle::LeftBorderWidth:
            key = "border-width-left";
            value = QString::number(properties[id].toDouble()) ;
            break;
        case KoParagraphStyle::TopBorderWidth:
            key = "border-width-top";
            value = QString::number(properties[id].toDouble()) ;
            break;
        case KoParagraphStyle::RightBorderWidth:
            key = "border-width-right";
            value = QString::number(properties[id].toDouble()) ;
            break;
        case KoParagraphStyle::BottomBorderWidth:
            key = "border-width-bottom";
            value = QString::number(properties[id].toDouble()) ;
            break;
        case KoParagraphStyle::LeftBorderStyle:
            key = "border-style-left";
            value = QString::number(properties[id].toDouble()) ;
            break;
        case KoParagraphStyle::LeftBorderSpacing:
            key = "inner-border-spacing-left";
            value = QString::number(properties[id].toDouble()) ;
            break;
        case KoParagraphStyle::LeftInnerBorderWidth:
            key = "inner-border-width-left";
            value = QString::number(properties[id].toDouble()) ;
            break;
#endif
        case KoParagraphStyle::TabStopDistance:
            key = "tab-stop-distance";
            value = QString::number(properties[id].toDouble());
            break;
        case KoParagraphStyle::TabPositions:
            key = "tab-stops";
            value = "";
            foreach(const QVariant & qvtab, qvariant_cast<QList<QVariant> >(properties[id])) {
                KoText::Tab tab = qvtab.value<KoText::Tab>();
                value.append("{");
                value.append(" pos:").append(QString::number(tab.position));
                value.append(" type:").append(QString::number(tab.type));
                if (! tab.delimiter.isNull())
                    value.append(" delim:").append(QString(tab.delimiter));
                value.append(" leadertype:").append(QString::number(tab.leaderType));
                value.append(" leaderstyle:").append(QString::number(tab.leaderStyle));
                value.append(" leaderweight:").append(QString::number(tab.leaderWeight));
                value.append(" leaderwidth:").append(QString().setNum(tab.leaderWidth));
                value.append(" leadercolor:").append(tab.leaderColor.name());
                if (! tab.leaderText.isEmpty())
                    value.append(" leadertext:").append(QString(tab.leaderText));
                value.append("}, ");
            }
            break;
        case KoParagraphStyle::DropCaps:
            key = "drop-caps";
            value = QString::number(properties[id].toBool());
            break;
        case KoParagraphStyle::DropCapsLines:
            key = "drop-caps-lines";
            value = QString::number(properties[id].toInt());
            break;
        case KoParagraphStyle::DropCapsLength:
            key = "drop-caps-length";
            value = QString::number(properties[id].toInt());
            break;
        case KoParagraphStyle::DropCapsDistance:
            key = "drop-caps-distance";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFormat::BlockBottomMargin:
            value = QString::number(properties[id].toDouble());
            if (value != "0")
                key = "block-bottom-margin";
            break;
        case QTextFormat::BlockTopMargin:
            value = QString::number(properties[id].toDouble());
            if (value != "0")
                key = "block-top-margin";
            break;
        case QTextFormat::BlockLeftMargin:
            value = QString::number(properties[id].toDouble());
            if (value != "0")
                key = "block-left-margin";
            break;
        case QTextFormat::BlockRightMargin:
            value = QString::number(properties[id].toDouble());
            if (value != "0")
                key = "block-right-margin";
            break;
        case KoParagraphStyle::UnnumberedListItem:
            key = "unnumbered-list-item";
            value = QString::number(properties[id].toBool());
            break;
        case KoParagraphStyle::IsListHeader:
            key = "list-header";
            value = '1';
            break;
        case KoParagraphStyle::ListLevel:
            key = "list-level";
            value = QString::number(properties[id].toInt());
            break;
        default:
            break;
        }
        if (!key.isEmpty())
            attrs.append(" ").append(key).append("=\"").append(value).append("\"");
    }
    return attrs;
}

QString KoTextDebug::listAttributes(const QTextListFormat &listFormat)
{
    QString attrs;
    KoStyleManager *styleManager = document ? KoTextDocument(document).styleManager() : 0;
    if (styleManager && listFormat.hasProperty(KoListStyle::StyleId)) {
        int id = listFormat.intProperty(KoListStyle::StyleId);
        KoListStyle *listStyle = styleManager->listStyle(id);
        attrs.append(" listStyle=\"id:").append(QString::number(id));
        if (listStyle)
            attrs.append(" name:").append(listStyle->name());
        attrs.append("\"");
    }

    QMap<int, QVariant> properties = listFormat.properties();
    foreach(int id, properties.keys()) {
        QString key, value;
        switch (id) {
        case QTextListFormat::ListStyle:
            key = "type";
            value = QString::number(properties[id].toInt());
            break;
        case QTextListFormat::ListIndent:
            key = "indent";
            value = QString::number(properties[id].toDouble());
            break;
        case KoListStyle::ListItemPrefix:
            key = "prefix";
            value = properties[id].toString();
            break;
        case KoListStyle::ListItemSuffix:
            key = "suffix";
            value = properties[id].toString();
            break;
        case KoListStyle::StartValue:
            key = "start-value";
            value = QString::number(properties[id].toInt());
            break;
        case KoListStyle::Level:
            key = "level";
            value = QString::number(properties[id].toInt());
            break;
        case KoListStyle::DisplayLevel:
            key = "display-level";
            value = QString::number(properties[id].toInt());
            break;
        case KoListStyle::CharacterStyleId:
            key = "charstyleid";
            value = QString::number(properties[id].toInt());
            break;
        case KoListStyle::Alignment:
            key = "alignment";
            value = QString::number(properties[id].toInt());
            break;
        case KoListStyle::BulletSize:
            key = "bullet-size";
            value = QString::number(properties[id].toInt());
            break;
        case KoListStyle::BulletCharacter:
            key = "bullet-char";
            value = properties[id].toString();
            break;
        case KoListStyle::LetterSynchronization:
            key = "letter-sync";
            value = QString::number(properties[id].toInt());
            break;
        case KoListStyle::StyleId:
            key = "styleid";
            value = QString::number(properties[id].toInt());
            break;
        case KoListStyle::ContinueNumbering:
            key = "continue-numbering";
            value = QString::number(properties[id].toInt());
            break;
        case KoListStyle::MinimumWidth:
            key = "minimum-width";
            value = QString::number(properties[id].toDouble());
            break;
        case KoListStyle::ListId:
            key = "list-id";
            value = QString::number(properties[id].toInt());
            break;
        case KoListStyle::IsOutline:
            key = "is-outline";
            value = properties[id].toBool();
            break;
        case KoListStyle::Indent:
            key = "indent";
            value = QString::number(properties[id].toInt());
            break;
        case KoListStyle::MinimumDistance:
            key = "minimum-distance";
            value = QString::number(properties[id].toDouble());
            break;
        case KoListStyle::Width:
            key = "width";
            value = QString::number(properties[id].toDouble());
            break;
        case KoListStyle::Height:
            key = "height";
            value = QString::number(properties[id].toDouble());
            break;
        case KoListStyle::BulletImageKey:
            key = "bullet-image-key";
            value = QString::number(properties[id].toInt());
            break;
        default:
            break;
        }
        if (!key.isEmpty())
            attrs.append(" ").append(key).append("=\"").append(value).append("\"");
    }
    return attrs;
}

QString KoTextDebug::tableAttributes(const KoTableStyle &tableStyle)
{
    QTextTableFormat format;
    tableStyle.applyStyle(format);
    return tableAttributes(format);
}

QString KoTextDebug::tableAttributes(const QTextTableFormat &tableFormat)
{
    QString attrs;
    KoStyleManager *styleManager = document ? KoTextDocument(document).styleManager() : 0;
    if (styleManager) {
        int id = tableFormat.intProperty(KoTableStyle::StyleId);
        KoTableStyle *tableStyle = styleManager->tableStyle(id);
        attrs.append(" tableStyle=\"id:").append(QString::number(id));
        if (tableStyle)
            attrs.append(" name:").append(tableStyle->name());
        attrs.append("\"");
    }

    QMap<int, QVariant> properties = tableFormat.properties();
    foreach(int id, properties.keys()) {
        QString key, value;
        switch (id) {
        case QTextTableFormat::TableColumnWidthConstraints:
        case QTextFormat::BackgroundBrush:
            key = "background";
            value = qvariant_cast<QBrush>(properties[id]).color().name(); // beware!
            break;
        case QTextFormat::BlockAlignment:
            key = "alignment";
            switch (properties[id].toInt()) {
                case Qt::AlignLeft:
                    value = "left";
                    break;
                case Qt::AlignRight:
                    value = "right";
                    break;
                case Qt::AlignHCenter:
                    value = "center";
                    break;
                case Qt::AlignJustify:
                    value = "justify";
                    break;
                default:
                    value = "";
                    break;
            }
            break;
        case KoTableStyle::KeepWithNext:
            key = "keep-with-next";
            value = properties[id].toBool() ? "true" : "false";
            break;
        case KoTableStyle::BreakBefore:
            key = "break-before";
            value = properties[id].toBool() ? "true" : "false";
            break;
        case KoTableStyle::BreakAfter:
            key = "break-after";
            value = properties[id].toBool() ? "true" : "false";
            break;
        case KoTableStyle::MayBreakBetweenRows:
            key = "may-break-between-rows";
            value = properties[id].toBool() ? "true" : "false";
            break;
        case KoTableStyle::MasterPageName:
            key = "master-page-name";
            value = properties[id].toString();
            break;
        case QTextTableFormat::TableColumns:
            key = "columns";
            value = QString::number(properties[id].toInt());
            break;
        case QTextTableFormat::TableCellSpacing:
            key = "cell-spacing";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextTableFormat::TableHeaderRowCount:
            key = "header-row-count";
            value = QString::number(properties[id].toInt());
            break;
        default:
            break;
        }
        if (!key.isEmpty())
            attrs.append(" ").append(key).append("=\"").append(value).append("\"");
    }
    return attrs;
}

QString KoTextDebug::frameAttributes(const QTextFrameFormat &frameFormat)
{
    QString attrs;

    QMap<int, QVariant> properties = frameFormat.properties();
    foreach(int id, properties.keys()) {
        QString key, value;
        switch (id) {
        case QTextFrameFormat::FrameBorderBrush:
            break;
        case QTextFrameFormat::FrameBorderStyle:
            key = "border-style";
            // determine border style.
            switch (properties[id].toInt()) {
            case QTextFrameFormat::BorderStyle_None:
                value = "None";
                break;
            case QTextFrameFormat::BorderStyle_Dotted:
                value = "Dotted";
                break;
            case QTextFrameFormat::BorderStyle_Dashed:
                value = "Dashed";
                break;
            case QTextFrameFormat::BorderStyle_Solid:
                value = "Solid";
                break;
            case QTextFrameFormat::BorderStyle_Double:
                value = "Double";
                break;
            case QTextFrameFormat::BorderStyle_DotDash:
                value = "DotDash";
                break;
            case QTextFrameFormat::BorderStyle_DotDotDash:
                value = "DotDotDash";
                break;
            case QTextFrameFormat::BorderStyle_Groove:
                value = "Groove";
                break;
            case QTextFrameFormat::BorderStyle_Ridge:
                value = "Ridge";
                break;
            case QTextFrameFormat::BorderStyle_Inset:
                value = "Inset";
                break;
            case QTextFrameFormat::BorderStyle_Outset:
                value = "Outset";
                break;
            default:
                value = "Unknown";
                break;
            }
            break;
        case QTextFrameFormat::FrameBorder:
            key = "border";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFrameFormat::FrameMargin:
            key = "margin";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFrameFormat::FramePadding:
            key = "padding";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFrameFormat::FrameWidth:
            key = "width";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFrameFormat::FrameHeight:
            key = "height";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFrameFormat::FrameTopMargin:
            key = "top-margin";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFrameFormat::FrameBottomMargin:
            key = "bottom-margin";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFrameFormat::FrameLeftMargin:
            key = "left-margin";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFrameFormat::FrameRightMargin:
            key = "right-margin";
            value = QString::number(properties[id].toDouble());
            break;
        default:
            break;
        }
        if (!key.isEmpty())
            attrs.append(" ").append(key).append("=\"").append(value).append("\"");
    }
    return attrs;
}

QString KoTextDebug::tableCellAttributes(const KoTableCellStyle &tableCellStyle)
{
    QTextTableCellFormat format;
    tableCellStyle.applyStyle(format);
    return tableCellAttributes(format);
}

QString KoTextDebug::tableCellAttributes(const QTextTableCellFormat &tableCellFormat)
{
    QString attrs;
    KoStyleManager *styleManager = document ? KoTextDocument(document).styleManager() : 0;
    if (styleManager) {
        int id = tableCellFormat.intProperty(KoTableCellStyle::StyleId);
        KoTableCellStyle *tableCellStyle = styleManager->tableCellStyle(id);
        attrs.append(" tableCellStyle=\"id:").append(QString::number(id));
        if (tableCellStyle)
            attrs.append(" name:").append(tableCellStyle->name());
        attrs.append("\"");
    }

    QMap<int, QVariant> properties = tableCellFormat.properties();
    foreach(int id, properties.keys()) {
        QString key, value;
        switch (id) {
        case QTextTableCellFormat::TableCellRowSpan:
            key = "row-span";
            value = QString::number(properties[id].toInt());
            break;
        case QTextTableCellFormat::TableCellColumnSpan:
            key = "column-span";
            value = QString::number(properties[id].toInt());
            break;
        case QTextFormat::TableCellTopPadding:
            key = "top-padding";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFormat::TableCellBottomPadding:
            key = "bottom-padding";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFormat::TableCellLeftPadding:
            key = "left-padding";
            value = QString::number(properties[id].toDouble());
            break;
        case QTextFormat::TableCellRightPadding:
            key = "right-padding";
            value = QString::number(properties[id].toDouble());
            break;
        case KoTableCellStyle::TopBorderOuterPen: {
            key = "top-border-outer";
            QPen pen = qvariant_cast<QPen>(properties[id]);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KoTableCellStyle::TopBorderSpacing:
            key = "top-border-spacing";
            value = QString::number(properties[id].toDouble());
            break;
        case KoTableCellStyle::TopBorderInnerPen: {
            key = "top-border-inner";
            QPen pen = qvariant_cast<QPen>(properties[id]);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KoTableCellStyle::LeftBorderOuterPen: {
            key = "left-border-outer";
            QPen pen = qvariant_cast<QPen>(properties[id]);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KoTableCellStyle::LeftBorderSpacing:
            key = "left-border-spacing";
            value = QString::number(properties[id].toDouble());
            break;
        case KoTableCellStyle::LeftBorderInnerPen: {
            key = "left-border-inner";
            QPen pen = qvariant_cast<QPen>(properties[id]);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KoTableCellStyle::BottomBorderOuterPen: {
            key = "bottom-border-outer";
            QPen pen = qvariant_cast<QPen>(properties[id]);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KoTableCellStyle::BottomBorderSpacing:
            key = "bottom-border-spacing";
            value = QString::number(properties[id].toDouble());
            break;
        case KoTableCellStyle::BottomBorderInnerPen: {
            key = "bottom-border-inner";
            QPen pen = qvariant_cast<QPen>(properties[id]);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KoTableCellStyle::RightBorderOuterPen: {
            key = "right-border-outer";
            QPen pen = qvariant_cast<QPen>(properties[id]);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KoTableCellStyle::RightBorderSpacing:
            key = "right-border-spacing";
            value = QString::number(properties[id].toDouble());
            break;
        case KoTableCellStyle::RightBorderInnerPen: {
            key = "right-border-inner";
            QPen pen = qvariant_cast<QPen>(properties[id]);
            if (pen.style() == Qt::NoPen) {
                value = "none";
            } else {
                value = QString::number(pen.widthF()) + QString(" pt ");
                switch (pen.style()) {
                    case Qt::SolidLine:
                        value += "solid";
                        break;
                    case Qt::DashLine:
                        value += "dash";
                        break;
                    case Qt::DotLine:
                        value += "dot";
                        break;
                    case Qt::DashDotLine:
                        value += "dash-dot";
                        break;
                    case Qt::DashDotDotLine:
                        value += "dash-dot-dot";
                        break;
                    case Qt::CustomDashLine:
                        value += "custom-dash";
                        break;
                    default:
                        value += "";
                        break;
                }
                value += QString(" ") + qvariant_cast<QBrush>(pen).color().name(); // beware!
            }
            break;
        }
        case KoTableCellStyle::MasterPageName:
            key = "master-page-name";
            value = properties[id].toString();
            break;
        default:
            break;
        }
        if (!key.isEmpty())
            attrs.append(" ").append(key).append("=\"").append(value).append("\"");
    }
    return attrs;
}

void KoTextDebug::dumpFrame(const QTextFrame *frame)
{
    depth += INDENT;

    QString attrs;
    attrs.append(frameAttributes(frame->frameFormat()));
    qDebug("%*s<frame%s>", depth, " ", qPrintable(attrs));

    QTextFrame::iterator iterator = frame->begin();

    for (; !iterator.atEnd() && !iterator.atEnd(); ++iterator) {
        QTextFrame *childFrame = iterator.currentFrame();
        QTextBlock textBlock = iterator.currentBlock();

        if (childFrame) {
            QTextTable *table = qobject_cast<QTextTable *>(childFrame);
            if (table) {
                dumpTable(table);
            } else {
                dumpFrame(frame);
            }
        } else if (textBlock.isValid()) {
            dumpBlock(textBlock);
        }
    }

    qDebug("%*s%s", depth, " ", "</frame>");
    depth -= INDENT;
}

void KoTextDebug::dumpBlock(const QTextBlock &block)
{
    depth += INDENT;

    QString attrs;
    attrs.append(paraAttributes(block.blockFormat()));
    //attrs.append(" blockcharformat=\"").append(textAttributes(QTextCursor(block).blockCharFormat())).append('\"');
    attrs.append(textAttributes(QTextCursor(block).blockCharFormat()));

    QTextList *list = block.textList();
    if (list) {
        attrs.append(" list=\"item:").append(QString::number(list->itemNumber(block) + 1)).append('/')
        .append(QString::number(list->count()));
        attrs.append('"');
        attrs.append(listAttributes(list->format()));
    }

    qDebug("%*s<block%s>", depth, " ", qPrintable(attrs));

    QTextBlock::Iterator iterator = block.begin();
    for (; !iterator.atEnd() && !iterator.atEnd(); ++iterator) {
        QTextFragment fragment = iterator.fragment();
        if (fragment.isValid()) {
            dumpFragment(fragment);
        }
    }
    qDebug("%*s%s", depth, " ", "</block>");
    depth -= INDENT;
    if (block.next().isValid())
        qDebug(" ");
}

void KoTextDebug::dumpTable(const QTextTable *table)
{
    depth += INDENT;

    QString attrs;
    attrs.append(tableAttributes(table->format()));
    attrs.append(frameAttributes(table->frameFormat())); // include frame attribues too.

    qDebug("%*s<table%s>", depth, " ", qPrintable(attrs));

    // loop through all the cells in the table and dump the cells.
    for (int row = 0; row < table->rows(); ++row) {
        for (int column = 0; column < table->columns(); ++column) {
            dumpTableCell(table->cellAt(row, column));
        }
    }

    qDebug("%*s%s", depth, " ", "</table>");
    depth -= INDENT;
}

void KoTextDebug::dumpTableCell(const QTextTableCell &cell)
{
    depth += INDENT;

    QString attrs;
    attrs.append(textAttributes(cell.format()));
    attrs.append(tableCellAttributes(cell.format().toTableCellFormat()));

    qDebug("%*s<cell%s>", depth, " ", qPrintable(attrs));

    // iterate through the cell content.
    QTextFrame::iterator cellIter = cell.begin();
    while (!cellIter.atEnd()) {
        if (cellIter.currentFrame() != 0) {
            // content is a frame or table.
            dumpFrame(cellIter.currentFrame());
        } else {
            // content is a block.
            dumpBlock(cellIter.currentBlock());
        }
        ++cellIter;
    }

    qDebug("%*s%s", depth, " ", "</cell>");

    depth -= INDENT;
}

void KoTextDebug::dumpFragment(const QTextFragment &fragment)
{
    depth += INDENT;

    KoTextDocumentLayout *lay = document ? qobject_cast<KoTextDocumentLayout *>(document->documentLayout()) : 0;
    QTextCharFormat charFormat = fragment.charFormat();
    KoInlineObject *inlineObject = lay ? lay->inlineTextObjectManager()->inlineTextObject(charFormat) : 0;
    if (inlineObject) {
        QString cf = inlineObjectAttributes(charFormat);

        qDebug("%*s<fragment%s/>", depth, " ", qPrintable(cf));
    } else {
        QString cf = textAttributes(charFormat);

        qDebug("%*s<fragment%s>", depth, " ", qPrintable(cf));
        qDebug("%*s|%s|", depth + INDENT, " ", qPrintable(fragment.text()));
        qDebug("%*s%s", depth, " ", "</fragment>");
    }

    depth -= INDENT;
}

