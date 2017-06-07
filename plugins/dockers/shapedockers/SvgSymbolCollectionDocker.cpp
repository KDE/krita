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

#include <KoResourceServerProvider.h>
#include <KoResourceServer.h>

#include "ui_WdgSvgCollection.h"

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
        QVariant v = QVariant::fromValue<KoSvgSymbolCollectionResource*>(r);
        m_wdgSvgCollection->cmbCollections->addItem(r->name(), v);
    }

    m_wdgSvgCollection->listCollection->setDragEnabled(true);
    m_wdgSvgCollection->listCollection->setDragDropMode(QAbstractItemView::DragOnly);
    m_wdgSvgCollection->listCollection->setSelectionMode(QListView::SingleSelection);

    collectionActivated(0);

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
    QVariant v = m_wdgSvgCollection->cmbCollections->itemData(index);
    KoSvgSymbolCollectionResource *r = v.value<KoSvgSymbolCollectionResource *>();
    if (r) {
        m_wdgSvgCollection->listCollection->clear();
        Q_FOREACH(KoSvgSymbol *symbol, r->symbols()) {
            QListWidgetItem *item = new QListWidgetItem(symbol->title);
            item->setIcon(QIcon(QPixmap::fromImage(symbol->icon)));
            m_wdgSvgCollection->listCollection->addItem(item);
        }
    }

}
