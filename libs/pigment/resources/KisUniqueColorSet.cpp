/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisUniqueColorSet.h"

#include <QHash>
#include <deque>
#include <algorithm>

uint qHash(const KoColor &color, uint seed = 0)
{
    // hash the color data bytes, while using the hash of the colorspace pointer as seed
    // TODO: take pixelSize directly from the color.m_size (private member)
    return qHashBits(color.data(), color.colorSpace()->pixelSize(), qHash(color.colorSpace(), seed));
}

struct KisUniqueColorSet::ColorEntry
{
    static bool less(const KisUniqueColorSet::ColorEntry *lhs, const KisUniqueColorSet::ColorEntry *rhs)
    {
        // larger key == more recent == earlier in list
        return (lhs->key > rhs->key);
    }

    KoColor color;
    quint64 key;
};

struct KisUniqueColorSet::Private
{
    QHash<KoColor, KisUniqueColorSet::ColorEntry*> colorHash;
    std::deque<ColorEntry*> history;
    size_t maxSize {200};
    quint64 key {0};
};

KisUniqueColorSet::KisUniqueColorSet(QObject *parent)
    : QObject(parent)
    , d(new Private)
{ }

KisUniqueColorSet::~KisUniqueColorSet()
{
    for (ColorEntry *entry: d->history) {
        delete entry;
    }
}

void KisUniqueColorSet::addColor(const KoColor &color)
{
    auto hashEntry = d->colorHash.find(color);
    if (hashEntry != d->colorHash.end()) {
        auto historyEl = std::lower_bound(d->history.begin(), d->history.end(), *hashEntry, &ColorEntry::less);
        if (historyEl != d->history.end()) {
            int oldPos = historyEl - d->history.begin();
            if (historyEl == d->history.begin()) {
                KIS_ASSERT((*historyEl)->key == d->key);
                return;
            }
            ColorEntry *node = *historyEl;
            d->history.erase(historyEl);
            node->key = ++d->key;
            d->history.push_front(node);
            Q_EMIT sigColorMoved(oldPos, 0);
        }
        else {
            qDebug() << "inconsistent color history state!";
        }
    }
    else {
        ColorEntry *entry;
        if (d->history.size() >= d->maxSize) {
            entry = d->history.back();
            d->history.pop_back();
            KIS_ASSERT(d->colorHash.remove(entry->color) == 1);
            entry->color = color;
            entry->key = ++d->key;
            Q_EMIT sigColorRemoved(d->maxSize - 1);
        }
        else {
            entry = new ColorEntry {color, ++d->key};
        }
        d->colorHash.insert(color, entry);
        d->history.push_front(entry);
        Q_EMIT sigColorAdded(0);
    }
}

KoColor KisUniqueColorSet::color(int index) const
{
    if (index < 0 || index >= static_cast<int>(d->history.size())) {
        return KoColor();
    }
    return d->history.at(index)->color;
}

int KisUniqueColorSet::size() const
{
    return static_cast<int>(d->history.size());
}

void KisUniqueColorSet::clear()
{
    for (ColorEntry *entry: d->history) {
        delete entry;
    }
    d->history.clear();
    d->colorHash.clear();
    d->key = 0;
    Q_EMIT sigReset();
}
