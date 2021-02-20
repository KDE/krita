/*
 *  SPDX-FileCopyrightText: 2007 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_config_notifier.h"

#include <QGlobalStatic>

#include <kis_debug.h>
#include "kis_signal_compressor.h"

Q_GLOBAL_STATIC(KisConfigNotifier, s_instance)

struct KisConfigNotifier::Private
{
    Private() : dropFramesModeCompressor(300, KisSignalCompressor::FIRST_ACTIVE) {}

    KisSignalCompressor dropFramesModeCompressor;
};

KisConfigNotifier::KisConfigNotifier()
    : m_d(new Private)
{
    connect(&m_d->dropFramesModeCompressor, SIGNAL(timeout()), SIGNAL(dropFramesModeChanged()));
}

KisConfigNotifier::~KisConfigNotifier()
{
    dbgRegistry << "deleting KisConfigNotifier";
}

KisConfigNotifier *KisConfigNotifier::instance()
{
    return s_instance;
}

void KisConfigNotifier::notifyConfigChanged(void)
{
    emit configChanged();
}

void KisConfigNotifier::notifyDropFramesModeChanged()
{
    m_d->dropFramesModeCompressor.start();
}

void KisConfigNotifier::notifyPixelGridModeChanged()
{
    emit pixelGridModeChanged();
}
