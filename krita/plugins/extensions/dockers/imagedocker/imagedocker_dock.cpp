/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "imagedocker_dock.h"
#include "kis_image_strip_scene.h"
#include "kis_image_view.h"

#include <klocale.h>
#include <KoCanvasResourceManager.h>
#include <KoCanvasBase.h>
#include <KoColorSpaceRegistry.h>

#include <QFileSystemModel>
#include <QImageReader>
#include <QSortFilterProxyModel>
#include <QFileInfo>
#include <QMouseEvent>
#include <QDir>
#include <QLineEdit>
#include <QLabel>
#include <QIcon>
#include <QAbstractListModel>
#include <QButtonGroup>

#include "ui_wdgimagedocker.h"
#include "ui_wdgImageViewPopup.h"

///////////////////////////////////////////////////////////////////////////////
// --------- ImageFilter --------------------------------------------------- //

class ImageFilter: public QSortFilterProxyModel
{
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
        QFileSystemModel* model = static_cast<QFileSystemModel*>(sourceModel());
        QModelIndex       index = sourceModel()->index(source_row, 0, source_parent);

        if(model->isDir(index))
            return true;

        m_reader.setFileName(model->filePath(index));
        return m_reader.canRead();
    }

    virtual bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const {
        Q_UNUSED(source_parent);
        return source_column == 0;
    }

    mutable QImageReader m_reader;
};


///////////////////////////////////////////////////////////////////////////////
// --------- ImageListModel ------------------------------------------------ //

class ImageListModel: public QAbstractListModel
{
    struct Data
    {
        QPixmap icon;
        QString text;
        qint64  id;
    };

public:
    void addImage(const QPixmap& pixmap, const QString& text, qint64 id) {
        Data data;
        data.icon = pixmap.scaled(70, 70, Qt::KeepAspectRatio);
        data.text = text;
        data.id   = id;
        emit layoutAboutToBeChanged();
        m_data.push_back(data);
        emit layoutChanged();
    }

    qint64 imageID(int index) const { return m_data[index].id; }

    void removeImage(qint64 id) {
        typedef QList<Data>::iterator Iterator;

        for(Iterator data=m_data.begin(); data!=m_data.end(); ++data) {
            if(data->id == id) {
                emit layoutAboutToBeChanged();
                m_data.erase(data);
                emit layoutChanged();
                return;
            }
        }
    }

    int indexFromID(qint64 id) {
        for(int i=0; i<m_data.size(); ++i) {
            if(m_data[i].id == id)
                return i;
        }
        return -1;
    }

    virtual int rowCount(const QModelIndex& parent=QModelIndex()) const {
        Q_UNUSED(parent);
        return m_data.size();
    }

    virtual QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const {
        if(index.isValid() && index.row() < m_data.size()) {
            switch(role)
            {
            case Qt::DisplayRole:
                return m_data[index.row()].text;
            case Qt::DecorationRole:
                return m_data[index.row()].icon;
            }
        }
        return QVariant();
    }

private:
    QList<Data> m_data;
};


///////////////////////////////////////////////////////////////////////////////
// --------- ImageDockerUI ------------------------------------------------- //

struct ImageDockerUI: public QWidget, public Ui_wdgImageDocker
{
    ImageDockerUI() {
        setupUi(this);
    }
};

///////////////////////////////////////////////////////////////////////////////
// --------- PopupWidgetUI ------------------------------------------------- //

struct PopupWidgetUI: public QWidget, public Ui_wdgImageViewPopup
{
    PopupWidgetUI() {
        setupUi(this);
    }
};


///////////////////////////////////////////////////////////////////////////////
// --------- ImageDockerDock ----------------------------------------------- //

