/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2007-2008 Pierre Ducroquet <pinaraf@pinaraf.info>
 * Copyright (C) 2009-2011 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2009-2012 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2010 Nandita Suri <suri.nandita@gmail.com>
 * Copyright (C) 2010 Ajay Pundhir <ajay.pratap@iiitb.net>
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
 * Copyright (C) 2011 Stuart Dickson <stuart@furkinfantasic.net>
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

#include "KoTextLayoutArea.h"

#include "KoTextLayoutEndNotesArea.h"
#include "KoTextLayoutTableArea.h"
#include "KoTextLayoutNoteArea.h"
#include "TableIterator.h"
#include "ListItemsHelper.h"
#include "RunAroundHelper.h"
#include "KoTextDocumentLayout.h"
#include "FrameIterator.h"

#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>
#include <KoListStyle.h>
#include <KoStyleManager.h>
#include <KoTextBlockData.h>
#include <KoTextBlockBorderData.h>
#include <KoTextBlockPaintStrategyBase.h>
#include <KoText.h>
#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoImageData.h>

#include <TextLayoutDebug.h>
#include <KoSection.h>
#include <KoSectionEnd.h>
#include <KoSectionUtils.h>

#include <QPainter>
#include <QTextTable>
#include <QTextList>
#include <QFontMetrics>
#include <QTextFragment>
#include <QTextLayout>
#include <QTextCursor>
#include <QTime>
#include "kis_painting_tweaks.h"

extern int qt_defaultDpiY();
Q_DECLARE_METATYPE(QTextDocument *)

#define DropCapsAdditionalFormattingId 25602902

#include "KoTextLayoutArea_p.h"

