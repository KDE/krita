/*
 *  Copyright (c) 2013 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_file_layer.h"

#include <QFile>

#include "kis_transform_worker.h"
#include "kis_filter_strategy.h"
#include "kis_doc2.h"
#include "kis_part2.h"
#include "kis_node_progress_proxy.h"
#include "kis_node_visitor.h"

#include <KoProgressUpdater.h>
#include <KoProgressProxy.h>

KisFileLayer::KisFileLayer(KisImageWSP image, const QString &filename, bool scaleToImageResolution, const QString &name, quint8 opacity)
    : KisExternalLayer(image, name, opacity)
    , m_part(new KisPart2(0))
    , m_doc(new KisDoc2(m_part))
    , m_filename(filename)
    , m_scaleToImageResolution(scaleToImageResolution)
{
    m_part->setDocument(m_doc);
    Q_ASSERT(QFile::exists(filename));
    if (QFile::exists(filename)) {
        m_fileWatcher.addPath(filename);
    }
    connect(&m_fileWatcher, SIGNAL(fileChanged(QString)), SLOT(reloadImage()));
    reloadImage();
}

KisFileLayer::~KisFileLayer()
{
    delete m_doc;
    delete m_part;
}

KisFileLayer::KisFileLayer(const KisFileLayer &rhs)
    : KisExternalLayer(rhs)
    , m_part(new KisPart2(0))
    , m_doc(new KisDoc2(m_part))
{
    m_part->setDocument(m_doc);
    Q_ASSERT(QFile::exists(rhs.m_filename));

    connect(&m_fileWatcher, SIGNAL(fileChanged(QString)), SLOT(reloadImage()));
    m_filename = rhs.m_filename;
    if (QFile::exists(m_filename)) {
        m_fileWatcher.addPath(m_filename);
    }

    m_scaleToImageResolution = rhs.m_scaleToImageResolution;
    reloadImage();
}

void KisFileLayer::resetCache()
{
    reloadImage();
}

const KoColorSpace *KisFileLayer::colorSpace() const
{
    return m_image->colorSpace();
}

KisPaintDeviceSP KisFileLayer::original() const
{
    return m_image;
}

KisPaintDeviceSP KisFileLayer::paintDevice() const
{
    return 0;
}

KoDocumentSectionModel::PropertyList KisFileLayer::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisLayer::sectionModelProperties();
    l << KoDocumentSectionModel::Property(i18n("File"), m_filename);
    return l;
}

void KisFileLayer::setFileName(const QString &filename)
{
    m_fileWatcher.removePath(m_filename);
    m_filename = filename;
    m_fileWatcher.addPath(m_filename);
    reloadImage();
}

QString KisFileLayer::fileName() const
{
    return m_filename;
}

void KisFileLayer::setScaleToImageResolution(bool scale)
{
    m_scaleToImageResolution = scale;
    reloadImage();
}

bool KisFileLayer::scaleToImageResolution() const
{
    return m_scaleToImageResolution;
}


void KisFileLayer::reloadImage()
{
    m_doc->openUrl(m_filename);
    KisImageWSP importedImage = m_doc->image();
    m_image = importedImage->projection();

    if (m_scaleToImageResolution && (image()->xRes() != importedImage->xRes()
                                     || image()->yRes() != importedImage->yRes()))
    {
        qreal xscale = image()->xRes() / importedImage->xRes();
        qreal yscale = image()->yRes() / importedImage->yRes();
        KisTransformWorker worker(m_image, xscale, yscale, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, KisFilterStrategyRegistry::instance()->get("Bicubic"));
        worker.run();
    }


    setDirty();
}

KisNodeSP KisFileLayer::clone() const
{
    return KisNodeSP(new KisFileLayer(*this));
}

bool KisFileLayer::allowAsChild(KisNodeSP node) const
{
    return node->inherits("KisMask");
}

bool KisFileLayer::accept(KisNodeVisitor& visitor)
{
    return visitor.visit(this);
}

void KisFileLayer::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    return visitor.visit(this, undoAdapter);
}

