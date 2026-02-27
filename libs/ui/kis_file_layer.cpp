/*
 *  SPDX-FileCopyrightText: 2013 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    connect(&m_loader, SIGNAL(loadingFinished(KisPaintDeviceSP,qreal,qreal,QSize)), SLOT(slotLoadingFinished(KisPaintDeviceSP,qreal,qreal,QSize)));
    connect(&m_loader, SIGNAL(loadingFailed()), SLOT(slotLoadingFailed()));
    connect(&m_loader, SIGNAL(fileExistsStateChanged(bool)), SLOT(slotFileExistsStateChanged(bool)));
    connect(this, SIGNAL(sigRequestOpenFile()), SLOT(openFile()));
}

KisFileLayer::KisFileLayer(KisImageWSP image, const QString &basePath, const QString &filename, ScalingMethod scaleToImageResolution, QString scalingFilter, const QString &name, quint8 opacity, const KoColorSpace *fallbackColorSpace)
    : KisExternalLayer(image, name, opacity)
    , m_basePath(basePath)
    , m_filename(filename)
    , m_scalingMethod(scaleToImageResolution)
    , m_scalingFilter(scalingFilter)
{
    /**
     * Set default paint device for a layer. It will be used in case
     * the file does not exist anymore. Or course, this can happen only
     * in the failing execution path.
     */
    m_paintDevice = new KisPaintDevice(fallbackColorSpace ? fallbackColorSpace : image->colorSpace());
    m_paintDevice->setDefaultBounds(new KisDefaultBounds(image));

    connect(&m_loader, SIGNAL(loadingFinished(KisPaintDeviceSP,qreal,qreal,QSize)), SLOT(slotLoadingFinished(KisPaintDeviceSP,qreal,qreal,QSize)));
    connect(&m_loader, SIGNAL(loadingFailed()), SLOT(slotLoadingFailed()));
    connect(&m_loader, SIGNAL(fileExistsStateChanged(bool)), SLOT(slotFileExistsStateChanged(bool)));
    connect(this, SIGNAL(sigRequestOpenFile()), SLOT(openFile()));

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
    m_scalingMethod = rhs.m_scalingMethod;
    m_scalingFilter = rhs.m_scalingFilter;

    m_generatedForImageSize = rhs.m_generatedForImageSize;
    m_generatedForXRes = rhs.m_generatedForXRes;
    m_generatedForYRes = rhs.m_generatedForYRes;
    m_state = rhs.m_state;

    m_paintDevice = new KisPaintDevice(*rhs.m_paintDevice);

    connect(&m_loader, SIGNAL(loadingFinished(KisPaintDeviceSP,qreal,qreal,QSize)), SLOT(slotLoadingFinished(KisPaintDeviceSP,qreal,qreal,QSize)));
    connect(this, SIGNAL(sigRequestOpenFile()), SLOT(openFile()));
    m_loader.setPath(path());
}

QIcon KisFileLayer::icon() const
{
    return KisIconUtils::loadIcon("fileLayer");
}

void KisFileLayer::resetCache(const KoColorSpace *colorSpace)
{
    Q_UNUSED(colorSpace);
    m_loader.reloadImage();
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
    KisLayer::setSectionModelProperties(properties);
    Q_FOREACH (const KisBaseNode::Property &property, properties) {
        if (property.id== KisLayerPropertiesIcons::openFileLayerFile.id()) {
            if (property.state.toBool() == false) {
                Q_EMIT sigRequestOpenFile();
            }
        }
    }
}

KisBaseNode::PropertyList KisFileLayer::sectionModelProperties() const
{
    KisBaseNode::PropertyList l = KisLayer::sectionModelProperties();
    l << KisBaseNode::Property(KoID("sourcefile", i18n("File")), m_filename);
    l << KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::openFileLayerFile, true);

    auto fileNameOrPlaceholder =
    [this] () {
        return !m_filename.isEmpty() ? m_filename : i18nc("placeholder test for a warning when not file is set in the file layer", "<No file name is set>");
    };

    if (m_state == FileNotFound) {
        l << KisLayerPropertiesIcons::getErrorProperty(i18nc("a tooltip shown when a file layer cannot find its linked file",
                                                             "Linked file not found: %1", fileNameOrPlaceholder()));
    } else if (m_state == FileLoadingFailed) {
        l << KisLayerPropertiesIcons::getErrorProperty(i18nc("a tooltip shown when a file layer cannot load its linked file",
                                                             "Failed to load linked file: %1", fileNameOrPlaceholder()));
    }

    const KoColorSpace *cs = m_paintDevice->colorSpace();
    KisImageSP image = this->image();
    if (image && *image->colorSpace() != *cs) {
        l << KisLayerPropertiesIcons::getColorSpaceMismatchProperty(cs);
    }

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
#ifndef Q_OS_ANDROID
        return QDir(m_basePath).filePath(QDir::cleanPath(m_filename));
#else
        return m_filename;
#endif
    }
}

