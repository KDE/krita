/*
 *  dlg_imagesize.h -- part of KimageShop^WKrayon^WKrita
 *
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
#ifndef DLG_IMAGESIZE
#define DLG_IMAGESIZE

#include <kdialog.h>

class KisFilterStrategy;
class WdgImageSize;
class QButtonGroup;

#include "ui_wdg_imagescale.h"

class WdgImageSize : public QWidget, public Ui::WdgImageScale
{
    Q_OBJECT

public:
    WdgImageSize(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 * This dialog allows the user to create a selection mask based
 * on a (range of) colors.
 */
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

private slots:

    void okClicked();
    void slotWidthPixelsChanged(int w);
    void slotHeightPixelsChanged(int h);
    void slotWidthPercentageChanged(double w);
    void slotHeightPercentageChanged(double h);
    void slotWidthPixelUnitChanged(int index);
    void slotHeightPixelUnitChanged(int index);
    void slotWidthPhysicalChanged(double w);
    void slotHeightPhysicalChanged(double h);
    void slotWidthUnitChanged(int index);
    void slotHeightUnitChanged(int index);
    void slotProtectChanged();
    void slotAspectChanged(bool keep);
    void slotResolutionChanged(double r);

private:

    void blockAll();
    void unblockAll();

    WdgImageSize * m_page;
    double m_origW, m_origH;
    double m_width, m_height;  // in points
    QButtonGroup *m_buttonGroup;
    double m_aspectRatio;
};

#endif // DLG_IMAGESIZE
