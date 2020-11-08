/*
 *  dlg_layersize.h -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DLG_LAYERSIZE
#define DLG_LAYERSIZE

#include <KoDialog.h>

#include "ui_wdg_layersize.h"

class KisDocumentAwareSpinBoxUnitManager;

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

    static const QString PARAM_PREFIX;
    static const QString PARAM_WIDTH_UNIT;
    static const QString PARAM_HEIGHT_UNIT;
    static const QString PARAM_KEEP_AR;
    static const QString PARAM_KEEP_PROP;

    DlgLayerSize(QWidget * parent, const char* name,
                 int width, int height, double resolution);
    ~DlgLayerSize() override;

    qint32 width();
    qint32 height();

    KisFilterStrategy *filterType();

private Q_SLOTS:
    void slotWidthChanged(double w);
    void slotHeightChanged(double h);
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

    KisDocumentAwareSpinBoxUnitManager* _widthUnitManager;
    KisDocumentAwareSpinBoxUnitManager* _heightUnitManager;
};

#endif // DLG_IMAGESIZE