void KoTextLayoutArea::paint(QPainter *painter, const KoTextDocumentLayout::PaintContext &context)
{
    if (d->startOfArea == 0 || d->endOfArea == 0) // We have not been layouted yet
        return;

    /*
    struct Timer {
        QTime d->time;
        Timer() { d->time.start(); }
        ~Timer() { warnTextLayout << "elapsed=" << d->time.elapsed(); }
    };
    Timer timer;
    */

    painter->save();
    painter->translate(0, d->verticalAlignOffset);

    painter->setPen(context.textContext.palette.color(QPalette::Text)); // for text that has no color.
    const QRegion clipRegion = KisPaintingTweaks::safeClipRegion(*painter); // fetch after painter->translate so the clipRegion is correct
    KoTextBlockBorderData *lastBorder = 0;
    QRectF lastBorderRect;

    QTextFrame::iterator it = d->startOfArea->it;
    QTextFrame::iterator stop = d->endOfArea->it;
    if (!stop.atEnd()) {
        if (!stop.currentBlock().isValid() || d->endOfArea->lineTextStart >= 0) {
            // Last thing we show is a frame (table) or first part of a paragraph split in two
            // The stop point should be the object after that
            // However if stop is already atEnd we shouldn't increment further
            ++stop;
        }
    }

    int tableAreaIndex = 0;
    int blockIndex = 0;
    int tocIndex = 0;
    for (; it != stop && !it.atEnd(); ++it) {
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();
        QTextBlockFormat format = block.blockFormat();

        if (!block.isValid()) {
            if (lastBorder) { // draw previous block's border
                lastBorder->paint(*painter, lastBorderRect);
                lastBorder = 0;
            }
        }

        if (table) {
            if (tableAreaIndex >= d->tableAreas.size()) {
                continue;
            }
            d->tableAreas[tableAreaIndex]->paint(painter, context);
            ++tableAreaIndex;
            continue;
        } else if (subFrame) {
            if (subFrame->format().intProperty(KoText::SubFrameType) == KoText::AuxillaryFrameType) {
                d->endNotesArea->paint(painter, context);
            }
            continue;
        } else {
            if (!block.isValid()) {
                continue;
            }
        }

        if (block.blockFormat().hasProperty(KoParagraphStyle::GeneratedDocument)) {
            // Possibly paint the selection of the entire Table of Contents
            // but since it's a secondary document we need to create a fake selection
            QVariant data = block.blockFormat().property(KoParagraphStyle::GeneratedDocument);
            QTextDocument *generatedDocument = data.value<QTextDocument *>();

            KoTextDocumentLayout::PaintContext tocContext = context;
            tocContext.textContext.selections = QVector<QAbstractTextDocumentLayout::Selection>();

            bool pure = true;
            Q_FOREACH (const QAbstractTextDocumentLayout::Selection &selection, context.textContext.selections) {
                if (selection.cursor.selectionStart()  <= block.position()
                    && selection.cursor.selectionEnd() >= block.position()) {
                    painter->fillRect(d->generatedDocAreas[tocIndex]->boundingRect(), selection.format.background());
                    if (pure) {
                        tocContext.textContext.selections.append(QAbstractTextDocumentLayout::Selection());
                        tocContext.textContext.selections[0].cursor = QTextCursor(generatedDocument);
                        tocContext.textContext.selections[0].cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
                        tocContext.textContext.selections[0].format = selection.format;
                        pure = false;
                    }
                }
            }
            d->generatedDocAreas[tocIndex]->paint(painter, tocContext);
            ++tocIndex;
            continue;
        }

        QTextLayout *layout = block.layout();
        KoTextBlockBorderData *border = 0;

        if (blockIndex >= d->blockRects.count())
            break;
        QRectF br = d->blockRects[blockIndex];
        ++blockIndex;

        if (!painter->hasClipping() || clipRegion.intersects(br.toRect())) {
            KoTextBlockData blockData(block);
            border = blockData.border();
            KoTextBlockPaintStrategyBase *paintStrategy = blockData.paintStrategy();

            KoTextBlockPaintStrategyBase dummyPaintStrategy;
            if (paintStrategy == 0) {
                paintStrategy = &dummyPaintStrategy;
            }
            if (!paintStrategy->isVisible()) {
                if (lastBorder) { // draw previous block's border
                    lastBorder->paint(*painter, lastBorderRect);
                    lastBorder = 0;
                }
                continue; // this paragraph shouldn't be shown so just skip it
            }

            // Check and update border drawing code
            if (lastBorder == 0) {
                lastBorderRect = br;
            } else if (lastBorder != border
                        || lastBorderRect.width() != br.width()
                        || lastBorderRect.x() != br.x()) {
                lastBorder->paint(*painter, lastBorderRect);
                lastBorderRect = br;
            } else {
                lastBorderRect = lastBorderRect.united(br);
            }
            lastBorder = border;

            painter->save();

            QBrush bg = paintStrategy->background(block.blockFormat().background());
            if (bg != Qt::NoBrush ) {
                painter->fillRect(br, bg);
            } else {
                bg = context.background;
            }

            paintStrategy->applyStrategy(painter);
            painter->save();
            drawListItem(painter, block);
            painter->restore();

            QVector<QTextLayout::FormatRange> selections;
            if (context.showSelections) {
                Q_FOREACH (const QAbstractTextDocumentLayout::Selection & selection, context.textContext.selections) {
                    QTextCursor cursor = selection.cursor;
                    int begin = cursor.position();
                    int end = cursor.anchor();
                    if (begin > end)
                        std::swap(begin, end);

                    if (end < block.position() || begin > block.position() + block.length())
                        continue; // selection does not intersect this block.
                    if (selection.cursor.hasComplexSelection()) {
                        continue; // selections of several table cells are covered by the within drawBorders above.
                    }
                    if (d->documentLayout->changeTracker()
                        && !d->documentLayout->changeTracker()->displayChanges()
                        && d->documentLayout->changeTracker()->containsInlineChanges(selection.format)
                        && d->documentLayout->changeTracker()->elementById(selection.format.property(KoCharacterStyle::ChangeTrackerId).toInt())->isEnabled()
                        && d->documentLayout->changeTracker()->elementById(selection.format.property(KoCharacterStyle::ChangeTrackerId).toInt())->getChangeType() == KoGenChange::DeleteChange) {
                        continue; // Deletions should not be shown.
                    }
                    QTextLayout::FormatRange fr;
                    fr.start = begin - block.position();
                    fr.length = end - begin;
                    fr.format = selection.format;
                    selections.append(fr);
                }
            }
            // this is a workaround to fix text getting cut of when format ranges are used. There
            // is a bug in Qt that can hit when text lines overlap each other. In case a format range
            // is used for formatting it can clip the lines above/below as Qt creates a clip rect for
            // the places it already painted for the format range which results in clippling. So use
            // the format range always to paint the text.
            QVector<QTextLayout::FormatRange> workaroundFormatRanges;
            for (QTextBlock::iterator it = block.begin(); !(it.atEnd()); ++it) {
                QTextFragment currentFragment = it.fragment();
                if (currentFragment.isValid()) {
                    bool formatChanged = false;

                    QTextCharFormat format = currentFragment.charFormat();
                    int changeId = format.intProperty(KoCharacterStyle::ChangeTrackerId);
                    if (changeId && d->documentLayout->changeTracker() && d->documentLayout->changeTracker()->displayChanges()) {
                        KoChangeTrackerElement *changeElement = d->documentLayout->changeTracker()->elementById(changeId);
                        switch(changeElement->getChangeType()) {
                            case (KoGenChange::InsertChange):
                            format.setBackground(QBrush(d->documentLayout->changeTracker()->getInsertionBgColor()));
                            break;
                            case (KoGenChange::FormatChange):
                            format.setBackground(QBrush(d->documentLayout->changeTracker()->getFormatChangeBgColor()));
                            break;
                            case (KoGenChange::DeleteChange):
                            format.setBackground(QBrush(d->documentLayout->changeTracker()->getDeletionBgColor()));
                            break;
                            case (KoGenChange::UNKNOWN):
                            break;
                        }
                        formatChanged = true;
                    }

                    if (format.isAnchor()) {
                        if (!format.hasProperty(KoCharacterStyle::UnderlineStyle))
                            format.setFontUnderline(true);
                        if (!format.hasProperty(QTextFormat::ForegroundBrush))
                            format.setForeground(Qt::blue);
                        formatChanged = true;
                    }

                    if (format.boolProperty(KoCharacterStyle::UseWindowFontColor)) {
                        QBrush backbrush = bg;
                        if (format.background() != Qt::NoBrush) {
                            backbrush = format.background();
                        }

                        QBrush frontBrush;
                        frontBrush.setStyle(Qt::SolidPattern);
                        // use the same luma calculation and threshold as msoffice
                        // see https://social.msdn.microsoft.com/Forums/en-US/os_binaryfile/thread/a02a9a24-efb6-4ba0-a187-0e3d2704882b
                        int luma = ((5036060/2) * backbrush.color().red()
                                    + (9886846/2) * backbrush.color().green()
                                    + (1920103/2) * backbrush.color().blue()) >> 23;
                        if (luma > 60) {
                            frontBrush.setColor(QColor(Qt::black));
                        } else {
                            frontBrush.setColor(QColor(Qt::white));
                        }
                        format.setForeground(frontBrush);

                        formatChanged = true;
                    }
                    if (formatChanged) {
                        QTextLayout::FormatRange fr;
                        fr.start = currentFragment.position() - block.position();
                        fr.length = currentFragment.length();
                        if (!format.hasProperty(KoCharacterStyle::InlineInstanceId)) {
                            if (format.background().style() == Qt::NoBrush) {
                                format.setBackground(QBrush(QColor(0, 0, 0, 0)));
                            }
                            if (format.foreground().style() == Qt::NoBrush) {
                                format.setForeground(QBrush(QColor(0, 0, 0)));
                            }
                        }
                        fr.format = format;
                        // the prepend is done so the selections are at the end.
                        selections.prepend(fr);
                    }
                    else {
                        if (!format.hasProperty(KoCharacterStyle::InlineInstanceId)) {
                            QTextLayout::FormatRange fr;
                            fr.start = currentFragment.position() - block.position();
                            fr.length = currentFragment.length();
                            QTextCharFormat f;
                            if (format.background().style() == Qt::NoBrush) {
                                f.setBackground(QBrush(QColor(0, 0, 0, 0)));
                            }
                            else {
                                f.setBackground(format.background());
                            }
                            if (format.foreground().style() == Qt::NoBrush) {
                                f.setForeground(QBrush(QColor(0, 0, 0)));
                            }
                            else {
                                f.setForeground(format.foreground());
                            }
                            fr.format = f;
                            workaroundFormatRanges.append(fr);
                        }
                    }
                }
            }

            if (!selections.isEmpty()) {
                selections = workaroundFormatRanges + selections;
            }

            //We set clip because layout-draw doesn't clip text to it correctly after all
            //and adjust to make sure we don't clip edges of glyphs. The clipping is
            //important for paragraph split across two pages.
            //20pt enlargement seems safe as pages is split by 50pt and this helps unwanted
            //glyph cutting
            painter->setClipRect(br.adjusted(-20,-20,20,20), Qt::IntersectClip);

            layout->draw(painter, QPointF(0, 0), selections);

            if (context.showSectionBounds) {
                decorateParagraphSections(painter, block);
            }
            decorateParagraph(painter, block, context.showFormattingCharacters, context.showSpellChecking);

            painter->restore();
        } else {
            if (lastBorder) {
                lastBorder->paint(*painter, lastBorderRect);
                lastBorder = 0;
            }
        }
    }
    if (lastBorder) {
        lastBorder->paint(*painter, lastBorderRect);
    }

    painter->translate(0, -d->verticalAlignOffset);
    painter->translate(0, bottom() - d->footNotesHeight);
    Q_FOREACH (KoTextLayoutNoteArea *footerArea, d->footNoteAreas) {
        footerArea->paint(painter, context);
        painter->translate(0, footerArea->bottom() - footerArea->top());
    }
    painter->restore();
}

