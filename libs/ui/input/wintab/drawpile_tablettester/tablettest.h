/*
   Drawpile - a collaborative drawing program.

   SPDX-FileCopyrightText: 2017 Calle Laakkonen

   SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TABLETTEST_WIDGET_H
#define TABLETTEST_WIDGET_H

#include <QWidget>
#include <kis_speed_smoother.h>

class TabletTester : public QWidget {
    Q_OBJECT
public:
    TabletTester(QWidget *parent=nullptr);

public Q_SLOTS:
    void clear();

Q_SIGNALS:
    void eventReport(const QString &msg);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void tabletEvent(QTabletEvent *e) override;

    QSize sizeHint() const override;

private:
    QPolygon m_mousePath;
    QPolygon m_tabletPath;

    bool m_mouseDown;
    bool m_tabletDown;
    KisSpeedSmoother m_tabletSpeedSmoother;
    KisSpeedSmoother m_mouseSpeedSmoother;
};

#endif
