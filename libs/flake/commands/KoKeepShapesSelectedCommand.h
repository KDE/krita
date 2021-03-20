/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOKEEPSHAPESSELECTEDCOMMAND_H
#define KOKEEPSHAPESSELECTEDCOMMAND_H

#include "kis_command_utils.h"
#include <kritaflake_export.h>

class KoSelectedShapesProxy;
class KoSelection;
class KoShape;

class KRITAFLAKE_EXPORT KoKeepShapesSelectedCommand : public KisCommandUtils::FlipFlopCommand
{
public:
    KoKeepShapesSelectedCommand(const QList<KoShape*> &selectedBefore,
                                const QList<KoShape*> &selectedAfter,
                                KoSelectedShapesProxy *selectionProxy,
                                bool isFinalizing,
                                KUndo2Command *parent);

protected:
    void partB() override;

private:
    QList<KoShape*> m_selectedBefore;
    QList<KoShape*> m_selectedAfter;
    KoSelectedShapesProxy *m_selectionProxy;
};

#endif // KOKEEPSHAPESSELECTEDCOMMAND_H
