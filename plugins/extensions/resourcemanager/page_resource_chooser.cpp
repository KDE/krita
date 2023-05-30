#include "page_resource_chooser.h"
#include "ui_pageresourcechooser.h"
#include "wdg_resource_preview.h"
#include "kisresourceitemviwer.h"
#include <kis_config.h>

#include <QPainter>
#include <QDebug>
#include <QLabel>

#include <KisResourceModel.h>
#include <KisTagFilterResourceProxyModel.h>

#define ICON_SIZE 48

PageResourceChooser::PageResourceChooser(KoResourceBundleSP bundle, QWidget *parent) :
    QWizardPage(parent),
    m_ui(new Ui::PageResourceChooser)
    , m_bundle(bundle)
{
    m_ui->setupUi(this);

    m_wdgResourcePreview = new WdgResourcePreview(0, this);
    m_ui->formLayout->addWidget(m_wdgResourcePreview);

    connect(m_wdgResourcePreview, SIGNAL(signalResourcesSelectionChanged(QModelIndex)), this, SLOT(slotResourcesSelectionChanged(QModelIndex)));
    connect(m_wdgResourcePreview, SIGNAL(resourceTypeSelected(int)), this, SLOT(slotresourceTypeSelected(int)));

//     m_ui->resourceItemSelectedView->setFixedToolTipThumbnailSize(QSize(128, 128));

    m_ui->tableSelected->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    m_ui->tableSelected->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // btnRemoveSelected
    connect(m_ui->btnRemoveSelected, SIGNAL(clicked(bool)), this, SLOT(slotRemoveSelected(bool)));

    KisResourceItemViwer *viewModeButton;
    viewModeButton = new KisResourceItemViwer(2, this);

    KisConfig cfg(true);
    m_mode = (cfg.readEntry<quint32>("ResourceItemsBCSelected.viewMode", 1) == 1)? ListViewMode::IconGrid : ListViewMode::Detail;

    if (m_mode == ListViewMode::IconGrid) {
        slotViewThumbnail();
    } else {
        slotViewDetails();
    }

    connect(viewModeButton, SIGNAL(onViewThumbnail()), this, SLOT(slotViewThumbnail()));
    connect(viewModeButton, SIGNAL(onViewDetails()), this, SLOT(slotViewDetails()));

    QLabel *label = new QLabel("Selected");
    m_ui->horizontalLayout_2->addWidget(label);

    m_ui->horizontalLayout_2->addWidget(viewModeButton);
}

void PageResourceChooser::slotViewThumbnail()
{
//     m_kisResourceItemDelegate->setShowText(false);
//     m_ui->resourceItemView->setItemDelegate(m_kisResourceItemDelegate);
//     m_ui->resourceItemView->setListViewMode(ListViewMode::IconGrid);
}

void PageResourceChooser::slotViewDetails()
{
//     m_kisResourceItemDelegate->setShowText(true);
//     m_ui->resourceItemView->setItemDelegate(m_kisResourceItemDelegate);
//     m_ui->resourceItemView->setListViewMode(ListViewMode::Detail);
//     m_ui->resourceItemView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void PageResourceChooser::slotResourcesSelectionChanged(QModelIndex selected)
{
    QModelIndexList list = m_wdgResourcePreview->geResourceItemsSelected();
    KisTagFilterResourceProxyModel* proxyModel = m_wdgResourcePreview->getResourceProxyModelsForResourceType()[m_wdgResourcePreview->getCurrentResourceType()];

//     m_ui->resourceItemSelectedView->setModel(proxyModel);
//     m_kisResourceItemDelegate = new KisResourceItemDelegate(this);
//     m_ui->resourceItemSelectedView->setItemDelegate(m_kisResourceItemDelegate);
//     m_ui->resourceItemSelectedView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    Q_FOREACH (QModelIndex idx, list) {
        int id = proxyModel->data(idx, Qt::UserRole + KisAllResourcesModel::Id).toInt();
        QImage image = (proxyModel->data(idx, Qt::UserRole + KisAllResourcesModel::Thumbnail)).value<QImage>();
        QString name = proxyModel->data(idx, Qt::UserRole + KisAllResourcesModel::Name).toString();

        // Function imageToIcon(QImage()) returns a square white pixmap and a warning "QImage::scaled: Image is a null image"
        //  while QPixmap() returns an empty pixmap.
        // The difference between them is relevant in case of Workspaces which has no images.
        // Using QPixmap() makes them appear in a dense list without icons, while imageToIcon(QImage())
        //  would give a list with big white rectangles and names of the workspaces.
        Qt::AspectRatioMode scalingAspectRatioMode = Qt::KeepAspectRatio;
        if (image.height() == 1) { // affects mostly gradients, which are very long but only 1px tall
            scalingAspectRatioMode = Qt::IgnoreAspectRatio;
        }
        QListWidgetItem *item = new QListWidgetItem(image.isNull() ? QPixmap() : imageToIcon(image, scalingAspectRatioMode), name);
        item->setData(Qt::UserRole, id);

        if (m_selectedResourcesIds.contains(id) == false) {
            m_ui->tableSelected->addItem(item);
            m_selectedResourcesIds.append(id);
        }
    }

    m_ui->tableSelected->sortItems();
}

