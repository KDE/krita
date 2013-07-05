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
#include <QToolButton>
#include <QSplitter>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>

#include <kfiledialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kcombobox.h>


#ifdef GHNS
#include <attica/version.h>
#include <knewstuff3/downloaddialog.h>
#include <knewstuff3/uploaddialog.h>
#endif

#include <KoIcon.h>

#include "KoResourceItemChooserContextMenu.h"
#include "KoResourceServerAdapter.h"
#include "KoResourceItemView.h"
#include "KoResourceItemDelegate.h"
#include "KoResourceModel.h"
#include "KoResource.h"

class TaggedResourceSet
{
public:
    TaggedResourceSet()
    {}

    TaggedResourceSet(const QString& tagName, const QList<KoResource*>& resources)
    : tagName(tagName), resources(resources)
    {}
    QString tagName;
    QList<KoResource*> resources;
};

class KoResourceItemChooser::Private
{
public:
    Private()
        : model(0)
        , view(0)
        , buttonGroup(0)
        , tiledPreview(false)
        , grayscalePreview(false)
        , showContextMenu(true)
    {}
    KoResourceModel* model;
    KoResourceItemView* view;
    QButtonGroup* buttonGroup;
    QButtonGroup* tagButtonGroup;
    KLineEdit *tagSearchLineEdit;
    QPushButton *tagSearchSaveButton;
    QToolButton *tagToolButton;
    KComboBox *tagOpComboBox;
    QString knsrcFile;
    QCompleter *tagCompleter;
    QScrollArea *previewScroller;
    QLabel *previewLabel;
    QSplitter *splitter;
    bool tiledPreview;
    bool grayscalePreview;
    bool showContextMenu;
    QString currentTag;
    QString tagSearchBarTooltip_disabled;
    QString tagSearchBarTooltip_enabled;
    QString unfilteredView;
    QList<KoResource*> originalResources;
    TaggedResourceSet lastDeletedTag;
    QAction* action_undeleteTag;
    QAction* action_deleteTag;
    KoLineEditAction* action_renameTag;
    QAction* action_purgeTagUndeleteList;

};

