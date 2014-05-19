/*
 *
 */

#include "kis_tooltip_manager.h"

#include <QAction>
#include <QInputDialog>
#include <kactioncollection.h>
#include <kdebug.h>

#include "kis_view2.h"

KisTooltipManager::KisTooltipManager(KisView2* view) : QObject(view), m_view(view)
{
}

KisTooltipManager::~KisTooltipManager()
{
}

void KisTooltipManager::record()
{
    foreach(QAction* action, m_view->actionCollection()->actions()) {
        action->disconnect();
        connect(action, SIGNAL(triggered()), this, SLOT(captureToolip()));
    }
}

void KisTooltipManager::captureToolip()
{
    QString id = sender()->objectName();

    QString oldTooltip;
    if (m_tooltipMap.contains(id)) {
        oldTooltip = m_tooltipMap[id];
    }

    bool ok;
    QString tooltip = QInputDialog::getText(m_view, "Add Tooltip",
                                            "New Tooltip:", QLineEdit::Normal,
                                            oldTooltip, &ok);
    if (ok && !tooltip.isEmpty()) {
        kDebug() << id << " has tooltip " << tooltip;
        dynamic_cast<QAction*>(sender())->setToolTip(tooltip);
        m_tooltipMap[id] = tooltip;
    }
}



#include "kis_tooltip_manager.moc"
