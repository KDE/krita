/* This file is part of the KDE project
   Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
   Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
   Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "KoResourceItemChooser.h"

#include <math.h>

#include <QGridLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QHeaderView>
#include <QAbstractProxyModel>
#include <QLabel>
#include <QScrollArea>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QSplitter>
#include <QToolButton>
#include <QWheelEvent>

#include <klocale.h>

#ifdef GHNS
#include <attica/version.h>
#include <knewstuff3/downloaddialog.h>
#include <knewstuff3/uploaddialog.h>
#endif

#include <KoIcon.h>
#include <KoFileDialog.h>

#include "KoResourceServerAdapter.h"
#include "KoResourceItemView.h"
#include "KoResourceItemDelegate.h"
#include "KoResourceModel.h"
#include "KoResource.h"
#include "KoResourceTaggingManager.h"
#include "KoResourceItemChooserSync.h"

class KoResourceItemChooser::Private
{
public:
    Private()
        : model(0)
        , view(0)
        , buttonGroup(0)
        , viewModeButton(0)
        , usePreview(false)
        , previewScroller(0)
        , previewLabel(0)
        , splitter(0)
        , tiledPreview(false)
        , grayscalePreview(false)
        , synced(false)
        , updatesBlocked(false)
        , savedResourceWhileReset(0)
    {}
    KoResourceModel* model;
    KoResourceTaggingManager* tagManager;
    KoResourceItemView* view;
    QButtonGroup* buttonGroup;
    QToolButton  *viewModeButton;

    QString knsrcFile;

    bool usePreview;
    QScrollArea *previewScroller;
    QLabel *previewLabel;
    QSplitter *splitter;
    bool tiledPreview;
    bool grayscalePreview;
    bool synced;
    bool updatesBlocked;

    KoResource *savedResourceWhileReset;
};

KoResourceItemChooser::KoResourceItemChooser(QSharedPointer<KoAbstractResourceServerAdapter> resourceAdapter, QWidget *parent, bool usePreview)
    : QWidget( parent ), d( new Private() )
{
    Q_ASSERT(resourceAdapter);

    d->splitter = new QSplitter(this);

    d->model = new KoResourceModel(resourceAdapter, this);
    connect(d->model, SIGNAL(beforeResourcesLayoutReset(KoResource*)), SLOT(slotBeforeResourcesLayoutReset(KoResource*)));
    connect(d->model, SIGNAL(afterResourcesLayoutReset()), SLOT(slotAfterResourcesLayoutReset()));

    d->view = new KoResourceItemView(this);
    d->view->setModel(d->model);
    d->view->setItemDelegate( new KoResourceItemDelegate( this ) );
    d->view->setSelectionMode( QAbstractItemView::SingleSelection );
    d->view->viewport()->installEventFilter(this);

    connect(d->view, SIGNAL(currentResourceChanged(QModelIndex)),
            this, SLOT(activated(QModelIndex)));
    connect (d->view, SIGNAL(contextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));

    connect (d->view, SIGNAL(sigSizeChanged()),
            this, SLOT(updateView()));

    d->splitter->addWidget(d->view);
    d->splitter->setStretchFactor(0, 2);

    d->usePreview = usePreview;
    if (d->usePreview) {
        d->previewScroller = new QScrollArea(this);
        d->previewScroller->setWidgetResizable(true);
        d->previewScroller->setBackgroundRole(QPalette::Dark);
        d->previewScroller->setVisible(true);
        d->previewScroller->setAlignment(Qt::AlignCenter);
        d->previewLabel = new QLabel(this);
        d->previewScroller->setWidget(d->previewLabel);
        d->splitter->addWidget(d->previewScroller);

        if (d->splitter->count() == 2) {
            d->splitter->setSizes(QList<int>() << 280 << 160);
        }
    }

    d->splitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    connect(d->splitter, SIGNAL(splitterMoved(int,int)), SIGNAL(splitterMoved()));

    d->buttonGroup = new QButtonGroup(this);
    d->buttonGroup->setExclusive(false);

    QGridLayout* layout = new QGridLayout(this);

    QGridLayout* buttonLayout = new QGridLayout;

    QPushButton *button = new QPushButton(this);
    button->setIcon(koIcon("document-open"));
    button->setToolTip(i18nc("@info:tooltip", "Import resource"));
    button->setEnabled(true);
    d->buttonGroup->addButton(button, Button_Import);
    buttonLayout->addWidget(button, 0, 0);

    button = new QPushButton(this);
    button->setIcon(koIcon("trash-empty"));
    button->setToolTip(i18nc("@info:tooltip", "Delete resource"));
    button->setEnabled(false);
    d->buttonGroup->addButton(button, Button_Remove);
    buttonLayout->addWidget(button, 0, 1);

    button = new QPushButton(this);
    button->setIcon(koIcon("download"));
    button->setToolTip(i18nc("@info:tooltip", "Download resource"));
    button->setEnabled(true);
    button->hide();
    d->buttonGroup->addButton(button, Button_GhnsDownload);
    buttonLayout->addWidget(button, 0, 3);

    button = new QPushButton(this);
    button->setIcon(koIcon("go-up"));
    button->setToolTip(i18nc("@info:tooltip", "Share Resource"));
    button->setEnabled(false);
    button->hide();
    d->buttonGroup->addButton( button, Button_GhnsUpload);
    buttonLayout->addWidget(button, 0, 4);

    connect( d->buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotButtonClicked(int)));

    buttonLayout->setColumnStretch(0, 1);
    buttonLayout->setColumnStretch(1, 1);
    buttonLayout->setColumnStretch(2, 2);
    buttonLayout->setSpacing(0);
    buttonLayout->setMargin(0);

    d->viewModeButton = new QToolButton(this);
    d->viewModeButton->setIcon(koIcon("view-choose"));
    d->viewModeButton->setPopupMode(QToolButton::InstantPopup);
    d->viewModeButton->setVisible(false);

    d->tagManager = new KoResourceTaggingManager(d->model, this);

    layout->addWidget(d->tagManager->tagChooserWidget(), 0, 0);
    layout->addWidget(d->viewModeButton, 0, 1);
    layout->addWidget(d->splitter, 1, 0, 1, 2);
    layout->addWidget(d->tagManager->tagFilterWidget(), 2, 0, 1, 2);
    layout->addLayout(buttonLayout, 3, 0, 1, 2);
    layout->setMargin(0);
    layout->setSpacing(0);
    updateButtonState();
    showTaggingBar(false,false);
    activated(d->model->index(0, 0));
}

KoResourceItemChooser::~KoResourceItemChooser()
{
    disconnect();
    delete d;
}

void KoResourceItemChooser::slotButtonClicked( int button )
{
    if (button == Button_Import ) {
        QString extensions = d->model->extensions();
        QString filter = QString("%1")
                .arg(extensions.replace(QString(":"), QString(" ")));


        KoFileDialog dialog(0, KoFileDialog::OpenFile, "OpenDocument");
        dialog.setNameFilter(filter);
        dialog.setCaption(i18nc("@title:window", "Choose File to Add"));
        QString filename = dialog.url();

        d->model->importResourceFile(filename);
    }
    else if( button == Button_Remove ) {
        QModelIndex index = d->view->currentIndex();
        int row = index.row();
        int column = index.column();
        if( index.isValid() ) {

            KoResource * resource = resourceFromModelIndex(index);
            if( resource ) {
                d->model->removeResource(resource);
            }
        }
        if (column == 0) {
            int rowMin = --row;
            row = qBound(0, rowMin, row);
        }
        int columnMin = --column;
        column = qBound(0, columnMin, column);
        setCurrentItem(row, column);
        activated(d->model->index(row, column));
    }
#ifdef GHNS
    else if (button == Button_GhnsDownload) {

        KNS3::DownloadDialog dialog(d->knsrcFile, this);
        dialog.exec();

        foreach (const KNS3::Entry& e, dialog.changedEntries()) {

            foreach(const QString &file, e.installedFiles()) {
                QFileInfo fi(file);
                d->model->importResourceFile( fi.absolutePath()+'/'+fi.fileName() , false );
            }

            foreach(const QString &file, e.uninstalledFiles()) {
                QFileInfo fi(file);
                d->model->removeResourceFile(fi.absolutePath()+'/'+fi.fileName());
            }
        }
    }
    else if (button == Button_GhnsUpload) {

        QModelIndex index = d->view->currentIndex();
        if( index.isValid() ) {


            KoResource * resource = resourceFromModelIndex(index);
            if( resource ) {
                KNS3::UploadDialog dialog(d->knsrcFile, this);
                dialog.setUploadFile(KUrl::fromLocalFile(resource->filename()));
                dialog.setUploadName(resource->name());
                dialog.exec();
            }
        }
    }
#endif
    updateButtonState();
}

void KoResourceItemChooser::showButtons( bool show )
{
    foreach( QAbstractButton * button, d->buttonGroup->buttons() )
        show ? button->show() : button->hide();
}

void KoResourceItemChooser::showGetHotNewStuff( bool showDownload, bool showUpload )
{
#ifdef GHNS

    QAbstractButton *button = d->buttonGroup->button(Button_GhnsDownload);
    showDownload ? button->show() : button->hide();

    // attica < 2.9 is broken for upload, so don't show the upload button. 2.9 is released as 3.0
    // because of binary incompatibility with 2.x.
    if (LIBATTICA_VERSION_MAJOR < 3) return;

    button = d->buttonGroup->button(Button_GhnsUpload);
    showUpload ? button->show() : button->hide();
#else
    Q_UNUSED(showDownload);
    Q_UNUSED(showUpload);
#endif
}

void KoResourceItemChooser::showTaggingBar(bool showSearchBar, bool showOpBar)
{
    d->tagManager->showTaggingBar(showSearchBar, showOpBar);
}

void KoResourceItemChooser::setRowCount( int rowCount )
{
    int resourceCount = d->model->resourcesCount();
    d->model->setColumnCount( static_cast<qreal>(resourceCount) / rowCount );
    //Force an update to get the right row height (in theory)
    QRect geometry = d->view->geometry();
    d->view->setViewMode(KoResourceItemView::FIXED_ROWS);
    d->view->setGeometry(geometry.adjusted(0, 0, 0, 1));
    d->view->setGeometry(geometry);
}

void KoResourceItemChooser::setColumnCount( int columnCount )
{
    d->model->setColumnCount( columnCount );
}

void KoResourceItemChooser::setRowHeight( int rowHeight )
{
    d->view->verticalHeader()->setDefaultSectionSize( rowHeight );
}

void KoResourceItemChooser::setColumnWidth( int columnWidth )
{
    d->view->horizontalHeader()->setDefaultSectionSize( columnWidth );
}

void KoResourceItemChooser::setItemDelegate( QAbstractItemDelegate * delegate )
{
    d->view->setItemDelegate(delegate);
}

KoResource *  KoResourceItemChooser::currentResource() const
{
    QModelIndex index = d->view->currentIndex();
    if (index.isValid()) {
        return resourceFromModelIndex(index);
    }
    return 0;
}

void KoResourceItemChooser::setCurrentResource(KoResource* resource)
{
    // don't update if the change came from the same chooser
    if (d->updatesBlocked) {
        return;
    }

    QModelIndex index = d->model->indexFromResource(resource);
    if( !index.isValid() )
        return;

    d->view->setCurrentIndex(index);
    updatePreview(resource);
}

void KoResourceItemChooser::slotBeforeResourcesLayoutReset(KoResource *activateAfterReset)
{
    d->savedResourceWhileReset = activateAfterReset ? activateAfterReset : currentResource();
}

void KoResourceItemChooser::slotAfterResourcesLayoutReset()
{
    if (d->savedResourceWhileReset) {
        this->blockSignals(true);
        setCurrentResource(d->savedResourceWhileReset);
        this->blockSignals(false);
    }
}

void KoResourceItemChooser::setPreviewOrientation(Qt::Orientation orientation)
{
    d->splitter->setOrientation(orientation);
}

void KoResourceItemChooser::setPreviewTiled(bool tiled)
{
    d->tiledPreview = tiled;
}

void KoResourceItemChooser::setGrayscalePreview(bool grayscale)
{
    d->grayscalePreview = grayscale;
}

void KoResourceItemChooser::setCurrentItem(int row, int column)
{
    QModelIndex index = d->model->index(row, column);
    if( !index.isValid() )
        return;

    d->view->setCurrentIndex(index);
    if (index.isValid()) {
        updatePreview(resourceFromModelIndex(index));
    }
}

void KoResourceItemChooser::setProxyModel( QAbstractProxyModel* proxyModel )
{
    proxyModel->setSourceModel(d->model);
    d->view->setModel(proxyModel);
}

void KoResourceItemChooser::activated(const QModelIndex &/*index*/)
{
    KoResource* resource = currentResource();
    if (resource) {
        d->updatesBlocked = true;
        emit resourceSelected( resource );
        d->updatesBlocked = false;

        updatePreview(resource);
        updateButtonState();
    }
}

