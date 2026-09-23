/*
 *  SPDX-FileCopyrightText: 2005 Bart Coppens <kde@bartcoppens.be>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *  SPDX-FileCopyrightText: 2026 Freya Lupen <penguinflyer2222@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_custom_brush_widget.h"

#include <QImage>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>

#include <kstandardguiitem.h>

#include <KoResourcePaths.h>
#include "KisResourceTypes.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_gbr_brush.h"
#include "KisBrushServerProvider.h"
#include "kis_paint_layer.h"
#include <kis_selection.h>
#include <KoProperties.h>
#include "kis_iterator_ng.h"
#include "KisImageBarrierLock.h"
#include <KisResourceUserOperations.h>
#include "kis_clipboard.h"
#include "KisMainWindow.h"
#include "KisViewManager.h"
#include "KisPart.h"
#include "KisImportExportFilter.h"
#include "KisImportExportManager.h"
#include "kis_config_widget.h"
#include "KisDocument.h"
#include "kis_properties_configuration.h"


const QString TEMPORARY_BRUSH_NAME = "Temporary custom brush";

enum BrushStyle {
    Regular,
    Animated
};

enum Source {
    Image,
    Clipboard
};

const QString KEY_NAME = "name";
const QString KEY_MASK = "mask";
const QString KEY_PRESERVE_ALPHA = "preserveAlpha";
const QString KEY_BRUSH_STYLE = "brushStyle";

KisCustomBrushWidget::KisCustomBrushWidget(QWidget *parent, const QString& caption)
    : QDialog(parent)
{
    setWindowTitle(caption);

    QVBoxLayout* layout = new QVBoxLayout();
    QHBoxLayout* hLayout = new QHBoxLayout();

    m_preview = new QLabel();
    m_preview->setFixedSize(110, 110);
    m_preview->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_preview->setScaledContents(false);
    m_preview->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setWordWrap(true);
    hLayout->addWidget(m_preview);

    QLabel* sourceLabel = new QLabel(i18nc("@label:listbox image source", "Source:"));
    hLayout->addWidget(sourceLabel);

    QComboBox* sourceBox = new QComboBox();
    sourceBox->addItem(i18nc("@item:inlistbox", "Image"), Source::Image);
    sourceBox->addItem(i18nc("@item:inlistbox", "Clipboard"), Source::Clipboard);
    hLayout->addWidget(sourceBox);

    QPushButton* reloadButton = new QPushButton(i18nc("@action:button reload image", "Reload"));
    hLayout->addWidget(reloadButton);

    layout->addLayout(hLayout);

    const QString mimetype = "image/x-gimp-brush";
    QSharedPointer<KisImportExportFilter> filter(KisImportExportManager::filterForMimeType(mimetype, KisImportExportManager::Export));
    KIS_SAFE_ASSERT_RECOVER_RETURN(filter);
    m_brushWidget = filter->createConfigurationWidget(0, 0, mimetype.toLatin1());
    connect(m_brushWidget, SIGNAL(sigConfigurationUpdated()), this, SLOT(slotUpdateBrush()));

    const QString animMimetype = "image/x-gimp-brush-animated";
    QSharedPointer<KisImportExportFilter> animFilter(KisImportExportManager::filterForMimeType(animMimetype, KisImportExportManager::Export));
    KIS_SAFE_ASSERT_RECOVER_RETURN(animFilter);
    m_animBrushWidget = animFilter->createConfigurationWidget(0, 0, animMimetype.toLatin1());
    connect(m_animBrushWidget, SIGNAL(sigConfigurationUpdated()), this, SLOT(slotUpdateBrush()));
    layout->addWidget(m_animBrushWidget);

    m_oldConfig = currentBrushWidget()->configuration();

    m_rServer = KisBrushServerProvider::instance()->brushServer();

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel|QDialogButtonBox::Save);
    KGuiItem::assign(m_buttonBox->button(QDialogButtonBox::Save), KStandardGuiItem::save());
    KGuiItem::assign(m_buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    layout->addWidget(m_buttonBox);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    setLayout(layout);

    connect(this, SIGNAL(accepted()), SLOT(slotAddPredefined()));
    connect(reloadButton, SIGNAL(clicked()), this, SLOT(slotReload()));
    connect(sourceBox, SIGNAL(activated(int)), this, SLOT(slotSourceChanged(int)));

    loadImage();
}

KisCustomBrushWidget::~KisCustomBrushWidget()
{
}

KisConfigWidget* KisCustomBrushWidget::currentBrushWidget()
{
    if (m_useClipboard) { // Can't be animated
        return m_brushWidget;
    } else {
        return m_animBrushWidget;
    }
}

KisConfigWidget* KisCustomBrushWidget::otherBrushWidget()
{
    if (m_useClipboard) { // Can't be animated
        return m_animBrushWidget;
    } else {
        return m_brushWidget;
    }
}

void KisCustomBrushWidget::swapBrushWidget()
{
    currentBrushWidget()->setConfiguration(otherBrushWidget()->configuration());
    layout()->replaceWidget(otherBrushWidget(), currentBrushWidget());
    otherBrushWidget()->hide();
    currentBrushWidget()->show();
}

int KisCustomBrushWidget::currentBrushStyle()
{
    return (BrushStyle)currentBrushWidget()->configuration()->getInt(KEY_BRUSH_STYLE);
}

QString KisCustomBrushWidget::fileName()
{
    const QString name = currentBrushWidget()->configuration()->getString(KEY_NAME);
    const QString suffix = currentBrushStyle() == BrushStyle::Regular ? ".gbr" : ".gih";
    // Export to a temp location, the resource system doesn't like importing from the resource folder
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + name.split(" ").join("_") + suffix;
}

KisImageWSP KisCustomBrushWidget::currentImage()
{
    KisMainWindow *mainWin = KisPart::instance()->currentMainwindow();
    if (!mainWin) { return nullptr; }
    KisViewManager *viewManager = mainWin->viewManager();
    if (!viewManager) { return nullptr; }

    // Animated uses the view for the layer (frames) count.
    m_animBrushWidget->setView(viewManager);
    return viewManager->image();
}

void KisCustomBrushWidget::slotSourceChanged(int source)
{
    bool useClipboard = source == Source::Clipboard;
    if (m_useClipboard == useClipboard) { return; }

    m_useClipboard = useClipboard;

    swapBrushWidget();

    if (m_useClipboard) {
        // Clipboard doesn't support animated.
        KisPropertiesConfigurationSP cfg = currentBrushWidget()->configuration();
        cfg->setProperty(KEY_BRUSH_STYLE, BrushStyle::Regular);
        currentBrushWidget()->setConfiguration(cfg);
    }

    slotReload();
}

void KisCustomBrushWidget::slotUpdateBrush()
{
    int brushStyle = currentBrushStyle();
    if (brushStyle != m_oldConfig->getInt(KEY_BRUSH_STYLE)) {
        loadImage();
    }
    updatePreviewImage(false);

    slotUpdateSaveButton();

    m_oldConfig = currentBrushWidget()->configuration();
}

void KisCustomBrushWidget::updatePreviewImage(bool updateDev)
{
    int brushStyle = currentBrushStyle();
    bool useColorMask = currentBrushWidget()->configuration()->getBool(KEY_MASK);
    bool preserveAlpha = currentBrushWidget()->configuration()->getBool(KEY_PRESERVE_ALPHA);
    bool needsUpdate = updateDev || (brushStyle != m_oldConfig->getInt(KEY_BRUSH_STYLE)) ||
        (useColorMask != m_oldConfig->getInt(KEY_MASK)) || (preserveAlpha != m_oldConfig->getInt(KEY_PRESERVE_ALPHA));
    if (!needsUpdate) {
        return;
    }

    // Create a temporary brush to generate a preview image
    KisPaintDeviceSP dev;
    QRect rc;
    if (currentBrushStyle() == BrushStyle::Regular) {
        dev = m_dev;
        rc = m_devRect;
    } else {
        if (!m_devices.isEmpty() && !m_devices[0].isEmpty()) {
            dev = m_devices[0][0];
            rc = QRect(QPoint(0,0), m_imageSize);
        }
    }

    QImage brushImage;
    if (dev) {
        KisBrushSP brush = KisBrushSP(new KisGbrBrush(dev, rc.x(), rc.y(), rc.width(), rc.height()));

        if (useColorMask) {
            static_cast<KisGbrBrush*>(brush.data())->makeMaskImage(preserveAlpha);
            static_cast<KisGbrBrush*>(brush.data())->setBrushApplication(preserveAlpha ? LIGHTNESSMAP : ALPHAMASK);
        } else {
            static_cast<KisGbrBrush*>(brush.data())->setBrushApplication(IMAGESTAMP);
        }
        brushImage = brush->brushTipImage();
    }

    if (brushImage.isNull()) {
        m_preview->setText(i18nc("@info empty image", "Empty"));
    } else {
        int w = m_preview->size().width() - m_preview->frameWidth() * 2;
        brushImage = brushImage.scaled(w, w, Qt::KeepAspectRatio);
        m_preview->setPixmap(QPixmap::fromImage(brushImage));
    }
}

void KisCustomBrushWidget::slotReload()
{
    m_dev = nullptr;
    m_devices.clear();
    loadImage();
}

void KisCustomBrushWidget::slotUpdateSaveButton()
{
    if (!m_rServer) { return; }

    const QString name = currentBrushWidget()->configuration()->getString(KEY_NAME);
    int brushStyle = currentBrushStyle();
    bool needsUpdate = (name != m_oldConfig->getString(KEY_NAME) || brushStyle != m_oldConfig->getInt(KEY_BRUSH_STYLE));
    if (!needsUpdate) { return; }

    const QString suffix = brushStyle == BrushStyle::Regular ? ".gbr" : ".gih";
    QString exportFilename = m_rServer->saveLocation() + "/" + name.split(" ").join("_") + suffix;
    if (QFileInfo(exportFilename).exists()) {
        m_buttonBox->button(QDialogButtonBox::Save)->setText(i18nc("@action:button", "Overwrite"));
    } else {
        m_buttonBox->button(QDialogButtonBox::Save)->setText(i18nc("@action:button", "Save"));
    }
}

void KisCustomBrushWidget::slotAddPredefined()
{
    if (!m_rServer || !(m_dev || !m_devices.isEmpty())) {
        close();
        return;
    }

    KisDocument* doc = KisPart::instance()->createTemporaryDocument();
    // Don't show a second export dialog when calling export later.
    doc->setFileBatchMode(true);
    KisImageSP image;

    if (currentBrushStyle() == BrushStyle::Regular) {
        image = new KisImage(0, m_devRect.width(), m_devRect.height(), m_dev->colorSpace(), TEMPORARY_BRUSH_NAME);
        KisNodeSP node = new KisPaintLayer(image, TEMPORARY_BRUSH_NAME, OPACITY_OPAQUE_U8, m_dev);
        image->addNode(node);
    } else /*if (currentBrushStyle() == BrushStyle::Animated)*/ {
        image = new KisImage(0, m_imageSize.width(), m_imageSize.height(), m_imageColorSpace, TEMPORARY_BRUSH_NAME);
        Q_FOREACH(KisPaintDeviceSP dev, m_devices[0]) {
            KisNodeSP node = new KisPaintLayer(image, TEMPORARY_BRUSH_NAME, OPACITY_OPAQUE_U8, dev);
            image->addNode(node);
        }
    }

    doc->setCurrentImage(image);

    const QByteArray mimetype = currentBrushStyle() == BrushStyle::Regular ? "image/x-gimp-brush" : "image/x-gimp-brush-animated";
    QString path = fileName();
    bool success = doc->exportDocumentSync(path, mimetype, currentBrushWidget()->configuration());
    doc->deleteLater();

    // Add it to the brush server, so that it automatically gets to the mediators, and
    // so to the other brush choosers can pick it up, if they want to
    if (success) {
        KoResourceSP resource = KisResourceUserOperations::importResourceFileWithUserInput(this, QString(), ResourceType::Brushes, path);
        if (resource) {
            Q_EMIT sigNewPredefinedBrush(resource);
        }
    }

    close();
}

