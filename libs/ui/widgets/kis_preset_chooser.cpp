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

#include <QEvent>
#include <QVBoxLayout>
#include <QPainter>
#include <QAbstractItemDelegate>
#include <QAbstractItemView>
#include <QStyleOptionViewItem>
#include <QPalette>
#include <QTimer>
#include <KisResourceModel.h>

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
#include <KisTagFilterResourceProxyModel.h>
#include <KisResourceThumbnailCache.h>
#include "KisBrushStrokePreviewCache.h"

/// Resource item delegate for rendering brush stroke previews
class KisPresetDelegate : public QAbstractItemDelegate
{
public:
    // Stroke preview ratio range: 1:3 to 1:4.
    static constexpr int SLIDER_MIN = 30;
    static constexpr int SLIDER_MAX = 80;
    static constexpr double ASPECT_RATIO_MIN = 3.0;
    static constexpr double ASPECT_RATIO_MAX = 4.0;

    KisPresetDelegate(QObject * parent = nullptr)
        : QAbstractItemDelegate(parent)
        , m_showText(false)
        , m_useDirtyPresets(false)
        , m_viewMode(KisPresetChooser::THUMBNAIL)
        , m_iconSize(50) {}

    ~KisPresetDelegate() override {}

    /// reimplemented
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;

    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override {
        if (m_viewMode == KisPresetChooser::STROKE) {
            int height = m_iconSize;
            int width = static_cast<int>(height * calculateAspectRatio(m_iconSize));
            return QSize(width, height);
        }
        return option.decorationSize;
    }

    void setShowText(bool showText) {
        m_showText = showText;
    }

    void setViewMode(KisPresetChooser::ViewMode mode) {
        m_viewMode = mode;
    }

    void setUseDirtyPresets(bool value) {
        m_useDirtyPresets = value;
    }

    void setIconSize(int size) {
        m_iconSize = qBound(SLIDER_MIN, size, SLIDER_MAX);
    }

    /// Interpolate aspect ratio from slider.
    static double calculateAspectRatio(int iconSize) {
        int clampedSize = qBound(SLIDER_MIN, iconSize, SLIDER_MAX);
        double t = static_cast<double>(clampedSize - SLIDER_MIN) / (SLIDER_MAX - SLIDER_MIN);
        return ASPECT_RATIO_MIN + t * (ASPECT_RATIO_MAX - ASPECT_RATIO_MIN);
    }

