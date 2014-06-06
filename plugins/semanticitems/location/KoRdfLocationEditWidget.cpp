/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2011 Ben Martin <monkeyiq@users.sourceforge.net> hacking for fun!

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

#include "KoRdfLocationEditWidget.h"
#include "KoDocumentRdf.h"
// marble for geolocation
#ifdef CAN_USE_MARBLE
#include <marble/LatLonEdit.h>
#include <marble/MarbleWidget.h>
#endif
// KDE
#include <kdebug.h>


class KoRdfLocationEditWidgetPrivate
{
public:
#ifdef CAN_USE_MARBLE
    Marble::LatLonEdit* xlat;
    Marble::LatLonEdit* xlong;
    Marble::MarbleWidget* map;
#endif
    };

KoRdfLocationEditWidget::KoRdfLocationEditWidget(QWidget *parent, Ui::KoRdfLocationEditWidget *ew)
    : QWidget(parent)
    , d(new KoRdfLocationEditWidgetPrivate())
{
    Q_UNUSED(ew);
}

KoRdfLocationEditWidget::~KoRdfLocationEditWidget()
{
    delete d;
}

#ifdef CAN_USE_MARBLE
void KoRdfLocationEditWidget::mouseMoveGeoPosition()
{
    kDebug(30015) << "KoRdfLocationEditWidget::mouseMoveGeoPosition()";
    if(d->map)
    {
        kDebug(30015) << "lat:" << d->map->centerLatitude() << " long:" << d->map->centerLongitude();

        d->xlat->setValue( d->map->centerLatitude());
        d->xlong->setValue(d->map->centerLongitude());
    }
}

void KoRdfLocationEditWidget::setupMap( Marble::MarbleWidget* _map,
                                        Marble::LatLonEdit* _xlat,
                                        Marble::LatLonEdit* _xlong)
{
    d->map   = _map;
    d->xlat  = _xlat;
    d->xlong = _xlong;
    kDebug(30015) << " map:" << d->map;

    connect(d->map, SIGNAL(visibleLatLonAltBoxChanged(GeoDataLatLonAltBox)), this, SLOT(mouseMoveGeoPosition()));
}
#endif
