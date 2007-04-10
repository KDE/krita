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

#ifndef KOPAMASTERPAGE_H
#define KOPAMASTERPAGE_H


#include "KoPAPage.h"

#include <KoPageLayout.h>

/// Stores the masterpage's shapes and settings
class KOPAGEAPP_EXPORT KoPAMasterPage : public KoPAPageBase
{
public:
    explicit KoPAMasterPage();
    ~KoPAMasterPage();

    /// @return the page layout set for this masterpage
    KoPageLayout & pageLayout() { return m_pageLayout; }
    const KoPageLayout & pageLayout() const { return m_pageLayout; }

    /// Set the page layout to @p layout
    void setPageLayout(const KoPageLayout& layout) { m_pageLayout = layout; }

protected:
    virtual void createOdfPageTag( KoPASavingContext *paContext ) const;

    KoPageLayout m_pageLayout;
};

#endif /* KOPAMASTERPAGE_H */