    static QSize calculateStrokeThumbnailSize(int iconSize) {
        int clampedSize = qBound(SLIDER_MIN, iconSize, SLIDER_MAX);
        double aspectRatio = calculateAspectRatio(clampedSize);
        return QSize(static_cast<int>(clampedSize * aspectRatio), clampedSize);
    }

private:
    void paintThumbnail(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintStrokePreview(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool m_showText;
    KisPresetChooser::ViewMode m_viewMode;
    bool m_useDirtyPresets;
    int m_iconSize;
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

    if (m_viewMode == KisPresetChooser::STROKE) {
        paintStrokePreview(painter, option, index);
    } else {
        paintThumbnail(painter, option, index);
    }

    painter->restore();
}

void KisPresetDelegate::paintThumbnail(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

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

    if (index.data(Qt::UserRole + KisAbstractResourceModel::BrokenStatus).toBool()) {
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
}

void KisPresetDelegate::paintStrokePreview(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool dirty = index.data(Qt::UserRole + KisAbstractResourceModel::Dirty).toBool();

    KoResourceSP resource;
    const KisAbstractResourceModel *resourceModel = dynamic_cast<const KisAbstractResourceModel*>(index.model());
    if (resourceModel) {
        resource = resourceModel->resourceForIndex(index);
    }
    KisPaintOpPresetSP preset = resource.dynamicCast<KisPaintOpPreset>();

    qreal devicePixelRatioF = painter->device()->devicePixelRatioF();
    const bool isSelected = option.state & QStyle::State_Selected;

    const QColor backgroundColor = option.palette.color(QPalette::Window);
    const QColor borderColor = option.palette.color(QPalette::Mid);
    QColor textColor = option.palette.color(QPalette::Text);
    if (isSelected) {
        textColor = textColor.lightnessF() < 0.5 ? QColor(Qt::black) : QColor(Qt::white);
    }

    painter->fillRect(option.rect, borderColor);
    QRect innerRect = option.rect.adjusted(2, 2, -2, -2);
    painter->fillRect(innerRect, backgroundColor);
    if (isSelected) {
        const QColor accentColor = option.palette.highlight().color();
        const qreal bgLightness = backgroundColor.lightnessF();
        const qreal accentLightness = accentColor.lightnessF();
        const bool sameSide =
            (bgLightness >= 0.5 && accentLightness >= 0.5) ||
            (bgLightness < 0.5 && accentLightness < 0.5);
        if (sameSide) {
            painter->setCompositionMode(QPainter::CompositionMode_HardLight);
            painter->setOpacity(1.0);
            painter->fillRect(innerRect, accentColor);
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
    }

    // Keep text height fixed; stroke area shrinks.
    QFontMetrics fm(painter->font());
    const int bottomPadding = 1;
    const int textAreaHeight = fm.height() + 4 + bottomPadding;
    const int textAreaTopTrim = 3;

    QRect strokeRect = innerRect;
    strokeRect.setHeight(qMax(10, innerRect.height() - (textAreaHeight - textAreaTopTrim)));

    QSize previewSize = strokeRect.size() * devicePixelRatioF;

    const QString paintOpId = preset ? preset->paintOp().id() : QString();
    const bool useStripedBackground = KisBrushStrokePreviewCache::needsStripedBackground(paintOpId);

    QImage strokePreview;
    if (preset) {
        strokePreview = KisBrushStrokePreviewCache::instance()->getPreview(preset, previewSize);
    }
    if (strokePreview.isNull()) {
        strokePreview = QImage(previewSize, QImage::Format_ARGB32_Premultiplied);
        strokePreview.fill(Qt::transparent);
    }

    if (useStripedBackground) {
        strokePreview.setDevicePixelRatio(devicePixelRatioF);
        painter->drawImage(strokeRect.topLeft(), strokePreview);
    } else {
        QImage tinted = strokePreview.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        QPainter maskPainter(&tinted);
        maskPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        maskPainter.fillRect(tinted.rect(), textColor);
        maskPainter.end();
        tinted.setDevicePixelRatio(devicePixelRatioF);
        painter->drawImage(strokeRect.topLeft(), tinted);
    }

    QString presetDisplayName = index.data(Qt::UserRole + KisAbstractResourceModel::Name)
                                    .toString()
                                    .replace(QLatin1Char('_'), QLatin1Char(' '));
    if (dirty) {
        presetDisplayName.append(QLatin1Char('*'));
    }
    QString elidedName = fm.elidedText(presetDisplayName, Qt::ElideRight, innerRect.width() - 14);

    QRect textRect = innerRect;
    textRect.setTop(innerRect.bottom() - textAreaHeight);
    textRect.setLeft(innerRect.left() + 7);
    textRect.setBottom(innerRect.bottom() - bottomPadding);
    painter->setPen(textColor);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);

    if (dirty) {
        const QIcon icon = KisIconUtils::loadIcon(QStringLiteral("dirty-preset"));
        QPixmap pixmap = icon.pixmap(QSize(16, 16));
        painter->drawPixmap(innerRect.x() + 3, innerRect.y() + 3, pixmap);
    }

    if (index.data(Qt::UserRole + KisAbstractResourceModel::BrokenStatus).toBool()) {
        const QIcon icon = KisIconUtils::loadIcon(QStringLiteral("broken-preset"));
        icon.paint(painter, QRect(innerRect.right() - 25, innerRect.y() + 3, 22, 22));
    }

    if (isSelected) {
        painter->setPen(QPen(option.palette.highlight(), 4, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        QRect selectedBorder = option.rect.adjusted(2, 2, -2, -2);
        painter->drawRect(selectedBorder);
    }
}

KisPresetChooser::KisPresetChooser(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("KisPresetChooser");

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_chooser = new KisResourceItemChooser(ResourceType::PaintOpPresets, false, this);
    m_chooser->setRowHeight(50);
    m_delegate = new KisPresetDelegate(this);
    m_chooser->setItemDelegate(m_delegate);
    m_chooser->setSynced(true);
    m_chooser->showImportExportBtns(false);
    layout->addWidget(m_chooser);

    connect(KisResourceItemChooserSync::instance(), SIGNAL(baseLengthChanged(int)),
            this, SLOT(slotBaseLengthChanged(int)));
    connect(m_chooser, SIGNAL(resourceSelected(KoResourceSP )),
            this, SIGNAL(resourceSelected(KoResourceSP )));
    connect(m_chooser, SIGNAL(resourceClicked(KoResourceSP )),
            this, SIGNAL(resourceClicked(KoResourceSP )));

    connect(m_chooser, SIGNAL(listViewModeChanged(ListViewMode)),
            this, SLOT(showHideBrushNames(ListViewMode)));

    connect(KisBrushStrokePreviewCache::instance(), SIGNAL(sigPreviewReady(int)),
            this, SLOT(slotStrokePreviewReady(int)));

    m_mode = ViewMode::THUMBNAIL;

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()),
            SLOT(notifyConfigChanged()));

    notifyConfigChanged();

    // Pre-generate stroke previews.
    QTimer::singleShot(0, this, SLOT(slotGenerateStrokePreviews()));
}

KisPresetChooser::~KisPresetChooser()
{
}

void KisPresetChooser::setViewMode(KisPresetChooser::ViewMode mode)
{
    m_mode = mode;
    updateViewSettings();
    KisBrushStrokePreviewCache::instance()->generateAllPreviews();
}

void KisPresetChooser::setViewModeToThumbnail()
{
    setViewMode(KisPresetChooser::ViewMode::THUMBNAIL);
}

void KisPresetChooser::setViewModeToDetail()
{
    setViewMode(KisPresetChooser::ViewMode::DETAIL);
}

void KisPresetChooser::setViewModeToStroke()
{
    setViewMode(KisPresetChooser::ViewMode::STROKE);
}

void KisPresetChooser::notifyConfigChanged()
{
    KisConfig cfg(true);
    m_delegate->setUseDirtyPresets(cfg.useDirtyPresets());
    int newIconSize = cfg.presetIconSize();
    m_delegate->setIconSize(newIconSize);
    setIconSize(newIconSize);
}

void KisPresetChooser::updateViewSettings()
{
    switch (m_mode) {
    case ViewMode::THUMBNAIL: {
        m_chooser->setListViewMode(ListViewMode::IconGrid);
        m_delegate->setShowText(false);
        m_delegate->setViewMode(ViewMode::THUMBNAIL);
        break;
    }
    case ViewMode::DETAIL: {
        m_chooser->setListViewMode(ListViewMode::Detail);
        m_delegate->setShowText(true);
        m_delegate->setViewMode(ViewMode::DETAIL);
        break;
    }
    case ViewMode::STROKE: {
        m_chooser->setListViewMode(ListViewMode::IconGrid);
        m_delegate->setShowText(false);
        m_delegate->setViewMode(ViewMode::STROKE);

        int currentIconSize = iconSize();
        m_delegate->setIconSize(currentIconSize);
        QSize thumbnailSize = KisPresetDelegate::calculateStrokeThumbnailSize(currentIconSize);

        m_chooser->itemView()->setGridSize(thumbnailSize);
        m_chooser->itemView()->setIconSize(thumbnailSize);
        m_chooser->itemView()->doItemsLayout();
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

    if (m_mode == ViewMode::STROKE) {
        m_delegate->setIconSize(newSize);
        QSize thumbnailSize = KisPresetDelegate::calculateStrokeThumbnailSize(newSize);

        m_chooser->itemView()->setGridSize(thumbnailSize);
        m_chooser->itemView()->setIconSize(thumbnailSize);
        m_chooser->itemView()->doItemsLayout();
    }

    KisBrushStrokePreviewCache::instance()->generateAllPreviews();
}

int KisPresetChooser::iconSize()
{
    KisResourceItemChooserSync* chooserSync = KisResourceItemChooserSync::instance();
    return chooserSync->baseLength();
}

void KisPresetChooser::saveIconSize()
{
    if (KisConfig(true).presetIconSize() != iconSize()) {
        KisConfig(false).setPresetIconSize(iconSize());
    }
}

void KisPresetChooser::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::PaletteChange) {
        if (m_mode == ViewMode::STROKE) {
            m_chooser->itemView()->viewport()->update();
        }
    }
}

void KisPresetChooser::showHideBrushNames(ListViewMode newViewMode)
{
    if (m_mode == ViewMode::STROKE) {
        m_delegate->setShowText(false);
        // Restore non-square grid size.
        int currentIconSize = iconSize();
        QSize thumbnailSize = KisPresetDelegate::calculateStrokeThumbnailSize(currentIconSize);
        m_chooser->itemView()->setGridSize(thumbnailSize);
        m_chooser->itemView()->setIconSize(thumbnailSize);
        m_chooser->itemView()->doItemsLayout();
        return;
    }
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

void KisPresetChooser::slotStrokePreviewReady(int presetId)
{
    if (m_mode != ViewMode::STROKE) {
        return;
    }

    KisTagFilterResourceProxyModel *model = m_chooser->tagFilterModel();
    if (!model) {
        m_chooser->itemView()->viewport()->update();
        return;
    }

    QModelIndex index = model->indexForResourceId(presetId);
    if (!index.isValid()) {
        return;
    }

    const QRect rect = m_chooser->itemView()->visualRect(index);
    if (!rect.isValid()) {
        return;
    }

    m_chooser->itemView()->viewport()->update(rect);
}

void KisPresetChooser::slotGenerateStrokePreviews()
{
    KisBrushStrokePreviewCache::instance()->generateAllPreviews();
}

void KisPresetChooser::slotBaseLengthChanged(int length)
{
    if (m_mode != ViewMode::STROKE) {
        return;
    }

    m_delegate->setIconSize(length);
    QSize thumbnailSize = KisPresetDelegate::calculateStrokeThumbnailSize(length);

    m_chooser->itemView()->setGridSize(thumbnailSize);
    m_chooser->itemView()->setIconSize(thumbnailSize);
    m_chooser->itemView()->doItemsLayout();
}