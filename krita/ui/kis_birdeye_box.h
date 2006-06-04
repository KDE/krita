/*
 *  Copyright (c) 2004 Kivio Team
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

#include "kis_types.h"

class KoBirdEyePanel;
class KisDoubleWidget;
class KisView;
class KisCanvasSubject;
class KoZoomAdapter;
class KoColorSpace;

class KisBirdEyeBox : public QWidget
{
    Q_OBJECT

public:

    KisBirdEyeBox(KisView * view, QWidget * parent = 0, const char* name=0);
    ~KisBirdEyeBox();

    void setImage(KisImageSP image);

public slots:
    void slotDocCommandExecuted();
    void slotImageUpdated(QRect r);
    void slotImageSizeChanged(qint32 w, qint32 h);
    void slotImageColorSpaceChanged(KoColorSpace *cs);

protected slots:
    void exposureValueChanged(double exposure);
    void exposureSliderPressed();
    void exposureSliderReleased();

private:
    KoBirdEyePanel * m_birdEyePanel;
    KisDoubleWidget * m_exposureDoubleWidget;
    QLabel *m_exposureLabel;
    KisView * m_view;
    KisCanvasSubject * m_subject;
    bool m_draggingExposureSlider;
    KoZoomAdapter * m_zoomAdapter;
    KisImageSP m_image;
    QRect m_dirtyRect;
};

#endif // KIS_BIRDEYE_BOX_H
