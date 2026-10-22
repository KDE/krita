/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef KIS_GAMUTMAP_CLIP_ADJUSTMENT_H
#define KIS_GAMUTMAP_CLIP_ADJUSTMENT_H

#include "KoColorTransformationFactory.h"

/**
 * @brief The KisGamutMapClipAdjustment class
 *
 * Normally, in the ICC workflow, a profile is supossed to handle
 * any kind of gamut mapping. This includes dealing with clipping
 * values that are out of gamut.
 *
 * This KoColorTransformation takes colors, converts them to an
 * opponent space, like Lab or ICtCp, and then desaturates the
 * colors until they are inside the target color gamut.
 *
 * There's more complex gamut mapping algorithms available, but
 * this particular transform focuses on simplicity and speed.
 */

class KisGamutMapClipAdjustmentFactory : public KoColorTransformationFactory
{
public:
    KisGamutMapClipAdjustmentFactory();

    QList< QPair< KoID, KoID > > supportedModels() const override;

    KoColorTransformation* createTransformation(const KoColorSpace* colorSpace, QHash<QString, QVariant> parameters) const override;
};

#endif // KIS_GAMUTMAP_CLIP_ADJUSTMENT_H
