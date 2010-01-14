/*
 *
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _PANORAMA_H_
#define _PANORAMA_H_

#include <QVariant>

#include <kparts/plugin.h>

#include <kis_types.h>
#include <kis_image_alignment.h>

class KisView2;
class Ui_WdgPanoramaCreation;

/**
 * Template of view plugin
 */
class PanoramaPlugin : public KParts::Plugin
{
    Q_OBJECT
public:
    PanoramaPlugin(QObject *parent, const QVariantList &);
    virtual ~PanoramaPlugin();

private slots:

    void slotAddImages();
    void slotRemoveImage();
    void slotImageUp();
    void slotImageDown();
    void slotCreatePanoramaLayer();
    void addImage(const QString& filename);
    void slotPreview();

private:
    void createPanorama(QList<KisImageAlignment::ImageInfo>& images, KisPaintDeviceSP dstdevice, QRect& area);
private:
    KisView2 * m_view;
    Ui_WdgPanoramaCreation* m_wdgPanoramaCreation;
};
#endif // PanoramaPlugin_H

