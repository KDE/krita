
#include "kis_keyframe_commands.h"

#include <algorithm>
#include <kis_pointer_utils.h>
#include <KisCollectionUtils.h>
#include "kis_time_range.h"
#include "kis_animation_cycle.h"

using CycleSP = QSharedPointer<KisAnimationCycle>;
using KeyframeMove = KisKeyframeCommands::KeyframeMove;
using ValidationResult = KisKeyframeCommands::ValidationResult;

KisKeyframeCommands::KeyframeMove::KeyframeMove(KisKeyframeBaseSP keyframe, int newTime)
    : keyframe(keyframe)
    , newTime(newTime)
{}

KeyframeMove KisKeyframeCommands::KeyframeMove::fromAddedKeyframe(KisKeyframeBaseSP keyframe)
{
    return fromAddedKeyframe(keyframe, keyframe->time());
}

KeyframeMove KisKeyframeCommands::KeyframeMove::fromAddedKeyframe(KisKeyframeBaseSP keyframe, int newTime)
{
    return KeyframeMove(keyframe, newTime);
}

KeyframeMove KisKeyframeCommands::KeyframeMove::fromDeletedKeyframe(KisKeyframeBaseSP keyframe)
{
    return KeyframeMove(keyframe, -1);
}

struct KeyframeMapping
{
    KisKeyframeChannel *channel;

    /**
     * Map of destination times for changed keyframes.
     * Added or moved keyframes map to their new, and deleted keyframes to -1.
     */
    QMap<KisKeyframeBaseSP, int> destinations;

    /**
     * Maps times to the keyframes which is moved or added to it.
     */
    QMap<int, KisKeyframeBaseSP> sources;

    KeyframeMapping(KisKeyframeChannel *channel, const QVector<KeyframeMove> &moves)
        : channel(channel)
    {
        Q_FOREACH(const KeyframeMove& move, moves) {
            const KisKeyframeBaseSP key = move.keyframe;
            const int newTime = move.newTime;

            if (key->channel() == channel) {
                destinations[key] = newTime;

                if (newTime != -1) {
                    sources[newTime] = key;
                }
            }
        }
    }

    bool isEmpty() const
    {
        return destinations.isEmpty();
    }

    bool isAffected(const KisKeyframeBaseSP &keyframe) const {
        return destinations.contains(keyframe);
    }

    int destination(const KisKeyframeSP &keyframe) const
    {
        const int oldTime = keyframe->time();
        const bool overwritten = sources.contains(oldTime);

        return (!isAffected(keyframe) && !overwritten) ? oldTime : destinations.value(keyframe, -1);
    }

    std::tuple<int, KisKeyframeBaseSP> firstAfter(int time) const
    {
        const auto sourceIt = KisCollectionUtils::firstAfter(sources, time);
        const int firstChangeTime = (sourceIt != sources.cend()) ? sourceIt.key() : INT_MAX;

        const KisRangedKeyframeIterator keys = channel->itemsWithin(KisTimeSpan(time + 1, firstChangeTime));
        for (KisKeyframeBaseSP keyframe : keys) {
            if (!isAffected(keyframe)) {
                return std::make_tuple(keyframe->time(), keyframe);
            }
        }

        const KisKeyframeBaseSP changedKeyframe = (sourceIt != sources.cend()) ? sourceIt.value() : nullptr;
        const int changedTime = changedKeyframe ? firstChangeTime : -1;
        return std::make_tuple(changedTime, changedKeyframe);
    }

    std::tuple<int, KisKeyframeBaseSP> lastBefore(int time) const
    {
        const auto sourceIt = KisCollectionUtils::lastBefore(sources, time);
        const int firstChangeTime = (sourceIt != sources.cend()) ? sourceIt.key() : -1;

        const KisRangedKeyframeIterator keys = channel->itemsWithin(KisTimeSpan(firstChangeTime, time - 1));
        KisRangedKeyframeIterator it = --keys.end();
        for (; it.isValid(); --it) {
            const KisKeyframeBaseSP &keyframe = *it;
            if (!isAffected(keyframe)) {
                return std::make_tuple(keyframe->time(), keyframe);
            }
        }

        const KisKeyframeBaseSP changedKeyframe = (sourceIt != sources.cend()) ? sourceIt.value() : nullptr;
        return std::make_tuple(firstChangeTime, changedKeyframe);
    }

    KisKeyframeSP firstTrueKeyframeAfter(int destinationTime) const
    {
        return nearestTrueKeyframe(destinationTime, true);
    }

    KisKeyframeSP lastTrueKeyframeBefore(int destinationTime) const
    {
        return nearestTrueKeyframe(destinationTime, false);
    }

    KisKeyframeSP nearestTrueKeyframe(int destinationTime, bool forward) const
    {
        int time = destinationTime;
        KisKeyframeBaseSP keyframe;
        do {
            std::tie(time, keyframe) = forward ? firstAfter(time) : lastBefore(time);

            KisKeyframeSP keyframeProper = keyframe.dynamicCast<KisKeyframe>();
            if (keyframeProper) {
                return keyframeProper;
            }
        } while(keyframe);

        return nullptr;
    }
};

