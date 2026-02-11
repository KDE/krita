/*
 *  SPDX-FileCopyrightText: 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisGamutMaskChooser.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QAbstractItemDelegate>
#include <QMenu>
#include <QActionGroup>
#include <QFontMetrics>
#include <QTextDocument>
#include <QTextLayout>

#include <KisResourceItemChooser.h>
#include <KisResourceItemListView.h>
#include <KisResourceModel.h>
#include <KisResourceThumbnailCache.h>
#include <kis_icon_utils.h>
#include <kis_config.h>
#include "KisPopupButton.h"
#include <KisResourceModelProvider.h>
#include <KisResourceMetaDataModel.h>


/// The resource item delegate for rendering the resource preview
class KisGamutMaskDelegate: public QAbstractItemDelegate
{
public:
    KisGamutMaskDelegate(KisResourceMetaDataModel *metaDataModel, QObject * parent = 0) : QAbstractItemDelegate(parent)
      , m_mode(KisGamutMaskChooser::ViewMode::THUMBNAIL) 
      , m_metaDataModel(metaDataModel)
    {
    }
    ~KisGamutMaskDelegate() override {}

    /// reimplemented
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;

    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override {
        return option.decorationSize;
    }

    void setViewMode(KisGamutMaskChooser::ViewMode mode) {
        m_mode = mode;
    }

private:
    KisGamutMaskChooser::ViewMode m_mode;
    KisResourceMetaDataModel *m_metaDataModel;
};

void KisGamutMaskDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (!index.isValid())
        return;

    QImage preview = index.data(Qt::UserRole + KisAbstractResourceModel::Thumbnail).value<QImage>();
    QString name = index.data(Qt::UserRole + KisAbstractResourceModel::Name).value<QString>();

    if(preview.isNull()) {
        return;
    }

    QRect paintRect = option.rect.adjusted(1, 1, -1, -1);

    qreal devicePixelRatioF = painter->device()->devicePixelRatioF();

    if (m_mode == KisGamutMaskChooser::ViewMode::THUMBNAIL) {
        QImage previewHighDPI =
            KisResourceThumbnailCache::instance()->getImage(index,
                                                            paintRect.size() * devicePixelRatioF,
                                                            Qt::IgnoreAspectRatio,
                                                            Qt::SmoothTransformation);
        previewHighDPI.setDevicePixelRatio(devicePixelRatioF);
        painter->drawImage(paintRect.x(), paintRect.y(), previewHighDPI);

        if (option.state & QStyle::State_Selected) {
            painter->setCompositionMode(QPainter::CompositionMode_Overlay);
            painter->setOpacity(0.5);
            painter->fillRect(paintRect, Qt::white);
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter->setOpacity(1.0);
            painter->setPen(QPen(option.palette.highlight(), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
            QRect selectedBorder = option.rect.adjusted(1, 1, -1, -1);
            painter->drawRect(selectedBorder);
        }
    } else {
        QSize previewSize(paintRect.height(), paintRect.height());

        QImage previewHighDPI =
            KisResourceThumbnailCache::instance()->getImage(index,
                                                            previewSize * devicePixelRatioF,
                                                            Qt::IgnoreAspectRatio,
                                                            Qt::SmoothTransformation);
        previewHighDPI.setDevicePixelRatio(devicePixelRatioF);

        if (option.state & QStyle::State_Selected) {
            painter->setPen(QPen(option.palette.highlight(), 2.0));
            painter->fillRect(option.rect, option.palette.highlight());
            painter->setPen(option.palette.highlightedText().color());
        }
        painter->drawImage(paintRect.x(), paintRect.y(), previewHighDPI);

        int leftMargin = 8;
        int rightMargin = 7;
        int vertMargin = 4;
        int descOffset = 7;
        QFont font = option.font;
        font.setBold(true);
        painter->setFont(font);
        QRectF titleRect(QPointF(previewSize.width() + leftMargin, paintRect.y() + vertMargin),
                       QPointF(paintRect.width() - rightMargin, paintRect.y() + descOffset + painter->fontMetrics().lineSpacing()));
        painter->drawText(titleRect, Qt::AlignLeft,
                          painter->fontMetrics().elidedText(
                              name, Qt::ElideRight, titleRect.width()
                              )
                          );
        const QString descriptionKey = "description";
        const int resourceId = index.data(Qt::UserRole + KisAbstractResourceModel::Id).toInt();
        const QString description = m_metaDataModel->metaDataValue(resourceId, descriptionKey).toString();
        if (!description.isEmpty() && !description.isNull()) {
            // Font size may be in pixels (on Android) or points (everywhere else.)
            if (font.pixelSize() == -1) {
                font.setPointSize(font.pointSize() - 1);
            } else {
                font.setPixelSize(font.pixelSize() - 1);
            }
            font.setBold(false);
            font.setStyle(QFont::StyleItalic);
            painter->setFont(font);
            QRectF descRect(QPointF(previewSize.width() + leftMargin, paintRect.y() + descOffset + painter->fontMetrics().lineSpacing()),
                            QPointF(paintRect.right() - rightMargin, paintRect.bottom() - vertMargin));

            int numLines = floor(((float)descRect.height() / (float)painter->fontMetrics().lineSpacing()));
            if (numLines > 0) {
                int elideWidth = 0;
                QTextLayout textLayout(description);
                textLayout.beginLayout();
                for (int i = 0; i < numLines; i++) {
                    QTextLine line = textLayout.createLine();
                    if (line.isValid()) {
                        line.setLineWidth(descRect.width());
                        elideWidth += line.naturalTextWidth();
                    }
                }

                QString elidedText = painter->fontMetrics().elidedText(description, Qt::ElideRight, elideWidth);
                painter->drawText(descRect, Qt::AlignLeft|Qt::TextWordWrap, elidedText);
            }
        }
    }

    painter->restore();
}


KisGamutMaskChooser::KisGamutMaskChooser(QWidget *parent) 
    : QWidget(parent)
{
    m_delegate = new KisGamutMaskDelegate(KisResourceModelProvider::resourceMetadataModel(), this);

    m_itemChooser = new KisResourceItemChooser(ResourceType::GamutMasks, false, this);
    m_itemChooser->setItemDelegate(m_delegate);
    m_itemChooser->showTaggingBar(true);
    m_itemChooser->showImportExportBtns(false);
    m_itemChooser->setSynced(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    // TODO: menu for view mode change
    QMenu* menu = new QMenu(this);
    menu->setStyleSheet("margin: 6px");
    menu->addSection(i18nc("@title Which elements to display (e.g., thumbnails or details)", "Display"));

    QActionGroup *actionGroup = new QActionGroup(this);

    KisConfig cfg(true);
    m_mode = KisGamutMaskChooser::ViewMode(cfg.readEntry<quint32>("GamutMasks.viewMode", KisGamutMaskChooser::THUMBNAIL));

    QAction* action = menu->addAction(KisIconUtils::loadIcon("view-preview"),
                                      i18n("Thumbnails"), this, SLOT(slotSetModeThumbnail()));
    action->setCheckable(true);
    action->setChecked(m_mode == KisGamutMaskChooser::THUMBNAIL);
    action->setActionGroup(actionGroup);

    action = menu->addAction(KisIconUtils::loadIcon("view-list-details"),
                             i18n("Details"), this, SLOT(slotSetModeDetail()));
    action->setCheckable(true);
    action->setChecked(m_mode == KisGamutMaskChooser::DETAIL);
    action->setActionGroup(actionGroup);

    // setting the view mode
    setViewMode(m_mode);
    m_itemChooser->showViewModeBtn(true);
    KisPopupButton* viewModeButton = m_itemChooser->viewModeButton();
    viewModeButton->setPopupWidget(menu);

    layout->addWidget(m_itemChooser);

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResourceSP )), this, SLOT(resourceSelected(KoResourceSP )));
}

KisGamutMaskChooser::~KisGamutMaskChooser()
{

}

void KisGamutMaskChooser::setCurrentResource(KoResourceSP resource)
{
    m_itemChooser->setCurrentResource(resource);
}

void KisGamutMaskChooser::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateViewSettings();
}

void KisGamutMaskChooser::setViewMode(KisGamutMaskChooser::ViewMode mode)
{
    m_mode = mode;
    updateViewSettings();
}

void KisGamutMaskChooser::updateViewSettings()
{
    KisConfig cfg(false);
    cfg.writeEntry("GamutMasks.viewMode", qint32(m_mode));

    if (m_mode == KisGamutMaskChooser::THUMBNAIL) {
        m_itemChooser->setListViewMode(ListViewMode::IconGrid);
        m_itemChooser->setRowHeight(this->fontMetrics().lineSpacing() * 4);
        m_itemChooser->setColumnWidth(this->fontMetrics().lineSpacing() * 4);
        m_delegate->setViewMode(m_mode);
    }
    else if (m_mode == KisGamutMaskChooser::DETAIL) {
        m_itemChooser->setListViewMode(ListViewMode::Detail);
        m_itemChooser->setRowHeight(this->fontMetrics().lineSpacing()*4);
        m_delegate->setViewMode(m_mode);
    }
}

void KisGamutMaskChooser::resourceSelected(KoResourceSP resource)
{
    Q_EMIT sigGamutMaskSelected(resource.staticCast<KoGamutMask>());
}

void KisGamutMaskChooser::slotSetModeThumbnail()
{
    setViewMode(KisGamutMaskChooser::THUMBNAIL);
}

void KisGamutMaskChooser::slotSetModeDetail()
{
    setViewMode(KisGamutMaskChooser::DETAIL);
}
