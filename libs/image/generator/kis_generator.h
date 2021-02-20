/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_GENERATOR_H_
#define _KIS_GENERATOR_H_

#include <QString>

#include <klocalizedstring.h>

#include "KoID.h"
#include "KoColorSpace.h"

#include "kis_types.h"
#include "kis_base_processor.h"
#include "kritaimage_export.h"

class KisProcessingInformation;

/**
 * Basic interface of a Krita generator: a generator is a program
 * that can fill a paint device with a color. A generator can have a
 * preferred colorspace.
 *
 * Generators can have initial parameter settings that determine the
 * way a particular generator works, but also state that allows the generator
 * to continue from one invocation of generate to another (handy for, e.g.,
 * painting)
 */
class KRITAIMAGE_EXPORT KisGenerator : public KisBaseProcessor
{
    friend class KisGeneratorConfigurationFactory;
public:

    KisGenerator(const KoID& id, const KoID & category, const QString & entry);
    ~KisGenerator() override;

public:

    /**
     * Override this function with the implementation of your generator.
     *
     * @param dst the destination paint device
     * @param size the size of the area that is to be filled
     * @param config the parameters of the filter
     * @param progressUpdater the progress updater
     */
    virtual void generate(KisProcessingInformation dst,
                          const QSize& size,
                          const KisFilterConfigurationSP config,
                          KoUpdater* progressUpdater
                         ) const = 0;

    /**
     * Provided for convenience when no progress reporting is needed.
     */
    virtual void generate(KisProcessingInformation dst,
                          const QSize& size,
                          const KisFilterConfigurationSP config
                         ) const;

    /**
     * @param _imageArea the rectangle of the image
     * @return the rectangle that is affected by this generator, if the generator
     *         is supposed to affect all pixels, then the function should return
     *         @p _imageArea
     */
    virtual QRect generatedRect(QRect _imageArea, const KisFilterConfigurationSP = 0) const;

    /**
     * Reports whether this generator can run properly while
     * tiling the image into patches (as opposed to over the whole
     * image in one single pass).
     * 
     * Generators that are known to not work properly should override
     * this function and return false.
     */ 
    virtual bool allowsSplittingIntoPatches() const { return true; }

protected:

    /// @return the name of config group in KConfig
    QString configEntryGroup() const;

};


#endif
