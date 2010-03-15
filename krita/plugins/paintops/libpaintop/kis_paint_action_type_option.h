/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_PAINT_ACTION_TYPE_OPTION_H
#define KIS_PAINT_ACTION_TYPE_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

class KisPaintActionWidget;

enum enumPaintActionType {
    UNSUPPORTED,
    BUILDUP,
    WASH,
    FRINGED // not used yet
};

/**
 * Allows the user to choose between two types of paint action:
 * * incremental (going over the same spot in one stroke makes it darker)
 * * indirect (like photoshop and gimp)
 */
class PAINTOP_EXPORT KisPaintActionTypeOption : public KisPaintOpOption
{
public:
    KisPaintActionTypeOption();

    ~KisPaintActionTypeOption();

    enumPaintActionType paintActionType() const;

    void writeOptionSetting(KisPropertiesConfiguration* setting) const;

    void readOptionSetting(const KisPropertiesConfiguration* setting);


private:

    KisPaintActionWidget * m_optionWidget;

};

#endif
