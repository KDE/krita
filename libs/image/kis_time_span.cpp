/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_time_span.h"

#include <QDebug>
#include "kis_keyframe_channel.h"
#include "kis_node.h"
#include "kis_layer_utils.h"

struct KisTimeSpanStaticRegistrar {
    KisTimeSpanStaticRegistrar() {
        qRegisterMetaType<KisTimeSpan>("KisTimeSpan");
    }
};

static KisTimeSpanStaticRegistrar __registrar;

QDebug operator<<(QDebug dbg, const KisTimeSpan &r)
{
    dbg.nospace() << "KisTimeSpan(" << r.start() << ", " << r.end() << ")";

    return dbg.space();
}

KisTimeSpan KisTimeSpan::calculateIdenticalFramesRecursive(const KisNode *node, int time)
{
    KisTimeSpan range = KisTimeSpan::infinite(0);

    KisLayerUtils::recursiveApplyNodes(node,
        [&range, time] (const KisNode *node) {
            if (node->visible()) {
                range &= calculateNodeIdenticalFrames(node, time);
            }
    });

    return range;
}

KisTimeSpan KisTimeSpan::calculateAffectedFramesRecursive(const KisNode *node, int time)
{
    KisTimeSpan range;

    KisLayerUtils::recursiveApplyNodes(node,
        [&range, time] (const KisNode *node) {
            if (node->visible()) {
                range |= calculateNodeIdenticalFrames(node, time);
            }
    });

    return range;
}

KisTimeSpan KisTimeSpan::calculateNodeIdenticalFrames(const KisNode *node, int time)
{
    KisTimeSpan range = KisTimeSpan::infinite(0);

    const QMap<QString, KisKeyframeChannel*> channels =
        node->keyframeChannels();

    Q_FOREACH (const KisKeyframeChannel *channel, channels) {
        // Intersection
        range &= channel->identicalFrames(time);
    }

    return range;
}

KisTimeSpan KisTimeSpan::calculateNodeAffectedFrames(const KisNode *node, int time)
{
    KisTimeSpan range;

    if (!node->visible()) return range;

    const QMap<QString, KisKeyframeChannel*> channels =
        node->keyframeChannels();

    // TODO: channels should report to the image which channel exactly has changed
    //       to avoid the dirty range to be stretched into infinity!

    if (channels.isEmpty() ||
        !channels.contains(KisKeyframeChannel::Raster.id())) {
        range = KisTimeSpan::infinite(0);
        return range;
    }

    Q_FOREACH (const KisKeyframeChannel *channel, channels) {
        // Union
        range |= channel->affectedFrames(time);
    }

    return range;
}

namespace KisDomUtils {

void saveValue(QDomElement *parent, const QString &tag, const KisTimeSpan &range)
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


bool loadValue(const QDomElement &parent, const QString &tag, KisTimeSpan *range)
{
    QDomElement e;
    if (!findOnlyElement(parent, tag, &e)) return false;

    if (!Private::checkType(e, "timerange")) return false;

    int start = toInt(e.attribute("from", "-1"));
    int end = toInt(e.attribute("to", "-1"));

    if (start == -1) {
        *range = KisTimeSpan();
    } else if (end == -1) {
        *range = KisTimeSpan::infinite(start);
    } else {
        *range = KisTimeSpan::fromTimeToTime(start, end);
    }
    return true;
}

}
