/*
 *  Copyright (c) 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
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

#ifndef KISSPINBOXUNITMANAGER_H
#define KISSPINBOXUNITMANAGER_H

#include <QObject>
#include <QStringList>

#include "kritawidgetutils_export.h"


/**
 * @brief The KisSpinBoxUnitManager class is an abstract interface for the unitspinboxes classes to manage different type of units.
 *
 * The class make a difference between unit dimension (distance, angle, time), unit reference mode (absolute or relative).
 * If one want to use relative units a reference must be configured using the proper function.
 *
 * The class allow to converte values between reference unit and apparent unit, but also to get other information like possible unit symbols.
 *
 */
class KRITAWIDGETUTILS_EXPORT KisSpinBoxUnitManager : public QObject
{
    Q_OBJECT

public:

    enum UnitDimension{
        LENGTH = 0,
        ANGLE = 1,
        TIME = 2
    };

    static inline bool isUnitId(int code) { return (code == LENGTH || code == ANGLE || code == TIME); }

    //! \brief this list hold the symbols of the referenc unit per dimension. The index is equal to the value in UnitDimension so that the dimension name can be used to index the list.
    static const QStringList referenceUnitSymbols;

    enum Constrain{
        NOCONSTR = 0,
        REFISINT = 1,
        VALISINT = 2

    };

    Q_DECLARE_FLAGS(Constrains, Constrain)

    explicit KisSpinBoxUnitManager(QObject *parent = 0);
    ~KisSpinBoxUnitManager();

    int getUnitDimensionType() const;
    QString getReferenceUnitSymbol() const;
    QString getApparentUnitSymbol() const;

    //! \brief get the position of the apparent unit in the list of units. It is usefull if we want to build a model for combo-box based unit management.
    int getApparentUnitId() const;

    QStringList getsUnitSymbolList(bool withName = false) const;

    qreal getReferenceValue(double apparentValue) const;
    qreal getApparentValue(double refValue) const;

    qreal getConversionFactor(UnitDimension dim, QString symbol) const;

Q_SIGNALS:

    void unitDimensionChanged(int dimCode);
    void unitChanged(QString symbol);
    void conversionFactorChanged(qreal newConversionFactor, qreal oldConversionFactor);
    void unitListChanged();

public Q_SLOTS:

    void setUnitDim(UnitDimension dim);
    void setApparentUnitFromSymbol(QString pSymbol);

    //! \brief configure the reference length (100%) in reference unit. This activate relative units.
    void configureRelativeUnitReference(qreal value);
    void configureRelativeUnitWidthAndHeight(qreal width, qreal height);

protected:

    class Private;
    Private * d;

};

#endif // KISSPINBOXUNITMANAGER_H
