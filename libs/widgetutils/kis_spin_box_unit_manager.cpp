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

#include "kis_spin_box_unit_manager.h"

#include "KoUnit.h"
#include <klocalizedstring.h>

#include <QtMath>


KisSpinBoxUnitManagerBuilder* KisSpinBoxUnitManagerFactory::builder = nullptr;

KisSpinBoxUnitManager* KisSpinBoxUnitManagerFactory::buildDefaultUnitManager(QObject* parent)
{
    if (builder == nullptr) {
        return new KisSpinBoxUnitManager(parent);
    }

    return builder->buildUnitManager(parent);
}

void KisSpinBoxUnitManagerFactory::setDefaultUnitManagerBuilder(KisSpinBoxUnitManagerBuilder* pBuilder)
{
    if (builder != nullptr) {
        delete builder; //The factory took over the lifecycle of the builder, so it delete it when replaced.
    }

    builder = pBuilder;
}

void KisSpinBoxUnitManagerFactory::clearUnitManagerBuilder()
{
    if (builder != nullptr) {
        delete builder; //The factory took over the lifecycle of the builder, so it delete it when replaced.
    }

    builder = nullptr;
}

const QStringList KisSpinBoxUnitManager::referenceUnitSymbols = {"pt", "px", "°", "frame"};

const QStringList KisSpinBoxUnitManager::documentRelativeLengthUnitSymbols = {"px", "vw", "vh"}; //px are relative to the resolution, vw and vh to the width and height.
const QStringList KisSpinBoxUnitManager::documentRelativeTimeUnitSymbols = {"s", "%"}; //secondes are relative to the framerate, % to the sequence length.

class Q_DECL_HIDDEN KisSpinBoxUnitManager::Private
{
public:
    Private(KisSpinBoxUnitManager::UnitDimension pDim = KisSpinBoxUnitManager::LENGTH,
            QString pUnitSymbol = "pt",
            double pConv = 1.0):
        dim(pDim),
        unitSymbol(pUnitSymbol),
        conversionFactor(pConv),
        conversionFactorIsFixed(true),
        conversionConstant(0),
        conversionConstantIsFixed(true),
        constrains(0),
        unitListCached(false),
        unitListWithNameCached(false),
        hasHundredPercent(false),
        canAccessDocument(false)
    {

    }

    KisSpinBoxUnitManager::UnitDimension dim;

    QString unitSymbol;
    mutable double conversionFactor;
    bool conversionFactorIsFixed; //tell if it's possible to trust the conversion factor stored or if it's needed to recompute it.
    mutable double conversionConstant;
    bool conversionConstantIsFixed; //tell if it's possible to trust the conversion constant stored or if it's needed to recompute it.

    KisSpinBoxUnitManager::Constrains constrains;

    mutable QStringList unitList;
    mutable bool unitListCached;

    mutable QStringList unitListWithName;
    mutable bool unitListWithNameCached;

    //it's possible to store a reference for the % unit, for length.
    bool hasHundredPercent;
    qreal hundredPercent;

    bool canAccessDocument;

    QVector<KisSpinBoxUnitManager*> connectedUnitManagers;
};

KisSpinBoxUnitManager::KisSpinBoxUnitManager(QObject *parent) : QAbstractListModel(parent)
{
    d = new Private();

    connect(this, (void (KisSpinBoxUnitManager::*)( QString )) &KisSpinBoxUnitManager::unitChanged, this, &KisSpinBoxUnitManager::newUnitSymbolToUnitIndex);
}
KisSpinBoxUnitManager::~KisSpinBoxUnitManager()
{
    delete d;
}

int KisSpinBoxUnitManager::getUnitDimensionType() const
{
    return d->dim;
}

QString KisSpinBoxUnitManager::getReferenceUnitSymbol() const
{
    return referenceUnitSymbols[d->dim];
}

QString KisSpinBoxUnitManager::getApparentUnitSymbol() const
{
    return d->unitSymbol;
}

