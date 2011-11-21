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

#ifndef H_IMAGE_STRIP_SCENE_H_
#define H_IMAGE_STRIP_SCENE_H_

#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QImageReader>
#include <QThread>
#include <QAtomicInt>
#include <QMutex>

class KisImageItem;

class KisImageLoader: public QThread
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
    
signals:
    void sigItemContentChanged(KisImageItem* item);
    
public:
    KisImageLoader(float size): m_size(size), m_run(true) { }
    
    void addPath(KisImageItem* item, const QString& path) {
        m_data[item] = Data(path);
    }
    
    bool isImageLoaded(KisImageItem* item) const {
        return m_data[item].isLoaded != 0;
    }
    
    QImage getImage(KisImageItem* item) const {
        return m_data[item].image;
    }
    
    void stopExecution() {
        m_run = false;
    }
    
    virtual void run();
    
private:
    float                     m_size;
    QHash<KisImageItem*,Data> m_data;
    QAtomicInt                m_run;
};

class KisImageItem: public QGraphicsWidget
{
public:
    KisImageItem(float size, const QString& path, KisImageLoader* loader):
        m_size(size), m_loader(loader), m_path(path)
    {
        setFlag(QGraphicsItem::ItemIsSelectable, true);
    }
    
    const QString& path() const { return m_path; }
    
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint=QSizeF()) const;
    virtual void   paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget=0);
    
private:
    float           m_size;
    KisImageLoader* m_loader;
    QString         m_path;
};

class KisImageStripScene: public QGraphicsScene
{
    Q_OBJECT
    
public:
    KisImageStripScene();
    bool setCurrentDirectory(const QString& path);
    
signals:
    void sigImageActivated(const QString& path);
    
private:
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
    
private slots:
    void slotItemContentChanged(KisImageItem* item);
    
private:
    float           m_imgSize;
    quint32         m_numItems;
    KisImageLoader* m_loader;
    QMutex          m_mutex;
};

#endif // H_IMAGE_STRIP_SCENE_H_