void KisCustomBrushWidget::loadImage()
{
    if (currentBrushStyle() == BrushStyle::Regular) {
        KisImageWSP image;
        if (!m_dev) {
            if (m_useClipboard) {
                KisClipboard* clipboard = KisClipboard::instance();
                if (clipboard->hasImage()) {
                    m_dev = clipboard->clip(QRect(), false);
                    if (m_dev) {
                        m_devRect = m_dev->exactBounds();
                    }
                }
            } else if ((image = currentImage())) {
                // create copy of the data
                image->barrierLock();
                m_dev = new KisPaintDevice(*image->projection());
                m_devRect = QRect(QPoint(0,0), image->size());
                image->unlock();

                KisSelectionSP selection = image->globalSelection();
                if (selection) {
                    // apply selection mask
                    QRect r = selection->selectedExactRect();

                    KisHLineIteratorSP pixelIt = m_dev->createHLineIteratorNG(r.x(), r.top(), r.width());
                    KisHLineConstIteratorSP maskIt = selection->projection()->createHLineIteratorNG(r.x(), r.top(), r.width());

                    for (qint32 y = r.top(); y <= r.bottom(); ++y) {
                        do {
                            m_dev->colorSpace()->applyAlphaU8Mask(pixelIt->rawData(), maskIt->oldRawData(), 1);
                        } while (pixelIt->nextPixel() && maskIt->nextPixel());

                        pixelIt->nextRow();
                        maskIt->nextRow();
                    }

                    m_dev->crop(r);
                    m_devRect = r;
                }
            }
            updatePreviewImage(true);
        }
    } else /*if (currentBrushStyle() == BrushStyle::Animated)*/ {
        KisImageWSP image;
        if (m_devices.isEmpty() && (image = currentImage())) {
            // For each layer in the current image, create a new image, and add it to the list
            m_devices.clear();
            m_devices.append(QVector<KisPaintDevice*>());

            KisImageReadOnlyBarrierLock lock(image);

            m_imageSize = image->size();
            m_imageColorSpace = image->colorSpace();

            // We only loop over the rootLayer. Since we actually should have a layer selection
            // list, no need to elaborate on that here and now
            KoProperties properties;
            properties.setProperty("visible", true);
            QList<KisNodeSP> layers = image->root()->childNodes(QStringList("KisLayer"), properties);
            Q_FOREACH (KisNodeSP node, layers) {
                m_devices[0].append(node->projection().data());
            }
        }
        updatePreviewImage(true);
    }

    m_buttonBox->button(QDialogButtonBox::Save)->setEnabled(m_dev || !m_devices.isEmpty());
}