int KisSpinBoxUnitManager::getApparentUnitId() const
{
    QStringList list = getsUnitSymbolList();
    return list.indexOf(d->unitSymbol);
}

int KisSpinBoxUnitManager::getApparentUnitRecommandedDecimals() const {

    switch (d->dim) {

    case LENGTH:
        if (d->unitSymbol == "px") {
            return 0;
        } else {
            return 2;
        }
    case IMLENGTH:
        if (d->unitSymbol == "px") {
            return 0;
        } else {
            return 2;
        }
    default: //by default return 2.
        break;
    }

    return 2;

}

QStringList KisSpinBoxUnitManager::getsUnitSymbolList(bool withName) const{

    QStringList list;

    if (withName) {
        if (d->unitListWithNameCached) {
            return d->unitListWithName;
        }
    } else {
        if (d->unitListCached) {
            return d->unitList;
        }
    }

    switch (d->dim) {

    case LENGTH:

        for (int i = 0; i < KoUnit::TypeCount; i++) {

            if (KoUnit::Type(i) == KoUnit::Pixel) {
                continue; //skip pixel, which is a document relative unit, in the base class.
            }

            if (withName) {
                list << KoUnit::unitDescription(KoUnit::Type(i));
            } else {
                list << KoUnit(KoUnit::Type(i)).symbol();
            }
        }

        if (hasPercent(LENGTH)) {

            if (withName) {
                list << i18n("percent (%)");
            } else {
                list << "%";
            }

        }

        if (d->canAccessDocument) {
            // ad document relative units
            if (withName) {
                list << KoUnit::unitDescription(KoUnit::Pixel) << i18n("percent of view width (vw)") << i18n("percent of view height (vh)");
            } else {
                list << documentRelativeLengthUnitSymbols;
            }
        }

        break;

    case IMLENGTH:

        if (withName) {
            list << KoUnit::unitDescription(KoUnit::Pixel);
        } else {
            list << "px";
        }

        if (hasPercent(IMLENGTH)) {

            if (withName) {
                list << i18n("percent (%)");
            } else {
                list << "%";
            }

        }

        if (d->canAccessDocument) {
            // ad document relative units
            if (withName) {
                list << i18n("percent of view width (vw)") << i18n("percent of view height (vh)");
            } else {
                list << "vw" << "vh";
            }
        }
        break;

    case ANGLE:

        if (withName) {
            list << i18n("degrees (°)") << i18n("radians (rad)") << i18n("gons (gon)") << i18n("percent of circle (%)");
        } else {
            list << "°" << "rad" << "gon" << "%";
        }
        break;

    case TIME:

        if (withName) {
            list << i18n("frames (f)");
        } else {
            list << "f";
        }

        if (d->canAccessDocument) {
            if (withName) {
                list << i18n("seconds (s)") << i18n("percent of animation (%)");
            } else {
                list << documentRelativeTimeUnitSymbols;
            }
        }

        break;

    }

    if (withName) {
        d->unitListWithName = list;
        d->unitListWithNameCached = true;
    } else {
        d->unitList = list;
        d->unitListCached = true;
    }

    return list;

}

qreal KisSpinBoxUnitManager::getConversionConstant(int dim, QString symbol) const
{
    Q_UNUSED(dim);
    Q_UNUSED(symbol);

    return 0; // all units managed here are transform via a linear function, so this will always be 0 in this class.
}

qreal KisSpinBoxUnitManager::getReferenceValue(double apparentValue) const
{
    if (!d->conversionFactorIsFixed) {
        recomputeConversionFactor();
    }

    if(!d->conversionConstantIsFixed) {
        recomputeConvesrionConstant();
    }

    qreal v = (apparentValue - d->conversionConstant)/d->conversionFactor;

    if (d->constrains &= REFISINT) {
        v = qFloor(v);
    }

    return v;

}

