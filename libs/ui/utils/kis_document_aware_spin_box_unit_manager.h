#ifndef KISDOCUMENTAWARESPINBOXUNITMANAGER_H
#define KISDOCUMENTAWARESPINBOXUNITMANAGER_H

#include "kis_spin_box_unit_manager.h"
#include "kis_double_parse_unit_spin_box.h"

#include "kritaui_export.h"


class KRITAUI_EXPORT KisDocumentAwareSpinBoxUnitManager : public KisSpinBoxUnitManager
{
    Q_OBJECT

public:

    enum PixDir {
        PIX_DIR_X,
        PIX_DIR_Y
    }; //in case the image has not the same x and y resolution, indicate on which direction get the resolution.

    //! \brief configure a KisDocumentAwareSpinBoxUnitManager for the given spinbox (make the manager a child of the spinbox and attach it to the spinbox).
    static void setDocumentAwarnessToExistingUnitSpinBox(KisDoubleParseUnitSpinBox* spinBox);

    //! \brief create a unitSpinBox that is already document aware.
    static KisDoubleParseUnitSpinBox* createUnitSpinBoxWithDocumentAwarness(QWidget* parent = 0);

    KisDocumentAwareSpinBoxUnitManager(QObject *parent = 0, int pPixDir = PIX_DIR_X);

    virtual qreal getConversionFactor(UnitDimension dim, QString symbol) const;
    virtual qreal getConversionConstant(UnitDimension dim, QString symbol) const;

private:

    PixDir pixDir;
};

#endif // KISDOCUMENTAWARESPINBOXUNITMANAGER_H
