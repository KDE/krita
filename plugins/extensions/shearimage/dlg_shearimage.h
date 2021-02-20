/*
 *  dlg_shearimage.h -- part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DLG_SHEARIMAGE
#define DLG_SHEARIMAGE

#include <KoDialog.h>

#include "ui_wdg_shearimage.h"

class WdgShearImage : public QWidget, public Ui::WdgShearImage
{
    Q_OBJECT

public:
    WdgShearImage(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class DlgShearImage: public KoDialog
{

    Q_OBJECT

public:

    DlgShearImage(QWidget * parent = 0,
                  const char* name = 0);
    ~DlgShearImage() override;

    void setAngleX(quint32 w);
    void setAngleY(quint32 w);
    qint32 angleX();
    qint32 angleY();

private Q_SLOTS:

    void okClicked();

private:

    WdgShearImage * m_page;
    double m_oldAngle {0.0};
    bool m_lock;

};

#endif // DLG_SHEARIMAGE
