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
#include <QFileInfo>

#include "kis_transform_worker.h"
#include "kis_filter_strategy.h"
#include "kis_node_progress_proxy.h"
#include "kis_node_visitor.h"
#include "kis_image.h"
#include "kis_types.h"
#include "commands_new/kis_node_move_command2.h"
#include "kis_default_bounds.h"
#include "kis_layer_properties_icons.h"
#include <KisPart.h>
#include <KisDocument.h>
#include <QDir>


KisFileLayer::KisFileLayer(KisImageWSP image, const QString &name, quint8 opacity)
    : KisExternalLayer(image, name, opacity)
{
    /**
     * Set default paint device for a layer. It will be used in case
     * the file does not exist anymore. Or course, this can happen only
     * in the failing execution path.
     */
    m_paintDevice = new KisPaintDevice(image->colorSpace());
    m_paintDevice->setDefaultBounds(new KisDefaultBounds(image));

    connect(&m_loader, SIGNAL(loadingFinished(KisPaintDeviceSP,int,int)), SLOT(slotLoadingFinished(KisPaintDeviceSP,int,int)));
}

KisFileLayer::KisFileLayer(KisImageWSP image, const QString &basePath, const QString &filename, ScalingMethod scaleToImageResolution, const QString &name, quint8 opacity)
    : KisExternalLayer(image, name, opacity)
    , m_basePath(basePath)
    , m_filename(filename)
    , m_scalingMethod(scaleToImageResolution)
{
    /**
     * Set default paint device for a layer. It will be used in case
     * the file does not exist anymore. Or course, this can happen only
     * in the failing execution path.
     */
    m_paintDevice = new KisPaintDevice(image->colorSpace());
    m_paintDevice->setDefaultBounds(new KisDefaultBounds(image));

    connect(&m_loader, SIGNAL(loadingFinished(KisPaintDeviceSP,int,int)), SLOT(slotLoadingFinished(KisPaintDeviceSP,int,int)));

    QFileInfo fi(path());
    if (fi.exists()) {
        m_loader.setPath(path());
        m_loader.reloadImage();
    }
}

KisFileLayer::~KisFileLayer()
{
}

KisFileLayer::KisFileLayer(const KisFileLayer &rhs)
    : KisExternalLayer(rhs)
{
    m_basePath = rhs.m_basePath;
    m_filename = rhs.m_filename;
    KIS_SAFE_ASSERT_RECOVER_NOOP(QFile::exists(path()));

    m_scalingMethod = rhs.m_scalingMethod;

    m_paintDevice = new KisPaintDevice(*rhs.m_paintDevice);

    connect(&m_loader, SIGNAL(loadingFinished(KisPaintDeviceSP,int,int)), SLOT(slotLoadingFinished(KisPaintDeviceSP,int,int)));
    m_loader.setPath(path());
}

QIcon KisFileLayer::icon() const
{
    return KisIconUtils::loadIcon("fileLayer");
}

void KisFileLayer::resetCache()
{
    m_loader.reloadImage();
}

const KoColorSpace *KisFileLayer::colorSpace() const
{
    return m_paintDevice->colorSpace();
}

KisPaintDeviceSP KisFileLayer::original() const
{
    return m_paintDevice;
}

KisPaintDeviceSP KisFileLayer::paintDevice() const
{
    return 0;
}

void KisFileLayer::setSectionModelProperties(const KisBaseNode::PropertyList &properties)
{
    KisBaseNode::setSectionModelProperties(properties);
    Q_FOREACH (const KisBaseNode::Property &property, properties) {
        if (property.id== KisLayerPropertiesIcons::openFileLayerFile.id()) {
            if (property.state.toBool() == false) {
                openFile();
            }
        }
    }
}

KisBaseNode::PropertyList KisFileLayer::sectionModelProperties() const
{
    KisBaseNode::PropertyList l = KisLayer::sectionModelProperties();
    l << KisBaseNode::Property(KoID("sourcefile", i18n("File")), m_filename);
    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::openFileLayerFile, true);
    return l;
}

void KisFileLayer::setFileName(const QString &basePath, const QString &filename)
{
    m_basePath = basePath;
    m_filename = filename;
    QFileInfo fi(path());
    if (fi.exists()) {
        m_loader.setPath(path());
        m_loader.reloadImage();
    }
}

QString KisFileLayer::fileName() const
{
    return m_filename;
}

QString KisFileLayer::path() const
{
    if (m_basePath.isEmpty()) {
        return m_filename;
    }
    else {
        return QDir(m_basePath).filePath(QDir::cleanPath(m_filename));
    }
}

void KisFileLayer::openFile() const
{
    bool fileAlreadyOpen = false;
    Q_FOREACH (KisDocument *doc, KisPart::instance()->documents()) {
        if (doc->url().toLocalFile()==path()){
            fileAlreadyOpen = true;
        }
    }
    if (!fileAlreadyOpen) {
        KisPart::instance()->openExistingFile(QUrl::fromLocalFile(QFileInfo(path()).absoluteFilePath()));
    }
}

KisFileLayer::ScalingMethod KisFileLayer::scalingMethod() const
{
    return m_scalingMethod;
}

void KisFileLayer::setScalingMethod(ScalingMethod method)
{
    m_scalingMethod = method;
}

void KisFileLayer::slotLoadingFinished(KisPaintDeviceSP projection, int xRes, int yRes)
{
    qint32 oldX = x();
    qint32 oldY = y();
    const QRect oldLayerExtent = m_paintDevice->extent();

    m_paintDevice->makeCloneFrom(projection, projection->extent());
    m_paintDevice->setDefaultBounds(new KisDefaultBounds(image()));

    QSize size = projection->exactBounds().size();

    if (m_scalingMethod == ToImagePPI && (image()->xRes() != xRes
                                          || image()->yRes() != yRes)) {
        qreal xscale = image()->xRes() / xRes;
        qreal yscale = image()->yRes() / yRes;

        KisTransformWorker worker(m_paintDevice, xscale, yscale, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, KisFilterStrategyRegistry::instance()->get("Bicubic"));
        worker.run();
    }
    else if (m_scalingMethod == ToImageSize) {
        QSize sz = size;
        sz.scale(image()->size(), Qt::KeepAspectRatio);
        qreal xscale =  (qreal)sz.width() / (qreal)size.width();
        qreal yscale = (qreal)sz.height() / (qreal)size.height();

        KisTransformWorker worker(m_paintDevice, xscale, yscale, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, KisFilterStrategyRegistry::instance()->get("Bicubic"));
        worker.run();
    }

    m_paintDevice->setX(oldX);
    m_paintDevice->setY(oldY);

    setDirty(m_paintDevice->extent() | oldLayerExtent);
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

KUndo2Command* KisFileLayer::crop(const QRect & rect)
{
    QPoint oldPos(x(), y());
    QPoint newPos = oldPos - rect.topLeft();

    return new KisNodeMoveCommand2(this, oldPos, newPos);
}

KUndo2Command* KisFileLayer::transform(const QTransform &/*transform*/)
{
    warnKrita << "WARNING: File Layer does not support transformations!" << name();
    return 0;
}

void KisFileLayer::setImage(KisImageWSP image)
{
    m_paintDevice->setDefaultBounds(new KisDefaultBounds(image));
    KisExternalLayer::setImage(image);
}

