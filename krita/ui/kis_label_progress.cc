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
#include <qeventloop.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QEvent>
#include <QProgressBar>

#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>

#include "kis_progress_subject.h"
#include "kis_label_progress.h"
#include "kis_cursor.h"

class EscapeButton : public QToolButton {

public:

    EscapeButton(QWidget * parent, const char * name) : QToolButton(parent, name) {};

    void keyReleaseEvent(QKeyEvent *e)
    {
        if (e->key()==Qt::Key_Escape)
            emit clicked();
    }
};

KisLabelProgress::KisLabelProgress(QWidget *parent, const char *name, Qt::WFlags f) : super(parent, name, f)
{
    m_subject = 0;
    m_modal = false;

    Q3HBoxLayout *box = new Q3HBoxLayout(this);
    box->setAutoAdd(true);

    QIcon cancelIconSet = SmallIconSet("stop");

    m_cancelButton = new EscapeButton(this, "cancel_button");
    m_cancelButton->setIconSet(cancelIconSet);
    m_cancelButton->setToolTip( i18n("Cancel"));
    connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(cancelPressed()));

    m_bar = new QProgressBar(this);
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

        connect(subject, SIGNAL(notifyProgress(int)), this, SLOT(update(int)));
        connect(subject, SIGNAL(notifyProgressStage(const QString&, int)), this, SLOT(updateStage(const QString&, int)));
        connect(subject, SIGNAL(notifyProgressDone()), this, SLOT(done()));
        connect(subject, SIGNAL(notifyProgressError()), this, SLOT(error()));
        connect(subject, SIGNAL(destroyed()), this, SLOT(subjectDestroyed()));

        show();

        if (canCancel) {
            if (modal) {
                kDebug() << "grabbing 1\n";
                m_cancelButton->grabMouse();
                m_cancelButton->grabKeyboard();
            }
        }
        else {
            m_cancelButton->hide();

            if (modal) {
                // Only visible widgets can grab.
                kDebug() << "grabbing 2\n";
                grabMouse();
                grabKeyboard();
            }
        }

        if (modal) {
            QApplication::setOverrideCursor(KisCursor::waitCursor());
        }

        m_bar->setValue(0);
    }
}

bool KisLabelProgress::event(QEvent * e)
{

    if (!e) return false;

    int type = e->type();

    switch (type) {
        case(KisProgress::ProgressEventBase + 1):
            {
                KisProgress::UpdateEvent * ue = dynamic_cast<KisProgress::UpdateEvent*>(e);
                update(ue->m_percent);
                break;
            }
        case(KisProgress::ProgressEventBase + 2):
            {
                KisProgress::UpdateStageEvent * use = dynamic_cast<KisProgress::UpdateStageEvent*>(e);
                updateStage(use->m_stage, use->m_percent);
                break;
            }
        case(KisProgress::ProgressEventBase + 3):
            done();
            break;
        case(KisProgress::ProgressEventBase + 4):
            error();
            break;
        case(KisProgress::ProgressEventBase + 5):
            subjectDestroyed();
            break;
        default:
            return QLabel::event(e);
    };

    return true;
}

void KisLabelProgress::reset()
{
    if (m_subject) {
        m_subject->disconnect(this);
        m_subject = 0;

        if (m_modal) {
            QApplication::restoreOverrideCursor();
        }

        m_modal = false;
    }

    releaseMouse();
    releaseKeyboard();
    m_cancelButton->releaseMouse();
    m_cancelButton->releaseKeyboard();
    hide();
}

void KisLabelProgress::update(int percent)
{
    m_bar->setValue(percent);

    KApplication *app = KApplication::kApplication();

    app->processEvents();
    // The following is safer, but makes cancel impossible:
    //QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput |
    //                                         QEventLoop::ExcludeSocketNotifiers);
}

void KisLabelProgress::updateStage(const QString&, int percent)
{
    m_bar->setValue(percent);

    KApplication *app = KApplication::kApplication();
    Q_ASSERT(app);

    app->processEvents();
}

void KisLabelProgress::cancelPressed()
{
    if (m_subject) {
        m_subject->cancel();
        reset();
    }
}

void KisLabelProgress::subjectDestroyed()
{
    reset();
}

void KisLabelProgress::done()
{
    reset();
}

void KisLabelProgress::error()
{
    reset();
}

#include "kis_label_progress.moc"