KoResourceItemChooser::KoResourceItemChooser(KoAbstractResourceServerAdapter * resourceAdapter, QWidget *parent )
    : QWidget( parent ), d( new Private() )
{
    Q_ASSERT(resourceAdapter);

    d->tagSearchBarTooltip_disabled = i18n (
            "<qt>Entering search terms here will add to, or remove resources from the current tag view."
            "<para>To filter based on the partial, case insensitive name of a resource:<br>"
            "<icode>partialname</icode> or <icode>!partialname</icode>.</para>"
            "<para>In-/exclusion of other tag sets:<br>"
            "<icode>[Tagname]</icode> or <icode>![Tagname]</icode>.</para>"
            "<para>Case sensitive and full name matching in-/exclusion:<br>"
            "<icode>\"ExactMatch\"</icode> or <icode>!\"ExactMatch\"</icode>.</para>"
            "Filter results cannot be saved for the <interface>All Presets</interface> view.<br>"
            "In this view, pressing <interface>Enter</interface> or clearing the filter box will restore all items.<br>"
            "Create and/or switch to a different tag if you want to save filtered resources into named sets.</qt>"
            );

    d->tagSearchBarTooltip_enabled = i18n (
            "<qt>Entering search terms here will add to, or remove resources from the current tag view."
            "<para>To filter based on the partial, case insensitive name of a resource:<br>"
            "<icode>partialname</icode> or <icode>!partialname</icode>.</para>"
            "<para>In-/exclusion of other tag sets:<br>"
            "<icode>[Tagname]</icode> or <icode>![Tagname]</icode>.</para>"
            "<para>Case sensitive and full name matching in-/exclusion:<br>"
            "<icode>\"ExactMatch\"</icode> or <icode>!\"ExactMatch\"</icode>.</para>"
            "Pressing <interface>Enter</interface> or clicking the <interface>Save</interface> button will save the changes.</qt>"
            );

    d->unfilteredView = i18n("All Presets");

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

    d->tagOpComboBox = new KComboBox(this);
    d->tagOpComboBox->setInsertPolicy(KComboBox::InsertAlphabetically);
    d->tagOpComboBox->setEnabled(true);
    d->tagOpComboBox->hide();

    d->tagOpComboBox->clear();
    QStringList tagNames = d->model->tagNamesList();
    tagNames.sort();
    tagNames.prepend(d->unfilteredView);
    d->tagOpComboBox->addItems(tagNames);


    connect(d->tagOpComboBox, SIGNAL(returnPressed(QString)), this, SLOT(tagChooserReturnPressed(QString)));
    connect(d->tagOpComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(tagChooserIndexChanged(QString)));

    QVBoxLayout* layout = new QVBoxLayout(this);
    QGridLayout* comboLayout = new QGridLayout;
    QGridLayout* filterBarLayout = new QGridLayout;

    layout->setMargin(0);
    comboLayout->addWidget(d->tagOpComboBox, 0, 0);

    d->tagToolButton = new QToolButton(this);
    d->tagToolButton->setIcon(koIcon("list-add"));
    d->tagToolButton->setToolTip(i18n("<qt>Show the tag box options.</qt>"));
    d->tagToolButton->setPopupMode(QToolButton::InstantPopup);
    d->tagToolButton->setEnabled(true);
    d->tagToolButton->hide();

    QMenu *popup = new QMenu(this);

    KoLineEditAction * addTagAction = new KoLineEditAction(popup);
    addTagAction->setClickMessage(i18n("New tag"));
    addTagAction->setIcon(koIcon("document-new"));
    addTagAction->closeParentOnTrigger(true);
    popup->addAction(addTagAction);

    connect(addTagAction, SIGNAL(triggered(QString)),
            this, SLOT(contextCreateNewTag(QString)));

    d->action_renameTag = new KoLineEditAction(popup);
    d->action_renameTag->setClickMessage(i18n("Rename tag"));
    d->action_renameTag->setIcon(koIcon("edit-rename"));
    d->action_renameTag->closeParentOnTrigger(true);
    popup->addAction(d->action_renameTag);

    connect(d->action_renameTag, SIGNAL(triggered(QString)), this, SLOT(renameCurrentlySelectedTag(QString)));

    popup->addSeparator();

    d->action_deleteTag = new QAction(popup);
    d->action_deleteTag->setText(i18n("Delete this tag"));
    d->action_deleteTag->setIcon(koIcon("edit-delete"));
    popup->addAction(d->action_deleteTag);

    connect(d->action_deleteTag, SIGNAL(triggered()),
            this, SLOT(tagBoxEntryDeletionRequested()));

    popup->addSeparator();

    d->action_undeleteTag = new QAction(popup);
    d->action_undeleteTag->setIcon(koIcon("edit-redo"));
    d->action_undeleteTag->setVisible(false);
    popup->addAction(d->action_undeleteTag);

    connect(d->action_undeleteTag, SIGNAL(triggered()), this, SLOT(undeleteLastDeletedTag()));

    d->action_purgeTagUndeleteList = new QAction(popup);
    d->action_purgeTagUndeleteList->setText(i18n("Clear undelete list"));
    d->action_purgeTagUndeleteList->setIcon(koIcon("edit-clear"));
    d->action_purgeTagUndeleteList->setVisible(false);
    popup->addAction(d->action_purgeTagUndeleteList);

    connect(d->action_purgeTagUndeleteList, SIGNAL(triggered()), this, SLOT(purgeTagUndeleteList()));

    connect(popup, SIGNAL(aboutToShow()), this, SLOT(tagOptionsContextMenuAboutToShow()));

    d->tagToolButton->setMenu(popup);

    comboLayout->addWidget(d->tagToolButton, 0, 1);

    comboLayout->setSpacing(0);
    comboLayout->setMargin(0);
    comboLayout->setColumnStretch(0, 3);

    layout->addLayout(comboLayout);
    layout->addWidget(d->splitter);

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
    button->setToolTip(i18n("Share Resource"));
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

    d->tagSearchLineEdit = new KLineEdit(this);
    d->tagSearchLineEdit->setClearButtonShown(true);
    d->tagSearchLineEdit->setClickMessage(i18n("Enter resource filters here"));
    d->tagSearchLineEdit->setToolTip(d->tagSearchBarTooltip_disabled);
    d->tagSearchLineEdit->setEnabled(true);
    d->tagSearchLineEdit->hide();

    filterBarLayout->setSpacing(0);
    filterBarLayout->setMargin(0);
    filterBarLayout->setColumnStretch(0, 1);
    filterBarLayout->addWidget(d->tagSearchLineEdit, 0, 0);

    d->tagSearchSaveButton = new QPushButton(this);
    d->tagSearchSaveButton->setIcon(koIcon("media-floppy"));
    d->tagSearchSaveButton->setToolTip(i18n("<qt>Save the currently filtered set as the new members of the current tag.</qt>"));
    d->tagSearchSaveButton->setEnabled(false);
    d->tagSearchSaveButton->hide();

    filterBarLayout->addWidget(d->tagSearchSaveButton, 0, 1);

    connect( d->tagSearchSaveButton, SIGNAL(pressed()), this, SLOT(tagSaveButtonPressed()));

    connect( d->tagSearchLineEdit, SIGNAL(returnPressed(QString)), this, SLOT(tagSearchLineEditActivated(QString)));
    connect( d->tagSearchLineEdit, SIGNAL(textChanged(QString)), this, SLOT(tagSearchLineEditTextChanged(QString)));

    layout->addLayout(filterBarLayout);
    layout->addLayout(buttonLayout);

    connect( d->model, SIGNAL(tagBoxEntryAdded(QString)), this, SLOT(syncTagBoxEntryAddition(QString)));
    connect( d->model, SIGNAL(tagBoxEntryRemoved(QString)), this, SLOT(syncTagBoxEntryRemoval(QString)));

    d->tagCompleter = new QCompleter(tagNamesList(QString()),this);
    d->tagSearchLineEdit->setCompleter(d->tagCompleter);

    enableContextMenu(false);
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
    showSearchBar ? d->tagSearchLineEdit->show() : d->tagSearchLineEdit->hide();
    showOpBar ? d->tagOpComboBox->show() : d->tagOpComboBox->hide();
    enableContextMenu(showOpBar);

    showOpBar ? d->tagToolButton->show() : d->tagToolButton->hide();
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
        d->tagOpComboBox->setEnabled(true);
        return;
    }

    removeButton->setEnabled( false );
    uploadButton->setEnabled(false);
    d->tagOpComboBox->setEnabled( true );
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