ValidationResult::Status validateMoveSources(const KisKeyframeChannel *channel, QVector<KeyframeMove> moves)
{
    Q_FOREACH(const KeyframeMove &move, moves) {
        if (move.keyframe->channel() != channel) return ValidationResult::KeyframesFromDifferentChannels;
    }

    std::sort(moves.begin(), moves.end(),
        [](const KeyframeMove &lhs, const KeyframeMove &rhs){ return lhs.keyframe->time() < rhs.keyframe->time(); }
    );

    for (int i = 1; i < moves.size(); i++) {
        if (moves[i - 1].keyframe == moves[i].keyframe) return ValidationResult::MultipleDestinations;
    }

    return ValidationResult::Valid;
}

CycleSP cycleAfterChanges(const CycleSP &cycle, const KeyframeMapping &movedKeyframes)
{
    const KisKeyframeChannel *channel = cycle->channel();

    int firstDestination = -1, lastDestination = -1;
    KisKeyframeSP newFirstKeyframe, newLastKeyframe;
    KisKeyframeSP oldLastKeyframe;

    const KisTimeSpan &oldRange = cycle->originalRange();
    for (KisKeyframeBaseSP key : channel->itemsWithin(oldRange)) {
        KisKeyframeSP keyframe = key.dynamicCast<KisKeyframe>();
        KIS_SAFE_ASSERT_RECOVER(keyframe) { continue; }

        const int newTime = mapping.destination(keyframe);

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

    const int trailingHold = oldRange.end() - oldLastKeyframe->time();

    const KisKeyframeBaseSP nextOldKeyframe = channel->nextItem(*oldLastKeyframe);
    const bool rangeEndedJustBeforeKeyframe = nextOldKeyframe && nextOldKeyframe->time() == oldRange.end() + 1;

    int timeOfNextKeyframe;
    std::tie(timeOfNextKeyframe, std::ignore) = mapping.firstAfter(lastDestination);

    int newEnd;
    if (timeOfNextKeyframe < 0) {
        newEnd = lastDestination + trailingHold;
    } else if (rangeEndedJustBeforeKeyframe) {
        newEnd = timeOfNextKeyframe - 1;
    } else {
        newEnd = qMin(lastDestination + trailingHold, timeOfNextKeyframe - 1);
    }

    const KisTimeSpan &newRange = KisTimeSpan(firstDestination, newEnd);
    if (newRange == oldRange) {
        return cycle;
    }

    KisAnimationCycle *newCycle = new KisAnimationCycle(*cycle, newRange);
    newCycle->setTime(firstDestination);
    return toQShared(newCycle);
}

QVector<CycleSP> resolveCycleOverlaps(QVector<CycleSP> &cycles)
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
                cycles[i] = toQShared(new KisAnimationCycle(*rhs, {lhsEnd + 1, rhsEnd}));
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
        const CycleSP newCycle = cycleAfterChanges(cycle, movedKeyframes);
        if (newCycle) {
            cycles << newCycle;
        }
    }

    return resolveCycleOverlaps(cycles);
}

