/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *                2004 Adrian Page     <adrian@pagenet.plus.com>
 *                2009 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KISPROGRESSWIDGET_H
#define KISPROGRESSWIDGET_H

#include <QWidget>
#include <QList>

#include <kis_progress_updater.h>

#include "krita_export.h"

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
    KisProgressWidget(QWidget* parent);
    virtual ~KisProgressWidget();

public:

    /**
     * create a new KoProgressUpdater instance that is
     * linked to this progress bar.
     *
     * Note: it is _your_ duty to call deleteLater on the
     * koprogressupdater when you are done!
     */
    KoProgressUpdater* createUpdater();
    void detachUpdater(KoProgressUpdater* updater);
    void attachUpdater(KoProgressUpdater* updater);

public slots:

    void cancel();


private:

    QToolButton* m_cancelButton;
    KoProgressBar* m_progressBar;
    QList<KoProgressUpdater*> m_activeUpdaters;

};

#endif // KISPROGRESSWIDGET_H
