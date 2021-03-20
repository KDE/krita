/*
 *  dlg_imagesize.h -- part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DLG_SEPARATE
#define DLG_SEPARATE

#include <KoDialog.h>
#include <kis_channel_separator.h>
#include "ui_wdg_separations.h"

class WdgSeparations : public QWidget, public Ui::WdgSeparations
{
public:
    WdgSeparations(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 * This dialog allows the user to configure the decomposition of an image
 * into layers: one layer for each color channel.
 */
class DlgSeparate: public KoDialog
{

    Q_OBJECT

public:

    DlgSeparate(const QString & imageCS, const QString & layerCS, QWidget * parent = 0,
                const char* name = 0);
    ~DlgSeparate() override;

public:

    enumSepAlphaOptions getAlphaOptions();
    enumSepSource getSource();

    bool getDownscale();
    void enableDownscale(bool enable);

    bool getToColor();
    bool getActivateCurrentChannel();


private Q_SLOTS:

    void slotSetColorSpaceLabel();
    void okClicked();
    void separateToColorActivated(bool disable);

private:

    WdgSeparations * m_page;
    QString m_imageCS;
    QString m_layerCS;
    bool m_canDownScale {true};

};

#endif // DLG_SEPARATE
