/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_color_history.h"
#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "KisView.h"
#include "kis_canvas_resource_provider.h"

#include <KoCompositeOpRegistry.h>

#include <QToolButton>
#include <QColor>
#include <QList>
#include <kis_icon.h>

KisColorHistory::KisColorHistory(QWidget *parent)
    : KisColorPatches("lastUsedColors", parent)
    , m_resourceProvider(0)
    , m_id(m_idCount++)
{
    m_clearButton = new QToolButton(this);
    m_clearButton->setIcon(KisIconUtils::loadIcon("dialog-cancel-16"));
    m_clearButton->setToolTip(i18n("Clear all color history"));
    m_clearButton->setAutoRaise(true);
    connect(m_clearButton, SIGNAL(clicked()), this, SLOT(clearColorHistory()));

    setAdditionalButtons({m_clearButton});

    restoreColorHistory();
}

void KisColorHistory::unsetCanvas()
{
    KisColorPatches::unsetCanvas();
    m_resourceProvider = 0;
}

void KisColorHistory::setCanvas(KisCanvas2 *canvas)
{
    if (!canvas) return;

    KisColorPatches::setCanvas(canvas);

    if (m_resourceProvider) {
        m_resourceProvider->disconnect(this);
    }

    m_resourceProvider = canvas->imageView()->resourceProvider();


    connect(canvas->imageView()->resourceProvider(), SIGNAL(sigFGColorUsed(KoColor)),
            this, SLOT(addColorToHistory(KoColor)), Qt::UniqueConnection);
}

KisColorSelectorBase* KisColorHistory::createPopup() const
{
    KisColorHistory* ret = new KisColorHistory();
    ret->setCanvas(m_canvas);
    ret->setColors(colors());
    ret->m_colorHistory=m_colorHistory;
    return ret;
}

void KisColorHistory::addColorToHistory(const KoColor& color)
{
    // don't add color in erase mode. See https://bugs.kde.org/show_bug.cgi?id=298940
    if (m_resourceProvider && m_resourceProvider->currentCompositeOp() == COMPOSITE_ERASE) return;

    m_colorHistory.removeAll(color);
    m_colorHistory.prepend(color);

    //the history holds 200 colors, but not all are displayed
    if (m_colorHistory.size()>200)  {
        m_colorHistory.removeLast();
    }

    setColors(m_colorHistory);
    m_lastUsed = m_id; // This history becomes the last used.
}

void KisColorHistory::clearColorHistory() {
    m_colorHistory.clear();
    setColors(m_colorHistory);
}

KisColorHistory::~KisColorHistory()
{
    // Only save the color history where the last color has been added.
    if (m_lastUsed == m_id) {
        KisConfig config(false);
        QList<QColor> history;
        history.reserve(m_colorHistory.size());

        Q_FOREACH(const KoColor &koColor, m_colorHistory) {
                history.push_back(koColor.toQColor());
        }
        config.setColorHistory(history);
    }
}

void KisColorHistory::restoreColorHistory()
{
    KisConfig config(true);
    QList<QColor> history = config.colorHistory();
    KoColor color;

    m_colorHistory.clear();
    Q_FOREACH(const QColor &qColor, history) {
        color.fromQColor(qColor);
        m_colorHistory.push_back(color);
    }
    setColors(m_colorHistory);
}

int KisColorHistory::m_idCount = 0;
int KisColorHistory::m_lastUsed = -1; // None initially
