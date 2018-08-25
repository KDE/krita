/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_time_range.h"

#include <QDebug>
#include "kis_keyframe_channel.h"
#include "kis_node.h"
#include "kis_layer_utils.h"

struct KisTimeRangeStaticRegistrar {
    KisTimeRangeStaticRegistrar() {
        qRegisterMetaType<KisTimeSpan>("KisTimeRange");
        qRegisterMetaType<KisTimeSpan>("KisTimeSpan");
        qRegisterMetaType<KisFrameSet>("KisFrameSet");
    }
};

static KisTimeRangeStaticRegistrar __registrar;

QDebug operator<<(QDebug dbg, const KisTimeRange &r)
{
    dbg.nospace() << "KisTimeRange(" << r.start() << ", " << r.end() << ")";

    return dbg.space();
}

QDebug operator<<(QDebug dbg, const KisTimeSpan &r)
{
    dbg.nospace() << "KisTimeSpan(" << r.start() << ", " << r.end() << ")";

    return dbg.space();
}

QDebug operator<<(QDebug dbg, const KisFrameSet &r)
{
    const QVector<KisTimeSpan> &spans = r.finiteSpans();

    dbg.nospace() << "KisFrameSet(";
    for (int i = 0; i < spans.size(); i++) {
        if (i > 0) dbg.nospace() << ", ";
        dbg.nospace() << spans[i].start() << ".." << spans[i].end();
    }
    if (r.isInfinite()) dbg.nospace() << ", " << r.firstFrameOfInfinity() << "...";
    dbg.nospace() << ")";

    return dbg.space();
}

KisFrameSet& KisFrameSet::operator|=(const KisFrameSet &rhs)
{
    if (rhs.isEmpty()) return *this;
    if (isEmpty()) {
        *this = rhs;
        return *this;
    }

    QVector<KisTimeSpan> spans;
    int lIndex = 0, rIndex = 0;

    KisTimeSpan currentSpan;

    int firstOfInfinite =
            !isInfinite() ? rhs.m_firstFrameOfInfinity :
            (!rhs.isInfinite()) ? m_firstFrameOfInfinity :
            qMin(m_firstFrameOfInfinity, rhs.m_firstFrameOfInfinity);

    while (lIndex < m_spans.size() || rIndex < rhs.m_spans.size()) {
        const bool leftRemaining = (lIndex < m_spans.size());
        const bool rightRemaining = (rIndex < rhs.m_spans.size());
        const bool leftFirst = leftRemaining && (!rightRemaining || m_spans[lIndex].start() < rhs.m_spans[rIndex].start());

        KisTimeSpan first;
        if (leftFirst) {
            first = m_spans[lIndex++];
        } else {
            first = rhs.m_spans[rIndex++];
        }

        if (isInfinite() && firstOfInfinite <= first.end()) {
            currentSpan = KisTimeSpan();
            firstOfInfinite = qMin(first.start(), firstOfInfinite);
            break;
        } else if (first.start() <= currentSpan.end() || currentSpan.isEmpty()) {
            currentSpan = currentSpan | first;
        } else {
            spans.append(currentSpan);
            currentSpan = first;
        }
    }

    if (!currentSpan.isEmpty()) {
        spans.append(currentSpan);
    }

    m_spans = spans;
    m_firstFrameOfInfinity = firstOfInfinite;
    return *this;
}


void addIntersectionAgainstInfinity(const QVector<KisTimeSpan> &src, int firstIndex , QVector<KisTimeSpan> &dst, int firstFrameOfInfinity)
{
    for (int index = firstIndex; index < src.size(); index++) {
        const KisTimeSpan span = src[index].truncateRight(firstFrameOfInfinity);
        if (!span.isEmpty()) dst.append(span);
    }
}

KisFrameSet& KisFrameSet::operator&=(const KisFrameSet &rhs)
{
    if (isEmpty() || rhs.isEmpty()) {
        *this = KisFrameSet();
        return *this;
    }

    QVector<KisTimeSpan> spans;

    int lIndex = 0, rIndex = 0;
    while (lIndex < m_spans.size() && rIndex < rhs.m_spans.size()) {
        KisTimeSpan span;

        const KisTimeSpan rSpan = rhs.m_spans[rIndex];
        const KisTimeSpan lSpan = m_spans[lIndex];

        span = lSpan & rSpan;

        if (!span.isEmpty()) {
            spans.append(span);
        }

        if (lSpan.start() < rSpan.start()) {
            lIndex++;
        } else {
            rIndex++;
        }
    }

    if (isInfinite()) addIntersectionAgainstInfinity(rhs.m_spans, rIndex, spans, m_firstFrameOfInfinity);
    if (rhs.isInfinite()) addIntersectionAgainstInfinity(m_spans, lIndex, spans, rhs.m_firstFrameOfInfinity);

    int firstOfInfinite = (isInfinite() && rhs.isInfinite()) ? qMax(m_firstFrameOfInfinity, rhs.m_firstFrameOfInfinity) : -1;

    m_spans = spans;
    m_firstFrameOfInfinity = firstOfInfinite;
    return *this;
}