void KoTextLayoutArea::drawListItem(QPainter *painter, QTextBlock &block)
{
    KoTextBlockData blockData(block);

    QTextList *list = block.textList();
    if (list && blockData.hasCounterData()) {
        QTextListFormat listFormat = list->format();
        if (! blockData.counterText().isEmpty()) {
            QFont font(blockData.labelFormat().font(), d->documentLayout->paintDevice());

            KoListStyle::Style listStyle = static_cast<KoListStyle::Style>(listFormat.style());
            QString result = blockData.counterText();

            QTextLayout layout(result, font, d->documentLayout->paintDevice());

            QList<QTextLayout::FormatRange> layouts;
            QTextLayout::FormatRange format;
            format.start = 0;
            format.length = blockData.counterText().length();
            format.format = blockData.labelFormat();

            layouts.append(format);
            layout.setAdditionalFormats(layouts);

            Qt::Alignment alignment = static_cast<Qt::Alignment>(listFormat.intProperty(KoListStyle::Alignment));

            if (alignment == 0) {
                alignment = Qt::AlignLeft | Qt::AlignAbsolute;
            }
            if (d->isRtl && (alignment & Qt::AlignAbsolute) == 0) {
                if (alignment & Qt::AlignLeft) {
                    alignment = Qt::AlignRight;
                } else if (alignment & Qt::AlignRight) {
                    alignment = Qt::AlignLeft;
                }
            }
            alignment |= Qt::AlignAbsolute;

            QTextOption option(alignment);
            option.setTextDirection(block.layout()->textOption().textDirection());
/*
            if (option.textDirection() == Qt::RightToLeft || blockData.counterText().isRightToLeft()) {
                option.setAlignment(Qt::AlignRight);
            }
*/
            layout.setTextOption(option);

            layout.beginLayout();

            QTextLine line = layout.createLine();
            line.setLineWidth(blockData.counterWidth());
            layout.endLayout();

            QPointF counterPosition = blockData.counterPosition();

            if (block.layout()->lineCount() > 0) {
                // if there is text, then baseline align the counter.
                QTextLine firstParagLine = block.layout()->lineAt(0);
                if (KoListStyle::isNumberingStyle(listStyle)) {
                    //if numbered list baseline align
                    counterPosition += QPointF(0, firstParagLine.ascent() - layout.lineAt(0).ascent());
                } else {
                     //for unnumbered list center align
                    counterPosition += QPointF(0, (firstParagLine.height() - layout.lineAt(0).height())/2.0);
                }
            }
            layout.draw(painter, counterPosition);

            //decorate the list label iff it is a numbered list
            if (KoListStyle::isNumberingStyle(listStyle)) {
                painter->save();
                decorateListLabel(painter, blockData, layout.lineAt(0), block);
                painter->restore();
            }
        }

        KoListStyle::Style listStyle = static_cast<KoListStyle::Style>(listFormat.style());
        if (listStyle == KoListStyle::ImageItem) {
            QFontMetricsF fm(blockData.labelFormat().font(), d->documentLayout->paintDevice());
            qreal x = qMax(qreal(1), blockData.counterPosition().x());
            qreal width = qMax(listFormat.doubleProperty(KoListStyle::Width), (qreal)1.0);
            qreal height = qMax(listFormat.doubleProperty(KoListStyle::Height), (qreal)1.0);
            qreal y = blockData.counterPosition().y() + fm.ascent() - fm.xHeight()/2 - height/2; // centered
            KoImageData *idata = listFormat.property(KoListStyle::BulletImage).value<KoImageData *>();
            if (idata) {
                painter->drawPixmap(x, y, width, height, idata->pixmap());
            }
        }
    }
}

void KoTextLayoutArea::decorateListLabel(QPainter *painter, const KoTextBlockData &blockData, const QTextLine &listLabelLine, const QTextBlock &listItem)
{
    const QTextCharFormat listLabelCharFormat = blockData.labelFormat();
    painter->setFont(listLabelCharFormat.font());

    int startOfFragmentInBlock = 0;
    Q_ASSERT_X(listLabelLine.isValid(), __FUNCTION__, QString("Invalid list label").toLocal8Bit());
    if (!listLabelLine.isValid()) {
        return;
    }

    int fragmentToLineOffset = 0;

    qreal x1 = blockData.counterPosition().x();
    qreal x2 = listItem.layout()->lineAt(0).x();

    if (x2 != x1) {
        drawStrikeOuts(painter, listLabelCharFormat, blockData.counterText(), listItem.layout()->lineAt(0), x1, x2, startOfFragmentInBlock, fragmentToLineOffset);
        drawOverlines(painter, listLabelCharFormat, blockData.counterText(), listItem.layout()->lineAt(0), x1, x2, startOfFragmentInBlock, fragmentToLineOffset);
        drawUnderlines(painter, listLabelCharFormat, blockData.counterText(), listItem.layout()->lineAt(0), x1, x2, startOfFragmentInBlock, fragmentToLineOffset);
    }
}

/**
 * Draw a line. Typically meant to underline text or similar.
 * @param painter the painter to paint on.
 * @param color the pen color to for the decoration line
 * @param type The type
 * @param style the type of line to draw.
 * @param width The thickness of the line, in pixels (the painter will be prescaled to points coordinate system).
 * @param x1 we are always drawing horizontal lines, this is the start point.
 * @param x2 we are always drawing horizontal lines, this is the end point.
 * @param y the y-offset to paint on.
 */
