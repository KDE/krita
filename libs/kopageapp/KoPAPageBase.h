/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

class KoPageLayout;

class KoShape;

class KoPAPageBase : public KoShapeContainer
{
public:
    explicit KoPAPageBase();
    virtual ~KoPAPageBase();

    virtual KoPageLayout & pageLayout() = 0;

    /**
     * Return page title
     * @param return page title
     */
    QString pageTitle() const;

    /**
     * Set page title
     * @param set page title
     */
    void setPageTitle( const QString &);

    virtual void paintComponent(QPainter& painter, const KoViewConverter& converter);

protected:
    QString m_pageTitle;
};

#endif /* KOPAPAGEBASE_H */
