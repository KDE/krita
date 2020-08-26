/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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


#ifndef KISASYNCHRONOUSLYMERGEABLECOMMANDINTERFACE_H
#define KISASYNCHRONOUSLYMERGEABLECOMMANDINTERFACE_H

class KUndo2Command;

/**
 * A special base class for commands that should be able to merge
 * (compress) when used inside KisProcessingApplicator (or anything
 * else that uses KisSavedMacroCommand).
 *
 * KisProcessignApplicator generates many commands, but undo stack
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
