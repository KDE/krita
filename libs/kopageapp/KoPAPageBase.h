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

#ifndef KOPAPAGEBASE_H
#define KOPAPAGEBASE_H


#include <QList>
#include <QString>

#include <KoShapeContainer.h>
#include <kopageapp_export.h>

class KoPageLayout;
class KoPASavingContext;

class KoShape;

/**
 * Base class used for KoPAMasterPage and KoPAPage
 *
 * A Page contains KoShapeLayer shapes as direct childs. The layers than can
 * contain all the different shapes.
 */
class KOPAGEAPP_EXPORT KoPAPageBase : public KoShapeContainer
{
public:
    explicit KoPAPageBase();
    virtual ~KoPAPageBase();

    /**
     * @brief Save a page
     *
     * See ODF 9.1.4 Drawing Pages
     *
     * @param paContext the pageapp saving context
     * @return true on success, false otherwise
     */
    bool saveOdf( KoPASavingContext &paContext ) const;

    /// @return the layout of the page
    virtual KoPageLayout & pageLayout() = 0;

    /**
     * @brief Get page title
     * @return page title
     */
    QString pageTitle() const;

    /**
     * @brief Set page title
     * @param title set page title
     */
    void setPageTitle( const QString &title);

    virtual void paintComponent(QPainter& painter, const KoViewConverter& converter);

protected:
    /**
     * @brief Create the page tag
     *
     * Master pages and normal pages use different tags
     *
     * @param paContext the pageapp saving context
     */
    virtual void createOdfPageTag( KoPASavingContext &paContext ) const = 0;

    /**
     * @brief Save the shapes of a page
     *
     * See ODF 9.2 Drawing Shapes
     *
     * @param paContext the pageapp saving context
     * @return true on success, false otherwise
     */
    bool saveOdfShapes( KoPASavingContext &paContext ) const;

    /**
     * @brief Save animations
     *
     * Here is a empty implementation as not all page apps need animations.
     *
     * @param paContext the pageapp saving context
     * @return true on success, false otherwise
     */
    virtual bool saveOdfAnimations( KoPASavingContext &paContext ) const { Q_UNUSED( paContext ); return true; }
    
    /**
     * @brief Save presentation notes
     *
     * Here is a empty implementation as not all page apps presentations notes.
     *
     * @return true on success, false otherwise
     */
    virtual bool saveOdfPresentationNotes() const { return true; }

    /**
     * @brief Save the style of the page
     *
     * See ODF 14.13.2 Drawing Page Style
     *
     * @return name of the page style
     */
    QString saveOdfPageStyle( KoPASavingContext &paContext ) const;

    /**
     * @brief Save special data of a style
     *
     * @param style the page style
     * @param paContext the pageapp saving context
     *
     * @see saveOdfPageStyle
     */
    virtual void saveOdfPageStyleData( KoGenStyle &style, KoPASavingContext &paContext ) const;

private:    
    QString m_pageTitle;
};

#endif /* KOPAPAGEBASE_H */
