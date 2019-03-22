
#include "kis_keyframe_commands.h"

#include <kis_pointer_utils.h>
#include <KisCollectionUtils.h>
#include "kis_time_range.h"
#include "kis_animation_cycle.h"

using CycleSP = QSharedPointer<KisAnimationCycle>;
using KeyframeMove = KisKeyframeCommands::KeyframeMove;

KisKeyframeCommands::KeyframeMove::KeyframeMove(KisKeyframeBaseSP keyframe, int newTime)
    : keyframe(keyframe)
    , oldTime(keyframe->time())
    , newTime(newTime)
{}

class KisMoveKeyframesCommand : public KUndo2Command
{
public:
    KisMoveKeyframesCommand(QVector<KeyframeMove> moves, KUndo2Command *parentCommand)
        : KUndo2Command(parentCommand)
        , m_moves(moves)
    {}

    void redo() override
    {
        Q_FOREACH(const KeyframeMove &move, m_moves) {
            move.keyframe->channel()->moveKeyframeImpl(move.keyframe, move.newTime);
        }
    }

    void undo() override
    {
        Q_FOREACH(const KeyframeMove &move, m_moves) {
            move.keyframe->channel()->moveKeyframeImpl(move.keyframe, move.oldTime);
        }
    }

private:
    QVector<KisKeyframeCommands::KeyframeMove> m_moves;
};

struct KeyframeMapping
{
    KisKeyframeChannel *channel;
    QMap<KisKeyframeBaseSP, int> destinations;
    QMap<int, KisKeyframeBaseSP> sources;

    KeyframeMapping(KisKeyframeChannel *channel, const QVector<KeyframeMove> &moves)
        : channel(channel)
    {
        const int count = moves.count();

        for (int i = 0; i < count; i++) {
            const int time = moves[i].newTime;
            const KisKeyframeBaseSP key = moves[i].keyframe;

            if (key->channel() == channel) {
                destinations[key] = time;
                sources[time] = key;
            }
        }
    }

    bool isEmpty() const
    {
        return destinations.isEmpty();
    }

    int destination(const KisKeyframeSP &keyframe) const
    {
        const int oldTime = keyframe->time();
        const int newTime = destinations.value(keyframe, -1);

        if (newTime == -1 && !sources.value(oldTime, KisKeyframeBaseSP())) {
            return oldTime;
        }

        return newTime;
    }

    KisKeyframeSP firstTrueKeyframeAfter(int destinationTime) const
    {
        KisKeyframeSP unmovedKeyframe;
        {
            int time = destinationTime;
            KisKeyframeSP key;
            do {
                key = channel->nextKeyframe(time);
                time = key ? key->time() : -1;
            } while(destinations.contains(key));
            unmovedKeyframe = key;
        }

        KisKeyframeSP movedKeyframe;
        int movedTime;
        {
            auto sourceIt = KisCollectionUtils::firstAfter(sources, destinationTime);
            // Skip non-keyframes (e.g. repeat frames)
            while (sourceIt != sources.cend() && !sourceIt.value().dynamicCast<KisKeyframe>()) {
                sourceIt++;
            }
            movedTime = (sourceIt != sources.cend()) ? sourceIt.key() : -1;
        }

        if (movedKeyframe && unmovedKeyframe) {
            return (movedTime < unmovedKeyframe->time()) ? movedKeyframe : unmovedKeyframe;
        } else {
            return movedKeyframe ? movedKeyframe : unmovedKeyframe;
        }
    }
};

bool areValidMoveSources(const KisKeyframeChannel *channel, QVector<KeyframeMove> moves)
{
    Q_FOREACH(const KeyframeMove &move, moves) {
        if (move.keyframe->channel() != channel) return false;
    }

    std::sort(moves.begin(), moves.end(),
        [](const KeyframeMove &lhs, const KeyframeMove &rhs){ return lhs.oldTime < rhs.oldTime; }
    );

    for (int i = 1; i < moves.size(); i++) {
        if (moves[i - 1].keyframe == moves[i].keyframe) return false;
    }

    return true;
}

