/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 Ariya Hidayat <ariya@kde.org>
    SPDX-FileCopyrightText: 2006 Peter Simonsson <peter.simonsson@gmail.com>
    SPDX-FileCopyrightText: 2006-2007 C. Boemann <cbo@boemann.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#include "KoZoomAction.h"
#include "KoZoomMode.h"
#include "KoZoomState.h"
#include "KoZoomWidget.h"

#include <KoIcon.h>

#include <QString>
#include <QLocale>
#include <QStringList>
#include <QList>
#include <QButtonGroup>

#include <klocalizedstring.h>
#include <kis_signal_compressor.h>
#include <KoZoomActionState.h>
#include <KisPortingUtils.h>

class Q_DECL_HIDDEN KoZoomAction::Private
{
public:

    Private(KoZoomAction *_parent)
        : parent(_parent)
        , guiUpdateCompressor(200, KisSignalCompressor::FIRST_ACTIVE)
        , currentActionState(currentZoomState)
    {}

    KoZoomAction *parent {nullptr};

    KoZoomState currentZoomState;
    KisSignalCompressor guiUpdateCompressor;

    KoZoomActionState currentActionState;
};


KoZoomAction::KoZoomAction(const QString& text, QObject *parent)
    : KSelectAction(text, parent)
    , d(new Private(this))
{
    setIcon(koIcon("zoom-original"));
    setEditable( true );
    setMaxComboViewCount( 15 );

    regenerateItems();
    connect(&d->guiUpdateCompressor, SIGNAL(timeout()), SLOT(slotUpdateGuiAfterZoom()));
}

KoZoomAction::~KoZoomAction()
{
    delete d;
}

void KoZoomAction::slotZoomStateChanged(const KoZoomState &zoomState)
{
    d->currentZoomState = zoomState;
    d->currentActionState.setZoomState(zoomState);

    d->guiUpdateCompressor.start();
}

void KoZoomAction::slotTextZoomChanged(const QString &value)
{
    // HACK ALERT: all items in predefined zooms are handled by
    //             slotZoomLevelChangedIndex(), which comes first;
    //             this implementation is for explicitly entered
    //             values only.

    bool isValid = false;
    qreal zoom = QLocale().toDouble(value, &isValid);

    if (!isValid) {
        using KisPortingUtils::stringRemoveFirst;
        using KisPortingUtils::stringRemoveLast;

        QString trimmedValue = value.trimmed();
        if (trimmedValue.endsWith('%')) {
            stringRemoveLast(trimmedValue);
        } else if (trimmedValue.startsWith('%')) {
            /**
             * In Turkish language, the percent sign can go before the
             * number...
             */
            stringRemoveFirst(trimmedValue);
        }
        zoom = QLocale().toDouble(trimmedValue, &isValid);
    }

    if (isValid) {
        /**
         * If the value has been selected from the dropdown, then it has already
         * been activated by slotZoomLevelChangedIndex() and we shouldn't emit it
         * once again.
         */
        const KoZoomState &zoomState = d->currentActionState.zoomState;
        if (zoomState.mode != KoZoomMode::ZOOM_CONSTANT ||
            qRound(zoomState.zoom * 1000.0) != qRound(zoom * 10.0)) {

            Q_EMIT zoomChanged(KoZoomMode::ZOOM_CONSTANT, zoom / 100.0);
        }

    }
}

void KoZoomAction::slotZoomLevelChangedIndex(int index)
{
    if (index < 0 || index >= d->currentActionState.realGuiLevels.size()) return;
    if (index == d->currentActionState.currentRealLevelIndex) return;

    KoZoomMode::Mode mode = std::get<KoZoomMode::Mode>(d->currentActionState.realGuiLevels[index]);
    qreal zoom = std::get<qreal>(d->currentActionState.realGuiLevels[index]);

    Q_EMIT zoomChanged(mode, zoom);
}

void KoZoomAction::regenerateItems()
{
    QStringList values;

    std::transform(d->currentActionState.realGuiLevels.begin(),
                   d->currentActionState.realGuiLevels.end(),
                   std::back_inserter(values),
                   [](const auto &item) {
                       return std::get<QString>(item);
                   });

    {
        QSignalBlocker b(this);
        setItems(values);
        setCurrentItem(d->currentActionState.currentRealLevelIndex);
    }

    Q_EMIT sigInternalUpdateZoomLevelsComboState(
        values,
        d->currentActionState.currentRealLevelIndex,
        d->currentActionState.currentRealLevelText);
}

void KoZoomAction::sliderValueChanged(int value)
{
    if (value < 0 || value >= d->currentActionState.standardLevels.size()) return;

    KoZoomMode::Mode mode = KoZoomMode::ZOOM_CONSTANT;
    qreal zoom = d->currentActionState.standardLevels[value];

    Q_EMIT zoomChanged(mode, zoom);
}

QWidget * KoZoomAction::createWidget(QWidget *parent)
{
    KoZoomWidget* zoomWidget = new KoZoomWidget(parent, d->currentActionState.standardLevels.size());

    connect(this, &KoZoomAction::sigInternalUpdateZoomLevelsComboState, zoomWidget, &KoZoomWidget::setZoomLevelsState);
    connect(this, &KoZoomAction::sigInternalUpdateZoomLevelsSliderState, zoomWidget, &KoZoomWidget::setSliderState);
    connect(this, &KoZoomAction::sigInternalUpdateUsePrintResolutionMode, zoomWidget, &KoZoomWidget::setUsePrintResolutionMode);

    connect(zoomWidget, SIGNAL(zoomLevelChanged(QString)), this, SLOT(slotTextZoomChanged(QString)));
    connect(zoomWidget, SIGNAL(sliderValueChanged(int)), this, SLOT(sliderValueChanged(int)));
    connect(zoomWidget, SIGNAL(zoomLevelChangedIndex(int)), this, SLOT(slotZoomLevelChangedIndex(int)));
    connect(zoomWidget, SIGNAL(sigUsePrintResolutionModeChanged(bool)), this, SIGNAL(sigUsePrintResolutionModeChanged(bool)));

    syncSliderWithZoom();
    regenerateItems();

    return zoomWidget;
}

void KoZoomAction::slotUpdateGuiAfterZoom()
{
    syncSliderWithZoom();
    regenerateItems();
}

void KoZoomAction::setUsePrintResolutionMode(bool value)
{
    Q_EMIT sigInternalUpdateUsePrintResolutionMode(value);
}

void KoZoomAction::syncSliderWithZoom()
{
    const int index = d->currentActionState.calcNearestStandardLevel();
    Q_EMIT sigInternalUpdateZoomLevelsSliderState(d->currentActionState.standardLevels.size(), index);
}
