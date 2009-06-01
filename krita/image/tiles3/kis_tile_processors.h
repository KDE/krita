/*
 *  Copyright (c) 2009 Dmitry  Kazakov <dimula73@gmail.com>
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

#ifndef KIS_TILE_PROCESSOR_H_
#define KIS_TILE_PROCESSOR_H_

#include "kis_tile.h"

#include <QRunnable>

//class KisTileData;
//class QRunnable;
class QRect;


/***************************************************
 * 
 *         KisBaseTileProcessor                    *
 *
 ***************************************************/

class KisTileProcessor : public KisShared/*public QRunnable*/
{
public:
    KisTileProcessor(){
    }
    /* why virtual? */
    virtual ~KisTileProcessor(){
    }
    
    virtual void run() = 0;
protected:
private:
//    Q_DISABLE_COPY(KisBaseTileProcessor);
};

class KisBaseTileReadWriteProcessor : public KisTileProcessor
{
public:
    enum ProcessorType {
      READ,
      WRITE
    };

public:
    KisBaseTileReadWriteProcessor(ProcessorType type,
				  QRect workRect,
				  KisTileSP tile,
				  QRect dataRect);
    /* why virtual? */
    virtual ~KisBaseTileReadWriteProcessor();
    
    virtual void run() = 0;

    virtual void setupProcessor(ProcessorType type,
                                QRect workRect,
                                KisTileSP tile,
                                QRect dataRect);

protected:

    void initExtents(QRect workRect,
		     KisTileSP tile,
		     QRect dataRect);
 
protected:
    /**
     * The tile to work with
     */
    KisTileSP m_tile;

    /**
     * To write or not to write... 
     */ 
    ProcessorType m_type;

    /**
     * Rect that stores coordinates 
     * relative to tile's topLeft()
     */
    QRect m_workRect;

    /**
     * m_data->rect()->width() :)
     * Note: in pixels, not bytes!
     */
    qint32 m_dataStride;

    /**
     * m_data->rect()->pixelToStartWith() :)
     */
    QPoint m_dataLeftTop;
};

/***************************************************
 * 
 *         KisTileReadWriteProcessor               *
 *
 ***************************************************/

class KisTileReadWriteProcessor : public KisBaseTileReadWriteProcessor
{
public:
    KisTileReadWriteProcessor(ProcessorType type,
			      QRect workRect,
			      KisTileSP tile,
			      QRect dataRect,
			      quint8 *data);
    /* why virtual? */
    virtual ~KisTileReadWriteProcessor();

    virtual void setupProcessor(ProcessorType type,
                                QRect workRect,
                                KisTileSP tile,
                                QRect dataRect,
                                quint8 *data);
    
    virtual void run();

protected:
    void copyData();

protected:
    /**
     * Pointer do the data array
     * where we take|put data from|to
     *
     * Note: access to @m_data is not serialized(!)
     * Every tileprocessor works on it's own rect
     * of data inside @m_data. They shouldn't intersect.
     */
    quint8 *m_data;

};


/***************************************************
 * 
 *         KisTileReadWritePlanarProcessor          *
 *
 ***************************************************/

class KisTileReadWritePlanarProcessor : public KisBaseTileReadWriteProcessor
{
public:
    KisTileReadWritePlanarProcessor(ProcessorType type,
				    QRect workRect,
				    KisTileSP tile,
				    QRect dataRect,
				    QVector<qint32> channelSizes,
				    QVector<quint8*> planarData);

    /* why virtual? */
    virtual ~KisTileReadWritePlanarProcessor();

    virtual void setupProcessor(ProcessorType type,
                                QRect workRect,
                                KisTileSP tile,
                                QRect dataRect,
                                QVector<qint32> channelSizes,
                                QVector<quint8*> planarData);    
    
    virtual void run();

protected:
    void copyPlanarData();

protected:
    /**
     * Vector of pointers to channels data
     * where we take|put data from|to
     *
     * Note: access to @m_data is not serialized(!)
     * Every tileprocessor works on it's own rect
     * of data inside @m_data. They shouldn't intersect.
     */
    QVector<quint8*> m_planarData;

    /**
     * How many bytes a channel takes in a pixel
     */
    QVector<qint32> m_channelSizes;

};


/***************************************************
 * 
 *              Processor Factories                *
 *
 ***************************************************/

typedef KisSharedPtr<KisTileProcessor> KisTileProcessorSP;

class KisTileProcessorFactory
{
public:
    KisTileProcessorFactory() {}
    virtual ~KisTileProcessorFactory() {}

    virtual KisTileProcessorSP getProcessor(QRect workRect,
                                            KisTileSP tile) = 0;
    virtual void setupProcessor(KisTileProcessorSP baseProcessor, 
                                QRect workRect,
                                KisTileSP tile) = 0;
};


class KisTileReadWriteProcessorFactory : public KisTileProcessorFactory
{
public:
    KisTileReadWriteProcessorFactory(KisBaseTileReadWriteProcessor::ProcessorType type,
                                     QRect dataRect, quint8* data)
        :m_dataRect(dataRect), m_data(data), m_type(type)
    {
    }

    virtual KisTileProcessorSP getProcessor(QRect workRect,
                                            KisTileSP tile)
    {
        return static_cast<KisTileProcessor*>(
            new KisTileReadWriteProcessor(m_type, workRect, tile,
                                          m_dataRect, m_data));
    }

    virtual void setupProcessor(KisTileProcessorSP baseProcessor, 
                                QRect workRect,
                                KisTileSP tile)
    {
        KisTileReadWriteProcessor *processor = 
            dynamic_cast<KisTileReadWriteProcessor*>(baseProcessor.data());
        processor->setupProcessor(m_type, workRect, tile,
                                  m_dataRect, m_data);      
    }

protected:
    QRect  m_dataRect;
    quint8 *m_data; 
    KisBaseTileReadWriteProcessor::ProcessorType m_type;
};


class KisTileReadWritePlanarProcessorFactory : public KisTileProcessorFactory
{
public:
    KisTileReadWritePlanarProcessorFactory(KisBaseTileReadWriteProcessor::ProcessorType type,
                                     QRect dataRect, 
                                     QVector<qint32> channelSizes,
                                     QVector<quint8*> planarData)
        :m_dataRect(dataRect), m_channelSizes(channelSizes),
         m_planarData(planarData), m_type(type)
    {
    }

    virtual KisTileProcessorSP getProcessor(QRect workRect,
                         KisTileSP tile)
    {
        return static_cast<KisTileProcessor*>(
            new KisTileReadWritePlanarProcessor(m_type, workRect, tile,
                                                m_dataRect, m_channelSizes, 
                                                m_planarData));
    }
    
    virtual void setupProcessor(KisTileProcessorSP baseProcessor, 
                                QRect workRect,
                                KisTileSP tile)
    {
        KisTileReadWritePlanarProcessor *processor = 
            dynamic_cast<KisTileReadWritePlanarProcessor*>(baseProcessor.data());
        processor->setupProcessor(m_type, workRect, tile,
                                  m_dataRect, m_channelSizes, 
                                  m_planarData);
    }

protected:
    QRect m_dataRect;
    QVector<qint32> m_channelSizes;
    QVector<quint8*> m_planarData;
    KisBaseTileReadWriteProcessor::ProcessorType m_type;
};

#endif  /* KIS_TILE_PROCESSOR_H_ */
