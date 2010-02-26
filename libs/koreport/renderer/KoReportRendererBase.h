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

#ifndef KORENDERERBASE_H
#define KORENDERERBASE_H
#include "koreport_export.h"
#include <KUrl>

class QPainter;
class QPrinter;
class ORODocument;

//! Context for executing rendering.
class KOREPORT_EXPORT KoReportRendererContext
{
    public:
        KoReportRendererContext();
        KUrl destinationUrl;
        QPainter *painter;
        QPrinter *printer;
};

//! Base class for report renderers.
class KOREPORT_EXPORT KoReportRendererBase
{
    public:
        KoReportRendererBase();
        
        virtual ~KoReportRendererBase();
        
        //! Render the page of the given document within the given context.
        //! If page == -1, renders the entire document.
        virtual bool render(const KoReportRendererContext& context, ORODocument *document, int page = -1) = 0;
};

//! Factory for creating renderers
//! @todo make it use plugins
class KOREPORT_EXPORT KoReportRendererFactory
{
    public:
        KoReportRendererFactory();

        KoReportRendererBase* createInstance(const QString& key);
};

#endif // KORENDERERBASE_H
