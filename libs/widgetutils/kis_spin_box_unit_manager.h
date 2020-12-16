/*
 *  SPDX-FileCopyrightText: 2017 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
 * The class differentiates between unit dimension (distance, angle, time).
 *
 * The class allows to convert values between reference unit and apparent unit, but also to get other information like possible units symbols.
 *
 * This class doesn't allow to use relative units (units whose conversion factor is dependent of the context), even if it's private data is prepared to manage it.
 * The reason for this is that from the library of this class it is very hard to easily access the information needed. So all will be managed by subclasses in other libs.
 *
 * The class is a subclass of QAbstractListModel, so that available list of units is easily accessed by other Qt standard components, like QComboBoxes.
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

    //! \brief this list holds the symbols of the reference unit per dimension. The index is equal to the value in UnitDimension so that the dimension name can be used to index the list.
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

    //! \brief get the position of the apparent unit in the list of units. It is useful if we want to build a model for combo-box based unit management.
    int getApparentUnitId() const;

    //! \brief get a hint of how many decimals the spinbox needs to display.
    int getApparentUnitRecommandedDecimals() const;

    virtual QStringList getsUnitSymbolList(bool withName = false) const;

    qreal getReferenceValue(double apparentValue) const;
    qreal getApparentValue(double refValue) const;

    //! \brief gets the conversion factor of a managed unit, or -1 in case of error. This method is the one that needs to be overridden to extend the ability of the KisSpinBoxUnitManager.
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

    //unit's that may be used only if access to the document information exists.
    static const QStringList documentRelativeLengthUnitSymbols;
    static const QStringList documentRelativeTimeUnitSymbols;

    void recomputeConversionFactor() const;
    void recomputeConvesrionConstant() const;

    //! \brief calling this method gives access to document relative units. Only subclasses that manage those units should call it.
    void grantDocumentRelativeUnits();

};

#endif // KISSPINBOXUNITMANAGER_H
