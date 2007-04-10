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

#include "KoPAMasterPage.h"

#include <KoGenStyle.h>
#include <KoXmlWriter.h>

#include "KoPASavingContext.h"

KoPAMasterPage::KoPAMasterPage()
: KoPAPageBase()
{
    m_pageLayout = KoPageLayout::standardLayout();
}

KoPAMasterPage::~KoPAMasterPage()
{
}

void KoPAMasterPage::createOdfPageTag( KoPASavingContext *paContext ) const
{
    KoGenStyle pageLayoutStyle = pageLayout().saveOasis();
    pageLayoutStyle.setAutoStyleInStylesDotXml( true );
    pageLayoutStyle.addAttribute( "style:page-usage", "all" );
    QString pageLayoutName( paContext->mainStyles().lookup( pageLayoutStyle, "pm" ) );

    paContext->xmlWriter().startElement( "style:master-page" );
    paContext->xmlWriter().addAttribute( "style:name", "Standard" ); //TODO
    paContext->addMasterPage( this, "Standard" );
    paContext->xmlWriter().addAttribute( "style:page-layout-name", pageLayoutName );
}
