/*
 *  Copyright (c) 2017 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISDOCUMENTAWARESPINBOXUNITMANAGER_H
#define KISDOCUMENTAWARESPINBOXUNITMANAGER_H

#include "kis_spin_box_unit_manager.h"
#include "kis_double_parse_unit_spin_box.h"

#include "kritaui_export.h"
class KisDocumentAwareSpinBoxUnitManagerBuilder : public KisSpinBoxUnitManagerBuilder
{

public:

    KisSpinBoxUnitManager* buildUnitManager(QObject* parent) override;
};

/*!
 * \brief The KisDocumentAwareSpinBoxUnitManager class is a KisSpinBoxUnitManager that is able to connect to the current document to compute transformation for document relative units (the ones that depend of the resolution, or the size in pixels of the image).
 * \see KisSpinBoxUnitManager
 */
class KRITAUI_EXPORT KisDocumentAwareSpinBoxUnitManager : public KisSpinBoxUnitManager
{
    Q_OBJECT

public:

    enum PixDir {
        PIX_DIR_X,
        PIX_DIR_Y
    }; //in case the image has not the same x and y resolution, indicate on which direction get the resolution.

    //! \brief configure a KisDocumentAwareSpinBoxUnitManager for the given spinbox (make the manager a child of the spinbox and attach it to the spinbox).
    static void setDocumentAwarnessToExistingUnitSpinBox(KisDoubleParseUnitSpinBox* spinBox, bool setUnitFromOutsideToggle = false);

    //! \brief create a unitSpinBox that is already document aware.
    static KisDoubleParseUnitSpinBox* createUnitSpinBoxWithDocumentAwarness(QWidget* parent = 0);

    KisDocumentAwareSpinBoxUnitManager(QObject *parent = 0, int pPixDir = PIX_DIR_X);

    //! \reimp \see KisSpinBoxUnitManager
    qreal getConversionFactor(int dim, QString psymbol) const override;
    //! \reimp \see KisSpinBoxUnitManager
    qreal getConversionConstant(int dim, QString symbol) const override;

protected:

    //! \reimp \see KisSpinBoxUnitManager
    virtual bool hasPercent(int unitDim) const override;

    PixDir pixDir;
};

#endif // KISDOCUMENTAWARESPINBOXUNITMANAGER_H
