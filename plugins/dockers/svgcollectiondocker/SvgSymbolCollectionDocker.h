/* This file is part of the KDE project
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef SVGSYMBOLCOLLECTIONDOCKER_H
#define SVGSYMBOLCOLLECTIONDOCKER_H

#include <QDockWidget>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QMap>
#include <QIcon>

#include <KoDockFactoryBase.h>
#include <KoCanvasObserverBase.h>
#include <KisKineticScroller.h>

#include "ui_WdgSvgCollection.h"

class KoSvgSymbolCollectionResource;

class SvgCollectionModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SvgCollectionModel(QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    QStringList mimeTypes() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDragActions() const override;
public:
    void setSvgSymbolCollectionResource(KoSvgSymbolCollectionResource *resource);
private:
    KoSvgSymbolCollectionResource *m_symbolCollection;
};


class SvgSymbolCollectionDockerFactory : public KoDockFactoryBase
{
public:
    SvgSymbolCollectionDockerFactory();

    QString id() const override;
    QDockWidget *createDockWidget() override;
    DockPosition defaultDockPosition() const override
    {
        return DockRight;
    }
};

class SvgSymbolCollectionDocker : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:

    explicit SvgSymbolCollectionDocker(QWidget *parent = 0);

    /// reimplemented
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

public Q_SLOTS:
    void slotScrollerStateChanged(QScroller::State state){KisKineticScroller::updateCursor(this, state);}

private Q_SLOTS:

    void collectionActivated(int index);
    void slotSetIconSize();
private:

    Ui_WdgSvgCollection *m_wdgSvgCollection;
    QVector<SvgCollectionModel*> m_models;
    QSlider* m_iconSizeSlider;
};

#endif //KOSHAPECOLLECTIONDOCKER_H