int KisSpinBoxUnitManager::rowCount(const QModelIndex &parent) const {
    if (parent == QModelIndex()) {
        return getsUnitSymbolList().size();
    }
    return 0;
}

QVariant KisSpinBoxUnitManager::data(const QModelIndex &index, int role) const {

    if (role == Qt::DisplayRole) {

        return getsUnitSymbolList(false).at(index.row());

    } else if (role == Qt::ToolTipRole) {

        return getsUnitSymbolList(true).at(index.row());

    }

    return QVariant();
}

qreal KisSpinBoxUnitManager::getApparentValue(double refValue) const
{
    if (!d->conversionFactorIsFixed) {
        recomputeConversionFactor();
    }

    if(!d->conversionConstantIsFixed) {
        recomputeConvesrionConstant();
    }

    qreal v = refValue*d->conversionFactor + d->conversionConstant;

    if (d->constrains &= VALISINT) {
        v = qFloor(v);
    }

    return v;
}

qreal KisSpinBoxUnitManager::getConversionFactor(int dim, QString symbol) const
{

    qreal factor = -1;

    switch (dim) {

    case LENGTH:
        do {
            if (symbol == "px") {
                break;
            }

            bool ok;
            KoUnit unit = KoUnit::fromSymbol(symbol, &ok);
            if (! ok) {
                break;
            }
            factor = unit.toUserValuePrecise(1.0); // use the precise function
        } while (0) ;
        break;

    case IMLENGTH:
        if (symbol == "px") {
            factor = 1;
        }
        break;

    case ANGLE:
        if (symbol == "°") {
            factor = 1.0;
            break;
        }
        if (symbol == "rad") {
            factor = acos(-1)/90.0;
            break;
        }
        if (symbol == "gon") {
            factor = 10.0/9.0;
            break;
        }
        if (symbol == "%") {
            factor = 2.5/9.0; //(25% of circle is 90°)
            break;
        }
        break;

    case TIME:

        if (symbol != "f") { //we have only frames for the moment.
            break;
        }
        factor = 1.0;
        break;

    default:
        break;
    }

    return factor;
}


void KisSpinBoxUnitManager::setUnitDimension(UnitDimension dimension)
{
    if (dimension == d->dim) {
        return;
    }

    d->dim = dimension;
    d->unitSymbol = referenceUnitSymbols[d->dim]; //Active dim is reference dim when just changed.
    d->conversionFactor = 1.0;

    emit unitDimensionChanged(d->dim);

}

void KisSpinBoxUnitManager::setApparentUnitFromSymbol(QString pSymbol)
{

    QString symbol = pSymbol.trimmed();

    if (symbol == d->unitSymbol) {
        return;
    }

    emit unitAboutToChange();

    QString newSymb = "";

    switch (d->dim) {

    case ANGLE:
        if (symbol.toLower() == "deg") {
            newSymb = "°";
            break;
        }
        goto default_indentifier; //always do default after handling possible special cases.

default_indentifier:
    default:
        QStringList list = getsUnitSymbolList();
        if (list.contains(symbol, Qt::CaseInsensitive)) {
            for (QString str : list) {
                if (str.toLower() == symbol.toLower()) {
                    newSymb = str; //official symbol may contain capitals letters, so better take the official version.
                    break;
                }
            }
            break;
        }

    }

    if(newSymb.isEmpty()) {
        return; //abort if it was impossible to locate the correct symbol.
    }

    if (d->canAccessDocument) {
        //manage document relative units.

        QStringList speUnits;

        switch (d->dim) {

        case LENGTH:
            speUnits = documentRelativeLengthUnitSymbols;
            goto default_identifier_conv_fact;

        case IMLENGTH:
            speUnits << "vw" << "vh";
            goto default_identifier_conv_fact;

        case TIME:
            speUnits = documentRelativeTimeUnitSymbols;
            goto default_identifier_conv_fact;

default_identifier_conv_fact:
        default:

            if (speUnits.isEmpty()) {
                d->conversionFactorIsFixed = true;
                break;
            }

            if (speUnits.contains(newSymb)) {
                d->conversionFactorIsFixed = false;
                break;
            }

            d->conversionFactorIsFixed = true;
            break;
        }

        if (d->dim == TIME) {
            if (newSymb == "%") {
                d->conversionConstantIsFixed = false;
            }
        } else {
            d->conversionConstantIsFixed = true;
        }

    }

    qreal conversFact = getConversionFactor(d->dim, newSymb);
    qreal oldConversFact = d->conversionFactor;

    d->conversionFactor = conversFact;
    emit conversionFactorChanged(d->conversionFactor, oldConversFact);

    d->unitSymbol = newSymb;
    emit unitChanged(newSymb);

}