void PageResourceChooser::slotresourceTypeSelected(int idx)
{
    QString resourceType = m_wdgResourcePreview->getCurrentResourceType();
    m_ui->tableSelected->clear();
    QString standarizedResourceType = (resourceType == "presets" ? ResourceType::PaintOpPresets : resourceType);

    KisResourceModel model(standarizedResourceType);
    for (int i = 0; i < model.rowCount(); i++) {
        QModelIndex idx = model.index(i, 0);
        QString filename = model.data(idx, Qt::UserRole + KisAbstractResourceModel::Filename).toString();
        int id = model.data(idx, Qt::UserRole + KisAbstractResourceModel::Id).toInt();

        if (resourceType == ResourceType::Gradients) {
            if (filename == "Foreground to Transparent" || filename == "Foreground to Background") {
                continue;
            }
        }

        QImage image = (model.data(idx, Qt::UserRole + KisAbstractResourceModel::Thumbnail)).value<QImage>();
        QString name = model.data(idx, Qt::UserRole + KisAbstractResourceModel::Name).toString();

        Qt::AspectRatioMode scalingAspectRatioMode = Qt::KeepAspectRatio;
        if (image.height() == 1) { // affects mostly gradients, which are very long but only 1px tall
            scalingAspectRatioMode = Qt::IgnoreAspectRatio;
        }
        QListWidgetItem *item = new QListWidgetItem(image.isNull() ? QPixmap() : imageToIcon(image, scalingAspectRatioMode), name);
        item->setData(Qt::UserRole, id);

        if (m_selectedResourcesIds.contains(id)) {
            m_ui->tableSelected->addItem(item);
        }
    }

    m_ui->tableSelected->sortItems();
}

void PageResourceChooser::slotRemoveSelected(bool)
{
    int row = m_ui->tableSelected->currentRow();

    Q_FOREACH (QListWidgetItem *item, m_ui->tableSelected->selectedItems()) {
        m_ui->tableSelected->takeItem(m_ui->tableSelected->row(item));
        m_selectedResourcesIds.removeAll(item->data(Qt::UserRole).toInt());
    }

    m_ui->tableSelected->setCurrentRow(row);
}

QPixmap PageResourceChooser::imageToIcon(const QImage &img, Qt::AspectRatioMode aspectRatioMode)
{
    QPixmap pixmap(ICON_SIZE, ICON_SIZE);
    pixmap.fill();
    QImage scaled = img.scaled(ICON_SIZE, ICON_SIZE, aspectRatioMode, Qt::SmoothTransformation);
    int x = (ICON_SIZE - scaled.width()) / 2;
    int y = (ICON_SIZE - scaled.height()) / 2;
    QPainter gc(&pixmap);
    gc.drawImage(x, y, scaled);
    gc.end();
    return pixmap;
}

QList<int> PageResourceChooser::getSelectedResourcesIds()
{
    return m_selectedResourcesIds;
}

PageResourceChooser::~PageResourceChooser()
{
    delete m_ui;
}
