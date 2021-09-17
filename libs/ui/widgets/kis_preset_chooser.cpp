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
#include "KisResourceServerProvider.h"
#include "kis_global.h"
#include "kis_slider_spin_box.h"
#include "kis_config_notifier.h"
#include <kis_icon.h>
#include <KisResourceModelProvider.h>
#include <KisTagFilterResourceProxyModel.h>
#include <KisResourceThumbnailPainter.h>


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
    KisResourceThumbnailPainter m_thumbnailPainter;
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


    qreal devicePixelRatioF = painter->device()->devicePixelRatioF();

    // get original thumbnail image (which also has style if selected)
    QSize originalImageSize(256,256);
    bool generateSelection = false; //this chooser also has a list view, so we are doing the selection logic here instead of the thumbnail
    QImage preview = m_thumbnailPainter.getReadyThumbnail(index, originalImageSize*devicePixelRatioF, qApp->palette(), generateSelection, true );
    preview.setDevicePixelRatio(devicePixelRatioF);


    // if we are showing text with the brush preset, we only want the thumbnail to take up a small portion...not the whole rectangle
    QImage previewHighDpi = preview.scaled(option.rect.size()*devicePixelRatioF, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    previewHighDpi.setDevicePixelRatio(devicePixelRatioF);
    if( m_showText) {
         QSize pixSize(option.rect.height(), option.rect.height());
         previewHighDpi = preview.scaled(pixSize*devicePixelRatioF, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    painter->drawImage(option.rect.x(), option.rect.y(), previewHighDpi);


    // // if we are showing showing the brush size next to the thumbnail
    //  qreal brushSize = metaData["paintopSize"].toReal();
    //  qDebug() << "brushsize" << brushSize;
    //  QString brushSizeText;
    //  // Disable displayed decimal precision beyond a certain brush size
    //  if (brushSize < 100) {
    //    brushSizeText = QString::number(brushSize, 'g', 3);
    //  }
    //  else {
    //     brushSizeText = QString::number(brushSize, 'f', 0);
    //  }
    //  painter->drawText(pixSize.width() + 10, option.rect.y() + option.rect.height() - 10, brushSizeText); // brush size


    // show name of brush and if it has changed from default (dirtyIndicator)
    QString dirtyPresetIndicator = QString("");
    if (m_useDirtyPresets && dirty) {
        dirtyPresetIndicator = QString("*");
    }
    if( m_showText) {
        QSize pixSize(option.rect.height(), option.rect.height());
        int textYPosition = option.rect.height() * 0.5 + (qApp->font().pixelSize() *0.5); // vertically center text
        int textMargin = 15;
        QString presetDisplayName = index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString().replace("_", " "); // don't need underscores that might be part of the file name
        painter->drawText(pixSize.width() + textMargin, option.rect.y() +
                          option.rect.height() - textYPosition, presetDisplayName.append(dirtyPresetIndicator));
    }


    // if our brush preset has been changed from the default (dirty), show indicator
    if (m_useDirtyPresets && dirty) {
        const QIcon icon = KisIconUtils::loadIcon("dirty-preset");
        QPixmap pixmap = icon.pixmap(QSize(16,16));
        painter->drawPixmap(option.rect.x() + 3, option.rect.y() + 3, pixmap);
    }


    // do we have a missing/broken brush tip image? Add an icon if we do
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
        icon.paint(painter, QRect(option.rect.x() + option.rect.height() - 25, option.rect.y() + option.rect.height() - 25, 25, 25));
    }


    // have a small highlight if we are selected
    // the thumbnail mode already does this...but if we are in list view we  need to highlight the whole row
    bool isOptionSelected = option.state & QStyle::State_Selected;
    if (isOptionSelected) {
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

KisPresetChooser::KisPresetChooser(QWidget *parent, const char *name)
    : QWidget(parent)
{
    setObjectName(name);
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setMargin(0);

    m_chooser = new KisResourceItemChooser(ResourceType::PaintOpPresets, false, this);
    m_chooser->setObjectName("ResourceChooser");
    m_chooser->setRowHeight(50);
    m_delegate = new KisPresetDelegate(this);
    m_chooser->setItemDelegate(m_delegate);
    m_chooser->setSynced(true);
    layout->addWidget(m_chooser);

    {
        QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(this->itemChooser()->itemView());
        if (scroller) {
            connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                    this, SLOT(slotScrollerStateChanged(QScroller::State)));
        }
    }

    connect(m_chooser, SIGNAL(resourceSelected(KoResourceSP )),
            this, SLOT(slotResourceWasSelected(KoResourceSP )));

    connect(m_chooser, SIGNAL(resourceSelected(KoResourceSP )),
            this, SIGNAL(resourceSelected(KoResourceSP )));
    connect(m_chooser, SIGNAL(resourceClicked(KoResourceSP )),
            this, SIGNAL(resourceClicked(KoResourceSP )));

    m_mode = THUMBNAIL;

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()),
            SLOT(notifyConfigChanged()));


    notifyConfigChanged();
}

KisPresetChooser::~KisPresetChooser()
{
}

void KisPresetChooser::showButtons(bool show)
{
    m_chooser->showButtons(show);
}

void KisPresetChooser::setViewMode(KisPresetChooser::ViewMode mode)
{
    m_mode = mode;
    updateViewSettings();
}

void KisPresetChooser::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateViewSettings();
}

void KisPresetChooser::notifyConfigChanged()
{
    KisConfig cfg(true);
    m_delegate->setUseDirtyPresets(cfg.useDirtyPresets());
    setIconSize(cfg.presetIconSize());

    updateViewSettings();
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
    emit m_chooser->tagFilterModel()->dataChanged(index,
                                               index,
                                               {Qt::UserRole + KisAbstractResourceModel::Thumbnail});
}

void KisPresetChooser::updateViewSettings()
{
    if (m_mode == THUMBNAIL) {
        m_chooser->setSynced(true);
        m_delegate->setShowText(false);
        m_chooser->itemView()->setViewMode(QListView::IconMode);
        m_chooser->itemView()->setFlow(QListView::LeftToRight);
    }
    else if (m_mode == DETAIL) {
        m_chooser->setSynced(false);
        m_chooser->itemView()->setViewMode(QListView::ListMode);
        m_chooser->itemView()->setFlow(QListView::TopToBottom);
        m_chooser->setColumnWidth(m_chooser->width());

        KisResourceItemChooserSync* chooserSync = KisResourceItemChooserSync::instance();
        m_chooser->setRowHeight(chooserSync->baseLength());
        m_delegate->setShowText(true);
    }
    else if (m_mode == STRIP) {
        m_chooser->setSynced(false);
        m_chooser->itemView()->setViewMode(QListView::ListMode);
        m_chooser->itemView()->setFlow(QListView::LeftToRight);
        // An offset of 7 keeps the cell exactly square, TODO: use constants, not hardcoded numbers
        m_chooser->setColumnWidth(m_chooser->viewSize().height() - 7);
        m_delegate->setShowText(false);
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
    updateViewSettings();
}

int KisPresetChooser::iconSize()
{
    KisResourceItemChooserSync* chooserSync = KisResourceItemChooserSync::instance();
    return chooserSync->baseLength();
}



void KisPresetChooser::saveIconSize()
{
    // save icon size
    KisConfig cfg(false);
    cfg.setPresetIconSize(iconSize());
}

void KisPresetChooser::slotScrollerStateChanged(QScroller::State state)
{
    KisKineticScroller::updateCursor(this, state);
}
