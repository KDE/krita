/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSTROKECOMPATIBILITYINFO_H
#define KISSTROKECOMPATIBILITYINFO_H

#include "kritaui_export.h"
#include <boost/operators.hpp>

#include <QString>
#include <KoColor.h>
#include <KoResourceSignature.h>
#include <KoCompositeOpRegistry.h>
#include <QUuid>


class KisResourcesSnapshot;


struct KRITAUI_EXPORT KisStrokeCompatibilityInfo : public boost::equality_comparable<KisStrokeCompatibilityInfo>
{
    KisStrokeCompatibilityInfo();
    KisStrokeCompatibilityInfo(KisResourcesSnapshot &resourcesSnapshot);

    friend bool operator==(const KisStrokeCompatibilityInfo &lhs, const KisStrokeCompatibilityInfo &rhs);

    KoColor currentFgColor;
    KoColor currentBgColor;
    KoResourceSignature currentPattern;
    KoResourceSignature currentGradient;
    KoResourceSignature currentPreset;
    QString currentGeneratorXml;
    QUuid currentNode;

    qreal opacity {OPACITY_OPAQUE_F};
    QString compositeOpId {COMPOSITE_OVER};

    QBitArray channelLockFlags;
};

#endif // KISSTROKECOMPATIBILITYINFO_H
