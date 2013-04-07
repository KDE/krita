/*
 *
 *  Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
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

#ifndef DLG_CANVASSIZE
#define DLG_CANVASSIZE

#include <math.h>

#include <kdialog.h>
#include <kicon.h>

class KisFilterStrategy;

#include "ui_wdg_canvassize.h"

class WdgCanvasSize : public QWidget, public Ui::WdgCanvasSize
{
    Q_OBJECT

public:
    WdgCanvasSize(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};



class DlgCanvasSize: public KDialog
{

    Q_OBJECT

public:
    enum anchor { NORTH_WEST = 0, NORTH, NORTH_EAST, WEST, CENTER, EAST, SOUTH_WEST, SOUTH, SOUTH_EAST, NONE};

    DlgCanvasSize(QWidget * parent, int width, int height);
    ~DlgCanvasSize();

    qint32 width();
    qint32 height();
    qint32 xOffset();
    qint32 yOffset();

protected slots:
    void slotAnchorButtonClicked(int id);
    void slotUpdateSizeTextBoxes();

    void slotAspectChanged(bool keep);
    void slotWidthChanged(int v);
    void slotHeightChanged(int v);
    void slotXOffsetChanged(int v);
    void slotYOffsetChanged(int v);

protected:
    void loadAnchorIcons();
    void updateAnchorIcons(int id);
    void updateOffset(int id);
    void expectedOffset(int id, int &xOffset, int &yOffset);
    void updateButtons(int forceId);

private:
    const int m_originalWidth, m_originalHeight;
    const double m_aspectRatio;
    bool m_keepAspect;
    int m_newWidth, m_newHeight;
    int m_xOffset, m_yOffset;
    KIcon m_anchorIcons[9];
    WdgCanvasSize * m_page;

    QButtonGroup *m_group;
};


#endif // DLG_CANVASSIZE
