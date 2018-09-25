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
        qRegisterMetaType<KisTimeRange>("KisTimeRange");
    }
};

static KisTimeRangeStaticRegistrar __registrar;

QDebug operator<<(QDebug dbg, const KisTimeRange &r)
{
    dbg.nospace() << "KisTimeRange(" << r.start() << ", " << r.end() << ")";

    return dbg.space();
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

}
