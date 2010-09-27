/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_common_colors.h"
#include <QImage>
#include <QList>
#include <QPushButton>
#include <QColor>
#include <QRunnable>
#include <QThreadPool>
#include <QApplication>

#include <KConfig>
#include <KConfigGroup>
#include <KComponentData>
#include <KGlobal>

#include "KoColor.h"
#include "KoColorSpaceRegistry.h"
#include "kis_canvas2.h"
#include "kis_image.h"
#include "kis_config.h"
#include "kis_common_colors_recalculation_runner.h"


KisCommonColors::KisCommonColors(QWidget *parent) :
    KisColorPatches("commonColors", parent)
{
    m_reloadButton = new QPushButton();
    m_reloadButton->setIcon(KIcon("view-refresh"));
    connect(m_reloadButton, SIGNAL(clicked()), this, SLOT(recalculate()));
    
    QList<QWidget*> tmpList;
    tmpList.append(m_reloadButton);
    setAdditionalButtons(tmpList);
    updateSettings();

    m_recalculationTimer.setInterval(2000);
    m_recalculationTimer.setSingleShot(true);

    connect(&m_recalculationTimer, SIGNAL(timeout()),
            this,                 SLOT(recalculate()));
}

void KisCommonColors::setCanvas(KisCanvas2 *canvas)
{
    KisColorPatches::setCanvas(canvas);

    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");
    if(cfg.readEntry("commonColorsAutoUpdate", false)) {
        connect(m_canvas->image(),     SIGNAL(sigImageUpdated(const QRect &)),
                &m_recalculationTimer, SLOT(start()), Qt::UniqueConnection);
        connect(m_canvas->image(),     SIGNAL(sigImageUpdated(const QRect &)),
                this,                  SLOT(setDirty(const QRect &)), Qt::UniqueConnection);
    }
    m_imageCache = QImage(m_canvas->image()->size(), QImage::Format_ARGB32_Premultiplied);
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

    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");
    if(cfg.readEntry("commonColorsAutoUpdate", false)) {
        connect(m_canvas->image(),     SIGNAL(sigImageUpdated(const QRect &)),
                &m_recalculationTimer, SLOT(start()), Qt::UniqueConnection);
        connect(m_canvas->image(),     SIGNAL(sigImageUpdated(const QRect &)),
                this,                  SLOT(setDirty(const QRect &)), Qt::UniqueConnection);
    }
    else {
        disconnect(m_canvas->image(),     SIGNAL(sigImageUpdated(const QRect &)),
                &m_recalculationTimer, SLOT(start()));
        disconnect(m_canvas->image(),     SIGNAL(sigImageUpdated(const QRect &)),
                this,                  SLOT(setDirty(const QRect &)));
    }

    m_reloadButton->setEnabled(true);
}

void KisCommonColors::setColors(QList<KoColor> colors)
{
    QMutexLocker locker(&m_mutex);
    m_reloadButton->setEnabled(true);
    KisColorPatches::setColors(colors);
    locker.unlock();
}

void KisCommonColors::recalculate()
{
    if(m_canvas == 0) {
        return;
    }
    if(m_reloadButton->isEnabled()==false) {
        // on old computation is still running
        // try later to recalculate
        m_recalculationTimer.start();
        return;
    }
    m_reloadButton->setEnabled(false);
    qApp->processEvents();

    updateImageCache();

    KisCommonColorsRecalculationRunner* runner = new KisCommonColorsRecalculationRunner(m_imageCache, patchCount(), this);
    QThreadPool::globalInstance()->start(runner);
}

void KisCommonColors::setDirty(const QRect &rc)
{
    m_dirtyRegion+=rc;
}

void KisCommonColors::updateImageCache()
{
    KisImageWSP kisImage = m_canvas->image();
    KisConfig cfg;
    const KoColorProfile* profile = KoColorSpaceRegistry::instance()->profileByName(cfg.monitorProfile());

    QVector<QRect> rects = m_dirtyRegion.rects();
    foreach(QRect rect, rects) {
        kisImage->lock();
        QImage imagePart = kisImage->convertToQImage(rect, profile);
        kisImage->unlock();

        for(int i=0; i<imagePart.width(); i++) {
            for(int j=0; j<imagePart.height(); j++) {
                m_imageCache.setPixel(i+rect.x(), j+rect.y(), imagePart.pixel(i, j));
            }
        }
    }

    m_dirtyRegion = QRegion();
}
