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

#include "KoPAPage.h"

#include <KoShapeSavingContext.h>
#include <KoShapeLayer.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>

#include "KoPAMasterPage.h"
#include "KoPASavingContext.h"
#include "KoPALoadingContext.h"

KoPAPage::KoPAPage( KoPAMasterPage * masterPage )
: KoPAPageBase()
, m_masterPage( masterPage )
{
}

KoPAPage::~KoPAPage()
{
}

KoPageLayout & KoPAPage::pageLayout()
{
    Q_ASSERT( m_masterPage );

    return m_masterPage->pageLayout();
}

void KoPAPage::createOdfPageTag( KoPASavingContext &paContext ) const
{
    paContext.xmlWriter().startElement( "draw:page" );
    paContext.xmlWriter().addAttribute( "draw:id", "page" + QString::number( paContext.page() ) );
    paContext.xmlWriter().addAttribute( "draw:master-page-name", paContext.masterPageName( m_masterPage ) );
}

void KoPAPage::loadOdfPageTag( const KoXmlElement &element, KoPALoadingContext &loadingContext )
{
    setPageTitle( element.attributeNS( KoXmlNS::draw, "name" ) );
    QString master = element.attributeNS (KoXmlNS::draw, "master-page-name" );
    setMasterPage( loadingContext.masterPageFromName( master ) );
}

void KoPAPage::setMasterPage( KoPAMasterPage * masterPage )
{
    m_masterPage = masterPage;
}
