/*
 *  dlg_imagesize.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef DLG_IMAGESIZE
#define DLG_IMAGESIZE

#include <kdialog.h>

class KisFilterStrategy;
class WdgImageSize;

#include "ui_wdg_imagesize.h"

class WdgImageSize : public QWidget, public Ui::WdgImageSize
{
    Q_OBJECT

public:
    WdgImageSize(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class DlgImageSize: public KDialog
{

    Q_OBJECT

public:
    DlgImageSize(QWidget * parent, int width, int height, double resolution);
    ~DlgImageSize();

    qint32 width();
    qint32 height();
    double resolution();

    KisFilterStrategy *filterType();

private Q_SLOTS:
    void slotPixelWidthChanged(int w);
    void slotPixelHeightChanged(int h);
    void slotPixelWidthChanged(double w);
    void slotPixelHeightChanged(double h);
    void slotPixelWidthUnitChanged();
    void slotPixelHeightUnitChanged();
    void slotPrintWidthChanged(double w);
    void slotPrintHeightChanged(double h);
    void slotPrintWidthUnitChanged();
    void slotPrintHeightUnitChanged();
    void slotAspectChanged(bool keep);
    void slotPrintResolutionChanged(double r);
    void slotPrintResolutionEditFinished();
    void slotPrintResolutionUnitChanged();

private:
    void updatePixelWidthUIValue(double value);
    void updatePixelHeightUIValue(double value);
    void updatePrintWidthUIValue(double value);
    void updatePrintHeightUIValue(double value);
    void updatePrintResolutionUIValue(double value);

    WdgImageSize *m_page;
    const double m_aspectRatio;
    const int m_originalWidth, m_originalHeight;
    int m_width, m_height;
    double m_printWidth, m_printHeight; // in points
    const double m_originalResolution;
    double m_resolution;
    bool m_keepAspect;
};

#endif // DLG_IMAGESIZE
