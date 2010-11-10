/*
 *  Copyright (c) 2010 Carlos Licea <carlos@kdab.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KOCELLSTYLE_H
#define KOCELLSTYLE_H

#include "KoStyle.h"
#include "KoBorder.h"

/**
 * A \class KoCellStyle represents a style of a cell to be applied to one or more cells.
 * 
 * As all the styles it can be shared
 */

class KoCellStyle : public KoStyle
{
    KoCellStyle();

public:
    KOSTYLE_DECLARE_SHARED_POINTER(KoCellStyle)

    virtual ~KoCellStyle();

    KoBorder* borders();

protected:
    virtual void prepareStyle( KoGenStyle& style ) const;
    virtual QString defaultPrefix() const;
    virtual KoGenStyle::Type styleType() const;
    virtual KoGenStyle::Type automaticstyleType() const;
    virtual const char* styleFamilyName() const;

private:
    KoBorder* m_borders;
};

#endif
