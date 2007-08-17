/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_PAINT_DEVICE_JOBS_H_
#define KIS_PAINT_DEVICE_JOBS_H_

#include <QDebug>
#include <QThread>
#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/Job.h>
#include <KoColorSpace.h>
#include <kis_paint_device.h>

using namespace ThreadWeaver;

/**
 * Definitions for various threadweaver jobs used in KisPaintDevice
 */


/**
 * Convert the specified set of pixels to another colorspace
 */
class ConversionJob : public Job
{
public:
    ConversionJob( const quint8* src,
                   quint8* dst,
                   const KoColorSpace* fromCS,
                   const KoColorSpace* toCS,
                   const quint32 nPixels,
                   const KoColorConversionTransformation::Intent renderingIntent,
                   QObject * parent )
        : Job( parent )
        , m_src( src )
        , m_dst( dst )
        , m_fromCS( fromCS )
        , m_toCS( toCS )
        , m_nPixels( nPixels )
        , m_renderingIntent( renderingIntent )
        {
        }

    ~ConversionJob()
        {
        }

    void run()
        {
            m_fromCS->convertPixelsTo(m_src, m_dst, m_toCS, m_nPixels, m_renderingIntent);
            delete this;
        }
private:

    const quint8* m_src;
    quint8* m_dst;
    const KoColorSpace * m_fromCS;
    const KoColorSpace * m_toCS;
    const quint32 m_nPixels;
    const KoColorConversionTransformation::Intent m_renderingIntent;
};

#endif
