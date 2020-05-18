/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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
