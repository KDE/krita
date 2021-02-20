/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ROUND_HUD_BUTTON_H
#define __KIS_ROUND_HUD_BUTTON_H

#include <QScopedPointer>
#include <QAbstractButton>



class KisRoundHudButton : public QAbstractButton
{
public:
    KisRoundHudButton(QWidget *parent);
    ~KisRoundHudButton() override;

    void setOnOffIcons(const QIcon &on, const QIcon &off);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool hitButton(const QPoint &pos) const override;

    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_ROUND_HUD_BUTTON_H */
