/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMASKINGBRUSHCOMPOSITEOPFACTORY_H
#define KISMASKINGBRUSHCOMPOSITEOPFACTORY_H

#include <QtGlobal>
#include <KoChannelInfo.h>

#include "kritaui_export.h"

class KisMaskingBrushCompositeOpBase;

class KRITAUI_EXPORT KisMaskingBrushCompositeOpFactory
{
public:
    static KisMaskingBrushCompositeOpBase* create(const QString &id, KoChannelInfo::enumChannelValueType channelType, int pixelSize, int alphaOffset);
    static KisMaskingBrushCompositeOpBase* create(const QString &id, KoChannelInfo::enumChannelValueType channelType, int pixelSize, int alphaOffset, qreal strength);

    static KisMaskingBrushCompositeOpBase* createForAlphaSrc(const QString &id, KoChannelInfo::enumChannelValueType channelType, int pixelSize, int alphaOffset);
    static KisMaskingBrushCompositeOpBase* createForAlphaSrc(const QString &id, KoChannelInfo::enumChannelValueType channelType, int pixelSize, int alphaOffset, qreal strength);

    static QStringList supportedCompositeOpIds();
};

#endif // KISMASKINGBRUSHCOMPOSITEOPFACTORY_H