QStringList KoResourceItemChooser::tagNamesList(const QString &lineEditText) const
{
    QStringList tagNamesList = d->model->tagNamesList();

    if(lineEditText.contains(", ")) {
        QStringList tagsList = lineEditText.split(", ");
        if(tagsList.contains("")) {
            tagsList.removeAll("");
        }

        QStringList autoCompletionTagsList;
        QString joinText;

        if(lineEditText.endsWith(", ")) {
            joinText=lineEditText;
        }
        else {
            tagsList.removeLast();
            joinText = tagsList.join(", ");
            joinText.append(", ");
        }

        for(int i=0; i< tagNamesList.count(); i++) {
            if (!tagsList.contains(tagNamesList.at(i))) {
                autoCompletionTagsList  << joinText + tagNamesList.at(i);
            }
        }
        return autoCompletionTagsList;
    }
    return tagNamesList;
}

KoResourceItemView *KoResourceItemChooser::itemView() const
{
    return d->view;
}

void KoResourceItemChooser::tagSearchLineEditActivated(const QString& /*lineEditText*/)
{
    if (!d->currentTag.isEmpty()) {
    QList<KoResource*> newResources = d->model->currentlyVisibleResources();
    foreach(KoResource * oldRes, d->originalResources) {
        if (!newResources.contains(oldRes))
            removeResourceTag(oldRes, d->currentTag);
    }
    foreach(KoResource * newRes, newResources) {
        if (!d->originalResources.contains(newRes))
            addResourceTag(newRes, d->currentTag);
    }
    d->model->tagCategoryMembersChanged();
    }
    updateTaggedResourceView();
    d->tagSearchLineEdit->clear();
    d->tagSearchSaveButton->setEnabled(false);
}

void KoResourceItemChooser::tagSearchLineEditTextChanged(const QString& lineEditText)
{
    d->model->searchTextChanged(lineEditText);
    d->model->updateServer();

    d->tagSearchSaveButton->setEnabled(!lineEditText.isEmpty());
    d->tagCompleter = new QCompleter(tagNamesList(lineEditText),this);
    d->tagSearchLineEdit->setCompleter(d->tagCompleter);
    if (d->currentTag.isEmpty()) {
        d->model->enableResourceFiltering(!lineEditText.isEmpty());
    }
    else {
        d->model->enableResourceFiltering(true);
    }
}

