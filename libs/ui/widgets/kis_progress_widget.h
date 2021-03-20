/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISPROGRESSWIDGET_H
#define KISPROGRESSWIDGET_H

#include <QWidget>
#include <QList>

#include <kis_progress_updater.h>

#include "kritaui_export.h"

class KoProgressUpdater;
class QToolButton;
class KoProgressBar;

/**
 * KisProgressWidget combines a KoProgressBar with a button
 * that can be pressed to cancel the current action.
 */
class KRITAUI_EXPORT KisProgressWidget : public QWidget, public KisProgressInterface
{

    Q_OBJECT

public:
    KisProgressWidget(QWidget* parent = 0);
    ~KisProgressWidget() override;

public:
    KoProgressProxy* progressProxy();


    /**
     * create a new KoProgressUpdater instance that is
     * linked to this progress bar.
     *
     * Note: it is _your_ duty to call deleteLater on the
     * koprogressupdater when you are done!
     */
    void detachUpdater(KoProgressUpdater* updater) override;
    void attachUpdater(KoProgressUpdater* updater) override;

public Q_SLOTS:

    void cancel();
    void correctVisibility(int progressValue);

Q_SIGNALS:
    void sigCancellationRequested();

private:

    QToolButton* m_cancelButton;
    KoProgressBar* m_progressBar;
    QList<KoProgressUpdater*> m_activeUpdaters;

};

#endif // KISPROGRESSWIDGET_H
