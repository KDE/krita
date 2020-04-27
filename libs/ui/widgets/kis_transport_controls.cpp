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

#include "kis_transport_controls.h"

#include "kis_icon.h"

#include <QHBoxLayout>
#include <QToolButton>

KisTransportControls::KisTransportControls(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(parent);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    {
        //KisIconUtils::loadIcon("prevframe")
        btnBack = new QToolButton(this);
        layout->addWidget(btnBack);

        //KisIconUtils::loadIcon("animation_stop")
        btnStop = new QToolButton(this);
        layout->addWidget(btnStop);

        //KisIconUtils::loadIcon("animation_play")
        btnPlayPause = new QToolButton(this);
        layout->addWidget(btnPlayPause);

        //KisIconUtils::loadIcon("nextframe")
        btnForward = new QToolButton(this);
        layout->addWidget(btnForward);
    }

    setLayout(layout);
}

KisTransportControls::~KisTransportControls()
{
}

QSize KisTransportControls::sizeHint() const
{
    return QSize(32, 32);
}
