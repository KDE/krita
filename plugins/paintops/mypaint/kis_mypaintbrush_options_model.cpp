#include "kis_mypaintbrush_options_model.h"
#include "kis_mypaint_curve_option.h"

KisMyPaintBrushOptionsModel::KisMyPaintBrushOptionsModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_curveOption(0)
{
}

KisMyPaintBrushOptionsModel::~KisMyPaintBrushOptionsModel()
{
}

void KisMyPaintBrushOptionsModel::setCurveOption(KisMyPaintCurveOption *curveOption)
{
    beginResetModel();
    m_curveOption = curveOption;
    endResetModel();
}

int KisMyPaintBrushOptionsModel::rowCount(const QModelIndex &/*parent*/) const
{
    if (m_curveOption) {
        return m_curveOption->sensors().size();
    }
    else {
        return 0;
    }
}

QVariant KisMyPaintBrushOptionsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole) {
        return KisMyPaintBrushOption::sensorsIds()[index.row()].name();
    }
    else if (role == Qt::CheckStateRole) {
        QString selectedSensorId = KisMyPaintBrushOption::sensorsIds()[index.row()].id();
        KisDynamicOptionSP sensor = m_curveOption->sensor(KisMyPaintBrushOption::id2Type(selectedSensorId), false);
        if (sensor) {
            //dbgKrita << sensor->id() << sensor->isActive();
            return QVariant(sensor->isActive() ? Qt::Checked : Qt::Unchecked);
        }
        else {
            return QVariant(Qt::Unchecked);
        }
    }
    return QVariant();
}

bool KisMyPaintBrushOptionsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool result = false;

    if (role == Qt::CheckStateRole) {
        bool checked = (value.toInt() == Qt::Checked);

        if (true) {//|| m_curveOption->activeSensors().size() != 1) { // Don't uncheck the last sensor (but why not?)
            KisDynamicOptionSP sensor = m_curveOption->sensor(KisMyPaintBrushOption::id2Type(KisMyPaintBrushOption::sensorsIds()[index.row()].id()), false);

            if (!sensor) {
                sensor = KisMyPaintBrushOption::id2Sensor(KisMyPaintBrushOption::sensorsIds()[index.row()].id(), "NOT_VALID_NAME");
                m_curveOption->replaceSensor(sensor);
            }

            sensor->setActive(checked);
            emit(parametersChanged());
            result = true;
        }
    }    
    return result;
}

Qt::ItemFlags KisMyPaintBrushOptionsModel::flags(const QModelIndex & /*index */) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

KisDynamicOptionSP KisMyPaintBrushOptionsModel::getSensor(const QModelIndex& index)
{
    if (!index.isValid()) return 0;
    QString id = KisMyPaintBrushOption::sensorsIds()[index.row()].id();
    return m_curveOption->sensor(KisMyPaintBrushOption::id2Type(id), false);
}

void KisMyPaintBrushOptionsModel::setCurrentCurve(const QModelIndex& currentIndex, const KisCubicCurve& curve, bool useSameCurve)
{
    if (!currentIndex.isValid()) return;

    QString selectedSensorId =  KisMyPaintBrushOption::sensorsIds()[currentIndex.row()].id();
    m_curveOption->setCurve(KisMyPaintBrushOption::id2Type(selectedSensorId), useSameCurve, curve);
}

QModelIndex KisMyPaintBrushOptionsModel::sensorIndex(KisDynamicOptionSP arg1)
{
    return index(KisMyPaintBrushOption::sensorsIds().indexOf(KoID(KisMyPaintBrushOption::id(arg1->sensorType()))));
}

void KisMyPaintBrushOptionsModel::resetCurveOption()
{
    beginResetModel();
    endResetModel();
}

