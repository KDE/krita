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
#include <qlayout.h>
#include <qtooltip.h>
#include <qtoolbutton.h>
#include <qcursor.h>

#include <kapplication.h>
#include <klocale.h>
#include <kprogress.h>
#include <kiconloader.h>

#include "kis_progress_subject.h"
#include "kis_label_progress.h"
#include "kis_cursor.h"

KisLabelProgress::KisLabelProgress(QWidget *parent, const char *name, WFlags f) : super(parent, name, f)
{
    m_subject = 0;
    m_modal = false;

    QHBoxLayout *box = new QHBoxLayout(this);
    box -> setAutoAdd(true);

    QIconSet cancelIconSet = SmallIconSet("stop");

    m_cancelButton = new QToolButton(this, "cancel_button");
    m_cancelButton -> setIconSet(cancelIconSet);
    QToolTip::add(m_cancelButton, i18n("Cancel"));
    connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(cancelPressed()));

    m_bar = new KProgress(100, this);
}

KisLabelProgress::~KisLabelProgress()
{
}

void KisLabelProgress::setSubject(KisProgressSubject *subject, bool modal, bool canCancel)
{
    reset();

    if (subject) {
        m_subject = subject;
        m_modal = modal;

        connect(subject, SIGNAL(notifyProgress(KisProgressSubject*, int)), this, SLOT(update(KisProgressSubject*, int)));
        connect(subject, SIGNAL(notifyProgressStage(KisProgressSubject*, const QString&, int)), this, SLOT(updateStage(KisProgressSubject*, const QString&, int)));
        connect(subject, SIGNAL(notifyProgressDone(KisProgressSubject*)), this, SLOT(done(KisProgressSubject*)));
        connect(subject, SIGNAL(notifyProgressError(KisProgressSubject*)), this, SLOT(error(KisProgressSubject*)));
        connect(subject, SIGNAL(destroyed()), this, SLOT(subjectDestroyed()));

        show();

        if (canCancel) {
            if (modal) {
                m_cancelButton -> grabMouse();
                m_cancelButton -> grabKeyboard();
            }
        }
        else {
            m_cancelButton -> hide();

            if (modal) {
                // Only visible widgets can grab.
                grabMouse();
                grabKeyboard();
            }
        }

        if (modal) {
            QApplication::setOverrideCursor(KisCursor::waitCursor());
        }

        m_bar -> setValue(0);
    }
}

void KisLabelProgress::reset()
{
    if (m_subject) {
        m_subject -> disconnect(this);
        m_subject = 0;

        if (m_modal) {
            QApplication::restoreOverrideCursor();
        }

        m_modal = false;
    }

    releaseMouse();
    releaseKeyboard();
    m_cancelButton -> releaseMouse();
    m_cancelButton -> releaseKeyboard();
    hide();
}

void KisLabelProgress::update(KisProgressSubject *, int percent)
{
    m_bar -> setValue(percent);

    KApplication *app = KApplication::kApplication();
    Q_ASSERT(app);

    app -> processEvents();
}

void KisLabelProgress::updateStage(KisProgressSubject *, const QString&, int percent)
{
    m_bar -> setValue(percent);

    KApplication *app = KApplication::kApplication();
    Q_ASSERT(app);

    app -> processEvents();
}

void KisLabelProgress::cancelPressed()
{
    if (m_subject) {
        m_subject -> cancel();
        reset();
    }
}

void KisLabelProgress::subjectDestroyed()
{
    reset();
}

void KisLabelProgress::done(KisProgressSubject *)
{
    reset();
}

void KisLabelProgress::error(KisProgressSubject *)
{
    reset();
}

#include "kis_label_progress.moc"

