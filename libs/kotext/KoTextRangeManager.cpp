/* This file is part of the KDE project
 * Copyright (c) 2012 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (c) 2012 C. Boemann <cbo@boemann.dk>
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
#include "KoTextRangeManager.h"

#include "KoAnnotation.h"
#include "KoBookmark.h"

#include "TextDebug.h"

KoTextRangeManager::KoTextRangeManager(QObject *parent)
    : QObject(parent)
{
}

KoTextRangeManager::~KoTextRangeManager()
{
}

void KoTextRangeManager::insert(KoTextRange *textRange)
{
    if (!textRange) {
        return;
    }


    if (m_textRanges.contains(textRange)) {
        return;
    }

    if (m_deletedTextRanges.contains(textRange)) {
        m_deletedTextRanges.remove(textRange);
        textRange->restore();
    } else {
        textRange->setManager(this);
    }

    KoBookmark *bookmark = dynamic_cast<KoBookmark *>(textRange);
    if (bookmark) {
        m_bookmarkManager.insert(bookmark->name(), bookmark);
    }
    else {
        KoAnnotation *annotation = dynamic_cast<KoAnnotation *>(textRange);
        if (annotation) {
            m_annotationManager.insert(annotation->name(), annotation);
        }
    }
    m_textRanges.insert(textRange);
}

void KoTextRangeManager::remove(KoTextRange *textRange)
{
    if (!textRange) {
        return;
    }

    KoBookmark *bookmark = dynamic_cast<KoBookmark *>(textRange);
    if (bookmark) {
        m_bookmarkManager.remove(bookmark->name());
    }
    else {
        KoAnnotation *annotation = dynamic_cast<KoAnnotation *>(textRange);
        if (annotation) {
            m_annotationManager.remove(annotation->name());
        }
    }

    m_textRanges.remove(textRange);
    m_deletedTextRanges.insert(textRange);
    textRange->snapshot();
}

const KoBookmarkManager *KoTextRangeManager::bookmarkManager() const
{
    return &m_bookmarkManager;
}

const KoAnnotationManager *KoTextRangeManager::annotationManager() const
{
    return &m_annotationManager;
}

QList<KoTextRange *> KoTextRangeManager::textRanges() const
{
    return m_textRanges.values();
}


QHash<int, KoTextRange *> KoTextRangeManager::textRangesChangingWithin(const QTextDocument *doc, int first, int last, int matchFirst, int matchLast) const
{
    QHash<int, KoTextRange *> ranges;
    foreach (KoTextRange *range, m_textRanges) {
        if (range->document() != doc) {
            continue;
        }
        if (!range->hasRange()) {
            if (range->rangeStart() >= first && range->rangeStart() <= last) {
                ranges.insertMulti(range->rangeStart(), range);
            }
        } else {
            if (range->rangeStart() >= first && range->rangeStart() <= last) {
                if (matchLast == -1 || range->rangeEnd() <= matchLast) {
                    if (range->rangeEnd() >= matchFirst) {
                        ranges.insertMulti(range->rangeStart(), range);
                    }
                }
            }
            if (range->rangeEnd() >= first && range->rangeEnd() <= last) {
                if (matchLast == -1 || range->rangeStart() <= matchLast) {
                    if (range->rangeStart() >= matchFirst) {
                        ranges.insertMulti(range->rangeEnd(), range);
                    }
                }
            }
            if (range->rangeStart() >= first && range->rangeStart() <= last) {
                if (matchLast == -1 || range->rangeEnd() >= matchLast) {
                    if (range->rangeEnd() >= matchFirst) {
                        ranges.insert(range->rangeStart(), range);
                    }
                }
            }
        }
    }
    return ranges;
}
