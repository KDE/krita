/*
 *  SPDX-FileCopyrightText: 2023 killy |0veufOrever <80536642@qq.com>
 *  SPDX-FileCopyrightText: 2023 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSAMPLESCREENCOLOR_H
#define KISSAMPLESCREENCOLOR_H

#include <QVariant>

#include <KisActionPlugin.h>
#include "kis_types.h"

class KisScreenColorSampler;

class KisSampleScreenColor : public KisActionPlugin
{
    Q_OBJECT

public:
    KisSampleScreenColor(QObject *parent, const QVariantList &);
    ~KisSampleScreenColor() override;

private Q_SLOTS:
    void slotSampleScreenColor(bool sampleRealCanvas);

private:
    KisScreenColorSampler *m_screenColorSampler {nullptr};
    bool m_lastSampleRealCanvas;
};

#endif // KISSAMPLESCREENCOLOR_H