ImageDockerDock::ImageDockerDock():
    QDockWidget(i18n("Reference Images")),
    m_canvas(0),
    m_currImageID(-1)
{
    m_ui           = new ImageDockerUI();
    m_popupUi      = new PopupWidgetUI();
    m_zoomButtons  = new QButtonGroup();
    m_imgListModel = new ImageListModel();
    m_thumbModel   = new KisImageStripScene();
    m_model        = new QFileSystemModel();
    m_proxyModel   = new ImageFilter();
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setDynamicSortFilter(true);

    m_ui->bnBack->setIcon(QIcon::fromTheme("go-previous"));
    m_ui->bnUp->setIcon(QIcon::fromTheme("go-up"));
    m_ui->bnHome->setIcon(QIcon::fromTheme("go-home"));
    m_ui->bnImgPrev->setIcon(QIcon::fromTheme("go-previous"));
    m_ui->bnImgNext->setIcon(QIcon::fromTheme("go-next"));
    m_ui->bnImgClose->setIcon(QIcon::fromTheme("window-close"));
    m_ui->thumbView->setScene(m_thumbModel);
    m_ui->treeView->setModel(m_proxyModel);
    m_ui->cmbImg->setModel(m_imgListModel);
    m_ui->bnPopup->setIcon(QIcon::fromTheme("zoom-original"));
    m_ui->bnPopup->setPopupWidget(m_popupUi);

    m_popupUi->zoomSlider->setRange(5, 500);
    m_popupUi->zoomSlider->setValue(100);

    m_zoomButtons->addButton(m_popupUi->bnZoomFit   , KisImageView::VIEW_MODE_FIT);
    m_zoomButtons->addButton(m_popupUi->bnZoomAdjust, KisImageView::VIEW_MODE_ADJUST);
    m_zoomButtons->addButton(m_popupUi->bnZoom25    , 25);
    m_zoomButtons->addButton(m_popupUi->bnZoom50    , 50);
    m_zoomButtons->addButton(m_popupUi->bnZoom75    , 75);
    m_zoomButtons->addButton(m_popupUi->bnZoom100   , 100);

    m_model->setRootPath(QDir::rootPath());
    m_ui->treeView->setRootIndex(m_proxyModel->mapFromSource(m_model->index(QDir::homePath())));
    updatePath(QDir::homePath());

    connect(m_ui->treeView           , SIGNAL(doubleClicked(const QModelIndex&))      , SLOT(slotItemDoubleClicked(const QModelIndex&)));
    connect(m_ui->bnBack             , SIGNAL(clicked(bool))                          , SLOT(slotBackButtonClicked()));
    connect(m_ui->bnHome             , SIGNAL(clicked(bool))                          , SLOT(slotHomeButtonClicked()));
    connect(m_ui->bnUp               , SIGNAL(clicked(bool))                          , SLOT(slotUpButtonClicked()));
    connect(m_thumbModel             , SIGNAL(sigImageActivated(const QString&))      , SLOT(slotOpenImage(QString)));
    connect(m_ui->bnImgNext          , SIGNAL(clicked(bool))                          , SLOT(slotNextImage()));
    connect(m_ui->bnImgPrev          , SIGNAL(clicked(bool))                          , SLOT(slotPrevImage()));
    connect(m_ui->bnImgClose         , SIGNAL(clicked(bool))                          , SLOT(slotCloseCurrentImage()));
    connect(m_ui->cmbImg             , SIGNAL(activated(int))                         , SLOT(slotImageChoosenFromComboBox(int)));
    connect(m_ui->imgView            , SIGNAL(sigColorSelected(const QColor&))        , SLOT(slotColorSelected(const QColor)));
    connect(m_ui->imgView            , SIGNAL(sigViewModeChanged(int, qreal))         , SLOT(slotViewModeChanged(int, qreal)));
    connect(m_popupUi->zoomSlider    , SIGNAL(valueChanged(int))                      , SLOT(slotZoomChanged(int)));
    connect(m_zoomButtons            , SIGNAL(buttonClicked(int))                     , SLOT(slotZoomChanged(int)));
    connect(m_zoomButtons            , SIGNAL(buttonClicked(int))                     , SLOT(slotCloseZoomPopup()));
    connect(this                     , SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));
    connect(this                     , SIGNAL(topLevelChanged(bool))                  , SLOT(slotTopLevelChanged(bool)));

    setWidget(m_ui);
}

ImageDockerDock::~ImageDockerDock()
{
    delete m_proxyModel;
    delete m_model;
    delete m_thumbModel;
    delete m_imgListModel;
    delete m_zoomButtons;
}

void ImageDockerDock::setCanvas(KoCanvasBase* canvas)
{
    // "Every connection you make emits a signal, so duplicate connections emit two signals"
    if(m_canvas)
        m_canvas->disconnectCanvasObserver(this);

    m_canvas = canvas;
}

