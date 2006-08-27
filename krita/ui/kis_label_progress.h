/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *                2004 Adrian Page <adrian@pagenet.plus.com>
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
#ifndef KIS_LABEL_PROGRESS_H_
#define KIS_LABEL_PROGRESS_H_

#include <QWidget>
#include <QEvent>

#include "kis_progress_display_interface.h"

class QToolButton;
class QProgressBar;
class QTimer;

class KisLabelProgress : public QWidget, public KisProgressDisplayInterface {
    Q_OBJECT
    typedef QWidget super;

public:
    KisLabelProgress(QWidget *parent, const char *name = 0);
    virtual ~KisLabelProgress();

public:
    // Implements KisProgressDisplayInterface
    void setSubject(KisProgressSubject *subject, bool modal, bool canCancel);

    // Overrides QLabel::event()
    bool event(QEvent * ev);

private slots:
    virtual void update(int percent);
    virtual void updateStage(const QString& stage, int percent);
    virtual void done();
    virtual void error();
    virtual void subjectDestroyed();

private slots:
    void updateTimeout();
    void cancelPressed();

private:
    void reset();

    KisProgressSubject *m_subject;
    QProgressBar *m_bar;
    QToolButton *m_cancelButton;
    bool m_modal;
    QTimer* m_timer;
};

#endif // KIS_LABEL_PROGRESS_H_

