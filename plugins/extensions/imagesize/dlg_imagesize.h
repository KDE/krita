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

#include <KoDialog.h>

class KisFilterStrategy;
class WdgImageSize;
class KisDocumentAwareSpinBoxUnitManager;
class KisSpinBoxUnitManager;
class KisAspectRatioLocker;

#include "ui_wdg_imagesize.h"

class WdgImageSize : public QWidget, public Ui::WdgImageSize
{
    Q_OBJECT

public:
    WdgImageSize(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class DlgImageSize: public KoDialog
{

    Q_OBJECT

public:

    static const QString PARAM_PREFIX;
    static const QString PARAM_IMSIZE_UNIT;
    static const QString PARAM_SIZE_UNIT;
    static const QString PARAM_RES_UNIT;
    static const QString PARAM_RATIO_LOCK;
    static const QString PARAM_PRINT_SIZE_SEPARATE;

    DlgImageSize(QWidget * parent, int width, int height, double resolution);
    ~DlgImageSize() override;

    qint32 width();
    qint32 height();
    double resolution();

    KisFilterStrategy *filterType();

private Q_SLOTS:
    void slotSyncPrintToPixelSize();
    void slotSyncPixelToPrintSize();
    void slotPrintResolutionChanged();
    void slotPrintResolutionUnitChanged();

    void slotLockPixelRatioSwitched(bool value);
    void slotLockPrintRatioSwitched(bool value);
    void slotLockAllRatioSwitched(bool value);
    void slotAdjustSeparatelySwitched(bool value);

private:
    qreal currentResolutionPPI() const;
    void setCurrentResolutionPPI(qreal value);

    void updatePrintSizeMaximum();

    WdgImageSize *m_page;

    KisAspectRatioLocker *m_pixelSizeLocker;
    KisAspectRatioLocker *m_printSizeLocker;

    KisDocumentAwareSpinBoxUnitManager* m_widthUnitManager;
    KisDocumentAwareSpinBoxUnitManager* m_heightUnitManager;
    KisSpinBoxUnitManager* m_printSizeUnitManager;
};

#endif // DLG_IMAGESIZE
