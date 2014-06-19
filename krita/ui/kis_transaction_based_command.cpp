/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