CycleSP cycleAfterMove(const CycleSP &cycle, const KeyframeMapping &movedKeyframes)
{
    const KisKeyframeChannel *channel = cycle->channel();

    int firstDestination = -1, lastDestination = -1;
    KisKeyframeSP oldLastKeyframe;
    KisKeyframeSP newFirstKeyframe, newLastKeyframe;

    const KisTimeSpan &oldRange = cycle->originalRange();
    for (KisKeyframeBaseSP key : channel->itemsWithin(oldRange)) {
        KisKeyframeSP keyframe = key.dynamicCast<KisKeyframe>();
        KIS_SAFE_ASSERT_RECOVER(keyframe) { continue; }

        const int newTime = movedKeyframes.destination(keyframe);

        if (newTime >= 0) {
            if (!newFirstKeyframe || newTime < firstDestination) {
                firstDestination = newTime;
                newFirstKeyframe = keyframe;
            }

            if (!newLastKeyframe || newTime > lastDestination) {
                lastDestination = newTime;
                newLastKeyframe = keyframe;
            }
        }

        oldLastKeyframe = keyframe;
    }

    if (!newLastKeyframe) return CycleSP();

    const int newEnd = lastDestination; // FIXME

    const KisTimeSpan &newRange = KisTimeSpan(firstDestination, newEnd);
    if (newRange == oldRange) {
        return cycle;
    }

    return toQShared(new KisAnimationCycle(*cycle, newRange));
}

QVector<CycleSP> resolveCycleOverlaps(QVector<CycleSP> &cycles, const KeyframeMapping &movedKeyframes)
{
    if (cycles.isEmpty()) return cycles;

    std::sort(cycles.begin(), cycles.end(), [](const CycleSP &lhs, const CycleSP &rhs) {
        return lhs->originalRange().start() < rhs->originalRange().start();
    });

    CycleSP lhs = cycles[0];
    for (int i = 1; i < cycles.size(); i++) {
        const CycleSP &rhs = cycles[i];

        const int lhsEnd = lhs->originalRange().end();
        const int rhsStart = rhs->originalRange().start();

        if (rhsStart < lhsEnd) {
            const int rhsEnd = rhs->originalRange().end();

            if (rhsEnd < lhsEnd) {
                // Rhs cycle is entirely inside lhs one: drop it
                cycles[i] = CycleSP();
                continue;
            } else {
                // TODO: logic for picking the cycle to truncate?

                const int truncatedStart = movedKeyframes.firstTrueKeyframeAfter(lhsEnd)->time();
                cycles[i] = toQShared(new KisAnimationCycle(*rhs, {truncatedStart, rhsEnd}));
            }
        }

        lhs = cycles[i];
    }
    return cycles;
}

QVector<CycleSP> cyclesAfterMoves(const KeyframeMapping &movedKeyframes)
{
    const KisKeyframeChannel *channel = movedKeyframes.destinations.begin().key()->channel();

    QVector<CycleSP> cycles;
    Q_FOREACH(const CycleSP cycle, channel->cycles()) {
        const CycleSP newCycle = cycleAfterMove(cycle, movedKeyframes);
        if (newCycle) {
            cycles << newCycle;
        }
    }

    return resolveCycleOverlaps(cycles, movedKeyframes);
}

bool validateRepeats(const QVector<CycleSP > &cycles, const KeyframeMapping &movedKeyframes)
{
    Q_FOREACH(const CycleSP &cycle, cycles) {
        if (!cycle) continue;

        const int cycleStart = cycle->originalRange().start();
        const int cycleEnd = cycle->originalRange().end();

        // If any repeat frame would land within the cycle, refuse the operation.

        auto keyIt = KisCollectionUtils::firstAfter(movedKeyframes.sources, cycleStart);
        while (keyIt != movedKeyframes.sources.cend() && keyIt.key() < cycleEnd) {
            const QSharedPointer<KisRepeatFrame> repeat = keyIt.value().dynamicCast<KisRepeatFrame>();
            if (repeat) return false;

            keyIt++;
        }
    }
    return true;
}

void updateCycles(const KisKeyframeChannel *channel, QVector<CycleSP> cyclesAfter, KUndo2Command *parentCommand)
{
    const QList<CycleSP> &cyclesBefore = channel->cycles();

    // Remove out-of-date definitions
    Q_FOREACH(const CycleSP &cycle, cyclesBefore) {
        if (!cyclesAfter.contains(cycle)) {
            new KisDefineCycleCommand(cycle, nullptr, parentCommand);
        }
    }

    // Add new cycle definitions
    Q_FOREACH(const CycleSP &cycle, cyclesAfter) {
        if (!cyclesBefore.contains(cycle)) {
            new KisDefineCycleCommand(nullptr, cycle, parentCommand);
        }
    }
}