static void drawDecorationLine(QPainter *painter, const QColor &color, KoCharacterStyle::LineType type, KoCharacterStyle::LineStyle style, qreal width, const qreal x1, const qreal x2, const qreal y)
{
    QPen penBackup = painter->pen();
    QPen pen = painter->pen();
    pen.setColor(color);
    pen.setWidthF(width);
    if (style == KoCharacterStyle::WaveLine) {
        // Ok, try the waves :)
        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);
        qreal x = x1;
        const qreal halfWaveWidth = 0.5 * width;
        const qreal halfWaveLength = 2 * width;
        const int startAngle = 0 * 16;
        const int middleAngle = 180 * 16;
        const int endAngle = 180 * 16;
        while (x < x2) {
            QRectF rectangle1(x, y, halfWaveLength, 2*halfWaveWidth);
            if (type == KoCharacterStyle::DoubleLine) {
                painter->translate(0, -pen.width());
                painter->drawArc(rectangle1, startAngle, middleAngle);
                painter->translate(0, 2*pen.width());
                painter->drawArc(rectangle1, startAngle, middleAngle);
                painter->translate(0, -pen.width());
            } else {
                painter->drawArc(rectangle1, startAngle, middleAngle);
            }
            if (x + halfWaveLength > x2)
                break;
            QRectF rectangle2(x + halfWaveLength, y, halfWaveLength, 2*halfWaveWidth);
            if (type == KoCharacterStyle::DoubleLine) {
                painter->translate(0, -pen.width());
                painter->drawArc(rectangle2, middleAngle, endAngle);
                painter->translate(0, 2*pen.width());
                painter->drawArc(rectangle2, middleAngle, endAngle);
                painter->translate(0, -pen.width());
            } else {
                painter->drawArc(rectangle2, middleAngle, endAngle);
            }
            x = x + 2 * halfWaveLength;
        }
    } else {
        if (style == KoCharacterStyle::LongDashLine) {
            QVector<qreal> dashes;
            dashes << 12 << 2;
            pen.setDashPattern(dashes);
        } else {
            pen.setStyle((Qt::PenStyle)style);
        }
        painter->setPen(pen);
        if (type == KoCharacterStyle::DoubleLine) {
            painter->translate(0, -pen.width());
            painter->drawLine(QPointF(x1, y), QPointF(x2, y));
            painter->translate(0, 2*pen.width());
            painter->drawLine(QPointF(x1, y), QPointF(x2, y));
            painter->translate(0, -pen.width());
        } else {
            painter->drawLine(QPointF(x1, y), QPointF(x2, y));
        }
    }
    painter->setPen(penBackup);
}

static void drawDecorationText(QPainter *painter, const QTextLine &line, const QColor &color, const QString& decorText, qreal x1, qreal x2)
{
    qreal y = line.position().y();
    QPen oldPen = painter->pen();
    painter->setPen(QPen(color));
    do {
        QRectF br;
        painter->drawText(QRectF(QPointF(x1, y), QPointF(x2, y + line.height())), Qt::AlignLeft | Qt::AlignVCenter, decorText, &br);
        x1 = br.right();
    } while (x1 <= x2);
    painter->setPen(oldPen);
}

static void drawDecorationWords(QPainter *painter, const QTextLine &line, const QString &text, const QColor &color, KoCharacterStyle::LineType type, KoCharacterStyle::LineStyle style, const QString& decorText, qreal width, const qreal y, const int fragmentToLineOffset, const int startOfFragmentInBlock)
{
    qreal wordBeginX = -1;
    int j = line.textStart()+fragmentToLineOffset;
    while (j < line.textLength() + line.textStart() && j-startOfFragmentInBlock<text.size()) {
        if (text[j-startOfFragmentInBlock].isSpace()) {
            if (wordBeginX != -1) {
                if (decorText.isEmpty())
                    drawDecorationLine(painter, color, type, style, width, wordBeginX, line.cursorToX(j), y);
                else
                    drawDecorationText(painter, line, color, decorText, wordBeginX, line.cursorToX(j));
            }
            wordBeginX = -1;
        } else if (wordBeginX == -1) {
            wordBeginX = line.cursorToX(j);
        }
    ++j;
    }
    if (wordBeginX != -1) {
        if (decorText.isEmpty())
            drawDecorationLine(painter, color, type, style, width, wordBeginX, line.cursorToX(j), y);
        else
            drawDecorationText(painter, line, color, decorText, wordBeginX, line.cursorToX(j));
    }
}

static qreal computeWidth(KoCharacterStyle::LineWeight weight, qreal width, const QFont& font)
{
    switch (weight) {
    case KoCharacterStyle::AutoLineWeight:
    case KoCharacterStyle::NormalLineWeight:
    case KoCharacterStyle::MediumLineWeight:
    case KoCharacterStyle::DashLineWeight:
        return QFontMetricsF(font).lineWidth();
    case KoCharacterStyle::BoldLineWeight:
    case KoCharacterStyle::ThickLineWeight:
        return QFontMetricsF(font).lineWidth() * 1.5;
    case KoCharacterStyle::ThinLineWeight:
        return QFontMetricsF(font).lineWidth() * 0.7;
    case KoCharacterStyle::PercentLineWeight:
        return QFontInfo(font).pointSizeF() * width / 100;
    case KoCharacterStyle::LengthLineWeight:
        return width;
    }
    Q_ASSERT(0); // illegal weight passed
    return 0;
}

void KoTextLayoutArea::decorateParagraphSections(QPainter *painter, QTextBlock &block)
{
    QTextLayout *layout = block.layout();
    QTextBlockFormat bf = block.blockFormat();

    QPen penBackup = painter->pen();
    QPen pen = painter->pen();

    pen.setWidth(1);
    pen.setColor(Qt::gray);
    painter->setPen(pen);

    qreal xl = layout->boundingRect().left();
    qreal xr = qMax(layout->boundingRect().right(), layout->boundingRect().left() + width());
    qreal yu = layout->boundingRect().top();
    qreal yd = layout->boundingRect().bottom();

    qreal bracketSize = painter->fontMetrics().height() / 2;

    const qreal levelShift = 3;

    QList<KoSection *> openList = KoSectionUtils::sectionStartings(bf);
    for (int i = 0; i < openList.size(); i++) {
        int sectionLevel = openList[i]->level();
        if (i == 0) {
            painter->drawLine(xl + sectionLevel * levelShift, yu,
                              xr - sectionLevel * levelShift, yu);
        }
        painter->drawLine(xl + sectionLevel * levelShift, yu,
                          xl + sectionLevel * levelShift, yu + bracketSize);

        painter->drawLine(xr - sectionLevel * levelShift, yu,
                          xr - sectionLevel * levelShift, yu + bracketSize);
    }

    QList<KoSectionEnd *> closeList = KoSectionUtils::sectionEndings(bf);
    for (int i = 0; i < closeList.size(); i++) {
        int sectionLevel = closeList[i]->correspondingSection()->level();
        if (i == closeList.count() - 1) {
            painter->drawLine(xl + sectionLevel * levelShift, yd,
                              xr - sectionLevel * levelShift, yd);
        }
        painter->drawLine(xl + sectionLevel * levelShift, yd,
                          xl + sectionLevel * levelShift, yd - bracketSize);

        painter->drawLine(xr - sectionLevel * levelShift, yd,
                          xr - sectionLevel * levelShift, yd - bracketSize);
    }

    painter->setPen(penBackup);
}

