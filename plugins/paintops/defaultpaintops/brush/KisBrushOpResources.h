/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISBRUSHOPRESOURCES_H
#define KISBRUSHOPRESOURCES_H

#include "KisDabCacheUtils.h"

#include <QScopedPointer>

class KisPainter;
class KisPaintInformation;


class KisBrushOpResources : public KisDabCacheUtils::DabRenderingResources
{
public:
    KisBrushOpResources(const KisPaintOpSettingsSP settings, KisPainter *painter);
    ~KisBrushOpResources() override;

    void syncResourcesToSeqNo(int seqNo, const KisPaintInformation &info) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISBRUSHOPRESOURCES_H
