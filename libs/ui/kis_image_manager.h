/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_IMAGE_MANAGER
#define KIS_IMAGE_MANAGER

#include <QObject>
#include <QPointer>

#include <QUrl>
#include <kritaui_export.h>

class KisViewManager;
class KisFilterStrategy;
class KisActionManager;
class KisView;

class KRITAUI_EXPORT KisImageManager : public QObject
{

    Q_OBJECT

public:


    KisImageManager(KisViewManager * view);
    ~KisImageManager() override {}

    void setView(QPointer<KisView>imageView);
    void setup(KisActionManager *actionManager);

public Q_SLOTS:

    void slotImportLayerFromFile();
    void slotImportLayerAsTransparencyMask();
    void slotImportLayerAsFilterMask();
    void slotImportLayerAsSelectionMask();

    /**
     * Import an image as a layer. If there is more than
     * one layer in the image, import all of them as separate
     * layers.
     *
     * @param url the url to the image file
     * @param layerType the layer type
     * @return the number of layers added
     */
    qint32 importImage(const QUrl &url, const QString &layerType = "KisPaintLayer");

    void resizeCurrentImage(qint32 w, qint32 h, qint32 xOffset, qint32 yOffset);
    void scaleCurrentImage(const QSize &size, qreal xres, qreal yres, KisFilterStrategy *filterStrategy);

    void rotateCurrentImage(double radians);
    void shearCurrentImage(double angleX, double angleY);
    void slotImageProperties();
    void slotImageColor();

private:
    KisViewManager * m_view;
};

#endif // KIS_IMAGE_MANAGER
