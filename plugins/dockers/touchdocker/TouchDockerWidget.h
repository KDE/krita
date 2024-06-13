/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TouchDockerWIDGET_H
#define TouchDockerWIDGET_H

#include <QWidget>

class KisCanvas2;

namespace Ui {
class TouchDockerWidget;
}

class TouchDockerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TouchDockerWidget(QWidget *parent = nullptr);
    ~TouchDockerWidget();

    void setCanvas(KisCanvas2 *canvas);
    void unsetCanvas();

private:
    Ui::TouchDockerWidget *ui;
};

#endif // TouchDockerWIDGET_H
