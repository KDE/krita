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

#ifndef _KIS_SOBEL_FILTER_H_
#define _KIS_SOBEL_FILTER_H_

#include "kis_filter.h"
#include "kis_filter_config_widget.h"

class KisSobelFilterConfiguration : public KisFilterConfiguration
{
public:
    KisSobelFilterConfiguration(bool doHorizontally, bool doVertically, bool keepSign, bool makeOpaque)
        : KisFilterConfiguration( "sobel", 1 )
        , m_doHorizontally(doHorizontally)
        , m_doVertically(doVertically)
        , m_keepSign(keepSign)
        , m_makeOpaque(makeOpaque)
        {};

    virtual void fromXML( const QString&  );
    virtual QString toString();

public:
    inline bool doHorizontally() { return m_doHorizontally; };
    inline bool doVertically() {return m_doVertically; };
    inline bool keepSign() {return m_keepSign; };
    inline bool makeOpaque() {return m_makeOpaque; };
    
private:
    bool m_doHorizontally;
    bool m_doVertically;
    bool m_keepSign;
    bool m_makeOpaque;
};

class KisSobelFilter : public KisFilter
{
public:
    KisSobelFilter();
public:
    virtual void process(KisPaintDeviceSP,KisPaintDeviceSP, KisFilterConfiguration* , const QRect&);
    static inline KoID id() { return KoID("sobel", i18n("Sobel")); };
    virtual bool supportsPainting() { return false; }
    virtual bool supportsPreview() { return true; }
    virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceSP )
        { std::list<KisFilterConfiguration*> list; list.insert(list.begin(), new KisSobelFilterConfiguration(true,true,true,true)); return list; }
public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceSP dev);
    virtual KisFilterConfiguration* configuration(QWidget*);
    virtual KisFilterConfiguration * configuration() { return new KisSobelFilterConfiguration( true, true, true, true); };
private:
    void prepareRow (KisPaintDeviceSP src, quint8* data, quint32 x, quint32 y, quint32 w, quint32 h);
    void sobel(const QRect & rc, KisPaintDeviceSP src, KisPaintDeviceSP dst, bool doHorizontal, bool doVertical, bool keepSign, bool makeOpaque);
};

#endif
