/*
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
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

#ifndef KISTRANSPORTCONTROLS_H
#define KISTRANSPORTCONTROLS_H

#include <QWidget>

#include "kritaui_export.h"

class KRITAUI_EXPORT KisTransportControls : public QWidget
{
    Q_OBJECT

public:
    KisTransportControls(QWidget* parent = nullptr);
    ~KisTransportControls();

    QSize sizeHint() const override;

public Q_SLOTS:
    void setPlaying(bool playing);

Q_SIGNALS:
    void playPause();
    void stop();
    void forward();
    void back();

private:
    class QPushButton* buttonBack;
    class QPushButton* buttonStop;
    class QPushButton* buttonPlayPause;
    class QPushButton* buttonForward;
};

#endif // KISTRANSPORTCONTROLS_H
