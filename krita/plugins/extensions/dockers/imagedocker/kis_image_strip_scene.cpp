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

#include "kis_image_strip_scene.h"

#include <KoIcon.h>

#include <QDir>
#include <QPainter>
#include <QHash>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>
#include <QMutexLocker>

/////////////////////////////////////////////////////////////////////////////////////////////
// ------------- KisImageLoader ---------------------------------------------------------- //

void KisImageLoader::run()
{
    typedef QHash<KisImageItem*,Data>::iterator Iterator;
    
    QImageReader reader;

#ifdef Q_OS_WIN
    for(Iterator data=m_data.begin(); data!=m_data.end() && m_run; ++data) {
        data->image = QImage(data->path).scaled(m_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        data->isLoaded = true;
        emit sigItemContentChanged(data.key());
    }
#else
    for(Iterator data=m_data.begin(); data!=m_data.end() && m_run; ++data) {
        reader.setFileName(data->path);
        qreal w = m_size;
        qreal h = m_size;
        

        if (reader.supportsOption(QImageIOHandler::Size)) {
            QSizeF imgSize = reader.size();
            
            if(imgSize.width() > imgSize.height()) {
                qreal div = m_size / imgSize.width();
                h = imgSize.height() * div;
            }
            else {
                qreal div = m_size / imgSize.height();
                w = imgSize.width() * div;
            }
        }

        
        reader.setScaledSize(QSize(w,h));
        data->image    = reader.read();
        data->isLoaded = true;
        emit sigItemContentChanged(data.key());
    }
#endif
}


/////////////////////////////////////////////////////////////////////////////////////////////
// ------------- KisImageItem ------------------------------------------------------------ //

void KisImageItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    if(m_loader->isImageLoaded(this)) {
        QImage  image = m_loader->getImage(this);
        
        if(!image.isNull()) {
            QPointF offset((m_size-image.width()) / 2.0, (m_size-image.height()) / 2.0);
            painter->drawImage(offset, image);
        }
        else {
            QIcon   icon = koIcon("image-missing");
            QRect   rect = boundingRect().toRect();
            QPixmap img  = icon.pixmap(rect.size());
            painter->drawPixmap(rect, img, img.rect());
        }
    }
    else {
        QIcon   icon = koIcon("image-loading");
        QRect   rect = boundingRect().toRect();
        QPixmap img  = icon.pixmap(rect.size());
        painter->drawPixmap(rect, img, img.rect());
    }
    
    if(isSelected())
        painter->fillRect(boundingRect(), Qt::Dense5Pattern);
    
    painter->drawRect(boundingRect());
}

QSizeF KisImageItem::sizeHint(Qt::SizeHint /*which*/, const QSizeF& /*constraint*/) const
{
    return QSizeF(m_size, m_size);
}


/////////////////////////////////////////////////////////////////////////////////////////////
// ------------- KisImageStripScene ------------------------------------------------------ //

KisImageStripScene::KisImageStripScene():
    m_imgSize(80), m_loader(0) { }

bool KisImageStripScene::setCurrentDirectory(const QString& path)
{
    QMutexLocker locker(&m_mutex);
    QDir         directory(path);
    QImageReader reader;
    
    if(directory.exists()) {
        clear();
        
        if(m_loader) {
            m_loader->disconnect(this);
            m_loader->stopExecution();
            
            if(!m_loader->wait(500)) {
                m_loader->terminate();
                m_loader->wait();
            }
        }
        
        delete m_loader;
        
        m_numItems = 0;
        m_loader   = new KisImageLoader(m_imgSize);
        connect(m_loader, SIGNAL(sigItemContentChanged(KisImageItem*)), SLOT(slotItemContentChanged(KisImageItem*)));
        
        QStringList            files  = directory.entryList(QDir::Files);
        QGraphicsLinearLayout* layout = new QGraphicsLinearLayout();
        
        for(QStringList::iterator name=files.begin(); name!=files.end(); ++name) {
            QString path = directory.absoluteFilePath(*name);
            reader.setFileName(path);
            
            if(reader.canRead()) {
                KisImageItem* item = new KisImageItem(m_imgSize, path, m_loader);
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

void KisImageStripScene::slotItemContentChanged(KisImageItem* item)
{
    QMutexLocker locker(&m_mutex);
    item->update();
}

void KisImageStripScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    KisImageItem* item = static_cast<KisImageItem*>(itemAt(event->scenePos()));
    
    if(item)
        emit sigImageActivated(item->path());
}