void KoTextLayoutArea::decorateParagraph(QPainter *painter, QTextBlock &block, bool showFormattingCharacters, bool showSpellChecking)
{
    QTextLayout *layout = block.layout();

    QTextBlockFormat bf = block.blockFormat();
    QVariantList tabList = bf.property(KoParagraphStyle::TabPositions).toList();
    QFont oldFont = painter->font();

    QTextBlock::iterator it;
    int startOfBlock = -1;
    int currentTabStop = 0;

//    qDebug() << "\n-------------------"
//             << "\nGoing to decorate block\n"
//             << block.text()
//             << "\n-------------------";

    // loop over text fragments in this paragraph and draw the underline and line through.
    for (it = block.begin(); !it.atEnd(); ++it) {
        QTextFragment currentFragment = it.fragment();
        if (currentFragment.isValid()) {

//            qDebug() << "\tGoing to layout fragment:" << currentFragment.text();

            QTextCharFormat fmt = currentFragment.charFormat();
            painter->setFont(fmt.font());

            // a block doesn't have a real start position, so use our own counter. Initialize
            // it with the position of the first text fragment in the block.
            if (startOfBlock == -1) {
                startOfBlock = currentFragment.position(); // start of this block w.r.t. the document
            }

            // the start of our fragment in the block is the absolute position of the fragment
            // in the document minus the start of the block in the document.
            int startOfFragmentInBlock = currentFragment.position() - startOfBlock;

            // a fragment can span multiple lines, but we paint the decorations per line.
            int firstLine = layout->lineForTextPosition(currentFragment.position() - startOfBlock).lineNumber();
            int lastLine = layout->lineForTextPosition(currentFragment.position() + currentFragment.length()
                    - startOfBlock).lineNumber();

//            qDebug() << "\tfirst line:" << firstLine << "last line:" << lastLine;

            for (int i = firstLine ; i <= lastLine ; ++i) {
                QTextLine line = layout->lineAt(i);
//                qDebug() << "\n\t\tcurrent line:" << i
//                         << "\n\t\tline length:" << line.textLength() << "width:"<< line.width() << "natural width" << line.naturalTextWidth()
//                         << "\n\t\tvalid:" << layout->isValidCursorPosition(currentFragment.position() - startOfBlock)
//                         << "\n\t\tcurrentFragment.position:"  << currentFragment.position()
//                         << "\n\t\tstartOfBlock:" << startOfBlock
//                         << "\n\t\tstartOfFragmentInBlock:" << startOfFragmentInBlock;

                if (layout->isValidCursorPosition(currentFragment.position() - startOfBlock)) {

                    // the start position for painting the decoration is the position of the fragment
                    // inside, but after the first line, the decoration always starts at the beginning
                    // of the line.  See bug: 264471
                    int p1 = startOfFragmentInBlock;
                    if (i > firstLine) {
                        p1 = line.textStart();
                    }

//                    qDebug() << "\n\t\tblock.text.length:" << block.text().length() << "p1" << p1;

                    if (block.text().length() > p1 && block.text().at(p1) != QChar::ObjectReplacementCharacter) {
                        Q_ASSERT_X(line.isValid(), __FUNCTION__, QString("Invalid line=%1 first=%2 last=%3").arg(i).arg(firstLine).arg(lastLine).toLocal8Bit()); // see bug 278682
                        if (!line.isValid())
                            continue;

                        // end position: note that x2 can be smaller than x1 when we are handling RTL
                        int p2 = startOfFragmentInBlock + currentFragment.length();
                        int lineEndWithoutPreedit = line.textStart() + line.textLength();
                        if (block.layout()->preeditAreaPosition() >= block.position() + line.textStart() &&
                                block.layout()->preeditAreaPosition() <= block.position() + line.textStart() + line.textLength()) {
                            lineEndWithoutPreedit -= block.layout()->preeditAreaText().length();
                        }
                        while (lineEndWithoutPreedit > line.textStart() && block.text().at(lineEndWithoutPreedit - 1) == ' ') {
                            --lineEndWithoutPreedit;
                        }
                        if (lineEndWithoutPreedit < p2) { //line caps
                            p2 = lineEndWithoutPreedit;
                        }
                        int fragmentToLineOffset = qMax(startOfFragmentInBlock - line.textStart(), 0);

                        qreal x1 = line.cursorToX(p1);
                        qreal x2 = line.cursorToX(p2);

                        //qDebug() << "\n\t\t\tp1:" << p1 << "x1:" << x1
                          //       << "\n\t\t\tp2:" << p2 << "x2:" << x2
                            //     << "\n\t\t\tlineEndWithoutPreedit" << lineEndWithoutPreedit;

                        if (x1 != x2) {
                            drawStrikeOuts(painter, fmt, currentFragment.text(), line, x1, x2, startOfFragmentInBlock, fragmentToLineOffset);
                            drawOverlines(painter, fmt, currentFragment.text(), line, x1, x2, startOfFragmentInBlock, fragmentToLineOffset);
                            drawUnderlines(painter, fmt, currentFragment.text(), line, x1, x2, startOfFragmentInBlock, fragmentToLineOffset);
                        }
                        decorateTabsAndFormatting(painter, currentFragment, line, startOfFragmentInBlock, tabList, currentTabStop, showFormattingCharacters);

                        // underline preedit strings
                        if (layout->preeditAreaPosition() > -1) {
                            int start = block.layout()->preeditAreaPosition();
                            int end = block.layout()->preeditAreaPosition() + block.layout()->preeditAreaText().length();

                            QTextCharFormat underline;
                            underline.setFontUnderline(true);
                            underline.setUnderlineStyle(QTextCharFormat::DashUnderline);

                            //qDebug() << "underline style" << underline.underlineStyle();
                            //qDebug() << "line start: " << block.position() << line.textStart();
                            qreal z1 = 0;
                            qreal z2 = 0;

                            // preedit start in this line, end in this line
                            if ( start >= block.position() + line.textStart() &&
                                 end <= block.position() + line.textStart() + line.textLength() ) {
                                z1 = line.cursorToX(start);
                                z2 = line.cursorToX(end);
                            }
                            // preedit start in this line, end after this line
                            if ( start >= block.position() + line.textStart() &&
                                 end > block.position() + line.textStart() + line.textLength() ) {
                                z1 = line.cursorToX(start);
                                z2 = line.cursorToX(block.position() + line.textStart() + line.textLength());
                            }
                            // preedit start before this line, end in this line
                            if ( start < block.position() + line.textStart() &&
                                 end <= block.position() + line.textStart() + line.textLength() ) {
                                z1 = line.cursorToX(block.position() + line.textStart());
                                z2 = line.cursorToX(end);
                            }
                            // preedit start before this line, end after this line
                            if ( start < block.position() + line.textStart() &&
                                 end > block.position() + line.textStart() + line.textLength() ) {
                                z1 = line.cursorToX(block.position() + line.textStart());
                                z2 = line.cursorToX(block.position() + line.textStart() + line.textLength());
                            }
                            if (z2 > z1) {
                                //qDebug() << "z1: " << z1 << "z2: " << z2;
                                KoCharacterStyle::LineStyle fontUnderLineStyle = KoCharacterStyle::DashLine;
                                KoCharacterStyle::LineType fontUnderLineType = KoCharacterStyle::SingleLine;
                                QTextCharFormat::VerticalAlignment valign = fmt.verticalAlignment();

                                QFont font(fmt.font());
                                if (valign == QTextCharFormat::AlignSubScript
                                        || valign == QTextCharFormat::AlignSuperScript)
                                    font.setPointSize(font.pointSize() * 2 / 3);
                                QFontMetricsF metrics(font, d->documentLayout->paintDevice());

                                qreal y = line.position().y();
                                if (valign == QTextCharFormat::AlignSubScript)
                                    y += line.height() - metrics.descent() + metrics.underlinePos();
                                else if (valign == QTextCharFormat::AlignSuperScript)
                                    y += metrics.ascent() + metrics.underlinePos();
                                else
                                    y += line.ascent() + metrics.underlinePos();

                                QColor color = fmt.foreground().color();
                                qreal width = computeWidth( // line thickness
                                                            (KoCharacterStyle::LineWeight) underline.intProperty(KoCharacterStyle::UnderlineWeight),
                                                            underline.doubleProperty(KoCharacterStyle::UnderlineWidth),
                                                            font);
                                if (valign == QTextCharFormat::AlignSubScript
                                        || valign == QTextCharFormat::AlignSuperScript) // adjust size.
                                    width = width * 2 / 3;

                                drawDecorationLine(painter, color, fontUnderLineType, fontUnderLineStyle, width, z1, z2, y);
                            }
                        }
                    }
                }
            }
        }
    }

    if (showFormattingCharacters) {
        QTextLine line = layout->lineForTextPosition(block.length()-1);
        qreal y = line.position().y() + line.ascent();
        qreal x = line.cursorToX(block.length()-1);
        painter->drawText(QPointF(x, y), QChar((ushort)0x00B6));
    }

    if (showSpellChecking) {
        // Finally let's paint our own spelling markings
        // TODO Should we make this optional at this point (right on/off handled by the plugin)
        // also we might want to provide alternative ways of drawing it
        KoTextBlockData blockData(block);
        QPen penBackup = painter->pen();
        QPen pen;
        pen.setColor(QColor(Qt::red));
        pen.setWidthF(1.5);
        QVector<qreal> pattern;
        pattern << 1 << 2;
        pen.setDashPattern(pattern);
        painter->setPen(pen);

        QList<KoTextBlockData::MarkupRange>::Iterator markIt = blockData.markupsBegin(KoTextBlockData::Misspell);
        QList<KoTextBlockData::MarkupRange>::Iterator markEnd = blockData.markupsEnd(KoTextBlockData::Misspell);
        for (int i = 0 ; i < layout->lineCount(); ++i) {
            if (markIt == markEnd) {
                break;
            }
            QTextLine line = layout->lineAt(i);
            // the y position is placed half way between baseline and descent of the line
            // this is fast and sufficient
            qreal y = line.position().y() + line.ascent() + 0.5 * line.descent();

            // first handle all those marking ranges that end on this line
            while (markIt != markEnd && markIt->lastChar < line.textStart() + line.textLength()
                && line.textStart() + line.textLength() <= block.length()) {
                if (!blockData.isMarkupsLayoutValid(KoTextBlockData::Misspell)) {
                    if (markIt->firstChar > line.textStart()) {
                        markIt->startX = line.cursorToX(markIt->firstChar);
                    }
                    markIt->endX = line.cursorToX(qMin(markIt->lastChar, block.length()));
                }
                qreal x1 = (markIt->firstChar > line.textStart()) ? markIt->startX : line.cursorToX(0);

                painter->drawLine(QPointF(x1, y), QPointF(markIt->endX, y));

                ++markIt;
            }

            // there may be a markup range on this line that extends to the next line
            if (markIt != markEnd && markIt->firstChar < line.textStart() + line.textLength()
                && line.textStart() + line.textLength() <= block.length()) {
                if (!blockData.isMarkupsLayoutValid(KoTextBlockData::Misspell)) {
                    if (markIt->firstChar > line.textStart()) {
                        markIt->startX = line.cursorToX(markIt->firstChar);
                    }
                }
                qreal x1 = (markIt->firstChar > line.textStart()) ? markIt->startX : line.cursorToX(0);

                painter->drawLine(QPointF(x1, y), QPointF(line.cursorToX(line.textStart() + line.textLength()), y));

                // since it extends to next line we don't increment the iterator
            }
        }
        blockData.setMarkupsLayoutValidity(KoTextBlockData::Misspell, true);

        painter->setPen(penBackup);
    }
    painter->setFont(oldFont);
}

