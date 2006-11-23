/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA..
 */
#ifndef KIS_IMAGE_MANAGER
#define KIS_IMAGE_MANAGER

#include <QObject>

#include <kurl.h>

class KisView2;
class KActionCollection;
class KisFilterStrategy;

class KisImageManager : public QObject {

    Q_OBJECT

public:


    KisImageManager(KisView2 * view );
    ~KisImageManager() {}

    void setup(KActionCollection * actionCollection);
    void updateGUI();

public slots:

    void slotInsertImageAsLayer();

    /**
     * Import an image as a layer. If there is more than
     * one layer in the image, import all of them as separate
     * layers.
     *
     * @param url the url to the image file
     * @return the number of layers added
     */
    qint32 importImage(const KUrl& url = KUrl());

    void resizeCurrentImage(qint32 w, qint32 h, bool cropLayers);
    void scaleCurrentImage(double sx, double sy, KisFilterStrategy *filterStrategy);
    void rotateCurrentImage(double angle);
    void shearCurrentImage(double angleX, double angleY);


private:
    KisView2 * m_view;
};

#endif // KIS_IMAGE_MANAGER
