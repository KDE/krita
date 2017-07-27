/*
 *
 *  Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
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

#ifndef DLG_CANVASSIZE
#define DLG_CANVASSIZE

#include <KoDialog.h>
#include <QIcon>


#include "ui_wdg_canvassize.h"

class KisDocumentAwareSpinBoxUnitManager;

class WdgCanvasSize : public QWidget, public Ui::WdgCanvasSize
{
    Q_OBJECT

public:
    WdgCanvasSize(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};



class DlgCanvasSize: public KoDialog
{

    Q_OBJECT

public:
    enum anchor { NORTH_WEST = 0, NORTH, NORTH_EAST, WEST, CENTER, EAST, SOUTH_WEST, SOUTH, SOUTH_EAST, NONE};

    static const QString PARAM_PREFIX;
    static const QString PARAM_WIDTH_UNIT;
    static const QString PARAM_HEIGTH_UNIT;
    static const QString PARAM_XOFFSET_UNIT;
    static const QString PARAM_YOFFSET_UNIT;

    DlgCanvasSize(QWidget * parent, int width, int height, double resolution);
    ~DlgCanvasSize() override;

    qint32 width();
    qint32 height();
    qint32 xOffset();
    qint32 yOffset();

private Q_SLOTS:
    void slotAspectChanged(bool keep);
    void slotAnchorButtonClicked(int id);

    void slotWidthChanged(double v);
    void slotHeightChanged(double v);

    void slotXOffsetChanged(double v);
    void slotYOffsetChanged(double v);

    void slotCanvasPreviewXOffsetChanged(int v);
    void slotCanvasPreviewYOffsetChanged(int v);

private:

    void loadAnchorIcons();
    void updateAnchorIcons(int id);
    void updateButtons(int forceId);
    void updateOffset(int id);
    void expectedOffset(int id, double &xOffset, double &yOffset);

    bool m_keepAspect;
    const double m_aspectRatio;
    const double m_resolution;
    const int m_originalWidth, m_originalHeight;
    int m_newWidth, m_newHeight;
    int m_xOffset, m_yOffset;

    WdgCanvasSize * m_page;
    QIcon m_anchorIcons[9];
    QButtonGroup *m_group;

    KisDocumentAwareSpinBoxUnitManager* _widthUnitManager;
    KisDocumentAwareSpinBoxUnitManager* _heightUnitManager;

    KisDocumentAwareSpinBoxUnitManager* _xOffsetUnitManager;
    KisDocumentAwareSpinBoxUnitManager* _yOffsetUnitManager;
};


#endif // DLG_CANVASSIZE