void KoTextLayoutArea::drawStrikeOuts(QPainter *painter, const QTextCharFormat &currentCharFormat, const QString &text, const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock, const int fragmentToLineOffset) const
{
    KoCharacterStyle::LineStyle strikeOutStyle = (KoCharacterStyle::LineStyle)
            currentCharFormat.intProperty(KoCharacterStyle::StrikeOutStyle);
    KoCharacterStyle::LineType strikeOutType = (KoCharacterStyle::LineType)
            currentCharFormat.intProperty(KoCharacterStyle::StrikeOutType);
    if ((strikeOutStyle != KoCharacterStyle::NoLineStyle) &&
            (strikeOutType != KoCharacterStyle::NoLineType)) {
        QTextCharFormat::VerticalAlignment valign = currentCharFormat.verticalAlignment();

        QFont font(currentCharFormat.font());
        if (valign == QTextCharFormat::AlignSubScript
                || valign == QTextCharFormat::AlignSuperScript)
            font.setPointSize(qRound(font.pointSize() * 2 / 3.));
        QFontMetricsF metrics(font, d->documentLayout->paintDevice());

        qreal y = line.position().y();
        if (valign == QTextCharFormat::AlignSubScript)
            y += line.height() - metrics.descent() - metrics.strikeOutPos();
        else if (valign == QTextCharFormat::AlignSuperScript)
            y += metrics.ascent() - metrics.strikeOutPos();
        else
            y += line.ascent() - metrics.strikeOutPos();

        QColor color = currentCharFormat.colorProperty(KoCharacterStyle::StrikeOutColor);
        if (!color.isValid())
            color = currentCharFormat.foreground().color();
        KoCharacterStyle::LineMode strikeOutMode =
            (KoCharacterStyle::LineMode) currentCharFormat.intProperty(KoCharacterStyle::StrikeOutMode);

        QString strikeOutText = currentCharFormat.stringProperty(KoCharacterStyle::StrikeOutText);
        qreal width = 0; // line thickness
        if (strikeOutText.isEmpty()) {
            width = computeWidth(
                        (KoCharacterStyle::LineWeight) currentCharFormat.intProperty(KoCharacterStyle::StrikeOutWeight),
                        currentCharFormat.doubleProperty(KoCharacterStyle::StrikeOutWidth),
                        font);
        }
        if (valign == QTextCharFormat::AlignSubScript
                || valign == QTextCharFormat::AlignSuperScript) // adjust size.
            width = width * 2 / 3;

        if (strikeOutMode == KoCharacterStyle::SkipWhiteSpaceLineMode) {
            drawDecorationWords(painter, line, text, color, strikeOutType,
                    strikeOutStyle, strikeOutText, width, y, fragmentToLineOffset,
                    startOfFragmentInBlock);
        } else {
            if (strikeOutText.isEmpty())
                drawDecorationLine(painter, color, strikeOutType, strikeOutStyle, width, x1, x2, y);
            else
                drawDecorationText(painter, line, color, strikeOutText, x1, x2);
        }
    }
}

