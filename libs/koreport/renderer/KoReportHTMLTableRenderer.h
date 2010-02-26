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
#ifndef __KRHTMLTABLERENDER_H__
#define __KRHTMLTABLERENDER_H__

#include "KoReportRendererBase.h"

class ORODocument;

//
// KRHtmlRender
//
class KoReportHTMLTableRenderer : public KoReportRendererBase
{
public:
    KoReportHTMLTableRenderer();

    virtual ~KoReportHTMLTableRenderer();

    virtual bool render(const KoReportRendererContext&, ORODocument *document, int page = -1);

private:
    QString renderTable(ORODocument*);

    //! This is the directory name that will go inside the HTML files
    QString m_actualDirName;

    //! This is the directory where the image and other files will be temporarily stored
    QString m_tempDirName;
};


#endif
