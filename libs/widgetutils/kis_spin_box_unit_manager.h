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

#ifndef KISSPINBOXUNITMANAGER_H
#define KISSPINBOXUNITMANAGER_H

#include <QObject>
#include <QStringList>
#include <QAbstractListModel>

#include "kritawidgetutils_export.h"

class KisSpinBoxUnitManager;
class KisSpinBoxUnitManagerBuilder;
class KisSpinBoxUnitManagerFactory;

/*!
 * \brief The KisSpinBoxUnitManagerFactory class is a factory that is used to build a default KisSpinBoxUnitManager.
 * \see KisSpinBoxUnitManagerBuilder
 */
class KRITAWIDGETUTILS_EXPORT KisSpinBoxUnitManagerFactory
{
public:

    static KisSpinBoxUnitManager* buildDefaultUnitManager(QObject* parent);
    //! \brief set a builder the factory can use. The factory should take on the lifecycle of the builder, so to delete it call clearUnitManagerBuilder();
    static void setDefaultUnitManagerBuilder(KisSpinBoxUnitManagerBuilder* pBuilder);
    static void clearUnitManagerBuilder();

private:

    static KisSpinBoxUnitManagerBuilder* builder;

};

/*!
 * \brief The KisSpinBoxUnitManagerBuilder class is the base class, used in the strategy pattern of KisSpinBoxUnitManagerFactory.
 * \see KisSpinBoxUnitManagerFactory.
 */
class KRITAWIDGETUTILS_EXPORT KisSpinBoxUnitManagerBuilder
{

public:

    virtual ~KisSpinBoxUnitManagerBuilder() {}

    virtual KisSpinBoxUnitManager* buildUnitManager(QObject* parent) = 0; //this pure virtual function is used to build a unitmanager, it will be used by the unitManagerFactory.
};

/**
 * @brief The KisSpinBoxUnitManager class is an abstract interface for the unitspinboxes classes to manage different type of units.
 *
 * The class make a difference between unit dimension (distance, angle, time).
 *
 * The class allow to convert values between reference unit and apparent unit, but also to get other informations like possible units symbols.
 *
 * This class don't allow to use relative units (units which conversion factor is dependant of the context), even if its private data are prepared to manage it.
 * The reason for this is that from the library of this class it is very hard to acess easily the informations needed. So all will be managed by subclasses in other libs.
 *
 * The class is a subclass of QAbstractListModel, so that available list of units is easily acessed by other Qt standard components, like QComboBoxes.
 *
 */
class KRITAWIDGETUTILS_EXPORT KisSpinBoxUnitManager : public  QAbstractListModel
{
    Q_OBJECT

public:

    enum UnitDimension{
        LENGTH = 0, //length, print size, reference is point
        IMLENGTH = 1, //length, image size, reference is pixel. This dimension is used when the printing units must be avoided
        ANGLE = 2,
        TIME = 3
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
    ~KisSpinBoxUnitManager() override;

    int getUnitDimensionType() const;
    QString getReferenceUnitSymbol() const;
    QString getApparentUnitSymbol() const;

    //! \brief get the position of the apparent unit in the list of units. It is usefull if we want to build a model for combo-box based unit management.
    int getApparentUnitId() const;

    //! \brief get a hint of how many decimals the spinbox need to display.
    int getApparentUnitRecommandedDecimals() const;

    virtual QStringList getsUnitSymbolList(bool withName = false) const;

    qreal getReferenceValue(double apparentValue) const;
    qreal getApparentValue(double refValue) const;

    //! \brief gets the conversion factor of a managed unit, or -1 in case of error. This method is the one that need to be overridden to extend the ability of the KisSpinBoxUnitManager.
    virtual qreal getConversionFactor(int dim, QString symbol) const;
    //! \brief some units conversions are done via an affine transform, not just a linear transform. This function gives the constant of this affine transform (usually 0).
    virtual qreal getConversionConstant(int dim, QString symbol) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

Q_SIGNALS:

    void unitDimensionChanged(int dimCode);
    void unitAboutToChange();
    void unitChanged(QString symbol);
    void unitChanged(int index);
    void conversionFactorChanged(qreal newConversionFactor, qreal oldConversionFactor) const;
    void conversionConstantChanged(qreal newConversionFactor, qreal oldConversionFactor) const;
    void unitListChanged();

public Q_SLOTS:

    void setUnitDimension(UnitDimension dimension);
    void setApparentUnitFromSymbol(QString pSymbol);
    void selectApparentUnitFromIndex(int index);

    void syncWithOtherUnitManager(KisSpinBoxUnitManager* other);
    void clearSyncWithOtherUnitManager(KisSpinBoxUnitManager* other);

protected:

    class Private;
    Private * d;

    //! \brief convert a unitChanged signal with a QString to one with an index.
    void newUnitSymbolToUnitIndex(QString symbol);

    //! \brief indicate if the unit manager has some kind of way of using a percent unit, used by the main class to add percent when necessary.
    virtual bool hasPercent(int unitDim) const;

    //unit's that may be used only if acess to the document informations exists.
    static const QStringList documentRelativeLengthUnitSymbols;
    static const QStringList documentRelativeTimeUnitSymbols;

    void recomputeConversionFactor() const;
    void recomputeConvesrionConstant() const;

    //! \brief calling this method give acess to document relative units. Only subclasses that manage thoses units should call it.
    void grantDocumentRelativeUnits();

};

#endif // KISSPINBOXUNITMANAGER_H
