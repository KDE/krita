/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
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

#include "KoPADocumentStructureDocker.h"
#include "KoPADocumentModel.h"
#include "KoPADocument.h"
#include "KoPAPageBase.h"
#include "KoPACanvas.h"
#include "KoPAView.h"
#include "KoPAMasterPage.h"
#include "KoPAPage.h"
#include "commands/KoPAPageInsertCommand.h"
#if 0 // XXX: Add undo for reordering pages, look at Karbon
#include <KarbonLayerReorderCommand.h>
#endif

#include <KoShapeManager.h>
#include <KoShapeBorderModel.h>
#include <KoShapeContainer.h>
#include <KoToolManager.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoShapeControllerBase.h>
#include <KoSelection.h>
#include <KoShapeCreateCommand.h>
#include <KoShapeDeleteCommand.h>
#include <KoShapeReorderCommand.h>
#include <KoShapeLayer.h>

#include <KMenu>
#include <klocale.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <KConfigGroup>

#include <QtGui/QGridLayout>
#include <QtGui/QToolButton>
#include <QtGui/QButtonGroup>
#include <QItemSelection>

enum ButtonIds
{
    Button_Raise,
    Button_Lower,
    Button_Delete
};

KoPADocumentStructureDockerFactory::KoPADocumentStructureDockerFactory( KoPACanvas* canvas, KoDocumentSectionView::DisplayMode mode )
    : m_canvas( canvas ), m_mode( mode )
{
}

QString KoPADocumentStructureDockerFactory::id() const
{
    return QString("document section view");
}

QDockWidget* KoPADocumentStructureDockerFactory::createDockWidget()
{
    KoPADocumentStructureDocker* docker = new KoPADocumentStructureDocker(m_mode);
    docker->setCanvas( m_canvas );

    return docker;
}

KoPADocumentStructureDocker::KoPADocumentStructureDocker( KoDocumentSectionView::DisplayMode mode, QWidget* parent )
    : QDockWidget( parent )
    , KoCanvasObserver()
    , m_canvas( 0 )
    , m_model( 0 )
{
    setWindowTitle( i18n( "Document" ) );

    QWidget *mainWidget = new QWidget( this );
    QGridLayout* layout = new QGridLayout( mainWidget );
    layout->addWidget( m_sectionView = new KoDocumentSectionView( mainWidget ), 0, 0, 1, -1 );

    QToolButton *button = new QToolButton( mainWidget );
    button->setIcon( SmallIcon( "list-add" ) );
    button->setToolTip( i18n("Add a new page or layer") );
    layout->addWidget( button, 1, 0 );

    KMenu *menu = new KMenu( button );
    button->setMenu(menu);
    button->setPopupMode(QToolButton::InstantPopup);
    menu->addAction( SmallIcon( "document-new" ), i18n( "Page" ), this, SLOT( addPage() ) );
    m_addLayerAction = menu->addAction( SmallIcon( "layer-new" ), i18n( "Layer" ), this, SLOT( addLayer() ) );

    m_buttonGroup = new QButtonGroup( mainWidget );
    m_buttonGroup->setExclusive( false );

    button = new QToolButton( mainWidget );
    button->setIcon( SmallIcon( "list-remove" ) );
    button->setToolTip( i18n("Delete selected objects") );
    m_buttonGroup->addButton( button, Button_Delete );
    layout->addWidget( button, 1, 1 );

    button = new QToolButton( mainWidget );
    button->setIcon( SmallIcon( "arrow-up" ) );
    button->setToolTip( i18n("Raise selected objects") );
    m_buttonGroup->addButton( button, Button_Raise );
    layout->addWidget( button, 1, 3 );

    button = new QToolButton( mainWidget );
    button->setIcon( SmallIcon( "arrow-down" ) );
    button->setToolTip( i18n("Lower selected objects") );
    m_buttonGroup->addButton( button, Button_Lower );
    layout->addWidget( button, 1, 4 );

    button = new QToolButton( mainWidget );
    menu = new KMenu( this );
    QActionGroup *group = new QActionGroup( this );

    m_viewModeActions.insert( KoDocumentSectionView::MinimalMode,
                              menu->addAction( SmallIcon( "view-list-text" ), i18n( "Minimal View" ), this, SLOT( minimalView() ) ) );
    m_viewModeActions.insert( KoDocumentSectionView::DetailedMode,
                              menu->addAction( SmallIcon( "view-list-details" ), i18n( "Detailed View" ), this, SLOT( detailedView() ) ) );
    m_viewModeActions.insert( KoDocumentSectionView::ThumbnailMode,
                              menu->addAction( SmallIcon( "view-preview" ), i18n( "Thumbnail View" ), this, SLOT( thumbnailView() ) ) );

    foreach (QAction* action, m_viewModeActions) {
        action->setCheckable( true );
        action->setActionGroup( group );
    }

    button->setMenu(menu);
    button->setPopupMode(QToolButton::InstantPopup);
    button->setIcon(SmallIcon("view-choose"));
    button->setText(i18n("View mode"));
    layout->addWidget(button, 1, 5);

    layout->setSpacing( 0 );
    layout->setMargin( 3 );
    layout->setColumnStretch( 2, 10 );

    setWidget( mainWidget );

    connect( m_buttonGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( slotButtonClicked( int ) ) );

    m_model = new KoPADocumentModel( this );
    m_sectionView->setModel( m_model );
    m_sectionView->setSelectionBehavior( QAbstractItemView::SelectRows );
    m_sectionView->setDragDropMode( QAbstractItemView::InternalMove );

    connect( m_sectionView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(itemClicked(const QModelIndex&)));
    connect( m_sectionView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             this, SLOT (itemSelected( const QItemSelection&, const QItemSelection& ) ) );

    KConfigGroup configGroup = KGlobal::config()->group( "KoPageApp/DocumentStructureDocker" );
    QString viewModeString = configGroup.readEntry("ViewMode", "");

    if( viewModeString.isEmpty() )
        setViewMode( mode );
    else
        setViewMode( viewModeFromString( viewModeString ) );
}

