/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_scalar_keyframe_channel.h"
#include "kis_node.h"
#include "kundo2command.h"
#include "kis_time_range.h"

#include <kis_global.h>
#include <kis_dom_utils.h>

struct KisScalarKeyframeChannel::Private
{
public:
    Private(qreal min, qreal max)
        : minValue(min), maxValue(max), firstFreeIndex(0)
    {}

    qreal minValue;
    qreal maxValue;
    QMap<int, qreal> values;
    int firstFreeIndex;

    struct InsertValueCommand;
    struct SetValueCommand;
};

KisScalarKeyframeChannel::KisScalarKeyframeChannel(const KoID &id, qreal minValue, qreal maxValue, KisDefaultBoundsBaseSP defaultBounds)
    : KisKeyframeChannel(id, defaultBounds),
      m_d(new Private(minValue, maxValue))
{
}

KisScalarKeyframeChannel::~KisScalarKeyframeChannel()
{}

bool KisScalarKeyframeChannel::hasScalarValue() const
{
    return true;
}

qreal KisScalarKeyframeChannel::minScalarValue() const
{
    return m_d->minValue;
}

qreal KisScalarKeyframeChannel::maxScalarValue() const
{
    return m_d->maxValue;
}

qreal KisScalarKeyframeChannel::scalarValue(const KisKeyframeSP keyframe) const
{
    return m_d->values[keyframe->value()];
}

struct KisScalarKeyframeChannel::Private::SetValueCommand : public KUndo2Command
{
    SetValueCommand(KisScalarKeyframeChannel *channel, KisKeyframeSP keyframe, qreal oldValue, qreal newValue, KUndo2Command *parentCommand)
        : KUndo2Command(parentCommand),
          m_channel(channel),
          m_keyframe(keyframe),
          m_oldValue(oldValue),
          m_newValue(newValue)
    {
    }

    void redo() {
        m_channel->setScalarValueImpl(m_keyframe, m_newValue);
    }

    void undo() {
        m_channel->setScalarValueImpl(m_keyframe, m_oldValue);
    }

private:
    KisScalarKeyframeChannel *m_channel;
    KisKeyframeSP m_keyframe;
    qreal m_oldValue;
    qreal m_newValue;
};

void KisScalarKeyframeChannel::setScalarValueImpl(KisKeyframeSP keyframe, qreal value)
{
    int index = keyframe->value();
    m_d->values[index] = value;

    QRect rect = affectedRect(keyframe);
    KisTimeRange range = affectedFrames(keyframe->time());

    requestUpdate(range, rect);
}

void KisScalarKeyframeChannel::setScalarValue(KisKeyframeSP keyframe, qreal value, KUndo2Command *parentCommand)
{
    QScopedPointer<KUndo2Command> tempCommand;
    if (!parentCommand) {
        tempCommand.reset(new KUndo2Command());
        parentCommand = tempCommand.data();
    }

    int index = keyframe->value();
    KUndo2Command *cmd = new Private::SetValueCommand(this, keyframe, m_d->values[index], value, parentCommand);
    cmd->redo();
}

qreal cubicBezier(qreal p0, qreal delta1, qreal delta2, qreal p3, qreal t) {
    qreal p1 = p0 + delta1;
    qreal p2 = p3 + delta2;

    qreal c = 1-t;
    return c*c*c * p0 + 3*c*c*t * p1 + 3*c*t*t * p2 + t*t*t * p3;
}

qreal findCubicCurveParameter(int time0, qreal delta0, qreal delta1, int time1, int time)
{
    if (time == time0) return 0.0;
    if (time == time1) return 1.0;

    qreal min_t = 0.0;
    qreal max_t = 1.0;

    while (true) {
        qreal t = (max_t + min_t) / 2;
        qreal time_t = cubicBezier(time0, delta0, delta1, time1, t);

        if (time_t < time - 0.05) {
            min_t = t;
        } else if (time_t > time + 0.05) {
            max_t = t;
        } else {
            // Close enough
            return t;
        }
    }
}

