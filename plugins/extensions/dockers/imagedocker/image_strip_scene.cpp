/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "image_strip_scene.h"

#include <kis_icon.h>

#include <QApplication>
#include <QDir>
#include <QPainter>
#include <QHash>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>
#include <QMutexLocker>
#include <kis_debug.h>

/////////////////////////////////////////////////////////////////////////////////////////////
// ------------- ImageLoader ---------------------------------------------------------- //

ImageLoader::ImageLoader(float size)
    : m_size(size)
    , m_run(true)
{
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(stopExecution()));
}

void ImageLoader::run()
{
    typedef QHash<ImageItem*,Data>::iterator Iterator;
    
    for (Iterator data = m_data.begin(); data != m_data.end() && m_run; ++data) {
        QImage img = QImage(data->path);
        if (!img.isNull()) {
            data->image = img.scaled(m_size, m_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        //dbgKrita << "Loaded" << data->path;
        data->isLoaded = true;
        emit sigItemContentChanged(data.key());
    }
}

void ImageLoader::stopExecution()
{
    m_run = false;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// ------------- ImageItem ------------------------------------------------------------ //

void ImageItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    if (m_loader->isImageLoaded(this)) {
        QImage  image = m_loader->getImage(this);
        
        if (!image.isNull()) {
            QPointF offset((m_size-image.width()) / 2.0, (m_size-image.height()) / 2.0);
            painter->drawImage(offset, image);
        }
        else {
            QIcon   icon = KisIconUtils::loadIcon("edit-delete");
            QRect   rect = boundingRect().toRect();
            QPixmap img  = icon.pixmap(rect.size());
            painter->drawPixmap(rect, img, img.rect());
        }
    }
    else {
        QIcon   icon = KisIconUtils::loadIcon("folder-pictures");
        QRect   rect = boundingRect().toRect();
        QPixmap img  = icon.pixmap(rect.size());
        painter->drawPixmap(rect, img, img.rect());
    }
    
    if (isSelected()) {
        painter->setCompositionMode(QPainter::CompositionMode_HardLight);
        painter->setOpacity(0.50);
        painter->fillRect(boundingRect().toRect(), palette().color(QPalette::Active, QPalette::Highlight));
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        QPen pen(palette().color(QPalette::Active, QPalette::Highlight), 3);
        painter->setPen(pen);
    }
    
    painter->drawRect(boundingRect());
}

QSizeF ImageItem::sizeHint(Qt::SizeHint /*which*/, const QSizeF& /*constraint*/) const
{
    return QSizeF(m_size, m_size);
}


/////////////////////////////////////////////////////////////////////////////////////////////
// ------------- ImageStripScene ------------------------------------------------------ //

ImageStripScene::ImageStripScene():
    m_imgSize(80)
  , m_loader(0)
{
}

ImageStripScene::~ImageStripScene()
{
    delete m_loader;
}

bool ImageStripScene::setCurrentDirectory(const QString& path)
{
    m_path = path;
    QMutexLocker locker(&m_mutex);
    QDir         directory(path);
    QImageReader reader;
    
    if (directory.exists()) {
        clear();
        
        if (m_loader) {
            m_loader->disconnect(this);
            m_loader->stopExecution();
            
            if (!m_loader->wait(500)) {
                m_loader->terminate();
                m_loader->wait();
            }
        }
        
        delete m_loader;
        
        m_numItems = 0;
        m_loader   = new ImageLoader(m_imgSize);
        connect(m_loader, SIGNAL(sigItemContentChanged(ImageItem*)), SLOT(slotItemContentChanged(ImageItem*)));
        
        QStringList            files  = directory.entryList(QDir::Files);
        QGraphicsLinearLayout* layout = new QGraphicsLinearLayout();
        
        for (QStringList::iterator name=files.begin(); name!=files.end(); ++name) {
            QString path = directory.absoluteFilePath(*name);
            QString fileExtension = QFileInfo(path).suffix();

            if (!fileExtension.compare("DNG", Qt::CaseInsensitive)) {
                warnKrita << "WARNING: Qt is known to crash when trying to open a DNG file. Skip it";
                continue;
            }

            reader.setFileName(path);

            if(reader.canRead()) {
                ImageItem* item = new ImageItem(m_imgSize, path, m_loader);
                m_loader->addPath(item, path);
                layout->addItem(item);
                ++m_numItems;
            }
        }
        
        QGraphicsWidget* widget = new QGraphicsWidget();
        widget->setLayout(layout);
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        
        addItem(widget);
        setSceneRect(widget->boundingRect());
        
        m_loader->start(QThread::LowPriority);
        return true;
    }
    
    return false;
}

void ImageStripScene::slotItemContentChanged(ImageItem* item)
{
    QMutexLocker locker(&m_mutex);
    item->update();
}

void ImageStripScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    ImageItem* item = static_cast<ImageItem*>(itemAt(event->scenePos()));
    
    if (item)
        emit sigImageActivated(item->path());
}

