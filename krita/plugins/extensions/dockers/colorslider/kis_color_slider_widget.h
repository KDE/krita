/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Wolthera van Hövell <griffinvalley@gmail.com>

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

#ifndef _KIS_COLOR_SLIDER_WIDGET_H_
#define _KIS_COLOR_SLIDER_WIDGET_H_

#include <QWidget>
#include "kis_canvas2.h"
#include <KoColor.h>

class KoColorSpace;
class KoColorDisplayRendererInterface;
class QVBoxLayout;
class KisColorSliderInput;
class KisSignalCompressor;
class QBitArray;

class KisColorSliderWidget : public QWidget
{
    Q_OBJECT
public:
    KisColorSliderWidget(KoColorDisplayRendererInterface *displayRenderer, QWidget* parent, KisCanvas2* canvas, QBitArray SlidersConfigArray);
    ~KisColorSliderWidget();

    //bool customColorSpaceUsed();
public slots:
    //void setColorSpace(const KoColorSpace*);
    void setColor(const KoColor&);
    void slotConfigChanged();
private slots:
    void update();
    void updateTimeout();
    void hueUpdate(int h);
    void satUpdate(int s, int type);
    void toneUpdate(int l, int type);
    void setSlidersVisible(QBitArray SlidersConfigArray);
    void setConfig();
    //void setCustomColorSpace(const KoColorSpace *);
signals:
    void colorChanged(const KoColor&);
    void updated();
    void hueUpdated(int);
    void satUpdated(int, int);
    void toneUpdated(int, int);
private:
    QList<KisColorSliderInput*> m_inputs;
    const KoColorSpace* m_colorSpace;
    QVBoxLayout *m_layout;
    KoColor m_color;
    bool m_updateAllowed;
    KisSignalCompressor *m_updateCompressor;
    KisSignalCompressor *m_configCompressor;
    KoColorDisplayRendererInterface *m_displayRenderer;
    KisCanvas2 *m_canvas;
    KisColorSliderInput* hsvH;
    KisColorSliderInput* hsvS;
    KisColorSliderInput* hsvV;
    KisColorSliderInput* hslH;
    KisColorSliderInput* hslS;
    KisColorSliderInput* hslL;
    KisColorSliderInput* hsiH;
    KisColorSliderInput* hsiS;
    KisColorSliderInput* hsiI;
    KisColorSliderInput* hsyH;
    KisColorSliderInput* hsyS;
    KisColorSliderInput* hsyY;
};
#endif
