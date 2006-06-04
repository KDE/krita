/*
 * This file is part of the KDE project
 *
 * Copyright (c) Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_SMALL_TILES_FILTER_H_
#define _KIS_SMALL_TILES_FILTER_H_

#include "kis_filter.h"
#include "kis_filter_config_widget.h"

class KisSmallTilesFilterConfiguration : public KisFilterConfiguration
{
public:
    KisSmallTilesFilterConfiguration(quint32 numberOfTiles)
        : KisFilterConfiguration( "smalltiles", 1 )
        , m_numberOfTiles(numberOfTiles) {};


    virtual void fromXML( const QString&  );
    virtual QString toString();
    
public:
    inline quint32 numberOfTiles() { return m_numberOfTiles; };

private:
    quint32 m_numberOfTiles;
};

class KisSmallTilesFilter : public KisFilter
{

public:
    KisSmallTilesFilter();

public:
    virtual void process(KisPaintDeviceSP,KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
    static inline KoID id() { return KoID("smalltiles", i18n("Small Tiles")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsPreview() { return true; }
    virtual bool supportsIncrementalPainting() { return false; }
    virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceSP )
    { std::list<KisFilterConfiguration*> list; list.insert(list.begin(), new KisSmallTilesFilterConfiguration(2)); return list; }

public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
    virtual KisFilterConfiguration * configuration(QWidget*);
    virtual KisFilterConfiguration * configuration() { return new KisSmallTilesFilterConfiguration( 2 ); };

private:
    void createSmallTiles(KisPaintDeviceSP src, KisPaintDeviceSP dst, const QRect& rect, quint32 numberOfTiles);
};

#endif