void deleteOverwrittenKeys(KeyframeMapping moves, KUndo2Command *parentCommand)
{
    QVector<KisKeyframeBaseSP> deletedKeys;

    for (auto it = moves.destinations.cbegin(); it != moves.destinations.cend(); ++it) {
        const KisKeyframeBaseSP &keyframe = it.key();
        const int newTime = it.value();

        const KisKeyframeBaseSP &originalKey = keyframe->channel()->itemAt(newTime);

        if (originalKey) {
            const bool isOverwritten = !moves.destinations.contains(originalKey);
            if (isOverwritten) {
                deletedKeys.append(originalKey);
            }
        }
    }

    Q_FOREACH(KisKeyframeBaseSP keyframe, deletedKeys) {
        new KisReplaceKeyframeCommand(keyframe->channel(), keyframe->time(), KisKeyframeBaseSP(), parentCommand);
    }
}

KUndo2CommandSP KisKeyframeCommands::tryMoveKeyframes(KisKeyframeChannel *channel, QVector<KeyframeMove> moves, KUndo2Command *parentCommand)
{
    KUndo2Command *command = new KUndo2Command(parentCommand);

    if (!areValidMoveSources(channel, moves)) return nullptr;

    const KeyframeMapping movedKeyframes(channel, moves);
    if (movedKeyframes.isEmpty()) return nullptr;

    const QVector<CycleSP> cycles = cyclesAfterMoves(movedKeyframes);

    if (!validateRepeats(cycles, movedKeyframes)) return nullptr;

    deleteOverwrittenKeys(movedKeyframes, command);
    new KisMoveKeyframesCommand(moves, command);
    updateCycles(channel, cycles, command);

    return toQShared(command);
}

KisReplaceKeyframeCommand::KisReplaceKeyframeCommand(KisKeyframeChannel *channel, int time, KisKeyframeBaseSP keyframe, KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand),
      m_channel(channel),
      m_time(time),
      m_keyframe(keyframe),
      m_existingKeyframe(0)
{
}

void KisReplaceKeyframeCommand::redo() {
    m_existingKeyframe = m_channel->replaceKeyframeAt(m_time, m_keyframe);
}

void KisReplaceKeyframeCommand::undo() {
    m_channel->replaceKeyframeAt(m_time, m_existingKeyframe);
}

KisMoveFrameCommand::KisMoveFrameCommand(KisKeyframeChannel *channel, KisKeyframeBaseSP keyframe, int oldTime, int newTime, KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand),
      m_channel(channel),
      m_keyframe(keyframe),
      m_oldTime(oldTime),
      m_newTime(newTime)
{
}

void KisMoveFrameCommand::redo() {
    m_channel->moveKeyframeImpl(m_keyframe, m_newTime);
}

void KisMoveFrameCommand::undo() {
    m_channel->moveKeyframeImpl(m_keyframe, m_oldTime);
}

KisSwapFramesCommand::KisSwapFramesCommand(KisKeyframeChannel *channel, KisKeyframeBaseSP lhsFrame, KisKeyframeBaseSP rhsFrame, KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand),
      m_channel(channel),
      m_lhsFrame(lhsFrame),
      m_rhsFrame(rhsFrame)
{
}

void KisSwapFramesCommand::redo()
{
    m_channel->swapKeyframesImpl(m_lhsFrame, m_rhsFrame);
}

void KisSwapFramesCommand::undo()
{
    m_channel->swapKeyframesImpl(m_lhsFrame, m_rhsFrame);
}

KisDefineCycleCommand::KisDefineCycleCommand(CycleSP oldCycle, CycleSP newCycle, KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand)
    , m_channel(oldCycle ? oldCycle->channel() : newCycle->channel())
    , m_oldCycle(oldCycle)
    , m_newCycle(newCycle)
{}

void KisDefineCycleCommand::redo()
{
    if (m_oldCycle) {
        m_channel->removeCycle(m_oldCycle);
    }

    if (m_newCycle) {
        m_channel->addCycle(m_newCycle);
    }
}

void KisDefineCycleCommand::undo()
{
    if (m_newCycle) {
        m_channel->removeCycle(m_newCycle);
    }

    if (m_oldCycle) {
        m_channel->addCycle(m_oldCycle);
    }
}

QSharedPointer<KisAnimationCycle> KisDefineCycleCommand::cycle() const
{
    return m_newCycle;
}
