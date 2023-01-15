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
    KisScreenColorSampler *screenColorSampler = new KisScreenColorSampler(false);
    screenColorSampler->setPerformRealColorSamplingOfCanvas(sampleRealCanvas);
    screenColorSampler->setCurrentColor(viewManager()->canvasResourceProvider()->fgColor());
    // screenColorSampler is a temporary top level widget own by no other
    // QObject, so it must be automatically deleted when it is closed 
    screenColorSampler->setAttribute(Qt::WA_DeleteOnClose);
    connect(screenColorSampler, &KisScreenColorSampler::sigNewColorSampled,
        [this, screenColorSampler](KoColor sampledColor)
        {
            viewManager()->canvasResourceProvider()->slotSetFGColor(sampledColor);
            screenColorSampler->close();
        }
    );
    connect(screenColorSampler, &KisScreenColorSampler::sigNewColorHovered,
        [this, screenColorSampler](KoColor sampledColor)
        {
            viewManager()->canvasResourceProvider()->slotSetFGColor(sampledColor);
        }
    );
    screenColorSampler->sampleScreenColor();
}

#include "KisSampleScreenColor.moc"
