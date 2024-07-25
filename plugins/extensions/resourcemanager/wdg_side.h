/*
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef WDG_SIDE_H
#define WDG_SIDE_H

#include <QWidget>
#include <KoResourceBundle.h>

namespace Ui {
class WdgSide;
}

class WdgSide : public QWidget
{
    Q_OBJECT

public:
    explicit WdgSide(KoResourceBundleSP bundle, QWidget *parent = nullptr);
    ~WdgSide();

private:
    Ui::WdgSide *m_ui;

public Q_SLOTS:
    void focusLabel(int id);

private:
    KoResourceBundleSP m_bundle;
};

#endif // WDG_SIDE_H
