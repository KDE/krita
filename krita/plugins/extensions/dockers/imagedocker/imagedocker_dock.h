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

#ifndef H_IMAGEDOCKER_DOCK_H_
#define H_IMAGEDOCKER_DOCK_H_

#include <QDockWidget>
#include <KoCanvasObserverBase.h>
#include <QStringList>
#include <QPixmap>
#include <QMap>

class QModelIndex;
class QFileSystemModel;
class QButtonGroup;
class KoCanvasBase;
class ImageFilter;
class KisImageStripScene;
class ImageListModel;
struct ImageDockerUI;
struct PopupWidgetUI;

class ImageDockerDock: public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
    
    struct ImageInfo
    {
        qint64  id;
        int     viewMode;
        QString path;
        QString name;
        float   scale;
        QPixmap pixmap;
        QPoint  scrollPos;
    };
    
    typedef QMap<qint64,ImageInfo>::iterator ImageInfoIter;
    
public:
    ImageDockerDock();
    virtual ~ImageDockerDock();
    virtual void setCanvas(KoCanvasBase* canvas);
    virtual void unsetCanvas() { m_canvas = 0; }
    
private slots:
    void slotItemDoubleClicked(const QModelIndex& index);
    void slotBackButtonClicked();
    void slotUpButtonClicked();
    void slotHomeButtonClicked();
    void slotCloseCurrentImage();
    void slotNextImage();
    void slotPrevImage();
    void slotOpenImage(const QString& path);
    void slotImageChoosenFromComboBox(int index);
    void slotZoomChanged(int zoom);
    void slotColorSelected(const QColor& color);
    void slotDockLocationChanged(Qt::DockWidgetArea area);
    void slotTopLevelChanged(bool topLevel);
    void slotViewModeChanged(int viewMode, qreal scale);
    void slotCloseZoomPopup();
    
private:
    void addCurrentPathToHistory();
    void updatePath(const QString& path);
    qint64 generateImageID() const;
    void setCurrentImage(qint64 imageID);
    bool isImageLoaded() const { return m_currImageID != -1; }
    void setZoom(const ImageInfo& info);
    
private:
    QFileSystemModel*      m_model;
    QButtonGroup*          m_zoomButtons;
    KoCanvasBase*          m_canvas;
    ImageFilter*           m_proxyModel;
    ImageListModel*        m_imgListModel;
    QStringList            m_history;
    KisImageStripScene*    m_thumbModel;
    ImageDockerUI*         m_ui;
    PopupWidgetUI*         m_popupUi;
    QMap<qint64,ImageInfo> m_imgInfoMap;
    qint64                 m_currImageID;
};

#endif // H_IMAGEDOCKER_DOCK_H_
