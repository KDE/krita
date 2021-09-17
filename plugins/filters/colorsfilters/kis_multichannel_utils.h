/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2021 Deif Lou <giniba@gmail.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KIS_MULTICHANNEL_FILTER_UTILS
#define KIS_MULTICHANNEL_FILTER_UTILS

#include <QtGlobal>
#include <QVector>
#include <QList>

#include "virtual_channel_info.h"

class KoColorSpace;
class KoColorTransformation;

namespace KisMultiChannelUtils {

/**
 * Get a list of adjustable channels for the color space.
 * If maxChannels is non-negative, the number of channels is capped to the number. This is useful configurations
 * from older documents (created in versions which supported fewer channels).
 */
QVector<VirtualChannelInfo> getVirtualChannels(const KoColorSpace *cs,
                                               int maxChannels = -1,
                                               bool supportsLightness = true,
                                               bool supportsHue = true,
                                               bool supportSaturation = true);

int findChannel(const QVector<VirtualChannelInfo> &virtualChannels, const VirtualChannelInfo::Type &channelType);

/**
 * @brief Create a composed per channel transformation object from the set of given transfers
 * @param cs a colorspace
 * @param transfers A collection of transfer luts
 * @param transferIsIdentity A collection of bools that indicate if the
 *        corresponding transfer has no effect (maps the input to itself)
 */
KoColorTransformation* createPerChannelTransformationFromTransfers(const KoColorSpace *cs,
                                                                   const QVector<QVector<quint16>> &transfers,
                                                                   const QList<bool> &transferIsIdentity);

}

#endif
