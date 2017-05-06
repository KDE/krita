#ifndef KIS_TIMED_SPACING_SELECTION_WIDGET_H
#define KIS_TIMED_SPACING_SELECTION_WIDGET_H

#include <QWidget>
#include <QScopedPointer>

#include <kritapaintop_export.h>
#include "ui_wdgtimedspacing.h"

class PAINTOP_EXPORT KisTimedSpacingSelectionWidget : public QWidget, public Ui::WdgTimedSpacing
{
    Q_OBJECT
public:
    KisTimedSpacingSelectionWidget(QWidget *parent);

    /**
     * @param enabled True if and only if timed spacing should be enabled
     * @param rate Indicates how fast the time-based spacing should be, in dabs per second
     */
    void setTimedSpacing(bool enabled, qreal rate);

    /**
     * @return True if and only if timed spacing is currently enabled
     */
    bool isTimedSpacingEnabled() const;

    /**
     * @return The time-based spacing rate, in dabs per second
     */
    qreal rate() const;

Q_SIGNALS:
    void sigTimedSpacingChanged();

private Q_SLOTS:
    void slotEnabledChanged(bool enabled);
    void slotRateChanged(qreal rate);
};

#endif // KIS_TIMED_SPACING_SELECTION_WIDGET_H
