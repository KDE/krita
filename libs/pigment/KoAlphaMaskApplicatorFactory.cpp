/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoAlphaMaskApplicatorFactory.h"

#include <KoColorModelStandardIdsUtils.h>
#include <kis_assert.h>

#include "KoAlphaMaskApplicatorFactoryImpl.h"

template <typename channels_type>
struct CreateApplicator
{
    KoAlphaMaskApplicatorBase *operator() (int numChannels, int alphaPos) {
        if (numChannels == 4) {
            KIS_ASSERT(alphaPos == 3);
            return createOptimizedClass<
                    KoAlphaMaskApplicatorFactoryImpl<
                        channels_type, 4, 3>>(0);
        } else if (numChannels == 5) {
            KIS_ASSERT(alphaPos == 4);
            return createOptimizedClass<
                    KoAlphaMaskApplicatorFactoryImpl<
                        channels_type, 5, 4>>(0);
        } else if (numChannels == 2) {
            KIS_ASSERT(alphaPos == 1);
            return createOptimizedClass<
                    KoAlphaMaskApplicatorFactoryImpl<
                        channels_type, 2, 1>>(0);
        } else if (numChannels == 1) {
            KIS_ASSERT(alphaPos == 0);
            return createOptimizedClass<
                    KoAlphaMaskApplicatorFactoryImpl<
                        channels_type, 1, 0>>(0);
        } else {
            KIS_ASSERT(0);
        }

        return 0;
    }
};

KoAlphaMaskApplicatorBase *KoAlphaMaskApplicatorFactory::create(KoID depthId, int numChannels, int alphaPos)
{
    return channelTypeForColorDepthId<CreateApplicator>(depthId, numChannels, alphaPos);
}