KoPADocumentStructureDocker::~KoPADocumentStructureDocker()
{
    KConfigGroup configGroup = KGlobal::config()->group( "KoPageApp/DocumentStructureDocker" );
    configGroup.writeEntry( "ViewMode", viewModeToString( m_sectionView->displayMode() ) );
}

void KoPADocumentStructureDocker::updateView()
{
    m_model->update();
}

void KoPADocumentStructureDocker::slotButtonClicked( int buttonId )
{
    switch( buttonId )
    {
        case Button_Raise:
            raiseItem();
            break;
        case Button_Lower:
            lowerItem();
            break;
        case Button_Delete:
            deleteItem();
            break;
    }
}

void KoPADocumentStructureDocker::itemClicked( const QModelIndex &index )
{
    Q_ASSERT(index.internalPointer());

    if( ! index.isValid() )
        return;

    KoShape *shape = static_cast<KoShape*>( index.internalPointer() );
    if( ! shape )
        return;
    if (dynamic_cast<KoPAPageBase*>(shape)) {
        emit pageChanged(dynamic_cast<KoPAPageBase*>(shape));
        return;
    }
    emit pageChanged(m_canvas->document()->pageByShape(shape));
    if( dynamic_cast<KoShapeLayer*>( shape ) )
        return;

    QList<KoPAPageBase*> selectedPages;
    QList<KoShapeLayer*> selectedLayers;
    QList<KoShape*> selectedShapes;

    // separate selected layers and selected shapes
    extractSelectedLayersAndShapes( selectedPages, selectedLayers, selectedShapes );

    // XXX: Do stuff withthe selected pages!

    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();

    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
    foreach( KoShape* shape, selection->selectedShapes() )
        shape->update();

    selection->deselectAll();

    foreach( KoShape* shape, selectedShapes )
    {
        if( shape )
        {
            selection->select( shape );
            shape->update();
        }
    }
}

