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


/**
 * @brief The KisTransportControls class provides a simple, reusable widget
 * for common transport controls, including play/pause, stop, seek and skip.
 * The client code will want to add this widget, configure it, and hook into
 * the appropriate signals.
 */
class KRITAUI_EXPORT KisTransportControls : public QWidget
{
    Q_OBJECT

public:
    KisTransportControls(QWidget* parent = nullptr);
    ~KisTransportControls();

    QSize sizeHint() const override;

public Q_SLOTS:
    /**
     * @brief setPlaying flips the icon on the play/pause button.
     * When playing, the button will show a pause icon.
     * When paused, the button will show a play icon.
     */
    void setPlaying(bool playing);

    void showStateButtons(bool show);
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
