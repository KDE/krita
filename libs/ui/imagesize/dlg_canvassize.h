/*
 *
 *  SPDX-FileCopyrightText: 2009 Edward Apap <schumifer@hotmail.com>
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    static const QString PARAM_HEIGHT_UNIT;
    static const QString PARAM_XOFFSET_UNIT;
    static const QString PARAM_YOFFSET_UNIT;

    DlgCanvasSize(QWidget * parent, int width, int height, double resolution);
    ~DlgCanvasSize() override;

    qint32 width();
    qint32 height();
    qint32 xOffset();
    qint32 yOffset();
    WdgCanvasSize * m_page;

private Q_SLOTS:
    void slotAspectChanged(bool keep);
    void slotAnchorButtonClicked(int id);

    void slotWidthChanged(double v);
    void slotHeightChanged(double v);

    void slotXOffsetChanged(double v);
    void slotYOffsetChanged(double v);

    void slotCanvasPreviewXOffsetChanged(int v);
    void slotCanvasPreviewYOffsetChanged(int v);

    void updatexOffsetIcon(bool v);
    void updateyOffsetIcon(bool v);

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

    QIcon m_anchorIcons[9];
    QButtonGroup *m_group;

    KisDocumentAwareSpinBoxUnitManager* _widthUnitManager;
    KisDocumentAwareSpinBoxUnitManager* _heightUnitManager;

    KisDocumentAwareSpinBoxUnitManager* _xOffsetUnitManager;
    KisDocumentAwareSpinBoxUnitManager* _yOffsetUnitManager;
};


#endif // DLG_CANVASSIZE
