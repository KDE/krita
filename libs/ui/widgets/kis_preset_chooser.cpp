/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2009 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *  SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *  SPDX-FileCopyrightText: 2011 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_preset_chooser.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QAbstractItemDelegate>
#include <QStyleOptionViewItem>
#include <QSortFilterProxyModel>
#include <KisResourceModel.h>
#include <QApplication>

#include <kis_config.h>
#include <klocalizedstring.h>
#include <KisKineticScroller.h>

#include <KoIcon.h>
#include <KisResourceItemChooser.h>
#include <KisResourceItemChooserSync.h>
#include <KisResourceItemListView.h>
#include <KisResourceLocator.h>
#include <KisResourceTypes.h>

#include <brushengine/kis_paintop_settings.h>
#include <brushengine/kis_paintop_preset.h>
#include "kis_config_notifier.h"
#include <kis_icon.h>
#include <KisResourceModelProvider.h>
#include <KisTagFilterResourceProxyModel.h>
#include <KisResourceThumbnailCache.h>


/// The resource item delegate for rendering the resource preview
class KisPresetDelegate : public QAbstractItemDelegate
{
public:
    KisPresetDelegate(QObject * parent = 0)
        : QAbstractItemDelegate(parent)
        , m_showText(false)
        , m_useDirtyPresets(false) {}

    ~KisPresetDelegate() override {}

    /// reimplemented
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;

    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override {
        return option.decorationSize;
    }

    void setShowText(bool showText) {
        m_showText = showText;
    }

    void setUseDirtyPresets(bool value) {
        m_useDirtyPresets = value;
    }

private:
    bool m_showText;
    bool m_useDirtyPresets;
};

void KisPresetDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    if (!(option.state & QStyle::State_Enabled)) {
        painter->setOpacity(0.2);
    }

    if (!index.isValid()) {
        painter->restore();
        return;
    }

    bool dirty = index.data(Qt::UserRole + KisAbstractResourceModel::Dirty).toBool();

    QImage preview = KisResourceThumbnailCache::instance()->getImage(index);

    if (preview.isNull()) {
        preview = QImage(512, 512, QImage::Format_RGB32);
        preview.fill(Qt::red);
    }

    qreal devicePixelRatioF = painter->device()->devicePixelRatioF();

    QRect paintRect = option.rect.adjusted(1, 1, -1, -1);
    if (!m_showText) {
        QImage previewHighDpi =
            KisResourceThumbnailCache::instance()->getImage(index,
                                                             paintRect.size() * devicePixelRatioF,
                                                             Qt::IgnoreAspectRatio,
                                                             Qt::SmoothTransformation);
        previewHighDpi.setDevicePixelRatio(devicePixelRatioF);
        painter->drawImage(paintRect.x(), paintRect.y(), previewHighDpi);
    }
    else {
        QSize pixSize(paintRect.height(), paintRect.height());
        QImage previewHighDpi = KisResourceThumbnailCache::instance()->getImage(index,
                                                                                 pixSize * devicePixelRatioF,
                                                                                 Qt::KeepAspectRatio,
                                                                                 Qt::SmoothTransformation);
        previewHighDpi.setDevicePixelRatio(devicePixelRatioF);
        painter->drawImage(paintRect.x(), paintRect.y(), previewHighDpi);

        // Put an asterisk after the preset if it is dirty. This will help in case the pixmap icon is too small

        QString dirtyPresetIndicator = QString("");
        if (m_useDirtyPresets && dirty) {
            dirtyPresetIndicator = QString("*");
        }

//        qreal brushSize = metaData["paintopSize"].toReal();
//        qDebug() << "brushsize" << brushSize;
//        QString brushSizeText;
//        // Disable displayed decimal precision beyond a certain brush size
//        if (brushSize < 100) {
//            brushSizeText = QString::number(brushSize, 'g', 3);
//        }
//        else {
//            brushSizeText = QString::number(brushSize, 'f', 0);
//        }

//        painter->drawText(pixSize.width() + 10, option.rect.y() + option.rect.height() - 10, brushSizeText); // brush size

        QString presetDisplayName = index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString().replace("_", " "); // don't need underscores that might be part of the file name
        painter->drawText(pixSize.width() + 10, option.rect.y() + option.rect.height() - 10, presetDisplayName.append(dirtyPresetIndicator));

    }

    if (m_useDirtyPresets && dirty) {
        const QIcon icon = KisIconUtils::loadIcon("dirty-preset");
        QPixmap pixmap = icon.pixmap(QSize(16,16));
        painter->drawPixmap(paintRect.x() + 3, paintRect.y() + 3, pixmap);
    }

    bool broken = false;
    QMap<QString, QVariant> metaData = index.data(Qt::UserRole + KisAbstractResourceModel::MetaData).value<QMap<QString, QVariant>>();
    QStringList requiredBrushes = metaData["dependent_resources_filenames"].toStringList();
    if (!requiredBrushes.isEmpty()) {
        KisAllResourcesModel *model = KisResourceModelProvider::resourceModel(ResourceType::Brushes);
        Q_FOREACH(const QString brushFile, requiredBrushes) {
            if (!model->resourceExists("", brushFile, "")) {
                qWarning() << "dependent resource" << brushFile << "misses.";
                broken = true;
            }
        }
    }

    if (broken) {
        const QIcon icon = KisIconUtils::loadIcon("broken-preset");
        icon.paint(painter, QRect(paintRect.x() + paintRect.height() - 25, paintRect.y() + paintRect.height() - 25, 25, 25));
    }

    if (option.state & QStyle::State_Selected) {
        painter->setCompositionMode(QPainter::CompositionMode_HardLight);
        painter->setOpacity(1.0);
        painter->fillRect(option.rect, option.palette.highlight());

        // highlight is not strong enough to pick out preset. draw border around it.
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->setPen(QPen(option.palette.highlight(), 4, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        QRect selectedBorder = option.rect.adjusted(2 , 2, -2, -2); // constrict the rectangle so it doesn't bleed into other presets
        painter->drawRect(selectedBorder);
    }

    painter->restore();

}

KisPresetChooser::KisPresetChooser(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("KisPresetChooser");

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setMargin(0);

    m_chooser = new KisResourceItemChooser(ResourceType::PaintOpPresets, false, this);
    m_chooser->setRowHeight(50);
    m_delegate = new KisPresetDelegate(this);
    m_chooser->setItemDelegate(m_delegate);
    m_chooser->setSynced(true);
    m_chooser->showImportExportBtns(false);
    layout->addWidget(m_chooser);

    connect(m_chooser, SIGNAL(resourceSelected(KoResourceSP )),
            this, SLOT(slotResourceWasSelected(KoResourceSP )));

    connect(m_chooser, SIGNAL(resourceSelected(KoResourceSP )),
            this, SIGNAL(resourceSelected(KoResourceSP )));
    connect(m_chooser, SIGNAL(resourceClicked(KoResourceSP )),
            this, SIGNAL(resourceClicked(KoResourceSP )));

    connect(m_chooser, &KisResourceItemChooser::listViewModeChanged, this, &KisPresetChooser::showHideBrushNames);

    m_mode = ViewMode::THUMBNAIL;

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()),
            SLOT(notifyConfigChanged()));


    notifyConfigChanged();
}

