/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KISASYNCHRONOUSLYMERGEABLECOMMANDINTERFACE_H
#define KISASYNCHRONOUSLYMERGEABLECOMMANDINTERFACE_H

class KUndo2Command;

/**
 * A special base class for commands that should be able to merge
 * (compress) when used inside KisProcessingApplicator (or anything
 * else that uses KisSavedMacroCommand).
 *
 * KisProcessingApplicator generates many commands, but undo stack
 * wants only one command to represent user's action. That is why
 * the action is wrapped into KisSavedMacroCommand. This macro command
 * class has a special algorithm for merging an action consisting of
 * multiple commands. This algorithm must know if **all** the commands
 * of the macro can be merged before doing the actual merge. Using
 * KUndo2Command::mergeWith() is not possible for this purpose, because
 * it merges commands right away and one cannot undo that.
 *
 * \see KisSavedMacroCommand::mergeWith()
 * \see example in KisNodeCompositeOpCommand
 */
class KisAsynchronouslyMergeableCommandInterface
{
public:
    virtual ~KisAsynchronouslyMergeableCommandInterface();

    /**
     * @return true if \p command can be merged with (*this) command
     *         using KUndo2Command::mergeWith() call.
     *
     * WARNING: if canMergeWith() returned true, then mergeWith() must
     *          also return true. Otherwise KisSavedMacroCommand will
     *          be able to enter inconsistent state and assert.
     */
    virtual bool canMergeWith(const KUndo2Command *command) const = 0;
};

#endif // KISASYNCHRONOUSLYMERGEABLECOMMANDINTERFACE_H