void KisSpinBoxUnitManager::selectApparentUnitFromIndex(int index) {

    if (index >= 0 && index < rowCount()) {
        setApparentUnitFromSymbol(getsUnitSymbolList().at(index));
    }

}


void KisSpinBoxUnitManager::syncWithOtherUnitManager(KisSpinBoxUnitManager* other) {

    if (d->connectedUnitManagers.indexOf(other) >= 0) {
        return;
    }

    if (other->getUnitDimensionType() == getUnitDimensionType()) { //sync only unitmanager of the same type.
        if (other->getsUnitSymbolList() == getsUnitSymbolList()) { //and if we have identical units available.

            connect(this, SIGNAL(unitChanged(int)), other, SLOT(selectApparentUnitFromIndex(int))); //sync units.
            connect(other, SIGNAL(unitChanged(int)), this, SLOT(selectApparentUnitFromIndex(int))); //sync units.

            d->connectedUnitManagers.append(other);

        }
    }

}

void KisSpinBoxUnitManager::clearSyncWithOtherUnitManager(KisSpinBoxUnitManager* other) {

    int id = d->connectedUnitManagers.indexOf(other);

    if (id < 0) {
        return;
    }

    disconnect(this, SIGNAL(unitChanged(int)), other, SLOT(selectApparentUnitFromIndex(int))); //unsync units.
    disconnect(other, SIGNAL(unitChanged(int)), this, SLOT(selectApparentUnitFromIndex(int))); //unsync units.

    d->connectedUnitManagers.removeAt(id);

}

void KisSpinBoxUnitManager::newUnitSymbolToUnitIndex(QString symbol) {
    int id = getsUnitSymbolList().indexOf(symbol);

    if (id >= 0) {
        emit unitChanged(id);
    }
}

bool KisSpinBoxUnitManager::hasPercent(int unitDim) const {

    if (unitDim == IMLENGTH || unitDim == LENGTH) {
        return false;
    }

    if (unitDim == TIME) {
        return d->canAccessDocument;
    }

    if (unitDim == ANGLE) {
        return true; //percent is fixed when considering angles.
    }

    return false;
}

void KisSpinBoxUnitManager::recomputeConversionFactor() const
{
    if (d->conversionFactorIsFixed) {
        return;
    }

    qreal oldConversionFactor = d->conversionFactor;

    d->conversionFactor = getConversionFactor(d->dim, d->unitSymbol);

    if (oldConversionFactor != d->conversionFactor) {
        emit conversionFactorChanged(d->conversionFactor, oldConversionFactor);
    }
}

void KisSpinBoxUnitManager::recomputeConvesrionConstant() const
{
    if (d->conversionConstantIsFixed) {
        return;
    }

    qreal oldConversionConstant = d->conversionConstant;

    d->conversionConstant = getConversionConstant(d->dim, d->unitSymbol);

    if (oldConversionConstant != d->conversionConstant) {
        emit conversionConstantChanged(d->conversionConstant, oldConversionConstant);
    }
}

void KisSpinBoxUnitManager::grantDocumentRelativeUnits()
{
    d->canAccessDocument = true;
}
