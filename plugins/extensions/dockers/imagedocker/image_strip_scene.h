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

#ifndef H_IMAGE_STRIP_SCENE_H_
#define H_IMAGE_STRIP_SCENE_H_

#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QImageReader>
#include <QThread>
#include <QAtomicInt>
#include <QMutex>

class ImageItem;

class ImageLoader: public QThread
{
    Q_OBJECT
    
    struct Data
    {
        Data() { }
        Data(const QString& p):
            path(p), isLoaded(false) { };
        Data(const Data& d):
            image(d.image), path(d.path), isLoaded(d.isLoaded) { };
        
        QImage     image;
        QString    path;
        QAtomicInt isLoaded;
    };
    
Q_SIGNALS:
    void sigItemContentChanged(ImageItem* item);
    
public:
    ImageLoader(float size);
    
    void addPath(ImageItem* item, const QString& path) {
        m_data[item] = Data(path);
    }
    
    bool isImageLoaded(ImageItem* item) const {
        return m_data[item].isLoaded != 0;
    }
    
    QImage getImage(ImageItem* item) const {
        return m_data[item].image;
    }

    virtual void run();

public Q_SLOTS:

    void stopExecution();

private:
    float                     m_size;
    QHash<ImageItem*,Data> m_data;
    QAtomicInt                m_run;
};

class ImageItem: public QGraphicsWidget
{
public:
    ImageItem(float size, const QString& path, ImageLoader* loader):
        m_size(size), m_loader(loader), m_path(path)
    {
        setFlag(QGraphicsItem::ItemIsSelectable, true);
    }
    
    const QString& path() const { return m_path; }
    
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint=QSizeF()) const;
    virtual void   paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget=0);
    
private:
    float m_size;
    ImageLoader* m_loader;
    QString m_path;
};

class ImageStripScene: public QGraphicsScene
{
    Q_OBJECT
    
public:
    ImageStripScene();
    ~ImageStripScene();
    bool setCurrentDirectory(const QString& path);
    QString currentPath() { return m_path; }
Q_SIGNALS:
    void sigImageActivated(const QString& path);
    
private:
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
    
private Q_SLOTS:
    void slotItemContentChanged(ImageItem* item);
    
private:
    float  m_imgSize;
    quint32 m_numItems;
    ImageLoader* m_loader;
    QMutex m_mutex;
    QString m_path;
};

#endif // H_IMAGE_STRIP_SCENE_H_
