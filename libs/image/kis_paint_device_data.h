/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_PAINT_DEVICE_DATA_H
#define __KIS_PAINT_DEVICE_DATA_H

#include "KoAlwaysInline.h"
#include "kundo2command.h"


struct DirectDataAccessPolicy {
    DirectDataAccessPolicy(KisDataManager *dataManager)
        : m_dataManager(dataManager) {}

    KisHLineConstIteratorSP createConstIterator(const QRect &rect) {
        const int xOffset = 0;
        const int yOffset = 0;
        return new KisHLineIterator2(m_dataManager, rect.x(), rect.y(), rect.width(), xOffset, yOffset, false);
    }

    KisHLineIteratorSP createIterator(const QRect &rect) {
        const int xOffset = 0;
        const int yOffset = 0;
        return new KisHLineIterator2(m_dataManager, rect.x(), rect.y(), rect.width(), xOffset, yOffset, true);
    }

    int pixelSize() const {
        return m_dataManager->pixelSize();
    }

    KisDataManager *m_dataManager;
};

class KisPaintDeviceData
{
public:
    KisPaintDeviceData(KisPaintDevice *paintDevice)
        : m_cache(paintDevice),
          m_x(0), m_y(0),
          m_colorSpace(0),
          m_levelOfDetail(0)
        {
        }

    KisPaintDeviceData(const KisPaintDeviceData *rhs, bool cloneContent)
        : m_dataManager(cloneContent ?
                        new KisDataManager(*rhs->m_dataManager) :
                        new KisDataManager(rhs->m_dataManager->pixelSize(), rhs->m_dataManager->defaultPixel())),
          m_cache(rhs->m_cache),
          m_x(rhs->m_x),
          m_y(rhs->m_y),
          m_colorSpace(rhs->m_colorSpace),
          m_levelOfDetail(rhs->m_levelOfDetail)
        {
            m_cache.setupCache();
        }

    void init(const KoColorSpace *cs, KisDataManagerSP dataManager) {
        m_colorSpace = cs;
        m_dataManager = dataManager;
        m_cache.setupCache();
    }

    class ChangeColorSpaceCommand : public KUndo2Command {
    public:
        ChangeColorSpaceCommand(KisPaintDeviceData *data,
                                KisDataManagerSP oldDm, KisDataManagerSP newDm,
                                const KoColorSpace *oldCs, const KoColorSpace *newCs,
                                KUndo2Command *parent)
            : KUndo2Command(parent),
              m_firstRun(true),
              m_data(data),
              m_oldDm(oldDm),
              m_newDm(newDm),
              m_oldCs(oldCs),
              m_newCs(newCs)
        {
        }

        void forcedRedo() {
            m_data->m_dataManager = m_newDm;
            m_data->m_colorSpace = m_newCs;
            m_data->m_cache.setupCache();
        }

        void redo() {
            KUndo2Command::redo();

            if (!m_firstRun) {
                m_firstRun = false;
                return;
            }

            forcedRedo();
        }

        void undo() {
            m_data->m_dataManager = m_oldDm;
            m_data->m_colorSpace = m_oldCs;
            m_data->m_cache.setupCache();

            KUndo2Command::undo();
        }

    private:
        bool m_firstRun;

        KisPaintDeviceData *m_data;
        KisDataManagerSP m_oldDm;
        KisDataManagerSP m_newDm;
        const KoColorSpace *m_oldCs;
        const KoColorSpace *m_newCs;
    };

    void assignColorSpace(const KoColorSpace *dstColorSpace) {
        KIS_ASSERT_RECOVER_RETURN(m_colorSpace->pixelSize() == dstColorSpace->pixelSize());

        m_colorSpace = dstColorSpace;
        m_cache.invalidate();
    }