void KoPADocumentStructureDocker::addLayer()
{
    bool ok = true;
    QString name = KInputDialog::getText( i18n( "New Layer" ), i18n( "Enter the name of the new layer:" ),
                                          i18n( "New layer" ), &ok, this );
    if( ok )
    {
        KoShapeLayer* layer = new KoShapeLayer();
        layer->setName( name );
        QUndoCommand *cmd = new KoShapeCreateCommand( m_canvas->document(), layer, 0 );
        cmd->setText( i18n( "Create Layer") );
        m_canvas->addCommand( cmd );
        m_model->update();
    }
}

void KoPADocumentStructureDocker::deleteItem()
{
    QList<KoPAPageBase*> selectedPages; // XXX: Fix this!
    QList<KoShapeLayer*> selectedLayers;
    QList<KoShape*> selectedShapes;

    // separate selected layers and selected shapes
    extractSelectedLayersAndShapes( selectedPages, selectedLayers, selectedShapes );

    QUndoCommand *cmd = 0;

    if( selectedLayers.count() )
    {
        if( m_canvas->document()->pages().count() > selectedPages.count() )
        {
            QList<KoShape*> deleteShapes;
            foreach( KoPAPageBase* page, selectedPages )
            {
                deleteShapes += page->iterator();
                deleteShapes.append( page );
            }
            cmd = new KoShapeDeleteCommand( m_canvas->document(), deleteShapes );
            cmd->setText( i18n( "Delete Layer" ) );
        }
        else
        {
            KMessageBox::error( 0L, i18n( "Could not delete all layers. At least one layer is required."), i18n( "Error deleting layers") );
        }
    }
    else if( selectedShapes.count() )
    {
        cmd = new KoShapeDeleteCommand( m_canvas->document(), selectedShapes );
    }

    if( cmd )
    {
        m_canvas->addCommand( cmd );
        m_model->update();
    }
}

void KoPADocumentStructureDocker::raiseItem()
{
    QList<KoPAPageBase*> selectedPages;
    QList<KoShapeLayer*> selectedLayers;
    QList<KoShape*> selectedShapes;

    // separate selected layers and selected shapes
    extractSelectedLayersAndShapes( selectedPages, selectedLayers, selectedShapes );

    QUndoCommand *cmd = 0;

    if( selectedLayers.count() )
    {
//         // check if all layers could be raised
//         foreach( KoShapeLayer* layer, selectedLayers )
//             if( ! m_document->canRaiseLayer( layer ) )
//                 return;

//        cmd = new KoPALayerReorderCommand( m_document, selectedLayers, KoPALayerReorderCommand::RaiseLayer );
    }
    else if( selectedShapes.count() )
    {
        cmd = KoShapeReorderCommand::createCommand( selectedShapes, m_canvas->shapeManager(), KoShapeReorderCommand::RaiseShape );
    }

    if( cmd )
    {
        m_canvas->addCommand( cmd );
        m_model->update();
    }
}

void KoPADocumentStructureDocker::lowerItem()
{
    QList<KoPAPageBase*> selectedPages;
    QList<KoShapeLayer*> selectedLayers;
    QList<KoShape*> selectedShapes;

    // separate selected layers and selected shapes
    extractSelectedLayersAndShapes( selectedPages, selectedLayers, selectedShapes );

    QUndoCommand *cmd = 0;

    if( selectedLayers.count() )
    {
//         // check if all layers could be raised
//         foreach( KoShapeLayer* layer, selectedLayers )
//             if( ! m_document->canLowerLayer( layer ) )
//                 return;

//        cmd = new KoPALayerReorderCommand( m_document, selectedLayers, KoPALayerReorderCommand::LowerLayer );
    }
    else if( selectedShapes.count() )
    {
        cmd = KoShapeReorderCommand::createCommand( selectedShapes, m_canvas->shapeManager(), KoShapeReorderCommand::LowerShape );
    }

    if( cmd )
    {
        m_canvas->addCommand( cmd );
        m_model->update();
    }
}

