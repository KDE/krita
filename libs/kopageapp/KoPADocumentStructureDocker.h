/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
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
#ifndef KOPADOCUMENTSTRUCTUREDOCKER_H
#define KOPADOCUMENTSTRUCTUREDOCKER_H

#include <QDockWidget>
#include <KoDockFactory.h>
#include <KoCanvasObserver.h>

class KoDocumentSectionView;
class KoShape;
class KoShapeLayer;
class KoPADocumentModel;
class QModelIndex;
class KoPAPageBase;
class KoPACanvas;

class KoPADocumentStructureDockerFactory : public KoDockFactory
{
public:
    KoPADocumentStructureDockerFactory( KoPACanvas* canvas );

    virtual QString id() const;
    virtual QDockWidget* createDockWidget();

private:
    KoPACanvas* m_canvas;
};

class KoPADocumentStructureDocker : public QDockWidget, public KoCanvasObserver
{
Q_OBJECT

public:
    explicit KoPADocumentStructureDocker( QWidget* parent = 0 );
    virtual ~KoPADocumentStructureDocker();
    
    virtual void setCanvas( KoCanvasBase* canvas);

public slots:
    void updateView();
private slots:
    void slotButtonClicked( int buttonId );
    void addLayer();
    void deleteItem();
    void raiseItem();
    void lowerItem();
    void itemClicked( const QModelIndex &index );
private:
    void extractSelectedLayersAndShapes( QList<KoPAPageBase*> &pages, QList<KoShapeLayer*> &layers, QList<KoShape*> &shapes );
    KoPACanvas* m_canvas;
    KoDocumentSectionView *m_sectionView;
    KoPADocumentModel *m_model;
};

#endif // KOPADOCUMENTSTRUCTUREDOCKER_H
