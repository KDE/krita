/*
 *  dlg_layersize.h -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2013 Juan Palacios <jpalaciosdev@gmail.com>
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
#ifndef DLG_LAYERSIZE
#define DLG_LAYERSIZE

#include <KoDialog.h>

#include "ui_wdg_layersize.h"

class WdgLayerSize : public QWidget, public Ui::WdgLayerSize
{
    Q_OBJECT

public:
    WdgLayerSize(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class KisFilterStrategy;

class DlgLayerSize: public KoDialog
{

    Q_OBJECT

public:

    DlgLayerSize(QWidget * parent, const char* name,
                 int width, int height, double resolution);
    ~DlgLayerSize();

    qint32 width();
    qint32 height();

    KisFilterStrategy *filterType();

private Q_SLOTS:
    void slotWidthChanged(int w);
    void slotHeightChanged(int h);
    void slotWidthChanged(double w);
    void slotHeightChanged(double h);
    void slotWidthUnitChanged(int index);
    void slotHeightUnitChanged(int index);
    void slotAspectChanged(bool keep);

private:
    void updateWidthUIValue(double value);
    void updateHeightUIValue(double value);

    WdgLayerSize * m_page;
    const double m_aspectRatio;
    const int m_originalWidth, m_originalHeight;
    int m_width, m_height;
    const double m_resolution;
    bool m_keepAspect;
};

#endif // DLG_IMAGESIZE
