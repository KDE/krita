/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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
    void end();

private:
    QList<KoShape*> m_selectedBefore;
    QList<KoShape*> m_selectedAfter;
    KoSelectedShapesProxy *m_selectionProxy;
};

#endif // KOKEEPSHAPESSELECTEDCOMMAND_H
