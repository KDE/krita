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

#include <qlabel.h>
#include <qevent.h>

#include "kis_progress_display_interface.h"

class QToolButton;
class KProgress;

class KisLabelProgress : public QLabel, public KisProgressDisplayInterface {
    Q_OBJECT
    typedef QLabel super;

public:
    KisLabelProgress(QWidget *parent, const char *name = 0, WFlags f = 0);
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
    void cancelPressed();

private:
    void reset();

    KisProgressSubject *m_subject;
    KProgress *m_bar;
    QToolButton *m_cancelButton;
    bool m_modal;
};

#endif // KIS_LABEL_PROGRESS_H_