qreal KisScalarKeyframeChannel::interpolatedValue(int time) const
{
    KisKeyframeSP activeKey = activeKeyframeAt(time);
    if (activeKey.isNull()) return qQNaN();

    if (activeKey->interpolationMode() == KisKeyframe::Constant) return scalarValue(activeKey);

    KisKeyframeSP nextKey = nextKeyframe(activeKey);
    if (nextKey.isNull()) return scalarValue(activeKey);

    int time0 = activeKey->time();
    int time1 = nextKey->time();
    int interval = time1 - time0;

    qreal value0 = scalarValue(activeKey);
    qreal value1 = scalarValue(nextKey);

    QPointF tangent0 = activeKey->rightTangent();
    QPointF tangent1 = nextKey->leftTangent();

    // To ensure that the curve is monotonic wrt time,
    // check that control points lie between the endpoints.
    // If not, force them into range by scaling down the tangents

    if (tangent0.x() < 0) tangent0 = QPointF(0,0);
    if (tangent1.x() > 0) tangent1 = QPointF(0,0);

    if (tangent0.x() > interval) {
        tangent0 = tangent0 * (interval / tangent0.x());
    }
    if (tangent1.x() < -interval) {
        tangent1 = tangent1 * (interval / -tangent1.x());
    }

    qreal t = findCubicCurveParameter(time0, tangent0.x(), tangent1.x(), time1, time);
    qreal res = cubicBezier(value0, tangent0.y(), tangent1.y(), value1, t);

    if (res > m_d->maxValue) return m_d->maxValue;
    if (res < m_d->minValue) return m_d->minValue;

    return res;
}

struct KisScalarKeyframeChannel::Private::InsertValueCommand : public KUndo2Command
{
    InsertValueCommand(KisScalarKeyframeChannel::Private *d, int index, qreal value, bool insert, KUndo2Command *parentCommand)
        : KUndo2Command(parentCommand),
          m_d(d),
          m_index(index),
          m_value(value),
          m_insert(insert)
    {
    }

    void redo() {
        doSwap(m_insert);
    }

    void undo() {
        doSwap(!m_insert);
    }

private:
    void doSwap(bool insert) {
        if (insert) {
            m_d->values[m_index] = m_value;
        } else {
            m_d->values.remove(m_index);
        }
    }

private:
    KisScalarKeyframeChannel::Private *m_d;
    int m_index;
    qreal m_value;
    bool m_insert;
};

KisKeyframeSP KisScalarKeyframeChannel::createKeyframe(int time, const KisKeyframeSP copySrc, KUndo2Command *parentCommand)
{
    qreal value = (copySrc.isNull() ? 0 : scalarValue(copySrc));
    int index = m_d->firstFreeIndex++;

    KUndo2Command *cmd = new Private::InsertValueCommand(m_d.data(), index, value, true, parentCommand);
    cmd->redo();

    return toQShared(new KisKeyframe(this, time, index));
}

void KisScalarKeyframeChannel::destroyKeyframe(KisKeyframeSP key, KUndo2Command *parentCommand)
{
    int index = key->value();

    KIS_ASSERT_RECOVER_RETURN(m_d->values.contains(index));

    KUndo2Command *cmd = new Private::InsertValueCommand(m_d.data(), index, m_d->values[index], false, parentCommand);
    cmd->redo();
}

void KisScalarKeyframeChannel::uploadExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, KisKeyframeSP dstFrame)
{
    KisScalarKeyframeChannel *srcScalarChannel = dynamic_cast<KisScalarKeyframeChannel*>(srcChannel);
    KIS_ASSERT_RECOVER_RETURN(srcScalarChannel);

    KisKeyframeSP srcFrame = srcScalarChannel->keyframeAt(srcTime);
    KIS_ASSERT_RECOVER_RETURN(srcFrame);

    const qreal newValue = scalarValue(srcFrame);

    const int dstId = dstFrame->value();
    KIS_ASSERT_RECOVER_RETURN(m_d->values.contains(dstId));
    m_d->values[dstId] = newValue;
}

QRect KisScalarKeyframeChannel::affectedRect(KisKeyframeSP key)
{
    Q_UNUSED(key);

    if (node()) {
        return node()->extent();
    } else {
        return QRect();
    }
}

void KisScalarKeyframeChannel::saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename)
{
    Q_UNUSED(layerFilename);
    keyframeElement.setAttribute("value", KisDomUtils::toString(m_d->values[keyframe->value()]));
}

KisKeyframeSP KisScalarKeyframeChannel::loadKeyframe(const QDomElement &keyframeNode)
{
    int time = keyframeNode.toElement().attribute("time").toUInt();
    qreal value = KisDomUtils::toDouble(keyframeNode.toElement().attribute("value"));

    KUndo2Command tempParentCommand;
    KisKeyframeSP keyframe = createKeyframe(time, KisKeyframeSP(), &tempParentCommand);
    setScalarValue(keyframe, value);

    return keyframe;
}
