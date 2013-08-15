/* This file is part of the KDE project
   Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
   Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
   Copyright (c) 2013 Sascha Suelzer <s_suelzer@lavabit.com>

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

#include <kfiledialog.h>
#include <klocale.h>

#ifdef GHNS
#include <attica/version.h>
#include <knewstuff3/downloaddialog.h>
#include <knewstuff3/uploaddialog.h>
#endif

#include <KoIcon.h>

#include "KoResourceServerAdapter.h"
#include "KoResourceItemView.h"
#include "KoResourceItemDelegate.h"
#include "KoResourceModel.h"
#include "KoResource.h"
#include "KoResourceTaggingInterface.h"

class KoResourceItemChooser::Private
{
public:
    Private()
        : model(0)
        , view(0)
        , buttonGroup(0)
        , tiledPreview(false)
        , grayscalePreview(false)
    {}
    KoResourceModel* model;
    KoResourceTaggingInterface* tagChooser;
    KoResourceItemView* view;
    QButtonGroup* buttonGroup;

    QString knsrcFile;
    QScrollArea *previewScroller;
    QLabel *previewLabel;
    QSplitter *splitter;
    bool tiledPreview;
    bool grayscalePreview;


};

KoResourceItemChooser::KoResourceItemChooser(KoAbstractResourceServerAdapter * resourceAdapter, QWidget *parent )
    : QWidget( parent ), d( new Private() )
{
    Q_ASSERT(resourceAdapter);

    d->splitter = new QSplitter(this);

    d->model = new KoResourceModel(resourceAdapter, this);
    d->view = new KoResourceItemView(this);
    d->view->setModel(d->model);
    d->view->setItemDelegate( new KoResourceItemDelegate( this ) );
    d->view->setSelectionMode( QAbstractItemView::SingleSelection );

    connect(d->view, SIGNAL(currentResourceChanged(QModelIndex)),
            this, SLOT(activated(QModelIndex)));
    connect (d->view, SIGNAL(contextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));

    d->previewScroller = new QScrollArea(this);
    d->previewScroller->setWidgetResizable(true);
    d->previewScroller->setBackgroundRole(QPalette::Dark);
    d->previewScroller->setVisible(false);
    d->previewScroller->setAlignment(Qt::AlignCenter);
    d->previewLabel = new QLabel(this);
    d->previewScroller->setWidget(d->previewLabel);

    d->splitter->addWidget(d->view);
    d->splitter->addWidget(d->previewScroller);
    connect(d->splitter, SIGNAL(splitterMoved(int,int)), SIGNAL(splitterMoved()));

    d->buttonGroup = new QButtonGroup(this);
    d->buttonGroup->setExclusive(false);

    QVBoxLayout* layout = new QVBoxLayout(this);

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

    d->tagChooser = new KoResourceTaggingInterface(d->model, this);
    
    layout->addWidget(d->tagChooser->tagChooserWidget());
    layout->addWidget(d->splitter);
    layout->addWidget(d->tagChooser->tagFilterWidget());
    layout->addLayout(buttonLayout);
    layout->setMargin(0);
    layout->setSpacing(0);
    updateButtonState();
    activated(d->model->index(0, 0));
}

KoResourceItemChooser::~KoResourceItemChooser()
{
    delete d;
}

void KoResourceItemChooser::slotButtonClicked( int button )
{
    if( button == Button_Import ) {
        QString extensions = d->model->extensions();
        QString filter = extensions.replace(QString(":"), QString(" "));
        QString filename = KFileDialog::getOpenFileName( KUrl(), filter, 0, i18nc("@title:window", "Choose File to Add"));

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
    d->tagChooser->showTaggingBar(showSearchBar, showOpBar);
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
    //Force an update to get the right column width
    QRect geometry = d->view->geometry();
    d->view->setGeometry(geometry.adjusted(0, 0, 0, 1));
    d->view->setGeometry(geometry);
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
    QModelIndex index = d->model->indexFromResource(resource);
    if( !index.isValid() )
        return;

    d->view->setCurrentIndex(index);
    updatePreview(resource);
}

void KoResourceItemChooser::showPreview(bool show)
{
    d->previewScroller->setVisible(show);
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
        emit resourceSelected( resource );
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
    if (!resource) return;

    QImage image = resource->image();

    /**
     * Most of our resources code expects the image() of the resource
     * to be in ARGB32 (or alike) format. If some resource returns the
     * image in another format, then it is actually a bug (and it may
     * result in a SIGSEGV somewhere). But here we will not assert
     * with it, just warn the user that something is wrong.
     */
    if (image.format() != QImage::Format_RGB32 &&
        image.format() != QImage::Format_ARGB32 &&
        image.format() != QImage::Format_ARGB32_Premultiplied) {

        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        qWarning() << "WARNING (KoResourceItemChooser::updatePreview): the resource" << resource->name() << "has created a non-rgb32 image thumbnail. It may not work properly.";
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

    if (d->grayscalePreview) {
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
    d->tagChooser->contextMenuRequested(currentResource(), pos);
}

#include <KoResourceItemChooser.moc>
