/*
 *  SPDX-FileCopyrightText: 2023 killy |0veufOrever <80536642@qq.com>
 *  SPDX-FileCopyrightText: 2023 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSampleScreenColor.h"

#include <KisScreenColorSampler.h>
#include <kpluginfactory.h>
#include <kis_action.h>
#include <kis_canvas_resource_provider.h>
#include <KisViewManager.h>

K_PLUGIN_FACTORY_WITH_JSON(KisSampleScreenColorFactory, "kritasamplescreencolor.json", registerPlugin<KisSampleScreenColor>();)

KisSampleScreenColor::KisSampleScreenColor(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    KisAction *action = createAction("sample_screen_color");
    connect(action, &QAction::triggered, [this](){ slotSampleScreenColor(false); });

    action = createAction("sample_screen_color_real_canvas");
    connect(action, &QAction::triggered, [this](){ slotSampleScreenColor(true); });
}

KisSampleScreenColor::~KisSampleScreenColor()
{}

void KisSampleScreenColor::slotSampleScreenColor(bool sampleRealCanvas)
{
    if (m_screenColorSampler) {
        // The action will cancel the previous one if it is still active
        m_screenColorSampler->cancel();
        // If the new action type is the same as the previous, cancelling is
        // enough, so we return. Otherwise, a new operation is started 
        if (sampleRealCanvas == m_lastSampleRealCanvas) {
            return;
        }
    }

    m_lastSampleRealCanvas = sampleRealCanvas;
    m_screenColorSampler = new KisScreenColorSampler(false);
    m_screenColorSampler->setPerformRealColorSamplingOfCanvas(sampleRealCanvas);
    m_screenColorSampler->setCurrentColor(viewManager()->canvasResourceProvider()->fgColor());
    // screenColorSampler is a temporary top level widget own by no other
    // QObject, so it must be automatically deleted when it is closed 
    m_screenColorSampler->setAttribute(Qt::WA_DeleteOnClose);
    connect(m_screenColorSampler, &KisScreenColorSampler::sigNewColorSampled, this,
        [this](KoColor sampledColor)
        {
            viewManager()->canvasResourceProvider()->slotSetFGColor(sampledColor);
            m_screenColorSampler->close();
            m_screenColorSampler = nullptr;
        }
    );
    connect(m_screenColorSampler, &KisScreenColorSampler::sigNewColorHovered, this,
        [this](KoColor sampledColor)
        {
            viewManager()->canvasResourceProvider()->slotSetFGColor(sampledColor);
        }
    );
    m_screenColorSampler->sampleScreenColor();
}

#include "KisSampleScreenColor.moc"
