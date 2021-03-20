/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_transaction_based_command.h"

KisTransactionBasedCommand::KisTransactionBasedCommand(const KUndo2MagicString &text, KUndo2Command* parent)
    : KUndo2Command(text, parent), m_transactionData(0)
{
}

KisTransactionBasedCommand::~KisTransactionBasedCommand()
{
    delete m_transactionData;
}

void KisTransactionBasedCommand::redo()
{
    if (!m_transactionData) {
        m_transactionData = paint();
    }

    if (m_transactionData) {
        m_transactionData->redo();
    }
}

void KisTransactionBasedCommand::undo()
{
    if (m_transactionData) {
        m_transactionData->undo();
    }
}

