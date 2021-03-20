/*
 *  SPDX-FileCopyrightText: 2020 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