KisFrameSet& KisFrameSet::operator-=(const KisFrameSet &rhs)
{
    if (rhs.isEmpty()) return *this;
    if (isEmpty()) {
         *this = KisFrameSet();
        return *this;
    }

    QVector<KisTimeSpan> spans;

    int firstOfInfinite = (isInfinite() && !rhs.isInfinite()) ?
                          qMax(m_firstFrameOfInfinity, rhs.m_spans.last().end() + 1) : -1;

    KisTimeSpan currentSpan = m_spans.first();

    int lIndex = 0, rIndex = 0;
    while (lIndex < m_spans.size() && rIndex < rhs.m_spans.size()) {
        const KisTimeSpan rSpan = rhs.m_spans[rIndex];

        if (currentSpan.isEmpty() || currentSpan.end() < rSpan.start()) {
            if (!currentSpan.isEmpty()) {
                spans.append(currentSpan);
            }
            lIndex++;
            currentSpan = (lIndex < m_spans.size()) ? m_spans[lIndex] : KisTimeSpan();
        } else {
            const KisTimeSpan tail = currentSpan.truncateRight(rSpan.end() + 1);
            const KisTimeSpan head = currentSpan.truncateLeft(rSpan.start() - 1);

            if (!head.isEmpty()) {
                spans.append(head);
            }
            currentSpan = tail;
            rIndex++;
        }
    }

    while (!currentSpan.isEmpty()) {
        if (rhs.isInfinite() && currentSpan.end() >= rhs.firstFrameOfInfinity()) {
            currentSpan = currentSpan.truncateLeft(rhs.firstFrameOfInfinity() - 1);
            if (!currentSpan.isEmpty()) spans.append(currentSpan);
            break;
        }

        spans.append(currentSpan);
        lIndex++;
        currentSpan = (lIndex < m_spans.size()) ? m_spans[lIndex] : KisTimeSpan();
    }

    m_spans = spans;
    m_firstFrameOfInfinity = firstOfInfinite;
    return *this;
}

KisTimeRange KisTimeRange::calculateIdenticalFramesRecursive(const KisNode *node, int time)
{
    KisTimeRange range = KisTimeRange::infinite(0);

    KisLayerUtils::recursiveApplyNodes(node,
        [&range, time] (const KisNode *node) {
            if (node->visible()) {
                range &= calculateNodeIdenticalFrames(node, time);
            }
    });

    return range;
}

KisTimeRange KisTimeRange::calculateAffectedFramesRecursive(const KisNode *node, int time)
{
    KisTimeRange range;

    KisLayerUtils::recursiveApplyNodes(node,
        [&range, time] (const KisNode *node) {
            if (node->visible()) {
                range |= calculateNodeIdenticalFrames(node, time);
            }
    });

    return range;
}

int KisFrameSet::firstExcludedSince(int time) const
{
    if (isEmpty()) return time;
    if (0 <= m_firstFrameOfInfinity && m_firstFrameOfInfinity <= time) return -1;
    if (time < start()) return time;
    if (time > m_spans.last().end()) return time;

    Q_FOREACH(const KisTimeSpan &span, m_spans) {
            if (span.start() > time) return time;
            if (span.end() >= time) return span.end() + 1;
        }

    KIS_SAFE_ASSERT_RECOVER_NOOP(false);
    return -1;
}

KisTimeRange KisTimeRange::calculateNodeIdenticalFrames(const KisNode *node, int time)
{
    KisTimeRange range = KisTimeRange::infinite(0);

    const QMap<QString, KisKeyframeChannel*> channels =
        node->keyframeChannels();

    Q_FOREACH (const KisKeyframeChannel *channel, channels) {
        // Intersection
        range &= channel->identicalFrames(time);
    }

    return range;
}

KisTimeRange KisTimeRange::calculateNodeAffectedFrames(const KisNode *node, int time)
{
    KisTimeRange range;

    if (!node->visible()) return range;

    const QMap<QString, KisKeyframeChannel*> channels =
        node->keyframeChannels();

    // TODO: channels should report to the image which channel exactly has changed
    //       to avoid the dirty range to be stretched into infinity!

    if (channels.isEmpty() ||
        !channels.contains(KisKeyframeChannel::Content.id())) {

        range = KisTimeRange::infinite(0);
        return range;
    }

    Q_FOREACH (const KisKeyframeChannel *channel, channels) {
        // Union
        range |= channel->affectedFrames(time);
    }

    return range;
}

namespace KisDomUtils {

void saveValue(QDomElement *parent, const QString &tag, const KisTimeRange &range)
{
    QDomDocument doc = parent->ownerDocument();
    QDomElement e = doc.createElement(tag);
    parent->appendChild(e);

    e.setAttribute("type", "timerange");

    if (range.isValid()) {
        e.setAttribute("from", toString(range.start()));

        if (!range.isInfinite()) {
            e.setAttribute("to", toString(range.end()));
        }
    }
}


bool loadValue(const QDomElement &parent, const QString &tag, KisTimeRange *range)
{
    QDomElement e;
    if (!findOnlyElement(parent, tag, &e)) return false;

    if (!Private::checkType(e, "timerange")) return false;

    int start = toInt(e.attribute("from", "-1"));
    int end = toInt(e.attribute("to", "-1"));

    if (start == -1) {
        range = new KisTimeRange();
    } else if (end == -1) {
        *range = KisTimeRange::infinite(start);
    } else {
        *range = KisTimeRange::fromTime(start, end);
    }
    return true;
}

    void saveValue(QDomElement *parent, const QString &tag, const KisTimeSpan &range)
    {
        QDomDocument doc = parent->ownerDocument();
        QDomElement e = doc.createElement(tag);
        parent->appendChild(e);

        e.setAttribute("type", "timerange");

        if (!range.isEmpty()) {
            e.setAttribute("from", toString(range.start()));
            e.setAttribute("to", toString(range.end()));
        }
    }

    bool loadValue(const QDomElement &parent, const QString &tag, KisTimeSpan *range)
    {
        QDomElement e;
        if (!findOnlyElement(parent, tag, &e)) return false;

        if (!Private::checkType(e, "timerange")) return false;

        int start = toInt(e.attribute("from", "-1"));
        int end = toInt(e.attribute("to", "-1"));

        if (start < 0 || end < 0) {
            *range = KisTimeSpan();
        } else {
            *range = KisTimeSpan(start, end);
        }
        return true;
    }

}