void KisFileLayer::openFile() const
{
    bool fileAlreadyOpen = false;
    Q_FOREACH (KisDocument *doc, KisPart::instance()->documents()) {
        if (doc->path()==path()){
            fileAlreadyOpen = true;
        }
    }
    if (qEnvironmentVariableIsSet("KRITA_ENABLE_ASSERT_TESTS")) {
        ENTER_FUNCTION() << ppVar(m_filename);
        if (m_filename.toLower() == "crash_me_with_safe_assert") {
            KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "safe assert for testing purposes");
        }
        if (m_filename.toLower() == "crash_me_with_normal_assert") {
            KIS_ASSERT_RECOVER_NOOP(0 && "normal assert for testing purposes");
        }
        if (m_filename.toLower() == "crash_me_with_qfatal") {
            qFatal("Testing fatal message");
        }

        if (m_filename.toLower() == "crash_me_with_asan") {
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
            /**
             * A simple out-of-bounds check test. It should be caught in the
             * ASAN build and "should not" cause any crash in a normal build,
             * since we are reading only one byte past the end of allocated area.
             *
             * We enable that check only for ASAN-capable builds, since having
             * such code in production builds may be unsafe.
             */
            int *array = new int[10];
            qDebug() << "Reading past the end of the allocated array" << array[10];
            delete[] array;
#else
            qDebug() << "ASAN is not enabled for this build!";
#endif
#endif
        }
    }

    if (!fileAlreadyOpen && QFile::exists(QFileInfo(path()).absoluteFilePath())) {
        KisPart::instance()->openExistingFile(QFileInfo(path()).absoluteFilePath());
    }
}

void KisFileLayer::changeState(State newState)
{
    const State oldState = m_state;
    m_state = newState;
    if (oldState != newState) {
        baseNodeChangedCallback();
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

QString KisFileLayer::scalingFilter() const
{
    return m_scalingFilter;
}

void KisFileLayer::setScalingFilter(QString filter)
{
    m_scalingFilter = filter;
}

void KisFileLayer::slotLoadingFinished(KisPaintDeviceSP projection,
                                       qreal xRes, qreal yRes,
                                       const QSize &size)
{
    qint32 oldX = x();
    qint32 oldY = y();
    const QRect oldLayerExtent = m_paintDevice->extent();


    m_paintDevice->makeCloneFrom(projection, projection->extent());
    m_paintDevice->setDefaultBounds(new KisDefaultBounds(image()));

    /**
     * This method can be transitively called from KisFileLayer::setImage(),
     * which, in turn, can be called from the KisImage's copy-ctor. The shared
     * pointer is, obviously, not initialized during construction, therefore
     * upgrading our constructor to a strong pointer will cause a crash.
     *
     * Therefore, we use a weak pointer here. It is extremely dangerous, but
     * since this method is usually called from the GUI thread synchronously
     * it should be "somewhat safe".
     */
    KisImageWSP image = this->image();
    if (image) {
        if (m_scalingMethod == ToImagePPI &&
                (!qFuzzyCompare(image->xRes(), xRes) ||
                 !qFuzzyCompare(image->yRes(), yRes))) {

            qreal xscale = image->xRes() / xRes;
            qreal yscale = image->yRes() / yRes;

            KisTransformWorker worker(m_paintDevice, xscale, yscale, 0.0, 0, 0, 0, 0, 0, KisFilterStrategyRegistry::instance()->get(m_scalingFilter));
            worker.run();
        }
        else if (m_scalingMethod == ToImageSize && size != image->size()) {
            QSize sz = size;
            sz.scale(image->size(), Qt::KeepAspectRatio);
            qreal xscale =  (qreal)sz.width() / (qreal)size.width();
            qreal yscale = (qreal)sz.height() / (qreal)size.height();

            KisTransformWorker worker(m_paintDevice, xscale, yscale, 0.0, 0, 0, 0, 0, 0, KisFilterStrategyRegistry::instance()->get(m_scalingFilter));
            worker.run();
        }

        m_generatedForImageSize = image->size();
        m_generatedForXRes = image->xRes();
        m_generatedForYRes = image->yRes();
    }

    m_paintDevice->setX(oldX);
    m_paintDevice->setY(oldY);

    changeState(FileLoaded);
    setDirty(m_paintDevice->extent() | oldLayerExtent);
}

void KisFileLayer::slotLoadingFailed()
{
    changeState(FileLoadingFailed);
}

void KisFileLayer::slotFileExistsStateChanged(bool exists)
{
    changeState(exists ? FileLoaded : FileNotFound);
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

void KisFileLayer::slotImageResolutionChanged()
{
    KisImageSP image = this->image();
    if (!image) return;

    if (m_scalingMethod == ToImagePPI &&
            qFuzzyCompare(image->xRes(), m_generatedForXRes) &&
            qFuzzyCompare(image->yRes(), m_generatedForYRes)) {

                m_loader.reloadImage();
    }
}

void KisFileLayer::slotImageSizeChanged()
{
    KisImageSP image = this->image();
    if (!image) return;

    if (m_scalingMethod == ToImageSize && image->size() != m_generatedForImageSize) {
        m_loader.reloadImage();
    }
}

void KisFileLayer::setImage(KisImageWSP image)
{
    KisImageWSP oldImage = this->image();
    m_imageConnections.clear();

    m_paintDevice->setDefaultBounds(new KisDefaultBounds(image));
    KisExternalLayer::setImage(image);

    if (image) {
        m_imageConnections.addUniqueConnection(image, SIGNAL(sigSizeChanged(QPointF,QPointF)), this, SLOT(slotImageSizeChanged()));
        m_imageConnections.addUniqueConnection(image, SIGNAL(sigResolutionChanged(double, double)), this, SLOT(slotImageResolutionChanged()));
    }

    if (m_scalingMethod != None && image && oldImage != image) {
        bool canSkipReloading = false;

        if (m_scalingMethod == ToImageSize && image && image->size() == m_generatedForImageSize) {
            canSkipReloading = true;
        }

        if (m_scalingMethod == ToImagePPI && image &&
                qFuzzyCompare(image->xRes(), m_generatedForXRes) &&
                qFuzzyCompare(image->yRes(), m_generatedForYRes)) {

            canSkipReloading = true;
        }

        if (!canSkipReloading) {
            m_loader.reloadImage();
        }
    }
}

