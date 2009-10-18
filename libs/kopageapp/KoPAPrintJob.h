/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOPAPRINTJOB_H
#define KOPAPRINTJOB_H

#include <KoPrintJob.h>

#include <QPrinter>

class KoPAView;
class KoPAPageBase;
class KoPAPageProvider;

/**
 * For now we print to the center of the page honoring the margins.
 * The page is zoomed to be as big as possible.
 */
class KoPAPrintJob : public KoPrintJob
{
public:
    KoPAPrintJob(KoPAView * view);
    virtual ~KoPAPrintJob();

    virtual QPrinter & printer();
    virtual QList<QWidget*> createOptionWidgets() const;

public slots:
    virtual void startPrinting(RemovePolicy removePolicy = DoNotDelete);

private:
    QPrinter m_printer;
    QList<KoPAPageBase*> m_pages;
    KoPAPageProvider * m_pageProvider;
};

#endif /* KOPAPRINTJOB_H */
