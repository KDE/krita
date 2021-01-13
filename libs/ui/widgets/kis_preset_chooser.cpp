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

#include <brushengine/kis_paintop_settings.h>
#include <brushengine/kis_paintop_preset.h>
#include "KisResourceServerProvider.h"
#include "kis_global.h"
#include "kis_slider_spin_box.h"
#include "kis_config_notifier.h"
#include <kis_icon.h>
#include <KisTagFilterResourceProxyModel.h>

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

    if (!index.isValid()) {
        qDebug() << "KisPresetDelegate::paint: index is invalid";
        painter->restore();
        return;
    }

    bool dirty = index.data(Qt::UserRole + KisAbstractResourceModel::Dirty).toBool();

    QImage preview = index.data(Qt::UserRole + KisAbstractResourceModel::Thumbnail).value<QImage>();

    if (preview.isNull()) {
        preview = QImage(512, 512, QImage::Format_RGB32);
        preview.fill(Qt::red);
    }

    QMap<QString, QVariant> metaData = index.data(Qt::UserRole + KisAbstractResourceModel::MetaData).value<QMap<QString, QVariant>>();

    qreal devicePixelRatioF = painter->device()->devicePixelRatioF();

    QRect paintRect = option.rect.adjusted(1, 1, -1, -1);
    if (!m_showText) {
        QImage previewHighDpi = preview.scaled(paintRect.size()*devicePixelRatioF, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        previewHighDpi.setDevicePixelRatio(devicePixelRatioF);
        painter->drawImage(paintRect.x(), paintRect.y(), previewHighDpi);
    }
    else {
        QSize pixSize(paintRect.height(), paintRect.height());
        QImage previewHighDpi = preview.scaled(pixSize*devicePixelRatioF, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        previewHighDpi.setDevicePixelRatio(devicePixelRatioF);
        painter->drawImage(paintRect.x(), paintRect.y(), previewHighDpi);

        // Put an asterisk after the preset if it is dirty. This will help in case the pixmap icon is too small

        QString dirtyPresetIndicator = QString("");
        if (m_useDirtyPresets && dirty) {
            dirtyPresetIndicator = QString("*");
        }

//        qreal brushSize = metaData["paintopSize"].toReal();
//        QString brushSizeText;

//        // Disable displayed decimal precision beyond a certain brush size
//        if (brushSize < 100) {
//            brushSizeText = QString::number(brushSize, 'g', 3);
//        } else {
//            brushSizeText = QString::number(brushSize, 'f', 0);
//        }

//        painter->drawText(pixSize.width() + 10, option.rect.y() + option.rect.height() - 10, brushSizeText); // brush size

        QString presetDisplayName = index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString().replace("_", " "); // don't need underscores that might be part of the file name
        painter->drawText(pixSize.width() + 40, option.rect.y() + option.rect.height() - 10, presetDisplayName.append(dirtyPresetIndicator));

    }

    if (m_useDirtyPresets && dirty) {
        const QIcon icon = KisIconUtils::loadIcon(koIconName("dirty-preset"));
        QPixmap pixmap = icon.pixmap(QSize(15,15));
        painter->drawPixmap(paintRect.x() + 3, paintRect.y() + 3, pixmap);
    }

//    if (!preset->settings() || !preset->settings()->isValid()) {
//        const QIcon icon = KisIconUtils::loadIcon("broken-preset");
//        icon.paint(painter, QRect(paintRect.x() + paintRect.height() - 25, paintRect.y() + paintRect.height() - 25, 25, 25));
//    }

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

#include "kis_preset_chooser.moc"
