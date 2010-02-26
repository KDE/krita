/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
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
#ifndef __KRSCREENRENDER_H__
#define __KRSCREENRENDER_H__

#include <QPainter>
#include "koreport_export.h"
#include "KoReportRendererBase.h"

class ORODocument;

class KoReportScreenRenderer : public KoReportRendererBase
{
public:
    KoReportScreenRenderer();
    virtual ~KoReportScreenRenderer();

    //void setPainter(QPainter *);
    //QPainter * painter() {
    //    return m_painter;
    //}

    virtual bool render(const KoReportRendererContext& context, ORODocument *document, int page);

protected:
    //QPainter* m_painter;
};

#endif // __ORPRINTRENDER_H__
