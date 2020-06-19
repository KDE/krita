/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KISCOLORSOURCETOGGLE_H
#define KISCOLORSOURCETOGGLE_H

#include <QAbstractButton>
#include <QScopedPointer>


class KisColorSourceToggle : public QAbstractButton
{
    Q_OBJECT
public:
    explicit KisColorSourceToggle(QWidget *parent = nullptr);
    virtual ~KisColorSourceToggle() override;

    void setForegroundColor(const QColor &color);
    void setBackgroundColor(const QColor &color);
protected:
    void paintEvent(QPaintEvent *e) override;
    QSize sizeHint() const override;

private:
    class Private;
    QScopedPointer<Private> m_d;
};

#endif // KISCOLORSOURCETOGGLE_H
