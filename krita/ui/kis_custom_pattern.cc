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

#include <KoImageResource.h>
#include <kdebug.h>
#include <QLabel>
#include <QImage>
#include <QPushButton>
#include <QComboBox>
//Added by qt3to4:
#include <QPixmap>
#include <QShowEvent>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ktempfile.h>

#include "kis_view.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_pattern.h"
#include "kis_custom_pattern.h"
#include "kis_resource_mediator.h"
#include "kis_resourceserver.h"
#include "kis_paint_layer.h"

KisCustomPattern::KisCustomPattern(QWidget *parent, const char* name, const QString& caption, KisView* view)
    : KisWdgCustomPattern(parent, name), m_view(view)
{
    Q_ASSERT(m_view);
    m_mediator = 0;
    setWindowTitle(caption);

    m_pattern = 0;

    preview->setScaledContents(true);

    connect(addButton, SIGNAL(pressed()), this, SLOT(slotAddPredefined()));
    connect(patternButton, SIGNAL(pressed()), this, SLOT(slotUsePattern()));
    connect(exportButton, SIGNAL(pressed()), this, SLOT(slotExport()));
}

KisCustomPattern::~KisCustomPattern() {
    delete m_pattern;
}

void KisCustomPattern::showEvent(QShowEvent *) {
    slotUpdateCurrentPattern(0);
}

void KisCustomPattern::slotUpdateCurrentPattern(int) {
    delete m_pattern;
    if (m_view->canvasSubject() && m_view->canvasSubject()->currentImg()) {
        createPattern();
        preview->setPixmap(QPixmap::fromImage(m_pattern->img()));
    } else {
        m_pattern = 0;
    }
}

void KisCustomPattern::slotExport() {
    ;
}

void KisCustomPattern::slotAddPredefined() {
    if (!m_pattern)
        return;

    // Save in the directory that is likely to be: ~/.kde/share/apps/krita/patterns
    // a unique file with this pattern name
    QString dir = KGlobal::dirs()->saveLocation("data", "krita/patterns");
    QString extension;

    KTempFile file(dir, ".pat");
    file.close(); // If we don't, and pattern->save first, it might get truncated!

    // Save it to that file
    m_pattern->setFilename(file.name());

    // Add it to the pattern server, so that it automatically gets to the mediators, and
    // so to the other pattern choosers can pick it up, if they want to
    if (m_server)
        m_server->addResource(m_pattern->clone());
}

void KisCustomPattern::slotUsePattern() {
    if (!m_pattern)
        return;
    KisPattern* copy = m_pattern->clone();

    Q_CHECK_PTR(copy);

    emit(activatedResource(copy));
}

void KisCustomPattern::createPattern() {
    KisImageSP img = m_view->canvasSubject()->currentImg();

    if (!img)
        return;

    m_pattern = new KisPattern(img->mergedImage().data(), 0, 0, img->width(), img->height());
}


#include "kis_custom_pattern.moc"
