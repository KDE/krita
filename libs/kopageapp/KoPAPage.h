/* This file is part of the KDE project
   Copyright (C) 2006-2009 Thorsten Zachmann <zachmann@kde.org>

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

    /// reimplemented
    virtual void saveOdf( KoShapeSavingContext & context ) const;

    /// @return the layout set by the masterpage
    KoPageLayout & pageLayout();
    const KoPageLayout & pageLayout() const;

    /// Set the masterpage for this page to @p masterPage
    void setMasterPage( KoPAMasterPage * masterPage );
    /// @return the masterpage of this page
    KoPAMasterPage * masterPage() { return m_masterPage; }

    /// reimplemented
    virtual void paintBackground( QPainter & painter, const KoViewConverter & converter );

    /// reimplemented
    virtual bool displayMasterShapes();

    /// reimplemented
    virtual void setDisplayMasterShapes( bool display );

    /// reimplemented
    virtual bool displayMasterBackground();

    /// reimplemented
    virtual void setDisplayMasterBackground( bool display );

    /// reimplemented
    virtual bool displayShape(KoShape *shape) const;

    /// reimplemented
    virtual void paintPage( QPainter & painter, KoZoomHandler & zoomHandler );

protected:
    /**
     * DisplayMasterBackground and DisplayMasterShapes are only saved loaded in a presentation
     * They are however implemented here to reduce code duplication.
     */
    enum PageProperty
    {
        UseMasterBackground = 1,        ///< Use the background of the master page. See ODF 14.13.2 Drawing Page Style
        DisplayMasterBackground = 2,    ///< If the master page is used this indicated if its backround should be used. See ODF 15.36.13 Background Visible
        DisplayMasterShapes = 4,         ///< Set if the shapes of the master page should be shown. See ODF 15.36.12 Background Objects Visible
        DisplayHeader = 8,       /// set if presentation:display-header is true
        DisplayFooter = 16,      /// set if presentation:display-footer is true
        DisplayPageNumber = 32,  /// set if presentation:display-page-number is true
        DisplayDateTime = 64     /// set if presentation:display-date-time is true
    };

    /// Reimplemented from KoPageBase
    virtual void loadOdfPageTag( const KoXmlElement &element, KoPALoadingContext &loadingContext );

    /// Reimplemented from KoPageBase
    virtual void saveOdfPageStyleData( KoGenStyle &style, KoPASavingContext &paContext ) const;

    /// reimplemented
    virtual QPixmap generateThumbnail( const QSize& size = QSize( 512, 512 ) );

    KoPAMasterPage * m_masterPage;

    int m_pageProperties;
};

#endif /* KOPAPAGE_H */
