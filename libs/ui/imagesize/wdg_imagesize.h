/*
 *  dlg_imagesize.h -- part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef WDG_IMAGESIZE
#define WDG_IMAGESIZE

#include <QWidget>

class KisFilterStrategy;
class PageImageSize;
class KisDocumentAwareSpinBoxUnitManager;
class KisSpinBoxUnitManager;
class KisAspectRatioLocker;

class WdgImageSize : public QWidget
{
    Q_OBJECT

public:

    static const QString PARAM_PREFIX;
    static const QString PARAM_IMSIZE_UNIT;
    static const QString PARAM_SIZE_UNIT;
    static const QString PARAM_RES_UNIT;
    static const QString PARAM_RATIO_LOCK;
    static const QString PARAM_PRINT_SIZE_SEPARATE;

    WdgImageSize(QWidget * parent, int width, int height, double resolution);
    ~WdgImageSize() override;

    qint32 desiredWidth();
    qint32 desiredHeight();
    double desiredResolution();

    KisFilterStrategy *filterType();

Q_SIGNALS:
    void sigDesiredSizeChanged(qint32 width, qint32 height, double resolution);

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

    PageImageSize *m_page;

    QSize m_originalSize;

    KisAspectRatioLocker *m_pixelSizeLocker;
    KisAspectRatioLocker *m_printSizeLocker;

    KisDocumentAwareSpinBoxUnitManager* m_widthUnitManager;
    KisDocumentAwareSpinBoxUnitManager* m_heightUnitManager;
    KisSpinBoxUnitManager* m_printSizeUnitManager;
};

#endif // WDG_IMAGESIZE
