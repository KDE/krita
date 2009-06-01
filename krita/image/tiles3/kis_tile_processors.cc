/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#include <string.h>
#include <QRunnable>
#include <QPoint>
#include <QRect>

#include "kis_tile.h"
#include "kis_tile_processors.h"

/***************************************************
 * 
 *         KisBaseTileReadWriteProcessor           *
 *
 ***************************************************/

KisBaseTileReadWriteProcessor::
KisBaseTileReadWriteProcessor(ProcessorType type,
			      QRect workRect,
			      KisTileSP tile,
			      QRect dataRect)
//    :KisTileProcessor()
{
    setupProcessor(type, workRect, tile, dataRect);
}

void KisBaseTileReadWriteProcessor::setupProcessor(ProcessorType type,
                                                   QRect workRect,
                                                   KisTileSP tile,
                                                   QRect dataRect)
{
    m_tile = tile;
    m_type = type;
    initExtents(workRect, tile, dataRect);
}

void KisBaseTileReadWriteProcessor::initExtents(QRect workRect,
						KisTileSP tile,
						QRect dataRect)
{
    QRect tileRect = tile->extent();
    QRect intersectedRect = workRect & dataRect & tileRect;

    m_workRect = intersectedRect.translated(-tileRect.topLeft());

    m_dataStride = dataRect.width();
    m_dataLeftTop = intersectedRect.translated(-dataRect.topLeft()).topLeft();

    Q_ASSERT(m_workRect.width()<=KisTileData::WIDTH);
    Q_ASSERT(m_workRect.height()<=KisTileData::HEIGHT);
}

KisBaseTileReadWriteProcessor::~KisBaseTileReadWriteProcessor()
{
}


/***************************************************
 * 
 *         KisTileReadWriteProcessor               *
 *
 ***************************************************/


KisTileReadWriteProcessor::
KisTileReadWriteProcessor(ProcessorType type,
			  QRect workRect,
			  KisTileSP tile,
			  QRect dataRect,
			  quint8 *data)
    : KisBaseTileReadWriteProcessor(type, workRect, tile, dataRect)
{
    m_data=data;
}

void KisTileReadWriteProcessor::setupProcessor(ProcessorType type,
                                               QRect workRect,
                                               KisTileSP tile,
                                               QRect dataRect,
                                               quint8 *data)
{
    KisBaseTileReadWriteProcessor::setupProcessor(type, workRect, tile, dataRect);
    m_data=data;
}

KisTileReadWriteProcessor::~KisTileReadWriteProcessor()
{
}

void KisTileReadWriteProcessor::run()
{
  copyData();
}


#define indexFromPoint(_point, _width) \
            ((_point).y()*(_width) + (_point).x())

void KisTileReadWriteProcessor::copyData()
{
    qint32 tileY;
    quint8  *tileIt;
    quint8  *dataIt;
    quint8 **src=0;
    quint8 **dst=0;

    const qint32 pixelSize = m_tile->pixelSize();

    const qint32 tileStride = KisTileData::WIDTH * pixelSize;
    const qint32 dataStride = m_dataStride * pixelSize;

    /* Number of bytes copied at once */
    const qint32 lineSize = m_workRect.width() * pixelSize;


    const qint32 dataIdx = indexFromPoint(m_dataLeftTop, m_dataStride);
    const qint32 tileIdx = indexFromPoint(m_workRect.topLeft(),
                                          KisTileData::WIDTH);

    switch(m_type)
    {
    case WRITE:
        m_tile->lockForWrite();
        src = &dataIt;
        dst = &tileIt;
        break;
    case READ:
        m_tile->lockForRead();
        src = &tileIt;
        dst = &dataIt;
        break;
    default:
        Q_ASSERT_X(0, "copyData", "Oops! Wrong direction");
    }

    tileIt = m_tile->data() + tileIdx*pixelSize; 
    dataIt = m_data + dataIdx*pixelSize;

    /*TODO: make test here */
    for(tileY=m_workRect.top(); tileY<=m_workRect.bottom(); tileY++) {
        memcpy(*dst, *src, lineSize);
        dataIt+=dataStride;
        tileIt+=tileStride;
    }

    m_tile->unlock();
}

/***************************************************
 * 
 *         KisTileReadWritePlanarProcessor         *
 *
 ***************************************************/

KisTileReadWritePlanarProcessor::
KisTileReadWritePlanarProcessor(ProcessorType type,
				QRect workRect,
				KisTileSP tile,
				QRect dataRect,
				QVector<qint32> channelSizes,
				QVector<quint8*> planarData)
    : KisBaseTileReadWriteProcessor(type, workRect, tile, dataRect)
{
    m_planarData = planarData;
    m_channelSizes=channelSizes;
}

void KisTileReadWritePlanarProcessor::setupProcessor(ProcessorType type,
                                                QRect workRect,
                                                KisTileSP tile,
                                                QRect dataRect,
                                                QVector<qint32> channelSizes,
                                                QVector<quint8*> planarData)
{
    KisBaseTileReadWriteProcessor::setupProcessor(type, workRect, tile, dataRect);
    m_planarData = planarData;
    m_channelSizes=channelSizes;
}

KisTileReadWritePlanarProcessor::~KisTileReadWritePlanarProcessor()
{
}

void KisTileReadWritePlanarProcessor::run()
{
  copyPlanarData();
}


//#define indexFromPoint(_point, _width)
//            ((_point).y()*(_width) + (_point).x())

#define forEachChannel(_idx, _channelSize)                              \
    for(qint32 _idx=0, _channelSize=m_channelSizes[_idx];               \
        _idx<numChannels && (_channelSize=m_channelSizes[_idx], 1);     \
        _idx++)


void KisTileReadWritePlanarProcessor::copyPlanarData()
{
    quint8  *tileIt;
    QVector<quint8*> dataIt;

    const qint32 numChannels = m_planarData.count();

    const qint32 pixelSize = m_tile->pixelSize();
    const qint32 pixelsInLine  = m_workRect.width();
    const qint32 numLines = m_workRect.height();

    const qint32 tileStride = (KisTileData::WIDTH-pixelsInLine) * pixelSize;
#define          dataStride(_channelSize) ((m_dataStride - pixelsInLine) * _channelSize)

    const qint32 dataIdx = indexFromPoint(m_dataLeftTop, m_dataStride);
    const qint32 tileIdx = indexFromPoint(m_workRect.topLeft(),
                                          KisTileData::WIDTH);

    if(m_type==WRITE)
        m_tile->lockForWrite();
    else
        m_tile->lockForRead();


    /**
     * Init iterators
     */
    tileIt = m_tile->data() + tileIdx*pixelSize; 

    dataIt=m_planarData;
    forEachChannel(i, channelSize) {
        dataIt[i] += dataIdx*channelSize;
    }

    for(qint32 y=0; y<numLines; y++) {
	for(qint32 x=0; x<pixelsInLine; x++) {
	    forEachChannel(i, channelSize) {
		if(m_type==WRITE)
 		    memmove(tileIt, dataIt[i], channelSize);
		else
 		    memmove(dataIt[i], tileIt, channelSize);
		
		dataIt[i] += channelSize;
		tileIt += channelSize;
	    }
	}

	/**
	 * Make a stride
	 */
	tileIt += tileStride;
	forEachChannel(i, channelSize) {
	    dataIt[i] += dataStride(channelSize);
	}
    }

    m_tile->unlock();
}

