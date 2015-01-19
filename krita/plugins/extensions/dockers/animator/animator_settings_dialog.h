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

#ifndef ANIMATORSETTINGSDIALOG_H
#define ANIMATORSETTINGSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QSpinBox>

#include "kis_animation.h"

/**
 * The animator settings dialog
 */
class AnimatorSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AnimatorSettingsDialog(QWidget *parent = 0);
    void setModel(KisAnimation* model);

private slots:
    void okClicked();
    void cancelClicked();

private:
    KisAnimation* m_model;
    QCheckBox* m_autoFrameBreak;
    QSpinBox* m_timelineWidth;

signals:
    void sigTimelineWithChanged(int);
};

#endif // ANIMATORSETTINGSDIALOG_H
