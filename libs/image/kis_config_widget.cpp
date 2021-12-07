/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_config_widget.h"
#include "kis_debug.h"
#include <QTimer>

KisConfigWidget::KisConfigWidget(QWidget * parent, Qt::WindowFlags f, int delay)
        : QWidget(parent, f)
        , m_compressor(delay, KisSignalCompressor::FIRST_ACTIVE)
{
    connect(this, SIGNAL(sigConfigurationItemChanged()), SLOT(slotConfigChanged()));
    connect(&m_compressor, SIGNAL(timeout()), SIGNAL(sigConfigurationUpdated()));
}

KisConfigWidget::~KisConfigWidget()
{
}

void KisConfigWidget::slotConfigChanged()
{
    if (!signalsBlocked()) {
        m_compressor.start();
    }
}

/// TODO: remove this method from KisConfigWidget, since
/// KisViewManager is from kritaui, but we are in
/// kritaimage
void KisConfigWidget::setView(KisViewManager *view)
{
    if (!view) {
        warnKrita << "KisConfigWidget::setView has got view == 0. That's a bug! Please report it!";
    }
}

void KisConfigWidget::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    m_canvasResourcesInterface = canvasResourcesInterface;
}

KoCanvasResourcesInterfaceSP KisConfigWidget::canvasResourcesInterface() const
{
    return m_canvasResourcesInterface;
}
