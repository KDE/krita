/*
 *  Copyright (c) 2014 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef ANIMATOR_PLAYBACK_DIALOG_H
#define ANIMATOR_PLAYBACK_DIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>

#include "kis_animation.h"

/**
 * The animator playback options dialog
 */
class AnimatorPlaybackDialog : public QDialog
{
    Q_OBJECT
public:
    AnimatorPlaybackDialog(QWidget* parent = 0);
    void setModel(KisAnimation* model);

private slots:
    void okClicked();
    void cancelClicked();

private:
    KisAnimation* m_model;
    QSpinBox* m_fpsInput;
    QSpinBox* m_localPlaybackRangeInput;
    QCheckBox* m_loopState;

signals:
    void playbackStateChanged();
};

#endif // ANIMATOR_PLAYBACK_DIALOG_H