void KoPADocumentStructureDocker::extractSelectedLayersAndShapes( QList<KoPAPageBase*> &pages, QList<KoShapeLayer*> &layers, QList<KoShape*> &shapes )
{
    pages.clear();
    layers.clear();
    shapes.clear();

    QModelIndexList selectedItems = m_sectionView->selectionModel()->selectedIndexes();
    if( selectedItems.count() == 0 )
        return;

    // separate selected layers and selected shapes
    foreach( QModelIndex index, selectedItems )
    {
        KoShape *shape = static_cast<KoShape*>( index.internalPointer() );
        KoShapeLayer *layer = dynamic_cast<KoShapeLayer*>( shape );
        if( layer )
            layers.append( layer );
        else if( ! selectedItems.contains( index.parent() ) )
            shapes.append( shape );
    }
}

void KoPADocumentStructureDocker::setCanvas( KoCanvasBase* canvas )
{
    m_canvas = static_cast<KoPACanvas*> ( canvas );
    m_model->setDocument( m_canvas->document() );
}

void KoPADocumentStructureDocker::setActivePage(KoPAPageBase *page)
{
    int row = m_canvas->document()->pageIndex(page);
    QModelIndex index = m_model->index(row, 0);
    m_sectionView->setCurrentIndex(index);
}

void KoPADocumentStructureDocker::setMasterMode(bool master)
{
    m_model->setMasterMode(master);
}

void KoPADocumentStructureDocker::minimalView()
{
    setViewMode(KoDocumentSectionView::MinimalMode);
}

void KoPADocumentStructureDocker::detailedView()
{
    setViewMode(KoDocumentSectionView::DetailedMode);
}

void KoPADocumentStructureDocker::thumbnailView()
{
    setViewMode(KoDocumentSectionView::ThumbnailMode);
}

void KoPADocumentStructureDocker::setViewMode(KoDocumentSectionView::DisplayMode mode)
{
    bool expandable = (mode != KoDocumentSectionView::ThumbnailMode);
    m_sectionView->setDisplayMode(mode);
    m_sectionView->setItemsExpandable(expandable);
    m_sectionView->setRootIsDecorated(expandable);
    m_sectionView->setSelectionMode(expandable ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);

    if(mode == KoDocumentSectionView::ThumbnailMode)
        m_sectionView->collapseAll();

    m_viewModeActions[mode]->setChecked (true);
}

KoDocumentSectionView::DisplayMode KoPADocumentStructureDocker::viewModeFromString( const QString& mode )
{
    if( mode == "Minimal" )
        return KoDocumentSectionView::MinimalMode;
    else if( mode == "Detailed" )
        return KoDocumentSectionView::DetailedMode;
    else if( mode == "Thumbnail" )
        return KoDocumentSectionView::ThumbnailMode;

    return KoDocumentSectionView::DetailedMode;
}

QString KoPADocumentStructureDocker::viewModeToString( KoDocumentSectionView::DisplayMode mode )
{
    switch (mode)
    {
        case KoDocumentSectionView::MinimalMode:
            return QString( "Minimal" );
            break;
        case KoDocumentSectionView::DetailedMode:
            return QString( "Detailed" );
            break;
        case KoDocumentSectionView::ThumbnailMode:
            return QString( "Thumbnail" );
            break;
    }

    return QString();
}

void KoPADocumentStructureDocker::itemSelected( const QItemSelection& selected, const QItemSelection& deselected )
{
    Q_UNUSED( deselected );

    if( selected.indexes().isEmpty() ) {
        m_buttonGroup->button( Button_Raise )->setEnabled( false );
        m_buttonGroup->button( Button_Lower )->setEnabled( false );
        m_buttonGroup->button( Button_Delete )->setEnabled( false );
        m_addLayerAction->setEnabled( false );
    } else {
        m_buttonGroup->button( Button_Delete )->setEnabled( true );
        m_buttonGroup->button( Button_Raise )->setEnabled( true );
        m_buttonGroup->button( Button_Lower )->setEnabled( true );
        m_addLayerAction->setEnabled( true );
    }
}

void KoPADocumentStructureDocker::addPage()
{
    if( !m_canvas )
        return;

    m_canvas->koPAView()->insertPage();
}

#include "KoPADocumentStructureDocker.moc"

// kate: replace-tabs on; space-indent on; indent-width 4; mixedindent off; indent-mode cstyle;
