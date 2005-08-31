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
#include "kis_view.h"
#include <kdebug.h>

class KisSobelFilterConfiguration : public KisFilterConfiguration
{
    public:
                KisSobelFilterConfiguration(bool doHorizontally, bool doVertically, bool keepSign, bool makeOpaque) : m_doHorizontally(doHorizontally), m_doVertically(doVertically), m_keepSign(keepSign), m_makeOpaque(makeOpaque) {};
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
    virtual void process(KisPaintDeviceImplSP,KisPaintDeviceImplSP, KisFilterConfiguration* , const QRect&);
    static inline KisID id() { return KisID("sobel", i18n("Sobel")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsPreview() { return true; }
    virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceImplSP )
    { std::list<KisFilterConfiguration*> list; list.insert(list.begin(), new KisSobelFilterConfiguration(true,true,true,true)); return list; }
public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceImplSP dev);
    virtual KisFilterConfiguration* configuration(QWidget*, KisPaintDeviceImplSP dev);
private:
    void prepareRow (KisPaintDeviceImplSP src, Q_UINT8* data, Q_UINT32 x, Q_UINT32 y, Q_UINT32 w, Q_UINT32 h);
    void sobel(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, bool doHorizontal, bool doVertical, bool keepSign, bool makeOpaque);
};

#endif