    void convertDataColorSpace(const KoColorSpace *dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags, KUndo2Command *parentCommand) {
        typedef KisSequentialIteratorBase<ReadOnlyIteratorPolicy<DirectDataAccessPolicy>, DirectDataAccessPolicy> InternalSequentialConstIterator;
        typedef KisSequentialIteratorBase<WritableIteratorPolicy<DirectDataAccessPolicy>, DirectDataAccessPolicy> InternalSequentialIterator;

        if (m_colorSpace == dstColorSpace || *m_colorSpace == *dstColorSpace) {
            return;
        }

        QRect rc = m_dataManager->region().boundingRect();

        const int dstPixelSize = dstColorSpace->pixelSize();
        QScopedArrayPointer<quint8> dstDefaultPixel(new quint8[dstPixelSize]);
        memset(dstDefaultPixel.data(), 0, dstPixelSize);
        m_colorSpace->convertPixelsTo(m_dataManager->defaultPixel(), dstDefaultPixel.data(), dstColorSpace, 1, renderingIntent, conversionFlags);

        KisDataManagerSP dstDataManager = new KisDataManager(dstPixelSize, dstDefaultPixel.data());


        if (!rc.isEmpty()) {
            InternalSequentialConstIterator srcIt(DirectDataAccessPolicy(m_dataManager.data()), rc);
            InternalSequentialIterator dstIt(DirectDataAccessPolicy(dstDataManager.data()), rc);

            int nConseqPixels = 0;

            do {
                nConseqPixels = srcIt.nConseqPixels();

                const quint8 *srcData = srcIt.rawDataConst();
                quint8 *dstData = dstIt.rawData();

                m_colorSpace->convertPixelsTo(srcData, dstData,
                                              dstColorSpace,
                                              nConseqPixels,
                                              renderingIntent, conversionFlags);


            } while(srcIt.nextPixels(nConseqPixels) &&
                    dstIt.nextPixels(nConseqPixels));
        }

        // becomes owned by the parent
        ChangeColorSpaceCommand *cmd =
            new ChangeColorSpaceCommand(this,
                                        m_dataManager, dstDataManager,
                                        m_colorSpace, dstColorSpace,
                                        parentCommand);
        cmd->forcedRedo();
    }

    void prepareClone(const KisPaintDeviceData *srcData, bool copyContent = false) {
        m_x = srcData->x();
        m_y = srcData->y();

        if (copyContent) {
            m_dataManager = new KisDataManager(*srcData->dataManager());
        } else if (m_dataManager->pixelSize() !=
                   srcData->dataManager()->pixelSize()) {
            // NOTE: we don't check default pixel value! it is the task of
            //       the higher level!

            m_dataManager = new KisDataManager(srcData->dataManager()->pixelSize(), srcData->dataManager()->defaultPixel());
            m_cache.setupCache();
        } else {
            m_dataManager->clear();
        }

        m_levelOfDetail = srcData->levelOfDetail();
        m_colorSpace = srcData->colorSpace();
        m_cache.invalidate();
    }

    ALWAYS_INLINE KisDataManagerSP dataManager() const {
        return m_dataManager;
    }

    ALWAYS_INLINE KisPaintDeviceCache* cache() {
        return &m_cache;
    }

    ALWAYS_INLINE qint32 x() const {
        return m_x;
    }
    ALWAYS_INLINE void setX(qint32 value) {
        m_x = value;
    }

    ALWAYS_INLINE qint32 y() const {
        return m_y;
    }
    ALWAYS_INLINE void setY(qint32 value) {
        m_y = value;
    }

    ALWAYS_INLINE const KoColorSpace* colorSpace() const {
        return m_colorSpace;
    }

    ALWAYS_INLINE qint32 levelOfDetail() const {
        return m_levelOfDetail;
    }
    ALWAYS_INLINE void setLevelOfDetail(qint32 value) {
        m_levelOfDetail = value;
    }

private:

    KisDataManagerSP m_dataManager;
    KisPaintDeviceCache m_cache;
    qint32 m_x;
    qint32 m_y;
    const KoColorSpace* m_colorSpace;
    qint32 m_levelOfDetail;
};

#endif /* __KIS_PAINT_DEVICE_DATA_H */