void KoResourceItemChooser::tagChooserIndexChanged(const QString& lineEditText)
{
    int index = d->tagOpComboBox->currentIndex();

    if ( index > 0) {
        d->currentTag = lineEditText;
        d->tagSearchSaveButton->show();
        d->model->enableResourceFiltering(true);
        d->tagSearchLineEdit->setToolTip(d->tagSearchBarTooltip_enabled);
    }
    else {
        d->model->enableResourceFiltering(false);
        d->tagSearchSaveButton->hide();
        d->tagSearchLineEdit->setToolTip(d->tagSearchBarTooltip_disabled);
        d->currentTag.clear();
    }

    d->tagSearchLineEdit->clear();
    updateTaggedResourceView();
}

void KoResourceItemChooser::updateTaggedResourceView()
{
    d->model->setCurrentTag(d->currentTag);
    d->model->updateServer();
    d->originalResources = d->model->currentlyVisibleResources();
}

void KoResourceItemChooser::renameCurrentlySelectedTag(const QString& newName)
{
    if (newName.isEmpty()) return;

    int index = d->tagOpComboBox->currentIndex();
    if (!d->model->tagNamesList().contains(newName) && index > 0) {
        QString oldName = d->currentTag;
        QList<KoResource*> resources = d->model->currentlyVisibleResources();
            foreach(KoResource * resource, resources) {
                removeResourceTag(resource, oldName);
                addResourceTag(resource,newName);
            }
            contextCreateNewTag(newName);
            d->model->tagCategoryRemoved(oldName);
            d->model->tagCategoryAdded(newName);
    }
}

void KoResourceItemChooser::tagChooserReturnPressed(const QString& lineEditText)
{
    int index = d->tagOpComboBox->currentIndex();
    QString oldTagname = d->tagOpComboBox->itemText(index);
    renameCurrentlySelectedTag(lineEditText);
    d->tagCompleter = new QCompleter(tagNamesList(lineEditText),this);
    d->tagOpComboBox->setCompleter(d->tagCompleter);

}

void KoResourceItemChooser::removeTagFromComboBox()
{
    int index = d->tagOpComboBox->currentIndex();
    if (index > 0) {
        QString tag = d->currentTag;

            QList<KoResource*> resources = d->model->currentlyVisibleResources();
            foreach(KoResource * resource, resources) {
                removeResourceTag(resource, tag);
            }
            d->model->tagCategoryRemoved(tag);
            d->lastDeletedTag = TaggedResourceSet(tag,resources);
    }
}

void KoResourceItemChooser::addResourceTag(KoResource * resource, const QString& tagName)
{
    QStringList tagsList = d->model->assignedTagsList(resource);
    if (tagsList.isEmpty()) {
        d->model->addTag(resource, tagName);
    }
    else {
        foreach(const QString &tag, tagsList) {
            if(tag.compare(tagName)) {
                d->model->addTag(resource, tagName);
            }
        }
    }
}

void KoResourceItemChooser::removeResourceTag(KoResource * resource, const QString& tagName)
{
    QStringList tagsList = d->model->assignedTagsList(resource);

    foreach(const QString &oldName, tagsList) {
        if(!oldName.compare(tagName)) {
            d->model->deleteTag(resource, oldName);
        }
    }
}

void KoResourceItemChooser::contextMenuRequested (const QPoint& pos)
{
    KoResource * resource = currentResource();
    if (!resource || !d->showContextMenu)
        return;

    KoResourceItemChooserContextMenu menu(
        resource,
        d->model->assignedTagsList(resource),
        d->currentTag,
        availableTags());

    connect(&menu, SIGNAL(resourceTagAdditionRequested(KoResource*,QString)),
            this, SLOT(contextAddTagToResource(KoResource*,QString)));

    connect(&menu, SIGNAL(resourceTagRemovalRequested(KoResource*,QString)),
            this, SLOT(contextRemoveTagFromResource(KoResource*,QString)));

    connect(&menu, SIGNAL(resourceAssignmentToNewTagRequested(KoResource*,QString)),
            this, SLOT(contextCreateNewTag(KoResource*,QString)));

    menu.exec(pos);
}

