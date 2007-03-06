/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOPASAVINGCONTEXT_H
#define KOPASAVINGCONTEXT_H

#include <QMap>
#include <QString>

#include <KoShapeSavingContext.h>

#include "kopageapp_export.h"

class KoPAMasterPage;

/**
 * Context needed for saving the data of a kopageapp.
 */
class KOPAGEAPP_EXPORT KoPASavingContext : public KoShapeSavingContext
{
public:
    /**
     * @brief Constructor
     *
     * @param xmlWriter used for writing the data to
     * @param context the saving context
     * @param page the starting page number
     */
	KoPASavingContext( KoXmlWriter &xmlWriter, KoSavingContext &context, int page );
     
    /**
     * @brief Destructor
     */
    ~KoPASavingContext();

    /**
     * @brief Add style name of master to lookup table
     *
     * @param masterPage the master page
     * @param name the style name of the master page
     */
    void addMasterPage( const KoPAMasterPage * masterPage, const QString &name );

    /**
     * @brief Get the name of the masterpage
     *
     * @param masterPage for which the style name should be returned
     *
     * @return the style name of the masterPage
     */
    QString masterPageName( const KoPAMasterPage * masterPage );

    /**
     * @brief Increment the page
     */
    void incrementPage(); 

    /**
     * @brief Get the current page number
     *
     * The page number starts at 1
     */
    int page();

private:
    QMap<const KoPAMasterPage *, QString> m_masterPageNames;
    int m_page;
};

#endif /* KOPASAVINGCONTEXT_H */
