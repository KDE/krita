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

#ifndef H_IMAGEDOCKER_DOCK_H_
#define H_IMAGEDOCKER_DOCK_H_

#include <QPointer>
#include <QDockWidget>
#include <KoCanvasObserverBase.h>
#include <QStringList>
#include <QPixmap>
#include <QMap>

#include <KoCanvasBase.h>

class QModelIndex;
class QFileSystemModel;
class QButtonGroup;
class ImageFilter;
class ImageStripScene;
class ImageListModel;
class QTemporaryFile;
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
    ~ImageDockerDock() override;
    QString observerName() override { return "ImageDockerDock"; }
    void setCanvas(KoCanvasBase* canvas) override;
    void unsetCanvas() override {
        m_canvas = 0; // Intentionally not disabled if there's no canvas
    }
    
private Q_SLOTS:
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
    void slotViewModeChanged(int viewMode, qreal scale);
    void slotCloseZoomPopup();
    void slotChangeRoot(const QString& path);
    
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void showEvent(QShowEvent *) override;
private:
    void addCurrentPathToHistory();
    void updatePath(const QString& path);
    qint64 generateImageID() const;
    void setCurrentImage(qint64 imageID);
    bool isImageLoaded() const { return m_currImageID != -1; }
    void setZoom(const ImageInfo& info);
    
    void saveConfigState();
    void loadConfigState();


private:
    QFileSystemModel*      m_model;
    QButtonGroup*          m_zoomButtons;
    QPointer<KoCanvasBase> m_canvas;
    ImageFilter*           m_proxyModel;
    ImageListModel*        m_imgListModel;
    QStringList            m_history;
    ImageStripScene*       m_imageStripScene;
    ImageDockerUI*         m_ui;
    PopupWidgetUI*         m_popupUi;
    QMap<qint64,ImageInfo> m_imgInfoMap;
    qint64                 m_currImageID;
    QList<QTemporaryFile*> m_temporaryFiles;
};

#endif // H_IMAGEDOCKER_DOCK_H_
