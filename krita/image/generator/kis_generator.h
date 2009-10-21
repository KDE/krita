/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _KIS_GENERATOR_H_
#define _KIS_GENERATOR_H_

#include <QString>

#include <klocale.h>

#include "KoID.h"
#include "KoColorSpace.h"

#include "kis_types.h"
#include "kis_base_processor.h"
#include "krita_export.h"

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
    virtual ~KisGenerator();

public:

    /**
     * Override this function with the implementation of your generator.
     *
     * @param dst the destination paint device
     * @param size the size of the area that is to be filled
     * @param config the parameters of the filter
     */
    virtual void generate(KisProcessingInformation dst,
                          const QSize& size,
                          const KisFilterConfiguration* config,
                          KoUpdater* progressUpdater
                         ) const = 0;

    /**
     * Provided for convenience when no progress reporting is needed.
     */
    virtual void generate(KisProcessingInformation dst,
                          const QSize& size,
                          const KisFilterConfiguration* config
                         ) const;

    /**
     * A generator may be specialized for working in a certain colorspace.
     * If so, this function returns in instance of that colorspace, else
     * it return 0.
     */
    virtual const KoColorSpace * colorSpace();

    /**
     * @param _imageArea the rectangle of the image
     * @return the rectangle that is affected by this generator, if the generator
     *         is supposed to affect all pixels, then the function should return
     *         @p _imageArea
     */
    virtual QRect generatedRect(QRect _imageArea, const KisFilterConfiguration* = 0) const;

protected:

    /// @return the name of config group in KConfig
    QString configEntryGroup() const;

};


#endif
