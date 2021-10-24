/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_common_colors.h"
#include <QImage>
#include <QList>
#include <QToolButton>
#include <QColor>
#include <QRunnable>
#include <QThreadPool>
#include <QApplication>

#include <kconfig.h>
#include <kconfiggroup.h>

#include <klocalizedstring.h>

#include <kis_icon.h>
#include "KoColor.h"
#include "kis_canvas2.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_config.h"
#include "kis_common_colors_recalculation_runner.h"


KisCommonColors::KisCommonColors(QWidget *parent) :
    KisColorPatches("commonColors", parent)
{
    m_reloadButton = new QToolButton();
    m_reloadButton->setIcon(KisIconUtils::loadIcon("reload-preset-16")); //small reload icon
    m_reloadButton->setToolTip(i18n("Create a list of colors from the image"));
    m_reloadButton->setAutoRaise(true);
    connect(m_reloadButton, SIGNAL(clicked()), this, SLOT(recalculate()));

    QList<QWidget*> tmpList;
    tmpList.append(m_reloadButton);
    setAdditionalButtons(tmpList);
    updateSettings();

    m_recalculationTimer.setInterval(2000);
    m_recalculationTimer.setSingleShot(true);

    connect(&m_recalculationTimer, SIGNAL(timeout()),
            this,                  SLOT(recalculate()));

}

void KisCommonColors::setCanvas(KisCanvas2 *canvas)
{
    KisColorPatches::setCanvas(canvas);

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    if (cfg.readEntry("commonColorsAutoUpdate", false)) {
        if (m_image) {
            m_image->disconnect(this);
        }
        if (m_canvas) {
            connect(m_canvas->image(), SIGNAL(sigImageUpdated(QRect)),
                    &m_recalculationTimer, SLOT(start()), Qt::UniqueConnection);
            m_image = m_canvas->image();
        }
        else {
            m_image = 0;
        }
    }
}

KisColorSelectorBase* KisCommonColors::createPopup() const
{
    KisCommonColors* ret = new KisCommonColors();
    ret->setCanvas(m_canvas);
    ret->setColors(colors());
    return ret;
}

void KisCommonColors::updateSettings()
{
    KisColorPatches::updateSettings();

    if(!(m_canvas && m_canvas->image()))
        return;

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    if (cfg.readEntry("commonColorsAutoUpdate", false)) {
        connect(m_canvas->image(), SIGNAL(sigImageUpdated(QRect)),
                &m_recalculationTimer, SLOT(start()), Qt::UniqueConnection);
    }
    else {
        disconnect(m_canvas->image(), SIGNAL(sigImageUpdated(QRect)),
                &m_recalculationTimer, SLOT(start()));
    }

    m_reloadButton->setEnabled(true);
}

void KisCommonColors::setColors(QList<KoColor> colors)
{
    QMutexLocker locker(&m_mutex);
    KisColorPatches::setColors(colors);
    m_reloadButton->setEnabled(true);
    m_calculatedColors = colors;
}

void KisCommonColors::recalculate()
{
    if (!m_canvas) {
        return;
    }
    if(!m_reloadButton->isEnabled()) {
        // on old computation is still running
        // try later to recalculate
        m_recalculationTimer.start();
        return;
    }
    m_reloadButton->setEnabled(false);
    qApp->processEvents();

    KisImageWSP kisImage = m_canvas->image();

    QImage image = kisImage->projection()->createThumbnail(1024, 1024, kisImage->bounds(), 1, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());

    KisCommonColorsRecalculationRunner* runner = new KisCommonColorsRecalculationRunner(image, patchCount(), this);
    QThreadPool::globalInstance()->start(runner);
}