void KoResourceItemChooser::contextAddTagToResource(KoResource* resource, const QString& tag)
{
    addResourceTag(resource, tag);
    d->model->tagCategoryMembersChanged();
    updateTaggedResourceView();
}

void KoResourceItemChooser::contextRemoveTagFromResource(KoResource* resource, const QString& tag)
{
    removeResourceTag(resource, tag);
    d->model->tagCategoryMembersChanged();
    updateTaggedResourceView();
}

void KoResourceItemChooser::contextCreateNewTag(const QString& tag)
{
        if (!tag.isEmpty()) {
        d->model->tagCategoryAdded(tag);
        d->tagOpComboBox->setCurrentIndex(d->tagOpComboBox->findText(tag));
    }
}

void KoResourceItemChooser::contextCreateNewTag(KoResource* resource , const QString& tag)
{
    if (!tag.isEmpty()) {
        d->model->tagCategoryAdded(tag);
        if (resource) {
        addResourceTag(resource, tag);
        }
    }
}

void KoResourceItemChooser::enableContextMenu(bool enable)
{
    d->showContextMenu = enable;
}

void KoResourceItemChooser::syncTagBoxEntryRemoval(const QString& tag)
{
    int pos = d->tagOpComboBox->findText(tag);
    if (pos >= 0) {
        d->tagOpComboBox->removeItem(pos);
    }
}

void KoResourceItemChooser::syncTagBoxEntryAddition(const QString& tag)
{
    QStringList tags = availableTags();
    tags.append(tag);
    tags.sort();
    tags.prepend(d->unfilteredView);
    int index = tags.indexOf(tag);
    if (d->tagOpComboBox->findText(tag) == -1) {
        d->tagOpComboBox->insertItem(index, tag);
    }
}

void KoResourceItemChooser::tagSaveButtonPressed()
{
    tagSearchLineEditActivated(QString());
}

QStringList KoResourceItemChooser::availableTags() const
{
    const int count = d->tagOpComboBox->count();
    QStringList out;
//      Skip the first item of the combo box since it is the `All Presets' view
//      and thus is not a real tag.
    for (int i = 1; i < count; ++i) {
        out.append(d->tagOpComboBox->itemText(i));
    }

    return out;
}

void KoResourceItemChooser::tagBoxEntryDeletionRequested()
{
    removeTagFromComboBox();
}

void KoResourceItemChooser::tagOptionsContextMenuAboutToShow()
{
    int index = d->tagOpComboBox->currentIndex();

    if (index > 0) {
        d->action_renameTag->setVisible(true);
        d->action_deleteTag->setVisible(true);
    }
    else {
        d->action_renameTag->setVisible(false);
        d->action_deleteTag->setVisible(false);
    }

    QString deletedTagName = d->lastDeletedTag.tagName;
    d->action_undeleteTag->setText(i18n("Undelete") +" "+ deletedTagName);
    d->action_undeleteTag->setVisible(!deletedTagName.isEmpty());
    d->action_purgeTagUndeleteList->setVisible(!deletedTagName.isEmpty());
}

void KoResourceItemChooser::undeleteLastDeletedTag()
{
    QString tagName = d->lastDeletedTag.tagName;
    QStringList allTags = availableTags();

    if (allTags.contains(tagName)) {
        bool ok;
        tagName = QInputDialog::getText(this, i18n("Unable to undelete tag"),
                i18n("<qt>The tag you are trying to undelete already exists in tag list.<br>Please enter a new, unique name for it.</qt>"), QLineEdit::Normal,
                tagName, &ok);

        if (!ok || allTags.contains(tagName) || tagName.isEmpty()) {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText(i18n("Tag was not undeleted."));
            msgBox.exec();
            return;
        }
    }

    QList<KoResource *> serverResources = d->model->serverResources();

    foreach(KoResource * resource, d->lastDeletedTag.resources) {
        if (serverResources.contains(resource)) {
            addResourceTag(resource,tagName);
        }
    }
    d->model->tagCategoryAdded(tagName);
    d->tagOpComboBox->setCurrentIndex(d->tagOpComboBox->findText(tagName));
    d->lastDeletedTag = TaggedResourceSet();
}

void KoResourceItemChooser::purgeTagUndeleteList()
{
    d->lastDeletedTag = TaggedResourceSet();
}


#include <KoResourceItemChooser.moc>