void KoTextLayoutArea::drawOverlines(QPainter *painter, const QTextCharFormat &currentCharFormat, const QString &text, const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock, const int fragmentToLineOffset) const
{
    KoCharacterStyle::LineStyle fontOverLineStyle = (KoCharacterStyle::LineStyle) currentCharFormat.intProperty(KoCharacterStyle::OverlineStyle);
    KoCharacterStyle::LineType fontOverLineType = (KoCharacterStyle::LineType) currentCharFormat.intProperty(KoCharacterStyle::OverlineType);
    if ((fontOverLineStyle != KoCharacterStyle::NoLineStyle) &&
            (fontOverLineType != KoCharacterStyle::NoLineType)) {
        QTextCharFormat::VerticalAlignment valign = currentCharFormat.verticalAlignment();

        QFont font(currentCharFormat.font());
        if (valign == QTextCharFormat::AlignSubScript
                || valign == QTextCharFormat::AlignSuperScript)
            font.setPointSize(font.pointSize() * 2 / 3);
        QFontMetricsF metrics(font, d->documentLayout->paintDevice());

        qreal y = line.position().y();
        if (valign == QTextCharFormat::AlignSubScript)
            y += line.height() - metrics.descent() - metrics.overlinePos();
        else if (valign == QTextCharFormat::AlignSuperScript)
            y += metrics.ascent() - metrics.overlinePos();
        else
            y += line.ascent() - metrics.overlinePos();

        QColor color = currentCharFormat.colorProperty(KoCharacterStyle::OverlineColor);
        if (!color.isValid())
            color = currentCharFormat.foreground().color();
        KoCharacterStyle::LineMode overlineMode =
            (KoCharacterStyle::LineMode) currentCharFormat.intProperty(KoCharacterStyle::OverlineMode);
        qreal width = computeWidth( // line thickness
                          (KoCharacterStyle::LineWeight) currentCharFormat.intProperty(KoCharacterStyle::OverlineWeight),
                          currentCharFormat.doubleProperty(KoCharacterStyle::OverlineWidth),
                          font);
        if (valign == QTextCharFormat::AlignSubScript
                || valign == QTextCharFormat::AlignSuperScript) // adjust size.
            width = width * 2 / 3;

        if (overlineMode == KoCharacterStyle::SkipWhiteSpaceLineMode) {
            drawDecorationWords(painter, line, text, color, fontOverLineType,
                    fontOverLineStyle, QString(), width, y, fragmentToLineOffset, startOfFragmentInBlock);
        } else {
            drawDecorationLine(painter, color, fontOverLineType, fontOverLineStyle, width, x1, x2, y);
        }
    }
}

void KoTextLayoutArea::drawUnderlines(QPainter *painter, const QTextCharFormat &currentCharFormat,const QString &text, const QTextLine &line, qreal x1, qreal x2, const int startOfFragmentInBlock, const int fragmentToLineOffset) const
{
    KoCharacterStyle::LineStyle fontUnderLineStyle = (KoCharacterStyle::LineStyle) currentCharFormat.intProperty(KoCharacterStyle::UnderlineStyle);
    KoCharacterStyle::LineType fontUnderLineType = (KoCharacterStyle::LineType) currentCharFormat.intProperty(KoCharacterStyle::UnderlineType);
    if ((fontUnderLineStyle != KoCharacterStyle::NoLineStyle) &&
            (fontUnderLineType != KoCharacterStyle::NoLineType)) {
        QTextCharFormat::VerticalAlignment valign = currentCharFormat.verticalAlignment();

        QFont font(currentCharFormat.font());
        if (valign == QTextCharFormat::AlignSubScript
                || valign == QTextCharFormat::AlignSuperScript)
            font.setPointSize(font.pointSize() * 2 / 3);
        QFontMetricsF metrics(font, d->documentLayout->paintDevice());

        qreal y = line.position().y();
        if (valign == QTextCharFormat::AlignSubScript)
            y += line.height() - metrics.descent() + metrics.underlinePos();
        else if (valign == QTextCharFormat::AlignSuperScript)
            y += metrics.ascent() + metrics.underlinePos();
        else
            y += line.ascent() + metrics.underlinePos();

        QColor color = currentCharFormat.underlineColor();
        if (!color.isValid())
            color = currentCharFormat.foreground().color();
        KoCharacterStyle::LineMode underlineMode =
            (KoCharacterStyle::LineMode) currentCharFormat.intProperty(KoCharacterStyle::UnderlineMode);
        qreal width = computeWidth( // line thickness
                          (KoCharacterStyle::LineWeight) currentCharFormat.intProperty(KoCharacterStyle::UnderlineWeight),
                          currentCharFormat.doubleProperty(KoCharacterStyle::UnderlineWidth),
                          font);
        if (valign == QTextCharFormat::AlignSubScript
                || valign == QTextCharFormat::AlignSuperScript) // adjust size.
            width = width * 2 / 3;

        if (underlineMode == KoCharacterStyle::SkipWhiteSpaceLineMode) {
            drawDecorationWords(painter, line, text, color, fontUnderLineType,
                    fontUnderLineStyle, QString(), width, y, fragmentToLineOffset, startOfFragmentInBlock);
        } else {
            drawDecorationLine(painter, color, fontUnderLineType, fontUnderLineStyle, width, x1, x2, y);
        }
    }
}

