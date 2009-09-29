/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_BIRDEYE_BOX_H
#define KIS_BIRDEYE_BOX_H

#include <QWidget>
#include <QLabel>

#include <QDockWidget>

#include <KoDockFactory.h>

#include "kis_types.h"

class KoBirdEyePanel;
class KisDoubleWidget;
class KisView2;
class KoZoomAdapter;
class KoColorSpace;

class KisBirdEyeBox : public QDockWidget
{
    Q_OBJECT

public:

    KisBirdEyeBox(KisView2 * view);
    ~KisBirdEyeBox();

    void setImage(KisImageWSP image);

public slots:
    void slotDocCommandExecuted();
    void slotImageUpdated(const QRect & r);
    void slotImageSizeChanged(qint32 w, qint32 h);
    void slotImageColorSpaceChanged(const KoColorSpace *cs);

protected slots:
    void exposureValueChanged(double exposure);
    void exposureSliderPressed();
    void exposureSliderReleased();

private:

    KoBirdEyePanel * m_birdEyePanel;
    KisDoubleWidget * m_exposureDoubleWidget;
    QLabel *m_exposureLabel;
    KisView2 * m_view;
    bool m_draggingExposureSlider;
    KoZoomAdapter * m_zoomAdapter;
    KisImageWSP m_image;
    QRect m_dirtyRegion;
};


class KisBirdEyeBoxFactory : public KoDockFactory
{
public:
    KisBirdEyeBoxFactory(KisView2 * view)
            : m_view(view) {
    }

    virtual QString id() const {
        return QString("KisBirdeyeBox");
    }

    virtual QDockWidget* createDockWidget() {
        KisBirdEyeBox * dockWidget = new KisBirdEyeBox(m_view);
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const {
        return DockMinimized;
    }
private:
    KisView2 * m_view;
};

#endif // KIS_BIRDEYE_BOX_H
