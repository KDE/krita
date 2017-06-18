/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
