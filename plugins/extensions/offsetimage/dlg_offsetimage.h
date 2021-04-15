/*
 * SPDX-FileCopyrightText: 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DLG_OFFSETIMAGE
#define DLG_OFFSETIMAGE

#include <KoDialog.h>
#include <kis_global.h>
#include <QDebug>
#include "ui_wdg_offsetimage.h"

class KisDocumentAwareSpinBoxUnitManager;

class WdgOffsetImage : public QWidget, public Ui::WdgOffsetImage
{
    Q_OBJECT

public:
    WdgOffsetImage(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class DlgOffsetImage: public KoDialog
{

    Q_OBJECT

public:

    static const QString PARAM_PREFIX;
    static const QString PARAM_XOFFSET_UNIT;
    static const QString PARAM_YOFFSET_UNIT;

    DlgOffsetImage(QWidget * parent = 0, const char* name = 0, QSize imageSize = QSize());
    ~DlgOffsetImage() override;

    int offsetX() const { return m_offsetX; }
    int offsetY() const { return m_offsetY; }


private Q_SLOTS:
    void okClicked();
    void slotOffsetXChanged(double);
    void slotOffsetYChanged(double);
    void slotMiddleOffset();

private:
    WdgOffsetImage * m_page {0};
    int m_offsetX {0};
    int m_offsetY {0};
    bool m_lock {false};
    QSize m_offsetSize;

    KisDocumentAwareSpinBoxUnitManager* _widthUnitManager {0};
    KisDocumentAwareSpinBoxUnitManager* _heightUnitManager {0};

};

#endif // DLG_OFFSETIMAGE
