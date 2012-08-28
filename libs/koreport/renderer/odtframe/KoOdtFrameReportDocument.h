/*
   Calligra Report Engine
   Copyright (C) 2010 by Adam Pigg (adam@piggz.co.uk)
   Copyright (C) 2011, 2012 by Dag Andersen (danders@get2net.dk)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KOODTFRAMESREPORTDOCUMENT_H
#define KOODTFRAMESREPORTDOCUMENT_H

#include "reportpageoptions.h"

#include <KoDocument.h>
#include <KoStore.h>

#include <QMap>
#include <QList>

class KoOdfWriteStore;
class KoXmlWriter;
class KoStoreDevice;
class KoOdtFrameReportPrimitive;
class KoStore;

class OROSection;

class KoOdtFramesReportDocument
{
public:
    KoOdtFramesReportDocument();
    ~KoOdtFramesReportDocument();

    void startTable(OROSection *section);
    void addPrimitive(KoOdtFrameReportPrimitive *data);
    void setPageOptions(const ReportPageOptions &pageOptions);
    QFile::FileError saveDocument(const QString&);

private:
    void createStyles(KoGenStyles &coll);
    bool createContent(KoOdfWriteStore* odfStore, KoGenStyles &coll);
    void createPages(KoXmlWriter* bodyWriter, KoGenStyles &coll);

    KoStore *m_store;
    KoXmlWriter* manifestWriter;
    ReportPageOptions m_pageOptions;
    KoStoreDevice *m_stylesDevice;

    QMap<int, QList<KoOdtFrameReportPrimitive*> > m_pagemap;
};

#endif // KOODTFRAMESREPORTDOCUMENT_H
