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

#include "kis_spin_box_unit_manager.h"

#include "KoUnit.h"
#include <klocalizedstring.h>

#include <QtMath>

const QStringList KisSpinBoxUnitManager::referenceUnitSymbols = {"pt", "°", "frame"};

class Q_DECL_HIDDEN KisSpinBoxUnitManager::Private
{
public:
    Private(KisSpinBoxUnitManager::UnitDimension pDim = KisSpinBoxUnitManager::LENGTH,
            QString pUnitSymbol = "pt",
            double pConv = 1.0):
        dim(pDim),
        unitSymbol(pUnitSymbol),
        conversionFactor(pConv),
        constrains(0),
        unitListCached(false),
        hasHundredPercent(false),
        hasWidthAndHeight(false)
    {

    }

    KisSpinBoxUnitManager::UnitDimension dim;

    QString unitSymbol;
    double conversionFactor;

    KisSpinBoxUnitManager::Constrains constrains;

    mutable QStringList unitList;
    mutable bool unitListCached;

    mutable QStringList unitListWithName;
    mutable bool unitListWithNameCached;

    bool hasHundredPercent;
    qreal hundredPercent;
    bool hasWidthAndHeight;
    qreal width;
    qreal height;
};

KisSpinBoxUnitManager::KisSpinBoxUnitManager(QObject *parent) : QObject(parent)
{
    d = new Private();
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

QStringList KisSpinBoxUnitManager::getsUnitSymbolList(bool withName) const{

    QStringList list; //TODO: cache

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
            if (withName) {
                list << KoUnit::unitDescription(KoUnit::Type(i));
            } else {
                list << KoUnit(KoUnit::Type(i)).symbol();
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

qreal KisSpinBoxUnitManager::getReferenceValue(double apparentValue) const
{

    qreal v = apparentValue/d->conversionFactor;

    if (d->constrains &= REFISINT) {
       v = qFloor(v);
    }

    return v;

}

qreal KisSpinBoxUnitManager::getApparentValue(double refValue) const
{

    qreal v = refValue*d->conversionFactor;

    if (d->constrains &= VALISINT) {
        v = qFloor(v);
    }

    return v;
}

qreal KisSpinBoxUnitManager::getConversionFactor(UnitDimension dim, QString symbol) const
{

    qreal factor = -1;

    switch (dim) {

    case LENGTH:
        do {
            bool ok;
            KoUnit unit = KoUnit::fromSymbol(symbol, &ok);
            if (! ok) {
                break;
            }
            factor = unit.toUserValue(1.0);
        } while (0) ;
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

    }

    return factor;
}


void KisSpinBoxUnitManager::setUnitDim(UnitDimension dim)
{
    if (dim == d->dim) {
        return;
    }

    d->dim = dim;
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

    QString newSymb = "";

    switch (d->dim) {

    case ANGLE:
        if (symbol.toLower() == "deg") {
            newSymb = "°";
            break;
        }
        goto default_indentifier; //alway do default after handling possible special cases.

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

    qreal conversFact = getConversionFactor(d->dim, newSymb);
    qreal oldConversFact = d->conversionFactor;

    d->conversionFactor = conversFact;
    emit conversionFactorChanged(d->conversionFactor, oldConversFact);

    d->unitSymbol = newSymb;
    emit unitChanged(newSymb);

}

void KisSpinBoxUnitManager::configureRelativeUnitReference(qreal value)
{
    if (d->hasHundredPercent && d->hundredPercent == value) {
        return;
    }

    if (!d->hasHundredPercent) { //in case we add relative units we need to clear cache for unitlists.
        d->unitListCached = false;
        d->unitListWithNameCached = false;
        emit unitListChanged();
    }

    d->hundredPercent = value;
    d->hasHundredPercent = true;
}

void KisSpinBoxUnitManager::configureRelativeUnitWidthAndHeight(qreal width, qreal height)
{
    if (d->hasWidthAndHeight && d->width == width && d->height == height) {
        return;
    }

    if (!d->hasWidthAndHeight) { //in case we add relative units we need to clear cache for unitlists.
        d->unitListCached = false;
        d->unitListWithNameCached = false;
        emit unitListChanged();
    }

    d->width = width;
    d->height = height;
    d->hasWidthAndHeight = true;

}
