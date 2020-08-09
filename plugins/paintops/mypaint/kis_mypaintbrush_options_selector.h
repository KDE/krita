#ifndef KIS_MYPAINTBRUSH_OPTIONS_SELECTOR_H
#define KIS_MYPAINTBRUSH_OPTIONS_SELECTOR_H

#include <QObject>

#include <QWidget>

class KisCubicCurve;
class QModelIndex;
class KisMyPaintCurveOption;

#include <kis_mypaintbrush_option.h>
#include <kis_mypaint_curve_option.h>

class KisMyPaintBrushOptionsSelector : public QWidget
{
    Q_OBJECT
public:

    KisMyPaintBrushOptionsSelector(QWidget* parent);
    ~KisMyPaintBrushOptionsSelector() override;

    void setCurveOption(KisMyPaintCurveOption *curveOption);
    void setCurrent(KisDynamicOptionSP _sensor);
    KisDynamicOptionSP currentHighlighted();
    void setCurrentCurve(const KisCubicCurve& curve, bool useSameCurve);
    void reload();

private Q_SLOTS:

    void sensorActivated(const QModelIndex& index);
    void setCurrent(const QModelIndex& index);

Q_SIGNALS:

    void sensorChanged(KisDynamicOptionSP sensor);

    /**
     * This signal is emitted when the parameters of sensor are changed.
     */
    void parametersChanged();

    void highlightedSensorChanged(KisDynamicOptionSP sensor);
private:
    struct Private;
    Private* const d;
};

#endif // KIS_MYPAINTBRUSH_OPTIONS_SELECTOR_H
