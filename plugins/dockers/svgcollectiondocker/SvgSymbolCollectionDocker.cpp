/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Peter Simonsson <peter.simonsson@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "SvgSymbolCollectionDocker.h"

#include <klocalizedstring.h>

#include <QDebug>
#include <QAbstractListModel>
#include <QMimeData>
#include <QDomDocument>
#include <QDomElement>
#include <KisSqueezedComboBox.h>
#include <QWidgetAction>
#include <QMenu>

#include <KoResourceServerProvider.h>
#include <KoResourceServer.h>
#include <KoShapeFactoryBase.h>
#include <KoProperties.h>
#include <KoDrag.h>

#include "kis_icon_utils.h"
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "ui_WdgSvgCollection.h"

#include <resources/KoSvgSymbolCollectionResource.h>

//
// SvgCollectionModel
//
SvgCollectionModel::SvgCollectionModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant SvgCollectionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() > m_symbolCollection->symbols().count()) {
        return QVariant();
    }

    switch (role) {
    case Qt::ToolTipRole:
        return m_symbolCollection->symbols()[index.row()]->title;

    case Qt::DecorationRole:
    {
        QPixmap px = QPixmap::fromImage(m_symbolCollection->symbols()[index.row()]->icon());
        QIcon icon(px);
        return icon;
    }
    case Qt::UserRole:
        return m_symbolCollection->symbols()[index.row()]->id;

    case Qt::DisplayRole:
        return m_symbolCollection->symbols()[index.row()]->title;

    default:
        return QVariant();
    }

    return QVariant();
}

int SvgCollectionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_symbolCollection->symbols().count();
}

QMimeData *SvgCollectionModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty()) {
        return 0;
    }

    QModelIndex index = indexes.first();

    if (!index.isValid()) {
        return 0;
    }

    if (m_symbolCollection->symbols().isEmpty()) {
        return 0;
    }

    QList<KoShape*> shapes;
    shapes.append(m_symbolCollection->symbols()[index.row()]->shape);
    KoDrag drag;
    drag.setSvg(shapes);
    QMimeData *mimeData = drag.mimeData();

    return mimeData;
}

QStringList SvgCollectionModel::mimeTypes() const
{
    return QStringList() << SHAPETEMPLATE_MIMETYPE << "image/svg+xml";
}

Qt::ItemFlags SvgCollectionModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        return QAbstractListModel::flags(index) | Qt::ItemIsDragEnabled;
    }
    return QAbstractListModel::flags(index);
}

Qt::DropActions SvgCollectionModel::supportedDragActions() const
{
    return Qt::CopyAction;
}

void SvgCollectionModel::setSvgSymbolCollectionResource(QSharedPointer<KoSvgSymbolCollectionResource> resource)
{
    m_symbolCollection = resource;
}



//
// SvgSymbolCollectionDockerFactory
//

SvgSymbolCollectionDockerFactory::SvgSymbolCollectionDockerFactory()
    : KoDockFactoryBase()
{
}

QString SvgSymbolCollectionDockerFactory::id() const
{
    return QString("SvgSymbolCollectionDocker");
}

QDockWidget *SvgSymbolCollectionDockerFactory::createDockWidget()
{
    SvgSymbolCollectionDocker *docker = new SvgSymbolCollectionDocker();

    return docker;
}

//
// SvgSymbolCollectionDocker
//

