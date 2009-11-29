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

#include <KoImageResource.h>
#include <kis_debug.h>
#include <QLabel>
#include <QImage>
#include <QPushButton>
#include <QComboBox>
//Added by qt3to4:
#include <QPixmap>
#include <QShowEvent>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>

#include "kis_view2.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_pattern.h"

#include "kis_resource_server_provider.h"
#include "kis_paint_layer.h"

KisCustomPattern::KisCustomPattern(QWidget *parent, const char* name, const QString& caption, KisView2* view)
        : KisWdgCustomPattern(parent, name), m_view(view)
{
    Q_ASSERT(m_view);
    setWindowTitle(caption);

    m_pattern = 0;

    preview->setScaledContents(true);

    KoResourceServer<KisPattern>* rServer = KisResourceServerProvider::instance()->patternServer();
    m_rServerAdapter = new KoResourceServerAdapter<KisPattern>(rServer);

    connect(addButton, SIGNAL(pressed()), this, SLOT(slotAddPredefined()));
    connect(patternButton, SIGNAL(pressed()), this, SLOT(slotUsePattern()));
    connect(exportButton, SIGNAL(pressed()), this, SLOT(slotExport()));
}

KisCustomPattern::~KisCustomPattern()
{
    delete m_pattern;
    delete m_rServerAdapter;
}

void KisCustomPattern::showEvent(QShowEvent *)
{
    slotUpdateCurrentPattern(0);
}

void KisCustomPattern::slotUpdateCurrentPattern(int)
{
    delete m_pattern;
    m_pattern = 0;
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

                QPixmap scaledPixmap = QPixmap::fromImage(m_pattern->image());
                preview->setPixmap(scaledPixmap.scaled(scaledWidth, scaledHeight));
            } else {
                preview->setPixmap(QPixmap::fromImage(m_pattern->image()));
            }
        }
    }
}

void KisCustomPattern::slotExport()
{
    ;
}

void KisCustomPattern::slotAddPredefined()
{
    if (!m_pattern)
        return;

    // Save in the directory that is likely to be: ~/.kde/share/apps/krita/patterns
    // a unique file with this pattern name
    QString dir = KGlobal::dirs()->saveLocation("data", "krita/patterns");
    QString extension;

    QString tempFileName;
    {
        KTemporaryFile file;
        file.setPrefix(dir);
        file.setSuffix(".pat");
        file.setAutoRemove(false);
        file.open();
        tempFileName = file.fileName();
    }

    // Save it to that file
    m_pattern->setFilename(tempFileName);

    // Add it to the pattern server, so that it automatically gets to the mediators, and
    // so to the other pattern choosers can pick it up, if they want to
    if (m_rServerAdapter)
        m_rServerAdapter->addResource(m_pattern->clone());
}

void KisCustomPattern::slotUsePattern()
{
    if (!m_pattern)
        return;
    KisPattern* copy = m_pattern->clone();

    Q_CHECK_PTR(copy);

    emit(activatedResource(copy));
}

void KisCustomPattern::createPattern()
{
    KisImageWSP image = m_view->image();

    if (!image)
        return;

    m_pattern = new KisPattern(image->mergedImage().data(), 0, 0, image->width(), image->height());
}


#include "kis_custom_pattern.moc"
