/*
 * SPDX-FileCopyrightText: 2021 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGCommonColorSet.h"

#include "WGCommonColorsCalculationRunner.h"
#include <QThreadPool>

WGCommonColorSet::WGCommonColorSet(QObject *parent)
    : KisUniqueColorSet(parent)
    , m_commonColors(new QVector<KoColor>)
{
    m_updateTimer.setInterval(2000);
    m_updateTimer.setSingleShot(true);
    connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateColors()));
}

void WGCommonColorSet::setImage(KisImageSP image)
{
    if (m_autoUpdate) {
        if (m_image) {
            disconnect(m_image, SIGNAL(sigImageUpdated(QRect)), &m_updateTimer, SLOT(start()));
        }
        if (image) {
            connect(image, SIGNAL(sigImageUpdated(QRect)), &m_updateTimer, SLOT(start()), Qt::UniqueConnection);
            m_updateTimer.start();
        }
    }
     m_image = image;
}

void WGCommonColorSet::setAutoUpdate(bool enabled)
{
    if (enabled == m_autoUpdate) {
        return;
    }

    m_autoUpdate = enabled;

    if (m_image) {
        if (enabled) {
            connect(m_image, SIGNAL(sigImageUpdated(QRect)), &m_updateTimer, SLOT(start()), Qt::UniqueConnection);
        }
        else {
            disconnect(m_image, SIGNAL(sigImageUpdated(QRect)), &m_updateTimer, SLOT(start()));
        }
    }
}

void WGCommonColorSet::slotUpdateColors()
{
    if (!m_image) {
        return;
    }

    if(!m_idle) {
        // Previous computation is still running, try again later
        m_updateTimer.start();
        return;
    }

    m_idle = false;
    Q_EMIT sigIdle(false);

    m_commonColors->clear();
    WGCommonColorsCalculationRunner* runner = new WGCommonColorsCalculationRunner(m_image, m_numColors, m_commonColors);
    connect(runner, SIGNAL(sigDone()), this, SLOT(slotCalculationDone()));
    QThreadPool::globalInstance()->start(runner);
}

void WGCommonColorSet::slotCalculationDone()
{
    blockSignals(true);

    clear();
    for (const KoColor &color : qAsConst(*m_commonColors)) {
        addColor(color);
    }

    blockSignals(false);
    Q_EMIT sigReset();
    m_idle = true;
    Q_EMIT sigIdle(true);
}
