    /*
 *  KisColorSpaceConversionDialog.h -- part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCOLORSPACECONVERSIONDIALOG_H
#define KISCOLORSPACECONVERSIONDIALOG_H


#include <QButtonGroup>

#include <KoDialog.h>

#include <KoID.h>
#include <KoColorConversionTransformation.h>
#include "kis_types.h"

#include "kritaui_export.h"

#include "ui_wdgconvertcolorspace.h"

class KoColorSpace;

class KRITAUI_EXPORT WdgConvertColorSpace : public QWidget, public Ui::WdgConvertColorSpace
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
class KRITAUI_EXPORT KisColorSpaceConversionDialog : public KoDialog
{

    Q_OBJECT

public:

    KisColorSpaceConversionDialog(QWidget * parent = 0, const char* name = 0);
    ~KisColorSpaceConversionDialog() override;

    void setInitialColorSpace(const KoColorSpace *cs, KisImageSP entireImage);

    const KoColorSpace *colorSpace() const;
    KoColorConversionTransformation::Intent conversionIntent() const;
    KoColorConversionTransformation::ConversionFlags conversionFlags() const;

    WdgConvertColorSpace * m_page;

    QButtonGroup m_intentButtonGroup;

public Q_SLOTS:
    void selectionChanged(bool);
    void okClicked();
    void slotColorSpaceChanged(const KoColorSpace *cs);

private:
    KisImageSP m_image;
};

#endif // KISCOLORSPACECONVERSIONDIALOG_H