// Decorate any tabs ('\t's) in 'currentFragment' and laid out in 'line'.
int KoTextLayoutArea::decorateTabsAndFormatting(QPainter *painter, const QTextFragment& currentFragment, const QTextLine &line, const int startOfFragmentInBlock, const QVariantList& tabList, int currentTabStop, bool showFormattingCharacters)
{
    // If a line in the layout represent multiple text fragments, this function will
    // be called multiple times on the same line, with different fragments.
    // Likewise, if a fragment spans two lines, then this function will be called twice
    // on the same fragment, once for each line.

    QString fragText = currentFragment.text();

    QFontMetricsF fm(currentFragment.charFormat().font(), d->documentLayout->paintDevice());
    qreal tabStyleLineMargin = fm.averageCharWidth() / 4; // leave some margin for the tab decoration line

    // currentFragment.position() : start of this fragment w.r.t. the document
    // startOfFragmentInBlock : start of this fragment w.r.t. the block
    // line.textStart() : start of this line w.r.t. the block

    int searchForCharFrom; // search for \t from this point onwards in fragText
    int searchForCharTill; // search for \t till this point in fragText

    if (line.textStart() >= startOfFragmentInBlock) { // fragment starts at or before the start of line
        // we are concerned with only that part of the fragment displayed in this line
        searchForCharFrom = line.textStart() - startOfFragmentInBlock;
        // It's a new line. So we should look at the first tab-stop properties for the next \t.
        currentTabStop = 0;
    } else { // fragment starts in the middle of the line
        searchForCharFrom = 0;
    }
    if (line.textStart() + line.textLength() > startOfFragmentInBlock + currentFragment.length()) {
        // fragment ends before the end of line. need to see only till the end of the fragment.
        searchForCharTill = currentFragment.length();
    } else {
        // line ends before the fragment ends. need to see only till the end of this line.
        // but then, we need to convert the end of line to an index into fragText
        searchForCharTill = line.textLength() + line.textStart() - startOfFragmentInBlock;
    }
    for (int i = searchForCharFrom ; i < searchForCharTill; i++) {
        if (currentTabStop >= tabList.size() && !showFormattingCharacters) // no more decorations
            break;

        if (fragText[i] == '\t') {
            qreal x1(0.0);
            qreal x2(0.0);

            if (showFormattingCharacters) {
                x1 = line.cursorToX(startOfFragmentInBlock + i);
                x2 = line.cursorToX(startOfFragmentInBlock + i + 1);
                qreal y = line.position().y() + line.ascent() - fm.xHeight()/2.0;
                qreal arrowDim = fm.xHeight()/2.0;
                QPen penBackup = painter->pen();
                QPen pen = painter->pen();
                pen.setWidthF(fm.ascent()/10.0);
                pen.setStyle(Qt::SolidLine);
                painter->setPen(pen);
                painter->drawLine(QPointF(x1, y), QPointF(x2, y));
                painter->drawLine(QPointF(x2 - arrowDim, y - arrowDim), QPointF(x2, y));
                painter->drawLine(QPointF(x2 - arrowDim, y + arrowDim), QPointF(x2, y));
                painter->setPen(penBackup);
            }
            if (currentTabStop < tabList.size()) { // still tabsstops worth examining
                if (!showFormattingCharacters) {
                    // only then was it  not calculated
                    x1 = line.cursorToX(startOfFragmentInBlock + i);
                }
                // find a tab-stop decoration for this tab position
                // for eg., if there's a tab-stop at 1in, but the text before \t already spans 1.2in,
                // we should look at the next tab-stop
                KoText::Tab tab;
                do {
                    tab = qvariant_cast<KoText::Tab>(tabList[currentTabStop]);
                    currentTabStop++;
                    // comparing with x1 should work for all of left/right/center/char tabs
                } while (tab.position <= x1 && currentTabStop < tabList.size());

                if (tab.position > x1) {
                    if (!showFormattingCharacters) {
                        // only then was it not calculated
                        x2 = line.cursorToX(startOfFragmentInBlock + i + 1);
                    }
                    qreal tabStyleLeftLineMargin = tabStyleLineMargin;
                    qreal tabStyleRightLineMargin = tabStyleLineMargin;
                    // no margin if its adjacent char is also a tab
                    if (i > searchForCharFrom && fragText[i-1] == '\t')
                        tabStyleLeftLineMargin = 0;
                    if (i < (searchForCharTill - 1) && fragText[i+1] == '\t')
                        tabStyleRightLineMargin = 0;

                    qreal y = line.position().y() + line.ascent() - 1;
                    x1 += tabStyleLeftLineMargin;
                    x2 -= tabStyleRightLineMargin;
                    QColor tabDecorColor = currentFragment.charFormat().foreground().color();
                    if (tab.leaderColor.isValid())
                        tabDecorColor = tab.leaderColor;
                    qreal width = computeWidth(tab.leaderWeight, tab.leaderWidth, painter->font());
                    if (x1 < x2) {
                        if (tab.leaderText.isEmpty()) {
                            drawDecorationLine(painter, tabDecorColor, tab.leaderType, tab.leaderStyle, width, x1, x2, y);
                        } else {
                            drawDecorationText(painter, line, tabDecorColor, tab.leaderText, x1, x2);
                        }
                    }
                }
            }
        } else if (showFormattingCharacters) {
            if (fragText[i] == ' ' || fragText[i] == QChar::Nbsp) {
                qreal x = line.cursorToX(startOfFragmentInBlock + i);
                qreal y = line.position().y() + line.ascent();

                painter->drawText(QPointF(x, y), QChar((ushort)0xb7));
            } else if (fragText[i] == QChar::LineSeparator){
                qreal x = line.cursorToX(startOfFragmentInBlock + i);
                qreal y = line.position().y() + line.ascent();

                painter->drawText(QPointF(x, y), QChar((ushort)0x21B5));
            }
        }
    }
    return currentTabStop;
}
