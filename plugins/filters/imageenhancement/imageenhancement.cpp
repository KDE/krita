/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "imageenhancement.h"
#include <stdlib.h>
#include <vector>

#include <QPoint>

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include "kis_simple_noise_reducer.h"
#include "kis_wavelet_noise_reduction.h"

K_PLUGIN_FACTORY_WITH_JSON(KritaImageEnhancementFactory, "kritaimageenhancement.json", registerPlugin<KritaImageEnhancement>();)

KritaImageEnhancement::KritaImageEnhancement(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisSimpleNoiseReducer());
    KisFilterRegistry::instance()->add(new KisWaveletNoiseReduction());
}

KritaImageEnhancement::~KritaImageEnhancement()
{
}

#include "imageenhancement.moc"
