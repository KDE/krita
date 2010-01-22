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
#ifndef KOSHAPECOLLECTIONDOCKER_H
#define KOSHAPECOLLECTIONDOCKER_H

#include <QDockWidget>
#include <QModelIndex>
#include <QMap>
#include <QIcon>

#include <KoDockFactoryBase.h>

class ShapeCollectionDockerFactory : public KoDockFactoryBase
{
    public:
        ShapeCollectionDockerFactory();

        virtual QString id() const;
        virtual QDockWidget* createDockWidget();
        DockPosition defaultDockPosition() const
        {
            return DockRight;
        }
};

class CollectionItemModel;
class KoShape;
class QListView;
class QListWidget;
class QListWidgetItem;
class QToolButton;
class QMenu;
class QSpacerItem;
class QGridLayout;

class ShapeCollectionDocker : public QDockWidget
{
    Q_OBJECT
    public:
        explicit ShapeCollectionDocker(QWidget* parent = 0);

    protected slots:
        /**
         * Activates the shape creation tool when a shape is selected.
         */
        void activateShapeCreationTool(const QModelIndex& index);
        void activateShapeCreationToolFromQuick(const QModelIndex& index);

        /**
         * Changes the current shape collection
         */
        void activateShapeCollection(QListWidgetItem *item);

        /**
         * Called when a collection is added from the add collection menu
         */
        void loadCollection();

        /// Called when an error occurred while loading a collection
        void onLoadingFailed(const QString& reason);

        /// Called when loading of a collection is finished
        void onLoadingFinished();

        /// Called when the close collection button is clicked
        void removeCurrentCollection();

        /// Called when the docker changes area
        void locationChanged(Qt::DockWidgetArea area);

    protected:
        /**
         * Load the default koffice shapes
         */
        void loadDefaultShapes();

        /**
         * Add a collection to the docker
         */
        bool addCollection(const QString& id, const QString& title, CollectionItemModel* model);
        void removeCollection(const QString& id);

        /**
         * Builds the menu for the Add Collection Button
         */
        void buildAddCollectionMenu();

        /// Generate an icon from @p shape
        QIcon generateShapeIcon(KoShape* shape);

    private:
        void scanCollectionDir(const QString& dirName, QMenu* menu);

    private:
        QListView *m_quickView;
        QToolButton *m_moreShapes;
        QMenu* m_moreShapesContainer;
        QListWidget *m_collectionChooser;
        QListView *m_collectionView;
        QToolButton *m_closeCollectionButton;
        QToolButton *m_addCollectionButton;
        QSpacerItem *m_spacer;
        QGridLayout *m_layout;

        QMap<QString, CollectionItemModel*> m_modelMap;
};

#endif //KOSHAPECOLLECTIONDOCKER_H
