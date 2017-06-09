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

#include "ShapeCollectionDocker.h"

#include "CollectionItemModel.h"

#include <KoShapeFactoryBase.h>
#include <KoShapeRegistry.h>
#include <KoCanvasController.h>
#include <KoToolManager.h>
#include <KoCreateShapesTool.h>
#include <KoShape.h>
#include <KoZoomHandler.h>
#include <KoShapePaintingContext.h>

#include <KoIcon.h>

#include <klocalizedstring.h>
#include <KoResourcePaths.h>
#include <kdesktopfile.h>
#include <kconfiggroup.h>
#include <kmessagebox.h>
#include <ksharedconfig.h>

#include <QGridLayout>
#include <QListView>
#include <QListWidget>
#include <QStandardItemModel>
#include <QList>
#include <QSize>
#include <QToolButton>
#include <QDir>
#include <QMenu>
#include <QPainter>
#include <QDebug>

//This class is needed so that the menu returns a sizehint based on the layout and not on the number (0) of menu items
class CollectionMenu : public QMenu
{
public:
    CollectionMenu(QWidget *parent = 0);
    QSize sizeHint() const override;
};

CollectionMenu::CollectionMenu(QWidget *parent)
    : QMenu(parent)
{
}
QSize CollectionMenu::sizeHint() const
{
    return layout()->sizeHint();
}

//
// ShapeCollectionDockerFactory
//

ShapeCollectionDockerFactory::ShapeCollectionDockerFactory()
    : KoDockFactoryBase()
{
}

QString ShapeCollectionDockerFactory::id() const
{
    return QString("ShapeCollectionDocker");
}

QDockWidget *ShapeCollectionDockerFactory::createDockWidget()
{
    ShapeCollectionDocker *docker = new ShapeCollectionDocker();

    return docker;
}


//
// ShapeCollectionDocker
//
ShapeCollectionDocker::ShapeCollectionDocker(QWidget *parent)
    : QDockWidget(parent)
{
    setWindowTitle(i18n("Add Shape"));

    m_quickView = new QListView(this);
    m_quickView->setViewMode(QListView::IconMode);
    m_quickView->setDragDropMode(QListView::DragOnly);
    m_quickView->setSelectionMode(QListView::SingleSelection);
    m_quickView->setResizeMode(QListView::Adjust);
    m_quickView->setFlow(QListView::LeftToRight);
    m_quickView->setGridSize(QSize(128, 128));
    m_quickView->setTextElideMode(Qt::ElideNone);
    m_quickView->setWordWrap(true);
    setWidget(m_quickView);

    // Load the default shapes and add them to the combobox
    loadDefaultShapes();
}

void ShapeCollectionDocker::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
}

void ShapeCollectionDocker::unsetCanvas()
{
    setEnabled(false);
}

void ShapeCollectionDocker::loadDefaultShapes()
{
    QList<KoCollectionItem> quicklist;

    Q_FOREACH (const QString &id, KoShapeRegistry::instance()->keys()) {
        KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value(id);
        // don't show hidden factories
        if (factory->hidden()) {
            continue;
        }
        bool oneAdded = false;

        Q_FOREACH (const KoShapeTemplate &shapeTemplate, factory->templates()) {

            oneAdded = true;
            KoCollectionItem temp;
            temp.id = shapeTemplate.id;
            temp.name = shapeTemplate.name;
            temp.toolTip = shapeTemplate.toolTip;
            temp.icon = KisIconUtils::loadIcon(shapeTemplate.iconName);
            temp.properties = shapeTemplate.properties;
            quicklist.append(temp);

            QString id = temp.id;
            if (!shapeTemplate.templateId.isEmpty()) {
                id += '_' + shapeTemplate.templateId;
            }

        }

        if (!oneAdded) {
            KoCollectionItem temp;
            temp.id = factory->id();
            temp.name = factory->name();
            temp.toolTip = factory->toolTip();
            temp.icon = KisIconUtils::loadIcon(factory->iconName());
            temp.properties = 0;
            quicklist.append(temp);
        }
    }

    CollectionItemModel *quickModel = new CollectionItemModel(this);
    quickModel->setShapeTemplateList(quicklist);
    m_quickView->setModel(quickModel);

}
