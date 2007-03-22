/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOPAPAGE_H
#define KOPAPAGE_H

#include "KoPAPageBase.h"

class KoPAMasterPage;

/// Class representing a page
class KOPAGEAPP_EXPORT KoPAPage : public KoPAPageBase
{
public:
    /** Constructor
     * @param masterPage masterpage used for this page
     */
    explicit KoPAPage( KoPAMasterPage * masterPage );
    ~KoPAPage();

    /// @return the layout set by the masterpage
    KoPageLayout & pageLayout();

    /// Set the masterpage for this page to @p masterPage
    void setMasterPage( KoPAMasterPage * masterPage );
    /// @return the masterpage of this page
    KoPAMasterPage * masterPage() { return m_masterPage; }
protected:
    virtual void createOdfPageTag( KoPASavingContext &paContext ) const;

    KoPAMasterPage * m_masterPage;
};

#endif /* KOPAPAGE_H */
