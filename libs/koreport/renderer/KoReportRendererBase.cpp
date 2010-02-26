/*
   KOffice Reporting Library
   Copyright (C) 2010 by Adam Pigg (adam@piggz.co.uk)
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoReportRendererBase.h"
#include "KoReportScreenRenderer.h"
#include "KoReportPrintRenderer.h"
#include "KoReportKSpreadRenderer.h"
#include "KoReportHTMLTableRenderer.h"
#include "KoReportHTMLCSSRenderer.h"

KoReportRendererContext::KoReportRendererContext()
 : painter(0), printer(0)
{
}

KoReportRendererBase::KoReportRendererBase()
{
}

KoReportRendererBase::~KoReportRendererBase()
{
}

KoReportRendererFactory::KoReportRendererFactory()
{
}

KoReportRendererBase* KoReportRendererFactory::createInstance(const QString& key)
{
    if (key.toLower() == QLatin1String("screen")) {
        return new KoReportScreenRenderer();
    }
    if (key.toLower() == QLatin1String("print")) {
        return new KoReportPrintRenderer();
    }
#ifdef HAVE_KSPREAD    
    if (key.toLower() == QLatin1String("kspread")) {
        return new KoReportKSpreadRenderer();
    }
#endif    
    if (key.toLower() == QLatin1String("htmltable")) {
        return new KoReportHTMLTableRenderer();
    }
    if (key.toLower() == QLatin1String("htmlcss")) {
        return new KoReportHTMLCSSRenderer();
    }
    return 0;
}
