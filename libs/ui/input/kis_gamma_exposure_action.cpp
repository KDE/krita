/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_gamma_exposure_action.h"

#include <QApplication>
#include <QIcon>

#include <klocalizedstring.h>
#include <kis_canvas2.h>
#include "kis_cursor.h"
#include "KisViewManager.h"
#include "kis_input_manager.h"
#include "krita_utils.h"
#include "kis_exposure_gamma_correction_interface.h"


class KisGammaExposureAction::Private
{
public:
    Private(KisGammaExposureAction *qq) : q(qq), baseExposure(0.0), baseGamma(0.0) {}

    KisGammaExposureAction *q;
    Shortcuts mode;

    qreal baseExposure;
    qreal baseGamma;

    void addExposure(qreal diff);
    void addGamma(qreal diff);
};


void KisGammaExposureAction::Private::addExposure(qreal diff)
{
    KisExposureGammaCorrectionInterface *interface =
        q->inputManager()->canvas()->exposureGammaCorrectionInterface();

    if (!interface->canChangeExposureAndGamma()) return;

    interface->setCurrentExposure(interface->currentExposure() + diff);
}

void KisGammaExposureAction::Private::addGamma(qreal diff)
{
    KisExposureGammaCorrectionInterface *interface =
        q->inputManager()->canvas()->exposureGammaCorrectionInterface();

    if (!interface->canChangeExposureAndGamma()) return;

    interface->setCurrentGamma(interface->currentGamma() + diff);
}

KisGammaExposureAction::KisGammaExposureAction()
    : KisAbstractInputAction("Exposure or Gamma")
    , d(new Private(this))
{
    setName(i18n("Exposure and Gamma"));
    setDescription(i18n("The <i>Exposure and Gamma</i> action changes the display mode of the canvas."));

    QHash< QString, int > shortcuts;
    shortcuts.insert(i18n("Exposure Mode"), ExposureShortcut);
    shortcuts.insert(i18n("Gamma Mode"), GammaShortcut);

    shortcuts.insert(i18n("Exposure +0.5"), AddExposure05Shortcut);
    shortcuts.insert(i18n("Exposure -0.5"), RemoveExposure05Shortcut);
    shortcuts.insert(i18n("Gamma +0.5"), AddGamma05Shortcut);
    shortcuts.insert(i18n("Gamma -0.5"), RemoveGamma05Shortcut);

    shortcuts.insert(i18n("Exposure +0.2"), AddExposure02Shortcut);
    shortcuts.insert(i18n("Exposure -0.2"), RemoveExposure02Shortcut);
    shortcuts.insert(i18n("Gamma +0.2"), AddGamma02Shortcut);
    shortcuts.insert(i18n("Gamma -0.2"), RemoveGamma02Shortcut);

    shortcuts.insert(i18n("Reset Exposure and Gamma"), ResetExposureAndGammaShortcut);
    setShortcutIndexes(shortcuts);
}

KisGammaExposureAction::~KisGammaExposureAction()
{
    delete d;
}

int KisGammaExposureAction::priority() const
{
    return 5;
}

void KisGammaExposureAction::activate(int shortcut)
{
    if (shortcut == ExposureShortcut) {
        QApplication::setOverrideCursor(KisCursor::changeExposureCursor());
    } else /* if (shortcut == GammaShortcut) */ {
        QApplication::setOverrideCursor(KisCursor::changeGammaCursor());
    }
}

void KisGammaExposureAction::deactivate(int shortcut)
{
    Q_UNUSED(shortcut);
    QApplication::restoreOverrideCursor();
}

void KisGammaExposureAction::begin(int shortcut, QEvent *event)
{
    KisAbstractInputAction::begin(shortcut, event);

    KisExposureGammaCorrectionInterface *interface =
        inputManager()->canvas()->exposureGammaCorrectionInterface();

    switch(shortcut) {
    case ExposureShortcut:
        d->baseExposure = interface->currentExposure();
        d->mode = (Shortcuts)shortcut;
        break;
    case GammaShortcut:
        d->baseGamma = interface->currentGamma();
        d->mode = (Shortcuts)shortcut;
        break;

    case AddExposure05Shortcut:
        d->addExposure(0.5);
        break;
    case RemoveExposure05Shortcut:
        d->addExposure(-0.5);
        break;
    case AddGamma05Shortcut:
        d->addGamma(0.5);
        break;
    case RemoveGamma05Shortcut:
        d->addGamma(-0.5);
        break;

    case AddExposure02Shortcut:
        d->addExposure(0.2);
        break;
    case RemoveExposure02Shortcut:
        d->addExposure(-0.2);
        break;
    case AddGamma02Shortcut:
        d->addGamma(0.2);
        break;
    case RemoveGamma02Shortcut:
        d->addGamma(-0.2);
        break;

    case ResetExposureAndGammaShortcut: {
        KisExposureGammaCorrectionInterface *interface =
            inputManager()->canvas()->exposureGammaCorrectionInterface();
        if (!interface->canChangeExposureAndGamma()) break;

        interface->setCurrentGamma(1.0);
        interface->setCurrentExposure(0.0);
        break;
    }
    }
}

void KisGammaExposureAction::cursorMovedAbsolute(const QPointF &startPos, const QPointF &pos)
{
    QPointF diff = -(pos - startPos);

    const int step = 200;

    KisExposureGammaCorrectionInterface *interface =
        inputManager()->canvas()->exposureGammaCorrectionInterface();

    if (!interface->canChangeExposureAndGamma()) return;


    if (d->mode == ExposureShortcut) {
        const qreal currentExposure = d->baseExposure + qreal(diff.y()) / step;
        interface->setCurrentExposure(currentExposure);
    } else if (d->mode == GammaShortcut) {
        const qreal currentGamma = d->baseExposure + qreal(diff.y()) / step;
        interface->setCurrentGamma(currentGamma);
    }
}

bool KisGammaExposureAction::isShortcutRequired(int shortcut) const
{
    Q_UNUSED(shortcut);
    return false;
}
