/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PAINT_DEVICE_DATA_H
#define __KIS_PAINT_DEVICE_DATA_H

#include "KoAlwaysInline.h"
#include "kundo2command.h"
#include "kis_command_utils.h"
#include "KisInterstrokeData.h"

struct DirectDataAccessPolicy {
    DirectDataAccessPolicy(KisDataManager *dataManager, KisIteratorCompleteListener *completionListener)
        : m_dataManager(dataManager),
          m_completionListener(completionListener){}


    KisHLineConstIteratorSP createConstIterator(const QRect &rect) {
        const int xOffset = 0;
        const int yOffset = 0;
        return new KisHLineIterator2(m_dataManager, rect.x(), rect.y(), rect.width(), xOffset, yOffset, false, m_completionListener);
    }

    KisHLineIteratorSP createIterator(const QRect &rect) {
        const int xOffset = 0;
        const int yOffset = 0;
        return new KisHLineIterator2(m_dataManager, rect.x(), rect.y(), rect.width(), xOffset, yOffset, true, m_completionListener);
    }

    int pixelSize() const {
        return m_dataManager->pixelSize();
    }

    KisDataManager *m_dataManager;
    KisIteratorCompleteListener *m_completionListener;
};

class KisPaintDeviceData;

class KisPaintDeviceData
{
public:
    KisPaintDeviceData(KisPaintDevice *paintDevice)
        : m_cache(paintDevice),
          m_x(0), m_y(0),
          m_colorSpace(0),
          m_levelOfDetail(0),
          m_cacheInvalidator(this)
        {
        }

    KisPaintDeviceData(KisPaintDevice *paintDevice, const KisPaintDeviceData *rhs, bool cloneContent)
        : m_dataManager(cloneContent ?
                        new KisDataManager(*rhs->m_dataManager) :
                        new KisDataManager(rhs->m_dataManager->pixelSize(), rhs->m_dataManager->defaultPixel())),
          m_cache(paintDevice),
          m_x(rhs->m_x),
          m_y(rhs->m_y),
          m_colorSpace(rhs->m_colorSpace),
          m_levelOfDetail(rhs->m_levelOfDetail),
          m_cacheInvalidator(this)
        {
            m_cache.setupCache();
            // WARNING: interstroke data is **not** copied while cloning
        }

    void init(const KoColorSpace *cs, KisDataManagerSP dataManager) {
        m_colorSpace = cs;
        m_dataManager = dataManager;
        m_cache.setupCache();
    }

    class ChangeProfileCommand : public KUndo2Command {
    public:
        ChangeProfileCommand(KisPaintDeviceData *data,
                             const KoColorSpace *oldCs, const KoColorSpace *newCs,
                             KUndo2Command *parent)
            : KUndo2Command(parent),
              m_data(data),
              m_oldCs(oldCs),
              m_newCs(newCs)
        {
        }

        void redo() override {
            KUndo2Command::redo();

            m_data->m_colorSpace = m_newCs;
            m_data->m_cache.setupCache();
        }

        void undo() override {
            m_data->m_colorSpace = m_oldCs;
            m_data->m_cache.setupCache();

            KUndo2Command::undo();
        }

    protected:
        KisPaintDeviceData *m_data;

    private:
        bool m_firstRun {true};
        const KoColorSpace *m_oldCs;
        const KoColorSpace *m_newCs;
    };


    class ChangeColorSpaceCommand : public ChangeProfileCommand {
    public:
        ChangeColorSpaceCommand(KisPaintDeviceData *data,
                             KisDataManagerSP oldDm, KisDataManagerSP newDm,
                             const KoColorSpace *oldCs, const KoColorSpace *newCs,
                             KUndo2Command *parent)
            : ChangeProfileCommand(data, oldCs, newCs, parent),
              m_oldDm(oldDm),
              m_newDm(newDm)
        {
        }

        void redo() override {
            ChangeProfileCommand::redo();
            m_data->m_dataManager = m_newDm;
        }

        void undo() override {
            m_data->m_dataManager = m_oldDm;
            ChangeProfileCommand::undo();
        }

    private:
        KisDataManagerSP m_oldDm;
        KisDataManagerSP m_newDm;
    };

