/*
 *  SPDX-FileCopyrightText: 2006 Bart Coppens <kde@bartcoppens.be>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <QSharedPointer>
#include <QFileInfo>
#include <KoFileDialog.h>
#include <QMessageBox>

#include "KisDocument.h"
#include "KisViewManager.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "kis_painter.h"

#include <kis_debug.h>
#include "KisResourceServerProvider.h"
#include <KisResourceLoaderRegistry.h>
#include "kis_paint_layer.h"
#include <KisResourceUserOperations.h>


KisCustomPattern::KisCustomPattern(QWidget *parent, const char* name, const QString& caption, KisViewManager* view)
    : KisWdgCustomPattern(parent, name)
    , m_view(view)
{
    Q_ASSERT(m_view);
    setWindowTitle(caption);

    m_pattern = 0;

    preview->setScaledContents(true);

    m_rServer = KoResourceServerProvider::instance()->patternServer();

    connect(addButton, SIGNAL(pressed()), this, SLOT(slotAddPredefined()));
    connect(patternButton, SIGNAL(pressed()), this, SLOT(slotUsePattern()));
    connect(updateButton, SIGNAL(pressed()), this, SLOT(slotUpdateCurrentPattern()));
    connect(cmbSource, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUpdateCurrentPattern()));

    lblWarning->setVisible(false);
    slotUpdateCurrentPattern();
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
    if (!m_pattern) return;

    // Save in the directory that is likely to be: ~/.kde/share/apps/krita/patterns
    // a unique file with this pattern name
    QString dir = KoResourceServerProvider::instance()->patternServer()->saveLocation();

    KoFileDialog dlg(this, KoFileDialog::SaveFile, "KisCustomPattern::slotAddPredefined");
    dlg.setDefaultDir(dir + "/" + m_pattern->name() + ".pat");
    dlg.setMimeTypeFilters(KisResourceLoaderRegistry::instance()->mimeTypes(ResourceType::Patterns));
    dlg.setCaption(i18n("Add to Predefined Patterns"));

    QString filename = dlg.filename();
    bool hadToChangeFilename = false;

    QFileInfo fi(filename);
    if (fi.suffix().isEmpty()) {
        fi.setFile(fi.baseName() + m_pattern->defaultFileExtension());
        hadToChangeFilename = true;
    }

    if (fi.baseName() != m_pattern->name()) {
        m_pattern->setName(fi.baseName());
    }

    bool overwrite = false;
    if (fi.exists()) {
        if (hadToChangeFilename) { // if not, the File Dialog would show the warning
            if (QMessageBox::warning(this,  i18nc("@title:window", "Krita"), i18n("This pattern already exists. Do you want to overwrite it?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                overwrite = true;
            }
        }
    }

    if (!filename.isEmpty()) {
        m_pattern->setFilename(fi.fileName()); // to make sure we include the suffix added earlier
        if (!fi.exists()) {
            if (!KisResourceUserOperations::addResourceWithUserInput(this, m_pattern->clone().dynamicCast<KoPattern>())) {
                qWarning() << "Could not add pattern with filename" << filename;
            }
            else {
                emit patternAdded(m_pattern);
            }
        }
        else if (overwrite) {
            if (!KisResourceUserOperations::updateResourceWithUserInput(this, m_pattern->clone().dynamicCast<KoPattern>())) {
                qWarning() << "Could not add pattern with filename" << filename;
            }
            else {
                emit patternUpdated(m_pattern);
            }
        }
    }
}

void KisCustomPattern::slotUsePattern()
{
    if (!m_pattern)
        return;
    KoPatternSP copy = m_pattern->clone().dynamicCast<KoPattern>();
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
        image->barrierLock();
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
    if (size.height() > 1000 || size.width() > 1000) {
        lblWarning->setVisible(true);
        size.scale(1000, 1000, Qt::KeepAspectRatio);
    }
    else {
        lblWarning->setVisible(false);
    }

    QString dir = KoResourceServerProvider::instance()->patternServer()->saveLocation();
    m_pattern = KoPatternSP(new KoPattern(cache->createThumbnail(size.width(), size.height(), rc, /*oversample*/ 1,
                                                                 KoColorConversionTransformation::internalRenderingIntent(),
                                                                 KoColorConversionTransformation::internalConversionFlags()), name, dir));
}