bool validateRepeats(const QVector<CycleSP > &cycles, const KeyframeMapping &movedKeyframes)
{
    Q_FOREACH(const CycleSP &cycle, cycles) {
        if (!cycle) continue;

        const int cycleStart = cycle->originalRange().start();
        const int cycleEnd = cycle->originalRange().end();

        // If any repeat frame would land within the cycle, refuse the operation.

        int time = cycleStart;
        while (time <= cycleEnd) {
            KisKeyframeBaseSP key;
            std::tie(time, key) = movedKeyframes.firstAfter(time);

            const QSharedPointer<KisRepeatFrame> repeat = key.dynamicCast<KisRepeatFrame>();
            if (repeat) return false;
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
        if (cycle && !cyclesBefore.contains(cycle)) {
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
            const bool isOverwritten = !moves.isAffected(originalKey);
            if (isOverwritten) {
                deletedKeys.append(originalKey);
            }
        }
    }

    KUndo2Command *command = new KUndo2Command(parentCommand);

    if (!validateRepeats(cycles, mapping)) return ValidationResult::RepeatKeyframeWithinCycleDefinition;

    Q_FOREACH(const KeyframeMove &move, operations) {
        new KisReplaceKeyframeCommand(move.keyframe, move.newTime, command);
    }

    updateCycles(channel, cycles, command);

    return ValidationResult(command);
}

KisKeyframeCommands::ValidationResult KisKeyframeCommands::tryAddKeyframes(KisKeyframeChannel *channel, const QVector<KisKeyframeBaseSP> &keyframes, KUndo2Command *parentCommand)
{
    QVector<KeyframeMove> operations(keyframes.size());
    KeyframeMove (*fromAddedKeyframe)(KisKeyframeBaseSP) = KeyframeMove::fromAddedKeyframe; // Force correct overload resolution with explicit type
    std::transform(keyframes.cbegin(), keyframes.cend(), operations.begin(), fromAddedKeyframe);

KisKeyframeCommands::ValidationResult KisKeyframeCommands::tryDefineCycle(KisKeyframeChannel *channel, KisTimeSpan range, KUndo2Command *parentCommand)
{
    if (range.isEmpty() || range.start() < 0) return ValidationResult::InvalidCycle;
    Q_FOREACH(const CycleSP &cycle, channel->cycles()) {
        // Cycles must not overlap
        if (range.overlaps(cycle->originalRange())) {
            return ValidationResult::OverlappingCycles;
        }

        Q_FOREACH(QWeakPointer<KisRepeatFrame> repeatWP, cycle->repeats()) {
            auto repeat = repeatWP.toStrongRef();
            if (repeat && range.contains(repeat->time())) {
                return ValidationResult::RepeatKeyframeWithinCycleDefinition;
            }
        }
    }

    QSharedPointer<KisAnimationCycle> cycle = toQShared(new KisAnimationCycle(channel, range));
    return ValidationResult(new KisDefineCycleCommand(nullptr, cycle, parentCommand));
}

KUndo2Command * KisKeyframeCommands::removeCycle(QSharedPointer<KisAnimationCycle> cycle, KUndo2Command *parentCommand)
{
    return new KisDefineCycleCommand(cycle, nullptr, parentCommand);
}

KisKeyframeCommands::ValidationResult KisKeyframeCommands::tryAddKeyframes(KisKeyframeChannel *channel, const QVector<KisKeyframeBaseSP> &keyframes, KUndo2Command *parentCommand)
{
    QVector<KeyframeMove> operations(keyframes.size());
    KeyframeMove (*fromAddedKeyframe)(KisKeyframeBaseSP) = KeyframeMove::fromAddedKeyframe; // Force correct overload resolution with explicit type
    std::transform(keyframes.cbegin(), keyframes.cend(), operations.begin(), fromAddedKeyframe);

    return tryCreateCommands(channel, operations, parentCommand);
}

KisKeyframeCommands::ValidationResult KisKeyframeCommands::tryRemoveKeyframes(KisKeyframeChannel *channel, const QVector<KisKeyframeBaseSP> &keyframes, KUndo2Command *parentCommand)
{
    QVector<KeyframeMove> operations(keyframes.size());
    std::transform(keyframes.cbegin(), keyframes.cend(), operations.begin(), KeyframeMove::fromDeletedKeyframe);

    return tryCreateCommands(channel, operations, parentCommand);
}

KisKeyframeCommands::ValidationResult KisKeyframeCommands::tryMoveKeyframes(KisKeyframeChannel *channel, const QVector<KeyframeMove> &moves, KUndo2Command *parentCommand)
{
    const ValidationResult::Status moveValidation = validateMoveSources(channel, moves);
    if (moveValidation != ValidationResult::Valid) return moveValidation;
    return tryCreateCommands(channel, moves, parentCommand);
}

KisReplaceKeyframeCommand::KisReplaceKeyframeCommand(KisKeyframeBaseSP keyframe, int newTime, KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand),
      m_channel(keyframe->channel()),
      m_keyframe(keyframe),
      m_newTime(newTime)
{
}

KisReplaceKeyframeCommand::KisReplaceKeyframeCommand(KisKeyframeChannel *channel, int time, KisKeyframeBaseSP keyframe,
                                                     KUndo2Command *parentCommand)
    : KUndo2Command(parentCommand)
    , m_channel(channel)
    , m_keyframe(keyframe)
    , m_newTime(time)
{}

void KisReplaceKeyframeCommand::redo() {
    if (m_newTime >= 0) {
        m_overwrittenKeyframe = m_channel->itemAt(m_newTime);

        if (m_overwrittenKeyframe) {
            m_channel->removeKeyframeLogical(m_overwrittenKeyframe);
        }
    }

    if (m_keyframe) {
        const bool currentlyOnChannel = m_channel->itemAt(m_keyframe->time()) == m_keyframe;
        m_oldTime = currentlyOnChannel ? m_keyframe->time() : -1;
    }

    moveKeyframeTo(m_newTime);
}

void KisReplaceKeyframeCommand::undo() {
    moveKeyframeTo(m_oldTime);

    if (m_overwrittenKeyframe) {
        m_overwrittenKeyframe->setTime(m_newTime);
        m_channel->insertKeyframeLogical(m_overwrittenKeyframe);
        m_overwrittenKeyframe = nullptr;
    }
}

void KisReplaceKeyframeCommand::moveKeyframeTo(int dstTime)
{
    if (!m_keyframe) return;

    const bool currentlyOnChannel = m_channel->itemAt(m_keyframe->time()) == m_keyframe;

    if (dstTime < 0) {
        if (currentlyOnChannel) {
            m_channel->removeKeyframeLogical(m_keyframe);
        }
    } else {
        if (currentlyOnChannel) {
            m_channel->moveKeyframeImpl(m_keyframe, dstTime);
        } else {
            m_keyframe->setTime(dstTime);
            m_channel->insertKeyframeLogical(m_keyframe);
        }
    }
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