void KoResourceItemChooser::updateButtonState()
{
    QAbstractButton * removeButton = d->buttonGroup->button( Button_Remove );
    if( ! removeButton )
        return;

    QAbstractButton * uploadButton = d->buttonGroup->button(Button_GhnsUpload);
    if(!uploadButton)
        return;

    KoResource * resource = currentResource();
    if( resource ) {
        removeButton->setEnabled( true );
        uploadButton->setEnabled(resource->removable());
        return;
    }

    removeButton->setEnabled( false );
    uploadButton->setEnabled(false);
}

void KoResourceItemChooser::updatePreview(KoResource *resource)
{
    if (!d->usePreview || !resource) return;

    QImage image = resource->image();

    if (image.format()!= QImage::Format_RGB32 ||
            image.format() != QImage::Format_ARGB32 ||
            image.format() != QImage::Format_ARGB32_Premultiplied) {

        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    if (d->tiledPreview) {
        int width = d->previewScroller->width() * 4;
        int height = d->previewScroller->height() * 4;
        QImage img(width, height, image.format());
        QPainter gc(&img);
        gc.fillRect(img.rect(), Qt::white);
        gc.setPen(Qt::NoPen);
        gc.setBrush(QBrush(image));
        gc.drawRect(img.rect());
        image = img;
    }

    // Only convert to grayscale if it is rgb. Otherwise, it's gray already.
    if (d->grayscalePreview && !image.isGrayscale()) {

        QRgb* pixel = reinterpret_cast<QRgb*>( image.bits() );
        for (int row = 0; row < image.height(); ++row ) {
            for (int col = 0; col < image.width(); ++col ){
                const QRgb currentPixel = pixel[row * image.width() + col];
                const int red = qRed(currentPixel);
                const int green = qGreen(currentPixel);
                const int blue = qBlue(currentPixel);
                const int grayValue = (red * 11 + green * 16 + blue * 5) / 32;
                pixel[row * image.width() + col] = qRgb(grayValue, grayValue, grayValue);
            }
        }
    }
    d->previewLabel->setPixmap(QPixmap::fromImage(image));

}

KoResource* KoResourceItemChooser::resourceFromModelIndex(const QModelIndex& index) const
{
    if(!index.isValid())
        return 0;

    const QAbstractProxyModel* proxyModel = dynamic_cast<const QAbstractProxyModel*>(index.model());
    if(proxyModel) {
        //Get original model index, because proxy models destroy the internalPointer
        QModelIndex originalIndex = proxyModel->mapToSource(index);
        return static_cast<KoResource*>( originalIndex.internalPointer() );
    }

    return static_cast<KoResource*>( index.internalPointer() );
}

void KoResourceItemChooser::setKnsrcFile(const QString& knsrcFileArg)
{
    d->knsrcFile = knsrcFileArg;
}

QSize KoResourceItemChooser::viewSize() const
{
    return d->view->size();
}

KoResourceItemView *KoResourceItemChooser::itemView() const
{
    return d->view;
}

void KoResourceItemChooser::contextMenuRequested(const QPoint& pos)
{
    d->tagManager->contextMenuRequested(currentResource(), pos);
}

void KoResourceItemChooser::setViewModeButtonVisible(bool visible)
{
    d->viewModeButton->setVisible(visible);
}

QToolButton* KoResourceItemChooser::viewModeButton() const
{
    return d->viewModeButton;
}

void KoResourceItemChooser::setSynced(bool sync)
{
    d->synced = sync;
    KoResourceItemChooserSync* chooserSync = KoResourceItemChooserSync::instance();
    if (sync) {
        connect(chooserSync, SIGNAL(baseLenghtChanged(int)), SLOT(baseLengthChanged(int)));
        baseLengthChanged(chooserSync->baseLength());
    } else {
        chooserSync->disconnect(this);
    }
}

void KoResourceItemChooser::baseLengthChanged(int length)
{
    if (d->synced) {
        int resourceCount = d->model->resourcesCount();
        int width = d->view->width();
        int maxColums = width/length;
        int cols = width/(2*length) + 1;
        while(cols <= maxColums) {
            int size = width/cols;
            int rows = ceil(resourceCount/(double)cols);
            if(rows*size < (d->view->height()-5)) {
                break;
            }
            cols++;
        }
        setColumnCount(cols);
    }
    d->view->updateView();
}

bool KoResourceItemChooser::eventFilter(QObject* object, QEvent* event)
{
    if (d->synced && event->type() == QEvent::Wheel) {
        KoResourceItemChooserSync* chooserSync = KoResourceItemChooserSync::instance();
        QWheelEvent* qwheel = dynamic_cast<QWheelEvent* >(event);
        if (qwheel->modifiers() & Qt::ControlModifier) {

            int degrees = qwheel->delta() / 8;
            int newBaseLength = chooserSync->baseLength() + degrees/15 * 10;
            chooserSync->setBaseLength(newBaseLength);
            return true;
        }
    }
    return QObject::eventFilter(object, event);
}

void KoResourceItemChooser::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateView();
}

void KoResourceItemChooser::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    updateView();
}

void KoResourceItemChooser::updateView()
{
    if (d->synced) {
        KoResourceItemChooserSync* chooserSync = KoResourceItemChooserSync::instance();
        baseLengthChanged(chooserSync->baseLength());
    }
}


#include <KoResourceItemChooser.moc>
