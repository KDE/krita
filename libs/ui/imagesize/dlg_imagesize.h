/*
 *  dlg_imagesize.h -- part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DLG_IMAGESIZE
#define DLG_IMAGESIZE

#include <KoDialog.h>

class KisFilterStrategy;
class WdgImageSize;

class DlgImageSize: public KoDialog
{

    Q_OBJECT

public:

    DlgImageSize(QWidget * parent, int width, int height, double resolution);
    ~DlgImageSize() override;

    qint32 desiredWidth();
    qint32 desiredHeight();
    double desiredResolution();

    KisFilterStrategy *filterType();

private:
    QScopedPointer<WdgImageSize> m_page;
};

#endif // DLG_IMAGESIZE