KisPresetChooser::~KisPresetChooser()
{
}

void KisPresetChooser::setViewMode(KisPresetChooser::ViewMode mode)
{
    m_mode = mode;
    updateViewSettings();
}

void KisPresetChooser::setViewModeToThumbnail()
{
    setViewMode(KisPresetChooser::ViewMode::THUMBNAIL);
}

void KisPresetChooser::setViewModeToDetail()
{
    setViewMode(KisPresetChooser::ViewMode::DETAIL);
}

void KisPresetChooser::notifyConfigChanged()
{
    KisConfig cfg(true);
    m_delegate->setUseDirtyPresets(cfg.useDirtyPresets());
    setIconSize(cfg.presetIconSize());
}

void KisPresetChooser::slotResourceWasSelected(KoResourceSP resource)
{
    m_currentPresetConnections.clear();
    if (!resource) return;

    KisPaintOpPresetSP preset = resource.dynamicCast<KisPaintOpPreset>();
    KIS_SAFE_ASSERT_RECOVER_RETURN(preset);

    m_currentPresetConnections.addUniqueConnection(
        preset->updateProxy(), SIGNAL(sigSettingsChanged()),
        this, SLOT(slotCurrentPresetChanged()));
}

void KisPresetChooser::slotCurrentPresetChanged()
{
    KoResourceSP currentResource = m_chooser->currentResource();
    if (!currentResource) return;

    QModelIndex index = m_chooser->tagFilterModel()->indexForResource(currentResource);
    Q_EMIT m_chooser->tagFilterModel()->dataChanged(index,
                                               index,
                                               {Qt::UserRole + KisAbstractResourceModel::Thumbnail});
}

void KisPresetChooser::updateViewSettings()
{
    switch (m_mode) {
    case ViewMode::THUMBNAIL: {
        m_chooser->setListViewMode(ListViewMode::IconGrid);
        m_delegate->setShowText(false);
        break;
    }
    case ViewMode::DETAIL: {
        m_chooser->setListViewMode(ListViewMode::Detail);
        m_delegate->setShowText(true);
        break;
    }
    }
}

void KisPresetChooser::setCurrentResource(KoResourceSP resource)
{
    m_chooser->setCurrentResource(resource);
}

KoResourceSP KisPresetChooser::currentResource() const
{
    return m_chooser->currentResource();
}

void KisPresetChooser::showTaggingBar(bool show)
{
    m_chooser->showTaggingBar(show);
}

KisResourceItemChooser *KisPresetChooser::itemChooser()
{
    return m_chooser;
}

void KisPresetChooser::setPresetFilter(const QString& paintOpId)
{
    QMap<QString, QVariant> metaDataFilter;
    if (!paintOpId.isEmpty()) { // empty means "all"
        metaDataFilter["paintopid"] = paintOpId;
    }
    m_chooser->tagFilterModel()->setMetaDataFilter(metaDataFilter);
    updateViewSettings();
}

void KisPresetChooser::setIconSize(int newSize)
{
    KisResourceItemChooserSync* chooserSync = KisResourceItemChooserSync::instance();
    chooserSync->setBaseLength(newSize);
}

int KisPresetChooser::iconSize()
{
    KisResourceItemChooserSync* chooserSync = KisResourceItemChooserSync::instance();
    return chooserSync->baseLength();
}

void KisPresetChooser::saveIconSize()
{
    // save icon size
    if (KisConfig(true).presetIconSize() != iconSize()) {
        KisConfig(false).setPresetIconSize(iconSize());
    }
}

void KisPresetChooser::showHideBrushNames(ListViewMode newViewMode)
{
    switch (newViewMode) {
    case ListViewMode::Detail: {
        m_delegate->setShowText(true);
        break;
    }
    default: {
        m_delegate->setShowText(false);
    }
    }
}
