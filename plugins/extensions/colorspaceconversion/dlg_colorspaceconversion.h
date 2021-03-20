    /*
 *  dlg_colorspaceconversion.h -- part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DLG_COLORSPACECONVERSION
#define DLG_COLORSPACECONVERSION

#include <QButtonGroup>

#include <KoDialog.h>

#include <KoID.h>
#include "kis_types.h"

#include "ui_wdgconvertcolorspace.h"

class KoColorSpace;

class WdgConvertColorSpace : public QWidget, public Ui::WdgConvertColorSpace
{
    Q_OBJECT

public:
    WdgConvertColorSpace(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 * XXX
 */
class DlgColorSpaceConversion: public KoDialog
{

    Q_OBJECT

public:

    DlgColorSpaceConversion(QWidget * parent = 0, const char* name = 0);
    ~DlgColorSpaceConversion() override;

    void setInitialColorSpace(const KoColorSpace *cs, KisImageSP entireImage);

    WdgConvertColorSpace * m_page;

    QButtonGroup m_intentButtonGroup;

public Q_SLOTS:
    void selectionChanged(bool);
    void okClicked();
    void slotColorSpaceChanged(const KoColorSpace *cs);

private:
    KisImageSP m_image;
};

#endif // DLG_COLORSPACECONVERSION
