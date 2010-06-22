/*
 * Kexi Report Plugin
 * Copyright (C) 2010 by Adam Pigg (adam@piggz.co.uk)
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

#ifndef KOREPORTODTRENDERER_H
#define KOREPORTODTRENDERER_H

#include <KoReportRendererBase.h>
#include <QTextCursor>

class QTextDocument;

class KoReportODTRenderer : public KoReportRendererBase
{
public:
    KoReportODTRenderer();
    virtual ~KoReportODTRenderer();
    virtual bool render(const KoReportRendererContext& context, ORODocument* document, int page = -1);

    private:
        QTextDocument * const m_document;
        QTextCursor m_cursor;
        
};

#endif // KOREPORTODTRENDERER_H