void ImageDockerDock::addCurrentPathToHistory()
{
    m_history.push_back(m_model->filePath(m_proxyModel->mapToSource(m_ui->treeView->rootIndex())));
}

void ImageDockerDock::updatePath(const QString& path)
{
    m_ui->cmbPath->lineEdit()->setText(path);
    m_ui->bnBack->setDisabled(m_history.empty());
    m_thumbModel->setCurrentDirectory(path);
}

qint64 ImageDockerDock::generateImageID() const
{
    static qint64 id = 0;
    return ++id;
}

void ImageDockerDock::setCurrentImage(qint64 imageID)
{
    if(m_imgInfoMap.contains(m_currImageID))
        m_imgInfoMap[m_currImageID].scrollPos = m_ui->imgView->getScrollPos();

    m_ui->bnImgClose->setDisabled(imageID < 0);
    m_ui->bnPopup->setDisabled(imageID < 0);

    if(imageID < 0) {
        m_currImageID = -1;
        m_ui->imgView->setPixmap(QPixmap());
    }
    else if(m_imgInfoMap.contains(imageID)) {
        ImageInfoIter info = m_imgInfoMap.find(imageID);

        m_ui->imgView->blockSignals(true);
        m_ui->imgView->setPixmap(info->pixmap);
        setZoom(*info);
        m_ui->imgView->blockSignals(false);

        m_ui->bnImgPrev->setDisabled(info == m_imgInfoMap.begin());
        m_ui->bnImgNext->setDisabled((info+1) == m_imgInfoMap.end());

        m_ui->cmbImg->blockSignals(true);
        m_ui->cmbImg->setCurrentIndex(m_imgListModel->indexFromID(imageID));
        m_ui->cmbImg->blockSignals(false);

        m_currImageID = imageID;
    }
}

void ImageDockerDock::setZoom(const ImageInfo& info)
{
    m_ui->imgView->setViewMode(info.viewMode, info.scale);
    m_ui->imgView->setScrollPos(info.scrollPos);

    int zoom = qRound(m_ui->imgView->getScale() * 100.0f);

    m_popupUi->zoomSlider->blockSignals(true);
    m_popupUi->zoomSlider->setValue(zoom);
    m_popupUi->zoomSlider->blockSignals(false);
}


// ------------ slots ------------------------------------------------- //

void ImageDockerDock::slotItemDoubleClicked(const QModelIndex& index)
{
    QModelIndex mappedIndex = m_proxyModel->mapToSource(index);
    mappedIndex = m_model->index(mappedIndex.row(), 0, mappedIndex.parent());
    QString path(m_model->filePath(mappedIndex));

    if(m_model->isDir(mappedIndex)) {
        addCurrentPathToHistory();
        updatePath(path);
        m_ui->treeView->setRootIndex(m_proxyModel->mapFromSource(mappedIndex));
    }
    else slotOpenImage(path);
}

void ImageDockerDock::slotBackButtonClicked()
{
    if(!m_history.empty()) {
        QString     path  = m_history.last();
        QModelIndex index = m_proxyModel->mapFromSource(m_model->index(path));
        m_ui->treeView->setRootIndex(index);
        m_history.pop_back();
        updatePath(path);
    }
}

void ImageDockerDock::slotHomeButtonClicked()
{
    addCurrentPathToHistory();
    QModelIndex index = m_proxyModel->mapFromSource(m_model->index(QDir::homePath()));
    m_ui->treeView->setRootIndex(index);
    updatePath(QDir::homePath());
}

void ImageDockerDock::slotUpButtonClicked()
{
    addCurrentPathToHistory();

    QModelIndex index = m_proxyModel->mapToSource(m_ui->treeView->rootIndex());
    QDir        dir(m_model->filePath(index));
    dir.makeAbsolute();

    if(dir.cdUp()) {
        index = m_proxyModel->mapFromSource(m_model->index(dir.path()));
        m_ui->treeView->setRootIndex(index);
        updatePath(dir.path());
    }
}

