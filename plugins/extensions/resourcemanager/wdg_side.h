/*
 *  SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 */
#ifndef WDG_SIDE_H
#define WDG_SIDE_H

#include <QWidget>

namespace Ui {
class WdgSide;
}

class WdgSide : public QWidget
{
    Q_OBJECT

public:
    explicit WdgSide(QWidget *parent = nullptr);
    ~WdgSide();

private:
    Ui::WdgSide *m_ui;

public Q_SLOTS:
    void focusLabel(int id);
};

#endif // WDG_SIDE_H
