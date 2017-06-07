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

struct KisTimeRangeStaticRegistrar {
    KisTimeRangeStaticRegistrar() {
        qRegisterMetaType<KisTimeRange>("KisTimeRange");
    }
};

static KisTimeRangeStaticRegistrar __registrar;

QDebug operator<<(QDebug dbg, const KisTimeRange &r)
{
    dbg.nospace() << "KisTimeRange(" << r.start() << ", " << r.end() << ")";

    return dbg.space();
}

void KisTimeRange::calculateTimeRangeRecursive(const KisNode *node, int time, KisTimeRange &range, bool exclusive)
{
    if (!node->visible()) return;
    Q_FOREACH (const KisKeyframeChannel *channel, node->keyframeChannels()) {
        if (exclusive) {
            // Intersection
            range &= channel->identicalFrames(time);
        } else {
            // Union
            range |= channel->affectedFrames(time);
        }
    }

    KisNodeSP child = node->firstChild();
    while (child) {
        calculateTimeRangeRecursive(child, time, range, exclusive);
        child = child->nextSibling();
    }
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

}