void ImageDockerDock::slotOpenImage(const QString& path)
{
    QPixmap pixmap(path);

    if(!pixmap.isNull()) {
        QFileInfo fileInfo(path);
        ImageInfo imgInfo;
        imgInfo.id        = generateImageID();
        imgInfo.name      = fileInfo.fileName();
        imgInfo.path      = fileInfo.absoluteFilePath();
        imgInfo.viewMode  = KisImageView::VIEW_MODE_FIT;
        imgInfo.scale     = 1.0f;
        imgInfo.pixmap    = pixmap;
        imgInfo.scrollPos = QPoint(0, 0);

        m_imgInfoMap[imgInfo.id] = imgInfo;
        m_imgListModel->addImage(imgInfo.pixmap, imgInfo.name, imgInfo.id);
        setCurrentImage(imgInfo.id);
        m_ui->tabWidget->setCurrentIndex(1);
    }
}

void ImageDockerDock::slotCloseCurrentImage()
{
    ImageInfoIter info = m_imgInfoMap.find(m_currImageID);

    if(info != m_imgInfoMap.end()) {
        ImageInfoIter next = info + 1;
        ImageInfoIter prev = info - 1;
        qint64        id   = -1;

        if(next != m_imgInfoMap.end())
            id = next->id;
        else if(info != m_imgInfoMap.begin())
            id = prev->id;

        m_imgListModel->removeImage(info->id);
        m_imgInfoMap.erase(info);
        setCurrentImage(id);

        if(id < 0)
            m_ui->tabWidget->setCurrentIndex(0);
    }
}

void ImageDockerDock::slotNextImage()
{
    ImageInfoIter info = m_imgInfoMap.find(m_currImageID);

    if(info != m_imgInfoMap.end()) {
        ++info;
        if(info != m_imgInfoMap.end())
            setCurrentImage(info->id);
    }
}

void ImageDockerDock::slotPrevImage()
{
    ImageInfoIter info = m_imgInfoMap.find(m_currImageID);

    if(info != m_imgInfoMap.end() && info != m_imgInfoMap.begin()) {
        --info;
        setCurrentImage(info->id);
    }
}

void ImageDockerDock::slotImageChoosenFromComboBox(int index)
{
    setCurrentImage(m_imgListModel->imageID(index));
}

void ImageDockerDock::slotZoomChanged(int zoom)
{
    if(isImageLoaded()) {
        ImageInfoIter info = m_imgInfoMap.find(m_currImageID);

        switch(zoom)
        {
        case KisImageView::VIEW_MODE_FIT:
        case KisImageView::VIEW_MODE_ADJUST:
            info->viewMode = zoom;
            break;

        default:
            info->viewMode = KisImageView::VIEW_MODE_FREE;
            info->scale    = float(zoom) / 100.0f;
            break;
        }

        setZoom(*info);
    }
}

void ImageDockerDock::slotColorSelected(const QColor& color)
{
    m_canvas->resourceManager()->setForegroundColor(
        KoColor(color, KoColorSpaceRegistry::instance()->rgb8())
    );
}

void ImageDockerDock::slotDockLocationChanged(Qt::DockWidgetArea area)
{
    if(area == Qt::AllDockWidgetAreas)
        m_ui->tabWidget->setTabPosition(QTabWidget::North);
    else if(area & Qt::LeftDockWidgetArea)
        m_ui->tabWidget->setTabPosition(QTabWidget::East);
    else if(area & Qt::RightDockWidgetArea)
        m_ui->tabWidget->setTabPosition(QTabWidget::West);
    else
        m_ui->tabWidget->setTabPosition(QTabWidget::North);
}

void ImageDockerDock::slotTopLevelChanged(bool topLevel)
{
    if(topLevel)
        m_ui->tabWidget->setTabPosition(QTabWidget::North);
}

void ImageDockerDock::slotViewModeChanged(int viewMode, qreal scale)
{
    if(isImageLoaded()) {
        m_imgInfoMap[m_currImageID].viewMode = viewMode;
        m_imgInfoMap[m_currImageID].scale    = scale;

        int zoom = qRound(scale * 100.0);

        m_popupUi->zoomSlider->blockSignals(true);
        m_popupUi->zoomSlider->setValue(zoom);
        m_popupUi->zoomSlider->blockSignals(false);
    }
}

void ImageDockerDock::slotCloseZoomPopup()
{
    m_ui->bnPopup->hidePopupWidget();
}
