/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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
#include <QHash>
#include <KoDockFactoryBase.h>
#include <KoCanvasObserverBase.h>
#include <KoDocumentSectionView.h>
#include <KoPageApp.h>

class KoShape;
class KoShapeLayer;
class KoPADocument;
class KoPADocumentModel;
class KoPAPageBase;
class QModelIndex;
class QAction;
class QButtonGroup;

namespace KParts
{
    class Part;
}

class KoPADocumentStructureDockerFactory : public KoDockFactoryBase
{
public:
    explicit KoPADocumentStructureDockerFactory( KoDocumentSectionView::DisplayMode mode, KoPageApp::PageType pageType = KoPageApp::Page);

    virtual QString id() const;
    virtual QDockWidget* createDockWidget();

    DockPosition defaultDockPosition() const
    {
        return DockRight;
    }

private:
    KoDocumentSectionView::DisplayMode m_mode;
    KoPageApp::PageType m_pageType;
};

class KoPADocumentStructureDocker : public QDockWidget, public KoCanvasObserverBase
{
Q_OBJECT

public:
    explicit KoPADocumentStructureDocker( KoDocumentSectionView::DisplayMode mode, KoPageApp::PageType pageType, QWidget* parent = 0 );
    virtual ~KoPADocumentStructureDocker();

    virtual void setCanvas( KoCanvasBase* canvas);
    void setActivePage(KoPAPageBase *page);
    void setMasterMode(bool master);

protected:
    /// This is the context menu for the slide show in the KoPADocumentStructure docker
    void contextMenuEvent(QContextMenuEvent* event);

signals:
    void pageChanged(KoPAPageBase *page);

    /// This signal will be emitted after the model for this docker has been reset
    void dockerReset();

public slots:
    void updateView();
    /// Set the KPart::Part or the document for this docker, this will reset the document model for this docker
    /// and eventually dockerReset() signal will be emitted
    void setPart( KParts::Part * part );

private slots:
    void slotButtonClicked( int buttonId );
    void addLayer();
    void addPage();
    void deleteItem();
    void raiseItem();
    void lowerItem();
    void itemClicked( const QModelIndex &index );
    void minimalView();
    void detailedView();
    void thumbnailView();

    void itemSelected( const QItemSelection& selected, const QItemSelection& deselected );
    void editCut();
    void editCopy();
    void editPaste(); 

private:
    void extractSelectedLayersAndShapes( QList<KoPAPageBase*> &pages, QList<KoShapeLayer*> &layers, QList<KoShape*> &shapes );
    void setViewMode(KoDocumentSectionView::DisplayMode mode);
    QModelIndex getRootIndex( const QModelIndex &index ) const;

    KoDocumentSectionView::DisplayMode viewModeFromString( const QString& mode );
    QString viewModeToString( KoDocumentSectionView::DisplayMode mode );

    KoPADocument * m_doc;
    KoDocumentSectionView *m_sectionView;
    KoPADocumentModel *m_model;
    QHash<KoDocumentSectionView::DisplayMode, QAction*> m_viewModeActions;
    QList<KoShape *> m_selectedShapes;
    QButtonGroup *m_buttonGroup;
    QAction* m_addLayerAction;
};

#endif // KOPADOCUMENTSTRUCTUREDOCKER_H
