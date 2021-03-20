/*
 *  dlg_rotateimage.h -- part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef DLG_ROTATEIMAGE
#define DLG_ROTATEIMAGE

#include <KoDialog.h>

#include "kis_global.h"

#include "ui_wdg_rotateimage.h"

enum enumRotationDirection {
    CLOCKWISE,
    COUNTERCLOCKWISE
};

class WdgRotateImage : public QWidget, public Ui::WdgRotateImage
{
    Q_OBJECT

public:
    WdgRotateImage(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class DlgRotateImage: public KoDialog
{

    Q_OBJECT

public:

    DlgRotateImage(QWidget * parent = 0,
                   const char* name = 0);
    ~DlgRotateImage() override;

    void setAngle(quint32 w);
    double angle();

    void setDirection(enumRotationDirection direction);
    enumRotationDirection direction();

private Q_SLOTS:

    void okClicked();
    void resetPreview();
    void slotAngleValueChanged(double);
    void slotRadioCCWToggled(bool toggled);
    void slotRadioCWToggled(bool toggled);

private:

    WdgRotateImage * m_page;
    double m_oldAngle {0.0};
    bool m_lock;

};

#endif // DLG_ROTATEIMAGE
