/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KRKSPREADRENDERER_H
#define KRKSPREADRENDERER_H

#include "KoReportRendererBase.h"

/**
 @author Adam Pigg <adam@piggz.co.uk>
*/
class KoReportKSpreadRenderer : public KoReportRendererBase
{
public:
    KoReportKSpreadRenderer();
    ~KoReportKSpreadRenderer();

    virtual bool render(const KoReportRendererContext&, ORODocument*, int page = -1);

private:

};

#endif
