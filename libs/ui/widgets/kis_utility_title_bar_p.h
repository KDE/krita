#ifndef KIS_UTILITY_TITLE_BAR_P_H
#define KIS_UTILITY_TITLE_BAR_P_H

#include "kis_utility_title_bar.h"
#include "KoIcon.h"

#include <WidgetsDebug.h>

#include <QAbstractButton>
#include <QAction>
#include <QLabel>
#include <QLayout>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionFrame>
#include <QDockWidget>
class KSqueezedTextLabel;

class Q_DECL_HIDDEN KisUtilityTitleBar::Private
{
public:
    Private(KisUtilityTitleBar* thePublic)
        : thePublic(thePublic)
        , locked(false)
    {
    }

    KisUtilityTitleBar* thePublic {nullptr};
    QIcon lockIcon, floatIcon, removeIcon;
    QHBoxLayout *mainLayout;
    QPushButton* closeButton {nullptr};
    QPushButton* floatButton {nullptr};
    QAbstractButton* lockButton {nullptr};
    KSqueezedTextLabel* titleLabel {nullptr};
    bool locked {false};
    QDockWidget::DockWidgetFeatures features;

    void toggleFloating();
    void topLevelChanged(bool topLevel);
    void featuresChanged(QDockWidget::DockWidgetFeatures features);
    void dockWidgetTitleChanged(const QString &title);
    void updateIcons();
    void updateButtonSizes();
};

#endif // KIS_UTILITY_TITLE_BAR_P_H