    void assignColorSpace(const KoColorSpace *dstColorSpace, KUndo2Command *parentCommand) {
        if (*m_colorSpace->profile() == *dstColorSpace->profile()) return;

        KIS_ASSERT_RECOVER_RETURN(m_colorSpace->pixelSize() == dstColorSpace->pixelSize());

        ChangeProfileCommand *cmd =
            new ChangeProfileCommand(this,
                                     m_colorSpace, dstColorSpace,
                                     parentCommand);

        // NOTE: first redo is skipped on a higher level,
        //       at DeviceChangeColorSpaceCommand
        cmd->redo();

        if (!parentCommand) {
            delete cmd;
        }
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
            InternalSequentialConstIterator srcIt(DirectDataAccessPolicy(m_dataManager.data(), cacheInvalidator()), rc);
            InternalSequentialIterator dstIt(DirectDataAccessPolicy(dstDataManager.data(), cacheInvalidator()), rc);

            int nConseqPixels = srcIt.nConseqPixels();

            // since we are accessing data managers directly, the columns are always aligned
            KIS_SAFE_ASSERT_RECOVER_NOOP(srcIt.nConseqPixels() == dstIt.nConseqPixels());

            while(srcIt.nextPixels(nConseqPixels) &&
                  dstIt.nextPixels(nConseqPixels)) {

                nConseqPixels = srcIt.nConseqPixels();

                const quint8 *srcData = srcIt.rawDataConst();
                quint8 *dstData = dstIt.rawData();

                m_colorSpace->convertPixelsTo(srcData, dstData,
                                              dstColorSpace,
                                              nConseqPixels,
                                              renderingIntent, conversionFlags);
            }
        }

        // becomes owned by the parent
        ChangeColorSpaceCommand *cmd =
            new ChangeColorSpaceCommand(this,
                                        m_dataManager, dstDataManager,
                                        m_colorSpace, dstColorSpace,
                                        parentCommand);

        // NOTE: first redo is skipped on a higher level,
        //       at DeviceChangeColorSpaceCommand
        cmd->redo();

        if (!parentCommand) {
            delete cmd;
        }
    }

    void reincarnateWithDetachedHistory(bool copyContent, KUndo2Command *parentCommand) {
        struct SwitchDataManager : public KUndo2Command
        {
            SwitchDataManager(KisPaintDeviceData *data,
                              KisDataManagerSP oldDm, KisDataManagerSP newDm,
                              KUndo2Command *parent = 0)
                : KUndo2Command(parent),
                  m_data(data),
                  m_oldDm(oldDm),
                  m_newDm(newDm)
            {
            }

            void redo() override {
                m_data->m_dataManager = m_newDm;
                m_data->cache()->invalidate();
            }

            void undo() override {
                m_data->m_dataManager = m_oldDm;
                m_data->cache()->invalidate();
            }

        private:
            KisPaintDeviceData *m_data;
            KisDataManagerSP m_oldDm;
            KisDataManagerSP m_newDm;
        };

        new KisCommandUtils::LambdaCommand(parentCommand,
            [this, copyContent] () {
                KisDataManagerSP newDm =
                    copyContent ?
                    new KisDataManager(*this->dataManager()) :
                    new KisDataManager(this->dataManager()->pixelSize(), this->dataManager()->defaultPixel());
                return new SwitchDataManager(this, this->dataManager(), newDm);
            });
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
            const quint8 *srcDefPixel = srcData->dataManager()->defaultPixel();

            const int cmp =
                memcmp(srcDefPixel,
                       m_dataManager->defaultPixel(),
                       m_dataManager->pixelSize());

            if (cmp != 0) {
                m_dataManager->setDefaultPixel(srcDefPixel);
            }
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

    ALWAYS_INLINE KisIteratorCompleteListener* cacheInvalidator() {
        return &m_cacheInvalidator;
    }

    ALWAYS_INLINE KisInterstrokeDataSP interstrokeData() const {
        return m_interstrokeData;
    }

    ALWAYS_INLINE KUndo2Command* createChangeInterstrokeDataCommand(KisInterstrokeDataSP value) {
        struct SwapInterstrokeDataCommand : public KUndo2Command
        {
            SwapInterstrokeDataCommand(KisPaintDeviceData *q, KisInterstrokeDataSP data)
                : m_q(q),
                  m_data(data)
            {
            }

            void redo() override {
                std::swap(m_data, m_q->m_interstrokeData);
            }

            void undo() override {
                std::swap(m_data, m_q->m_interstrokeData);
            }

        private:
            KisPaintDeviceData *m_q;
            KisInterstrokeDataSP m_data;
        };

        return new SwapInterstrokeDataCommand(this, value);
    }


private:
    struct CacheInvalidator : public KisIteratorCompleteListener {
        CacheInvalidator(KisPaintDeviceData *_q) : q(_q) {}

        void notifyWritableIteratorCompleted() override {
            q->cache()->invalidate();
        }
    private:
        KisPaintDeviceData *q;
    };


private:

    KisDataManagerSP m_dataManager;
    KisPaintDeviceCache m_cache;
    qint32 m_x;
    qint32 m_y;
    const KoColorSpace* m_colorSpace;
    qint32 m_levelOfDetail;
    CacheInvalidator m_cacheInvalidator;
    KisInterstrokeDataSP m_interstrokeData;
};

#endif /* __KIS_PAINT_DEVICE_DATA_H */
