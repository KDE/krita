/*
 *  Copyright (c) 2006 Bart Coppens <kde@bartcoppens.be>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_custom_pattern.h"

#include <KoResourceServerProvider.h>
#include <resources/KoPattern.h>

#include <QLabel>
#include <QImage>
#include <QPushButton>
#include <QComboBox>
#include <QPixmap>
#include <QShowEvent>


#include <QTemporaryFile>

#include "KisDocument.h"
#include "KisViewManager.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_painter.h"

#include <kis_debug.h>
#include "KisResourceServerProvider.h"
#include "kis_paint_layer.h"

KisCustomPattern::KisCustomPattern(QWidget *parent, const char* name, const QString& caption, KisViewManager* view)
    : KisWdgCustomPattern(parent, name), m_view(view)
{
    Q_ASSERT(m_view);
    setWindowTitle(caption);

    m_pattern = 0;

    preview->setScaledContents(true);

    KoResourceServer<KoPattern>* rServer = KoResourceServerProvider::instance()->patternServer();
    m_rServerAdapter = QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoPattern>(rServer));

    connect(addButton, SIGNAL(pressed()), this, SLOT(slotAddPredefined()));
    connect(patternButton, SIGNAL(pressed()), this, SLOT(slotUsePattern()));
    connect(updateButton, SIGNAL(pressed()), this, SLOT(slotUpdateCurrentPattern()));
    connect(cmbSource, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUpdateCurrentPattern()));
}

KisCustomPattern::~KisCustomPattern()
{
    m_pattern.clear();
}

void KisCustomPattern::slotUpdateCurrentPattern()
{
    m_pattern.clear();
    if (m_view && m_view->image()) {
        createPattern();
        if (m_pattern) {
            const qint32 maxSize = 150;
            if ((m_pattern->width() > maxSize) || (m_pattern->height() > maxSize)) {
                float aspectRatio = (float)m_pattern->width() / m_pattern->height();
                qint32 scaledWidth, scaledHeight;

                if (m_pattern->width() > m_pattern->height()) {
                    scaledWidth = maxSize;
                    scaledHeight = maxSize / aspectRatio;
                } else {
                    scaledWidth = maxSize * aspectRatio;
                    scaledHeight = maxSize;
                }

                if (scaledWidth == 0) scaledWidth++;
                if (scaledHeight == 0) scaledHeight++;

                QPixmap scaledPixmap = QPixmap::fromImage(m_pattern->pattern());
                preview->setPixmap(scaledPixmap.scaled(scaledWidth, scaledHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                preview->setPixmap(QPixmap::fromImage(m_pattern->pattern()));
            }
        }
    }
}

void KisCustomPattern::slotAddPredefined()
{
    if (!m_pattern)
        return;

    // Save in the directory that is likely to be: ~/.kde/share/apps/krita/patterns
    // a unique file with this pattern name
    QString dir = KoResourceServerProvider::instance()->patternServer()->saveLocation();
    QString extension;

    QString tempFileName;
    {
        QTemporaryFile file(dir +  QLatin1String("/krita_XXXXXX") + QLatin1String(".pat") );
        file.setAutoRemove(false);
        file.open();
        tempFileName = file.fileName();
    }

    // Save it to that file
    m_pattern->setFilename(tempFileName);

    // Add it to the pattern server, so that it automatically gets to the mediators, and
    // so to the other pattern choosers can pick it up, if they want to
    m_rServerAdapter->addResource(m_pattern->clone());
}

void KisCustomPattern::slotUsePattern()
{
    if (!m_pattern)
        return;
    KoPatternSP copy = m_pattern->clone();
    emit(activatedResource(copy));
}

void KisCustomPattern::createPattern()
{
    if (!m_view) return;

    KisPaintDeviceSP dev;
    KisPaintDeviceSP cache;
    QString name;
    KisImageWSP image = m_view->image();
    if (!image) return;
    QRect rc = image->bounds();

    if (cmbSource->currentIndex() == 0) {
        dev = m_view->activeNode()->projection();
        name = m_view->activeNode()->name();
        QRect rc2 = dev->exactBounds();
        rc = rc.intersected(rc2);
    }
    else {
        image->lock();
        dev = image->projection();
        image->unlock();
        name = image->objectName();
    }
    if (!dev) return;

    if(m_view->selection()) {
        KisSelectionSP selection = m_view->selection();
        QRect selectionRect = selection->selectedExactRect();
        cache = dev->createCompositionSourceDevice();
        KisPainter gc(cache);
        gc.setSelection(selection);
        gc.bitBlt(selectionRect.topLeft(), dev, selectionRect);
        rc = selectionRect;
    } else {
        cache = dev;
    }
    if (!cache) return;


    // warn when creating large patterns

    QSize size = rc.size();
    if (size.width() > 1000 || size.height() > 1000) {
        lblWarning->setText(i18n("The current image is too big to create a pattern. "
                                 "The pattern will be scaled down."));
        size.scale(1000, 1000, Qt::KeepAspectRatio);
    }

    QString dir = KoResourceServerProvider::instance()->patternServer()->saveLocation();
    m_pattern = KoPatternSP(new KoPattern(cache->createThumbnail(size.width(), size.height(), rc, /*oversample*/ 1,
                                                                 KoColorConversionTransformation::internalRenderingIntent(),
                                                                 KoColorConversionTransformation::internalConversionFlags()), name, dir));
}


