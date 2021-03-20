/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_TRANSACTION_BASED_COMMAND_H
#define KIS_TRANSACTION_BASED_COMMAND_H

#include <kritaimage_export.h>
#include <kundo2command.h>

class KRITAIMAGE_EXPORT KisTransactionBasedCommand : public KUndo2Command
{
public:
    KisTransactionBasedCommand(const KUndo2MagicString &text = KUndo2MagicString(), KUndo2Command *parent = 0);

    ~KisTransactionBasedCommand() override;

    void redo() override;
    void undo() override;

protected:
    virtual KUndo2Command* paint() = 0;
private:
    KUndo2Command *m_transactionData;
};
#endif // KIS_TRANSACTION_BASED_COMMAND_H
