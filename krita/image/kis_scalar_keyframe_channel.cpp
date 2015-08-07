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

KisScalarKeyframeChannel::KisScalarKeyframeChannel(const KoID &id, KisNodeWSP node, qreal minValue, qreal maxValue)
    : KisKeyframeChannel(id, node),
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

qreal KisScalarKeyframeChannel::scalarValue(const KisKeyframe *keyframe) const
{
    return m_d->values[keyframe->value()];
}

struct KisScalarKeyframeChannel::Private::SetValueCommand : public KUndo2Command
{
    SetValueCommand(KisScalarKeyframeChannel::Private *d, int index, qreal oldValue, qreal newValue, KUndo2Command *parentCommand)
        : KUndo2Command(parentCommand),
          m_d(d),
          m_index(index),
          m_oldValue(oldValue),
          m_newValue(newValue)
    {
    }

    void redo() {
        m_d->values[m_index] = m_newValue;
    }

    void undo() {
        m_d->values[m_index] = m_oldValue;
    }

private:
    KisScalarKeyframeChannel::Private *m_d;
    int m_index;
    qreal m_oldValue;
    qreal m_newValue;
};

void KisScalarKeyframeChannel::setScalarValue(KisKeyframe *keyframe, qreal value, KUndo2Command *parentCommand)
{
    QScopedPointer<KUndo2Command> tempCommand;
    if (!parentCommand) {
        tempCommand.reset(new KUndo2Command());
        parentCommand = tempCommand.data();
    }

    int index = keyframe->value();
    KUndo2Command *cmd = new Private::SetValueCommand(m_d.data(), index, m_d->values[index], value, parentCommand);
    cmd->redo();
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

KisKeyframe *KisScalarKeyframeChannel::createKeyframe(int time, const KisKeyframe *copySrc, KUndo2Command *parentCommand)
{
    qreal value = (copySrc != 0) ? scalarValue(copySrc) : 0;
    int index = m_d->firstFreeIndex++;

    KUndo2Command *cmd = new Private::InsertValueCommand(m_d.data(), index, value, true, parentCommand);
    cmd->redo();

    return new KisKeyframe(this, time, index);
}

bool KisScalarKeyframeChannel::canDeleteKeyframe(KisKeyframe *key)
{
    Q_UNUSED(key);
    return true;
}

void KisScalarKeyframeChannel::destroyKeyframe(KisKeyframe *key, KUndo2Command *parentCommand)
{
    int index = key->value();

    KIS_ASSERT_RECOVER_RETURN(m_d->values.contains(index));

    KUndo2Command *cmd = new Private::InsertValueCommand(m_d.data(), index, m_d->values[index], false, parentCommand);
    cmd->redo();
}

QRect KisScalarKeyframeChannel::affectedRect(KisKeyframe *key)
{
    Q_UNUSED(key);
    return QRect();
}

void KisScalarKeyframeChannel::saveKeyframe(KisKeyframe *keyframe, QDomElement keyframeElement, const QString &layerFilename)
{
    Q_UNUSED(layerFilename);
    keyframeElement.setAttribute("value", m_d->values[keyframe->value()]);
}

KisKeyframeSP KisScalarKeyframeChannel::loadKeyframe(const QDomElement &keyframeNode)
{
    int time = keyframeNode.toElement().attribute("time").toUInt();
    QVariant value = keyframeNode.toElement().attribute("value");

    KUndo2Command tempParentCommand;
    KisKeyframe *keyframe = createKeyframe(time, 0, &tempParentCommand);
    setScalarValue(keyframe, value.toReal());

    return toQShared(keyframe);
}
