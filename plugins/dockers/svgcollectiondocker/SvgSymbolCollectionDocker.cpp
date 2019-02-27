/* This file is part of the KDE project
 * Copyright (C) 2008 Peter Simonsson <peter.simonsson@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

void SvgCollectionModel::setSvgSymbolCollectionResource(KoSvgSymbolCollectionResource *resource)
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

    KoResourceServer<KoSvgSymbolCollectionResource>  *svgCollectionProvider = KoResourceServerProvider::instance()->svgSymbolCollectionServer();
    Q_FOREACH(KoSvgSymbolCollectionResource *r, svgCollectionProvider->resources()) {
        m_wdgSvgCollection->cmbCollections->addSqueezedItem(r->name());
        SvgCollectionModel *model = new SvgCollectionModel();
        model->setSvgSymbolCollectionResource(r);
        m_models.append(model);
    }

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
    m_wdgSvgCollection->vectorPresetsConfigureButton->setIcon(KisIconUtils::loadIcon("configure"));
    m_wdgSvgCollection->vectorPresetsConfigureButton->setPopupMode(QToolButton::InstantPopup);



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
}

void SvgSymbolCollectionDocker::slotSetIconSize()
{
    m_wdgSvgCollection->listCollection->setIconSize(QSize(m_iconSizeSlider->value(),m_iconSizeSlider->value()));
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
    if (index < m_models.size()) {
        KConfigGroup cfg =  KSharedConfig::openConfig()->group("SvgSymbolCollection");
        cfg.writeEntry("currentCollection", index);
        m_wdgSvgCollection->listCollection->setModel(m_models[index]);
    }

}
