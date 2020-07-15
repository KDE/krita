#ifndef KIS_MYPAINTBRUSH_OPTIONS_MODEL_H
#define KIS_MYPAINTBRUSH_OPTIONS_MODEL_H

#include <QObject>

#include <QAbstractListModel>
#include <kis_mypaintbrush_option.h>
#include <kis_mypaint_curve_option.h>

class KisCubicCurve;
class KisCurveOption;


class KisMyPaintBrushOptionsModel : public QAbstractListModel
{
    Q_OBJECT
public:

    explicit KisMyPaintBrushOptionsModel(QObject* parent = 0);

    ~KisMyPaintBrushOptionsModel() override;

    void setCurveOption(KisMyPaintCurveOption *curveOption);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex & index) const override;

    KisDynamicOptionSP getSensor(const QModelIndex& index);

    void setCurrentCurve(const QModelIndex& currentIndex, const KisCubicCurve& curve, bool useSameCurve);

    /**
     * Create an index that correspond to the sensor given in argument.
     */
    QModelIndex sensorIndex(KisDynamicOptionSP arg1);

    void resetCurveOption();

Q_SIGNALS:

    void sensorChanged(KisDynamicOptionSP sensor);

    /**
     * This signal is emitted when the parameters of sensor are changed.
     */
    void parametersChanged();

private:

    KisMyPaintCurveOption *m_curveOption;
};

#endif // KIS_MYPAINTBRUSH_OPTIONS_MODEL_H
