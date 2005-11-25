/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <koImageResource.h>
#include <kdebug.h>
#include <qlabel.h>
#include <qimage.h>
#include <qpushbutton.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ktempfile.h>

#include "kis_view.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_brush.h"
#include "kis_custom_brush.h"
#include "kis_resource_mediator.h"
#include "kis_resourceserver.h"

KisCustomBrush::KisCustomBrush(QWidget *parent, const char* name, const QString& caption, KisView* view)
    : KisWdgCustomBrush(parent, name), m_view(view)
{
    m_mediator = 0;
    setCaption(caption);

    m_brush = new KisBrush(m_view -> getCanvasSubject() -> currentImg());
    preview -> setScaledContents(true);
    preview -> setPixmap(QPixmap(m_brush -> img()));

    connect(addButton, SIGNAL(pressed()), this, SLOT(slotAddPredefined()));
    connect(brushButton, SIGNAL(pressed()), this, SLOT(slotUseBrush()));
    connect(exportButton, SIGNAL(pressed()), this, SLOT(slotExport()));
}

KisCustomBrush::~KisCustomBrush() {
    delete m_brush;
}

void KisCustomBrush::showEvent(QShowEvent *) {
    delete m_brush;
    m_brush = new KisBrush(m_view -> getCanvasSubject() -> currentImg());
    preview -> setPixmap(QPixmap(m_brush -> img()));
}

void KisCustomBrush::slotExport() {
    ;
}

void KisCustomBrush::slotAddPredefined() {
    // Save in the directory that is likely to be: ~/.kde/share/apps/krita/brushes
    // a unique file with this brushname
    QString dir = KGlobal::dirs() -> saveLocation("data", "krita/brushes");
    KTempFile file(dir, ".gbr");
    file.close(); // If we don't, and brush -> save first, it might get truncated!

    // Save it to that file 
    m_brush -> setFilename(file.name());
    m_brush -> save();

    // Add it to the brush server, so that it automatically gets to the mediators, and
    // so to the other brush choosers can pick it up, if they want to
    if (m_server)
        m_server -> addResource(new KisBrush(m_brush -> img(), m_brush -> name()));
}

void KisCustomBrush::slotUseBrush() {
    KisBrush* copy = new KisBrush(m_brush -> img(), m_brush -> name());

    Q_CHECK_PTR(copy);

    emit(activatedResource(copy));
}


#include "kis_custom_brush.moc"
