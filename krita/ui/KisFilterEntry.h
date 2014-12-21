/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright     2007       David Faure <faure@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef __ko_filter_entry_h__
#define __ko_filter_entry_h__

#include <kservice.h>
#include <ksharedptr.h>
#include <QList>
#include "krita_export.h"

class QObject;
class QStringList;
class KisImportExportFilter;
class KisFilterChain;
/**
 *  Represents an available filter.
 */
class KRITAUI_EXPORT KisFilterEntry : public KShared
{

public:
    typedef KSharedPtr<KisFilterEntry> Ptr;

    //KisFilterEntry() : weight( 0 ) { m_service = 0; } // for QList
    explicit KisFilterEntry(const KService::Ptr& service);
    ~KisFilterEntry() { }

    KisImportExportFilter* createFilter(KisFilterChain* chain, QObject* parent = 0);

    /**
     *  The imported mimetype(s).
     */
    QStringList import;

    /**
     *  The exported mimetype(s).
     */
    QStringList export_;

    /**
     *  The "weight" of this filter path. Has to be > 0 to be valid.
     */
    unsigned int weight;

    /**
     *  Do we have to check during runtime?
     */
    QString available;

    /**
     *  @return TRUE if the filter can import the requested mimetype.
     */
    bool imports(const QString& _mimetype) const {
        return (import.contains(_mimetype));
    }

    /**
     *  @return TRUE if the filter can export the requested mimetype.
     */
    bool exports(const QString& _m) const {
        return (export_.contains(_m));
    }

    /**
     *  This function will query KDED to find all available filters.
     */
    static QList<KisFilterEntry::Ptr> query();

    KService::Ptr service() const {
        return m_service;
    }

private:
    KService::Ptr m_service;
};

#endif
