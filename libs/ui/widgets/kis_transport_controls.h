/*
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISTRANSPORTCONTROLS_H
#define KISTRANSPORTCONTROLS_H

#include "kritaui_export.h"

#include <QWidget>
class QPushButton;

class KRITAUI_EXPORT KisTransportControls : public QWidget
{
    Q_OBJECT

public:
    KisTransportControls(QWidget* parent = nullptr);
    ~KisTransportControls();

    QSize sizeHint() const override;

public Q_SLOTS:
    void setPlaying(bool playing);
    void showSeekButtons(bool show);
    void showSkipButtons(bool show);

Q_SIGNALS:
    void skipBack();
    void back();
    void stop();
    void playPause();
    void forward();
    void skipForward();

private:
    QPushButton* buttonSkipBack;
    QPushButton* buttonBack;
    QPushButton* buttonStop;
    QPushButton* buttonPlayPause;
    QPushButton* buttonForward;
    QPushButton* buttonSkipForward;
};

#endif // KISTRANSPORTCONTROLS_H
