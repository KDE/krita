/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kogmbh.com>
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

#include "KoTextBlockData.h"

#include "KoTextBlockBorderData.h"
#include "KoTextBlockPaintStrategyBase.h"

class Q_DECL_HIDDEN KoTextBlockData::Private : public QTextBlockUserData
{
public:
    Private()
        : counterWidth(-1.0)
        , counterSpacing(0)
        , counterIsImage(false)
        , counterIndex(1)
        , border(0)
        , paintStrategy(0)
    {
        layoutedMarkupRanges[KoTextBlockData::Misspell] = false;
        layoutedMarkupRanges[KoTextBlockData::Grammar] = false;
    }

    ~Private() {
        if (border && !border->deref())
            delete border;
        delete paintStrategy;
    }
    qreal counterWidth;
    qreal counterSpacing;
    QString counterPrefix;
    QString counterPlainText;
    QString counterSuffix;
    QString partialCounterText;
    bool counterIsImage;
    int counterIndex;
    QPointF counterPos;
    QTextCharFormat labelFormat;
    KoTextBlockBorderData *border;
    KoTextBlockPaintStrategyBase *paintStrategy;
    QMap<KoTextBlockData::MarkupType, QList<MarkupRange> > markupRangesMap;
    QMap<KoTextBlockData::MarkupType, bool> layoutedMarkupRanges;
};

KoTextBlockData::KoTextBlockData(QTextBlock &block)
    : d(block.userData() ? dynamic_cast<KoTextBlockData::Private *>(block.userData())
                         : new Private())
{
    block.setUserData(d);
}

KoTextBlockData::KoTextBlockData(QTextBlockUserData *userData)
    : d(dynamic_cast<KoTextBlockData::Private *>(userData))
{
    Q_ASSERT(d);
}

KoTextBlockData::~KoTextBlockData()
{
    // explicitly do not delete the d-pointer here
}


void KoTextBlockData::appendMarkup(MarkupType type, int firstChar, int lastChar)
{
    Q_ASSERT(d->markupRangesMap[type].isEmpty() || d->markupRangesMap[type].last().lastChar < firstChar);

    MarkupRange range;
    range.firstChar = firstChar;
    range.lastChar = lastChar;
    d->layoutedMarkupRanges[type] = false;

    d->markupRangesMap[type].append(range);
}

void KoTextBlockData::clearMarkups(MarkupType type)
{
    d->markupRangesMap[type].clear();
    d->layoutedMarkupRanges[type] = false;
}

KoTextBlockData::MarkupRange KoTextBlockData::findMarkup(MarkupType type, int positionWithin) const
{
    foreach (const MarkupRange &range, d->markupRangesMap[type]) {
        if (positionWithin <= range.lastChar) {
            // possible hit
            if (positionWithin >= range.firstChar) {
                return range;
            } else {
                return MarkupRange(); // we have passed it without finding
            }
        }
    }
    return MarkupRange(); // either no ranges or not in last either
}

void KoTextBlockData::rebaseMarkups(MarkupType type, int fromPosition, int delta)
{
    QList<MarkupRange>::Iterator markIt = markupsBegin(type);
    QList<MarkupRange>::Iterator markEnd = markupsEnd(type);
    while (markIt != markEnd) {
        if (fromPosition <= markIt->lastChar) {
            // we need to modify the end of this
            markIt->lastChar += delta;
        }
        if (fromPosition < markIt->firstChar) {
            // we need to modify the end of this
            markIt->firstChar += delta;
        }
        ++markIt;
    }
}

void KoTextBlockData::setMarkupsLayoutValidity(MarkupType type, bool valid)
{
    d->layoutedMarkupRanges[type] = valid;
}

bool KoTextBlockData::isMarkupsLayoutValid(MarkupType type) const
{
    return d->layoutedMarkupRanges[type];
}

QList<KoTextBlockData::MarkupRange>::Iterator KoTextBlockData::markupsBegin(MarkupType type)
{
    return d->markupRangesMap[type].begin();
}

QList<KoTextBlockData::MarkupRange>::Iterator KoTextBlockData::markupsEnd(MarkupType type)
{
    return d->markupRangesMap[type].end();
}

bool KoTextBlockData::hasCounterData() const
{
    return d->counterWidth >= 0 && (!d->counterPlainText.isNull() || d->counterIsImage);
}

qreal KoTextBlockData::counterWidth() const
{
    return qMax(qreal(0), d->counterWidth);
}

void KoTextBlockData::setBorder(KoTextBlockBorderData *border)
{
    if (d->border && !d->border->deref())
        delete d->border;
    d->border = border;
    if (d->border)
        d->border->ref();
}

void KoTextBlockData::setCounterWidth(qreal width)
{
    d->counterWidth = width;
}

qreal KoTextBlockData::counterSpacing() const
{
    return d->counterSpacing;
}

void KoTextBlockData::setCounterSpacing(qreal spacing)
{
    d->counterSpacing = spacing;
}

QString KoTextBlockData::counterText() const
{
    return d->counterPrefix + d->counterPlainText + d->counterSuffix;
}

void KoTextBlockData::clearCounter()
{
    d->partialCounterText.clear();
    d->counterPlainText.clear();
    d->counterPrefix.clear();
    d->counterSuffix.clear();
    d->counterSpacing = 0.0;
    d->counterWidth = 0.0;
    d->counterIsImage = false;
}

void KoTextBlockData::setPartialCounterText(const QString &text)
{
    d->partialCounterText = text;
}

QString KoTextBlockData::partialCounterText() const
{
    return d->partialCounterText;
}

void KoTextBlockData::setCounterPlainText(const QString &text)
{
    d->counterPlainText = text;
}

QString KoTextBlockData::counterPlainText() const
{
    return d->counterPlainText;
}

void KoTextBlockData::setCounterPrefix(const QString &text)
{
    d->counterPrefix = text;
}

QString KoTextBlockData::counterPrefix() const
{
    return d->counterPrefix;
}

void KoTextBlockData::setCounterSuffix(const QString &text)
{
    d->counterSuffix = text;
}

QString KoTextBlockData::counterSuffix() const
{
    return d->counterSuffix;
}

void KoTextBlockData::setCounterIsImage(bool isImage)
{
    d->counterIsImage = isImage;
}

bool KoTextBlockData::counterIsImage() const
{
    return d->counterIsImage;
}

void KoTextBlockData::setCounterIndex(int index)
{
    d->counterIndex = index;
}

int KoTextBlockData::counterIndex() const
{
    return d->counterIndex;
}

void KoTextBlockData::setCounterPosition(const QPointF &position)
{
    d->counterPos = position;
}

QPointF KoTextBlockData::counterPosition() const
{
    return d->counterPos;
}

void KoTextBlockData::setLabelFormat(const QTextCharFormat &format)
{
    d->labelFormat = format;
}

QTextCharFormat KoTextBlockData::labelFormat() const
{
    return d->labelFormat;
}

KoTextBlockBorderData *KoTextBlockData::border() const
{
    return d->border;
}

void KoTextBlockData::setPaintStrategy(KoTextBlockPaintStrategyBase *paintStrategy)
{
    delete d->paintStrategy;
    d->paintStrategy = paintStrategy;
}

KoTextBlockPaintStrategyBase *KoTextBlockData::paintStrategy() const
{
    return d->paintStrategy;
}

bool KoTextBlockData::saveXmlID() const
{
    // as suggested by boemann, http://lists.kde.org/?l=calligra-devel&m=132396354701553&w=2
    return d->paintStrategy != 0;
}
