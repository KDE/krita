/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_command_utils.h"

namespace KisCommandUtils
{
    AggregateCommand::AggregateCommand(KUndo2Command *parent)
        : KUndo2Command(parent),
          m_firstRedo(true) {}

    AggregateCommand::AggregateCommand(const KUndo2MagicString &text, KUndo2Command *parent)
        : KUndo2Command(text, parent),
          m_firstRedo(true) {}

    void AggregateCommand::redo()
    {
        if (m_firstRedo) {
            m_firstRedo = false;

            populateChildCommands();
        }

        m_store.redoAll();
    }

    void AggregateCommand::undo()
    {
        m_store.undoAll();
    }

    void AggregateCommand::addCommand(KUndo2Command *cmd)
    {
        if (!cmd) return;
        m_store.addCommand(cmd);
    }

    LambdaCommand::LambdaCommand(std::function<KUndo2Command*()> createCommandFunc)
        : m_createCommandFunc(createCommandFunc)
    {

    }

    LambdaCommand::LambdaCommand(const KUndo2MagicString &text,
                                 std::function<KUndo2Command*()> createCommandFunc)
        : AggregateCommand(text),
          m_createCommandFunc(createCommandFunc)
    {

    }

    LambdaCommand::LambdaCommand(const KUndo2MagicString &text,
                                 KUndo2Command *parent,
                                 std::function<KUndo2Command*()> createCommandFunc)
        : AggregateCommand(text, parent),
          m_createCommandFunc(createCommandFunc)
    {
    }

    LambdaCommand::LambdaCommand(KUndo2Command *parent,
                                 std::function<KUndo2Command*()> createCommandFunc)
        : AggregateCommand(parent),
          m_createCommandFunc(createCommandFunc)
    {
    }

    void LambdaCommand::populateChildCommands()
    {
        if (m_createCommandFunc) {
            addCommand(m_createCommandFunc());
        }
    }

    SkipFirstRedoWrapper::SkipFirstRedoWrapper(KUndo2Command *child, KUndo2Command *parent)
        : KUndo2Command(child ? child->text() : kundo2_noi18n("<bug: unnamed command>"), parent), m_firstRedo(true), m_child(child) {}

    void SkipFirstRedoWrapper::redo()
    {
        if (m_firstRedo) {
            m_firstRedo = false;
        } else {
            if (m_child) {
                m_child->redo();
            }
            KUndo2Command::redo();
        }
    }

    void SkipFirstRedoWrapper::undo()
    {
        KUndo2Command::undo();
        if (m_child) {
            m_child->undo();
        }
    }

    SkipFirstRedoBase::SkipFirstRedoBase(bool skipFirstRedo, KUndo2Command *parent)
        : KUndo2Command(parent),
          m_firstRedo(skipFirstRedo)
    {
    }

    SkipFirstRedoBase::SkipFirstRedoBase(bool skipFirstRedo, const KUndo2MagicString &text, KUndo2Command *parent)
        : KUndo2Command(text, parent),
          m_firstRedo(skipFirstRedo)
    {
    }

    void SkipFirstRedoBase::redo()
    {
        if (m_firstRedo) {
            m_firstRedo = false;
        } else {
            redoImpl();
            KUndo2Command::redo();
        }
    }

    void SkipFirstRedoBase::undo()
    {
        KUndo2Command::undo();
        undoImpl();
    }

    void SkipFirstRedoBase::setSkipOneRedo(bool value)
    {
        m_firstRedo = value;
    }

    FlipFlopCommand::FlipFlopCommand(bool finalizing, KUndo2Command *parent)
        : KUndo2Command(parent)
    {
        m_currentState = finalizing ? State::FINALIZING : State::INITIALIZING;
    }

    FlipFlopCommand::FlipFlopCommand(State initialState, KUndo2Command *parent)
        : KUndo2Command(parent),
          m_currentState(initialState)
    {}

    void FlipFlopCommand::redo()
    {
        if (m_currentState == FlipFlopCommand::State::INITIALIZING) {
            partA();
        } else {
            partB();
        }

        m_firstRedo = false;
    }

    void FlipFlopCommand::undo()
    {
        if (m_currentState == FlipFlopCommand::State::FINALIZING) {
            partA();
        } else {
            partB();
        }
    }

    void FlipFlopCommand::partA() {}
    void FlipFlopCommand::partB() {}

    CompositeCommand::CompositeCommand(KUndo2Command *parent)
        : KUndo2Command(parent) {}

    CompositeCommand::~CompositeCommand() {
        qDeleteAll(m_commands);
    }

    void CompositeCommand::addCommand(KUndo2Command *cmd) {
        if (cmd) {
            m_commands << cmd;
        }
    }

    void CompositeCommand::redo() {
        KUndo2Command::redo();
        Q_FOREACH (KUndo2Command *cmd, m_commands) {
            cmd->redo();
        }
    }

    void CompositeCommand::undo() {
        for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
            (*it)->undo();
        }
        KUndo2Command::undo();
    }

}