SvgSymbolCollectionDocker::SvgSymbolCollectionDocker(QWidget *parent)
    : QDockWidget(parent)
    , m_wdgSvgCollection(new Ui_WdgSvgCollection())
{
    setWindowTitle(i18n("Vector Libraries"));
    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);
    m_wdgSvgCollection->setupUi(mainWidget);

    connect(m_wdgSvgCollection->cmbCollections, SIGNAL(activated(int)), SLOT(collectionActivated(int)));

    m_resourceModel = new KisResourceModel(ResourceType::Symbols, this);

    m_wdgSvgCollection->cmbCollections->setModel(m_resourceModel);
    m_wdgSvgCollection->cmbCollections->setModelColumn(KisAbstractResourceModel::Name);

    m_wdgSvgCollection->listCollection->setDragEnabled(true);
    m_wdgSvgCollection->listCollection->setDragDropMode(QAbstractItemView::DragOnly);
    m_wdgSvgCollection->listCollection->setSelectionMode(QListView::SingleSelection);

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(m_wdgSvgCollection->listCollection);
    if (scroller) {
        connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                this, SLOT(slotScrollerStateChanged(QScroller::State)));
    }

    // thumbnail icon changer
    QMenu* configureMenu = new QMenu(this);
    configureMenu->setStyleSheet("margin: 6px");
    m_wdgSvgCollection->vectorPresetsConfigureButton->setIcon(KisIconUtils::loadIcon("hamburger_menu_dots"));
    m_wdgSvgCollection->vectorPresetsConfigureButton->setPopupMode(QToolButton::InstantPopup);
    m_wdgSvgCollection->vectorPresetsConfigureButton->setAutoRaise(true);



    // add horizontal slider for changing the icon size
    m_iconSizeSlider = new QSlider(this);
    m_iconSizeSlider->setOrientation(Qt::Horizontal);
    m_iconSizeSlider->setRange(20, 80);
    m_iconSizeSlider->setValue(20);  // defaults to small icon size
    m_iconSizeSlider->setMinimumHeight(20);
    m_iconSizeSlider->setMinimumWidth(40);
    m_iconSizeSlider->setTickInterval(10);


    QWidgetAction *sliderAction= new QWidgetAction(this);
    sliderAction->setDefaultWidget(m_iconSizeSlider);

    configureMenu->addSection(i18n("Icon Size"));
    configureMenu->addAction(sliderAction);

    m_wdgSvgCollection->vectorPresetsConfigureButton->setMenu(configureMenu);
    connect(m_iconSizeSlider, SIGNAL(sliderReleased()), this, SLOT(slotSetIconSize())); // resizing while sliding is too heavy of an operation


    KConfigGroup cfg =  KSharedConfig::openConfig()->group("SvgSymbolCollection");
    int i = cfg.readEntry("currentCollection", 0);
    if (i > m_wdgSvgCollection->cmbCollections->count()) {
        i = 0;
    }
    m_wdgSvgCollection->cmbCollections->setCurrentIndex(i);
    collectionActivated(i);

    connect(m_resourceModel, SIGNAL(modelAboutToBeReset()), this, SLOT(slotResourceModelAboutToBeReset()));
    connect(m_resourceModel, SIGNAL(modelReset()), this, SLOT(slotResourceModelReset()));
}

SvgSymbolCollectionDocker::~SvgSymbolCollectionDocker()
{
    clearModels();
}

void SvgSymbolCollectionDocker::slotSetIconSize()
{
    m_wdgSvgCollection->listCollection->setIconSize(QSize(m_iconSizeSlider->value(),m_iconSizeSlider->value()));
}

void SvgSymbolCollectionDocker::slotResourceModelAboutToBeReset()
{
    int index = m_wdgSvgCollection->cmbCollections->currentIndex();
    QModelIndex idx = m_resourceModel->index(index, 0);
    int id = m_resourceModel->data(idx, Qt::UserRole + KisAbstractResourceModel::Id).toInt();
    m_rememberedSvgCollectionId = id;
}

void SvgSymbolCollectionDocker::slotResourceModelReset()
{
     int indexToSet = 0;
    if (m_rememberedSvgCollectionId < 0) {
        indexToSet = 0;
    } else {
        for (int i = 0; i < m_resourceModel->rowCount(); i++) {
            QModelIndex idx = m_resourceModel->index(i, 0);
            int id = m_resourceModel->data(idx, Qt::UserRole + KisAbstractResourceModel::Id).toInt();
            if (id == m_rememberedSvgCollectionId) {
                indexToSet = i;
                break;
            }
        }
    }
    // remove the current model from the view
    m_wdgSvgCollection->listCollection->setModel(0);
    // delete all models
    clearModels();
    // setting current index will create and set the model
    m_wdgSvgCollection->cmbCollections->setCurrentIndex(indexToSet);
    collectionActivated(indexToSet);
    m_rememberedSvgCollectionId = -1;
}

void SvgSymbolCollectionDocker::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
}

void SvgSymbolCollectionDocker::unsetCanvas()
{
    setEnabled(false);
}

void SvgSymbolCollectionDocker::collectionActivated(int index)
{
    if (index < m_resourceModel->rowCount()) {
        SvgCollectionModel *model;
        if (m_collectionsModelsCache.contains(index)) {
            model = m_collectionsModelsCache.value(index);
        } else {
            QModelIndex idx = m_resourceModel->index(index, 0);
            QSharedPointer<KoSvgSymbolCollectionResource> r = m_resourceModel->resourceForIndex(idx).dynamicCast<KoSvgSymbolCollectionResource>();
            model = new SvgCollectionModel();
            model->setSvgSymbolCollectionResource(r);
            m_collectionsModelsCache.insert(index, model);
        }

        KConfigGroup cfg =  KSharedConfig::openConfig()->group("SvgSymbolCollection");
        cfg.writeEntry("currentCollection", index);

        m_wdgSvgCollection->listCollection->setModel(model);

    }

}

void SvgSymbolCollectionDocker::clearModels()
{
    Q_FOREACH(int key, m_collectionsModelsCache.keys()) {
        delete m_collectionsModelsCache.value(key);
    }
    m_collectionsModelsCache.clear();
}
