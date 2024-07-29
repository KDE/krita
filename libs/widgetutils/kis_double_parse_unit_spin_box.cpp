/*
 *  SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_double_parse_unit_spin_box.h"
#include "kis_spin_box_unit_manager.h"
#include <klocalizedstring.h>

#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QtMath>

class Q_DECL_HIDDEN KisDoubleParseUnitSpinBox::Private
{
public:
    Private(double low, double up, double step, KisSpinBoxUnitManager* unitManager)
        : lowerInPoints(low),
          upperInPoints(up),
          stepInPoints(step),
          unit(KoUnit(KoUnit::Point)),
          outPutSymbol(""),
          unitManager(unitManager),
          defaultUnitManager(unitManager)
    {
    }

    double minStepForPrec {0};  /// minimum precision for given decimals
    double lowerInPoints {0.0}; ///< lowest value in points
    double upperInPoints {0.0}; ///< highest value in points
    double stepInPoints {0.0};  ///< step in points
    KoUnit unit;

    double previousValueInPoint {0.0}; ///< allow to store the previous value in point, useful in some cases, even if, usually, we prefer to refer to the actual value (in selected unit) and convert it, since this is not always updated.
    QString previousSymbol;
    QString outPutSymbol;

    KisSpinBoxUnitManager* unitManager {0}; //manage more units than permitted by KoUnit.
    KisSpinBoxUnitManager* defaultUnitManager {0}; //the default unit manager is the one the spinbox rely on and go back to if a connected unit manager is destroyed before the spinbox.

    bool isDeleting {false};

    bool unitHasBeenChangedFromOutSideOnce {false}; //in some part of the code the unit is reset. We want to prevent this overriding the unit defined by the user. We use this switch to do so.
    bool letUnitBeChangedFromOutsideMoreThanOnce {true};

    bool displayUnit {true};

    bool allowResetDecimals {true};

    bool mustUsePreviousText {false};
};

KisDoubleParseUnitSpinBox::KisDoubleParseUnitSpinBox(QWidget *parent) :
    KisDoubleParseSpinBox(parent),
    d(new Private(-9999, 9999, 1, KisSpinBoxUnitManagerFactory::buildDefaultUnitManager(this)))
{
    setUnit( KoUnit(KoUnit::Point) );
    setAlignment( Qt::AlignRight );

    connect(this, SIGNAL(valueChanged(double)), this, SLOT(privateValueChanged()));
    connect(lineEdit(), SIGNAL(textChanged(QString)),
            this, SLOT(detectUnitChanges()) );

    connect(d->unitManager, (void (KisSpinBoxUnitManager::*)()) &KisSpinBoxUnitManager::unitAboutToChange, this, (void (KisDoubleParseUnitSpinBox::*)()) &KisDoubleParseUnitSpinBox::prepareUnitChange);
    connect(d->unitManager, (void (KisSpinBoxUnitManager::*)( QString )) &KisSpinBoxUnitManager::unitChanged, this, (void (KisDoubleParseUnitSpinBox::*)( QString const& )) &KisDoubleParseUnitSpinBox::internalUnitChange);

    setDecimals(d->unitManager->getApparentUnitRecommendedDecimals());
}

KisDoubleParseUnitSpinBox::~KisDoubleParseUnitSpinBox()
{
    d->isDeleting = true;
    delete d->defaultUnitManager;
    delete d;
}

void KisDoubleParseUnitSpinBox::setUnitManager(KisSpinBoxUnitManager* unitManager)
{
    if (unitManager == d->unitManager) {
        // just in case we're trying to set manager with the current one...
        return;
    }

    KisSpinBoxUnitManager* oldUnitManager = 0;

    if (d->unitManager) {
        // current unit manager is still here (then setUnitManager not call because it has been destroyed)
        //
        oldUnitManager = d->unitManager;

        disconnect(oldUnitManager, &QObject::destroyed,
                this, &KisDoubleParseUnitSpinBox::disconnectExternalUnitManager); //there's no dependence anymore.
        disconnect(oldUnitManager, (void (KisSpinBoxUnitManager::*)()) &KisSpinBoxUnitManager::unitAboutToChange,
                this, (void (KisDoubleParseUnitSpinBox::*)()) &KisDoubleParseUnitSpinBox::prepareUnitChange);
        disconnect(oldUnitManager, (void (KisSpinBoxUnitManager::*)( QString )) &KisSpinBoxUnitManager::unitChanged,
                this, (void (KisDoubleParseUnitSpinBox::*)( QString const& )) &KisDoubleParseUnitSpinBox::internalUnitChange);
    }

    d->unitManager = unitManager;


    // decimals must be set before value, otherwise value/step/min/max values will be rounded to previous unit decimals value
    if (d->allowResetDecimals) { //if the user has not fixed the number of decimals.
        setDecimals(d->unitManager->getApparentUnitRecommendedDecimals());
    }

    qreal newVal = 0.0;

    double newMin;
    double newMax;
    double newStep;

    if (oldUnitManager == 0 ||
        oldUnitManager &&
        (d->unitManager->getApparentUnitSymbol() != oldUnitManager->getApparentUnitSymbol() ||
         d->unitManager->getUnitDimensionType() == oldUnitManager->getUnitDimensionType())) {

        if (oldUnitManager && d->unitManager->getUnitDimensionType() == oldUnitManager->getUnitDimensionType()) {
            //dimension is the same, calculate the new value
            newVal = d->unitManager->getApparentValue(oldUnitManager->getReferenceValue(KisDoubleParseSpinBox::value()));
        } else {
            newVal = d->unitManager->getApparentValue(d->lowerInPoints);
        }

        newMin = d->unitManager->getApparentValue(d->lowerInPoints);
        newMax = d->unitManager->getApparentValue(d->upperInPoints);
        newStep = d->unitManager->getApparentValue(d->stepInPoints);

        if (d->unitManager->getApparentUnitSymbol() == KoUnit(KoUnit::Pixel).symbol()) {
            // limit the pixel step by 1.0
            newStep = 1.0;
        }

        KisDoubleParseSpinBox::setMinimum(newMin);
        KisDoubleParseSpinBox::setMaximum(newMax);
        KisDoubleParseSpinBox::setSingleStep(newStep);
    }

    connect(d->unitManager, &QObject::destroyed,
            this, &KisDoubleParseUnitSpinBox::disconnectExternalUnitManager);
    connect(d->unitManager, (void (KisSpinBoxUnitManager::*)()) &KisSpinBoxUnitManager::unitAboutToChange,
            this, (void (KisDoubleParseUnitSpinBox::*)()) &KisDoubleParseUnitSpinBox::prepareUnitChange);
    connect(d->unitManager, (void (KisSpinBoxUnitManager::*)( QString )) &KisSpinBoxUnitManager::unitChanged,
            this, (void (KisDoubleParseUnitSpinBox::*)( QString const& )) &KisDoubleParseUnitSpinBox::internalUnitChange);

    KisDoubleParseSpinBox::setValue(newVal);
}

void KisDoubleParseUnitSpinBox::changeValue( double newValue )
{
    double apparentValue;
    double fact = 0.0;
    double cons = 0.0;

    if (d->outPutSymbol.isEmpty()) {
        apparentValue = d->unitManager->getApparentValue(newValue);
    } else {

        fact = d->unitManager->getConversionFactor(d->unitManager->getUnitDimensionType(), d->outPutSymbol);
        cons = d->unitManager->getConversionConstant(d->unitManager->getUnitDimensionType(), d->outPutSymbol);

        apparentValue = fact*newValue + cons;
    }

    if (apparentValue == KisDoubleParseSpinBox::value()) {
        return;
    }

    if (d->outPutSymbol.isEmpty()) {
        KisDoubleParseSpinBox::setValue( apparentValue );
    } else {
        KisDoubleParseSpinBox::setValue( d->unitManager->getApparentValue((newValue - cons)/fact) );
    }
}

void KisDoubleParseUnitSpinBox::changeValuePt( double newValue )
{
    double apparentValue = d->unitManager->getApparentValue(newValue);

    if (apparentValue == KisDoubleParseSpinBox::value()) {
        return;
    }
    KisDoubleParseSpinBox::setValue( apparentValue );
}


void KisDoubleParseUnitSpinBox::setUnit( const KoUnit & unit)
{
    if (d->unitHasBeenChangedFromOutSideOnce && !d->letUnitBeChangedFromOutsideMoreThanOnce) {
        return;
    }

    if (d->unitManager->getUnitDimensionType() != KisSpinBoxUnitManager::LENGTH) {
        d->unitManager->setUnitDimension(KisSpinBoxUnitManager::LENGTH); //setting the unit using a KoUnit mean you want to use a length.
    }

    setUnit(unit.symbol());
    d->unit = unit;
}

void KisDoubleParseUnitSpinBox::setUnit(const QString &symbol)
{
    d->unitManager->setApparentUnitFromSymbol(symbol); //via signals and slots, the correct functions should be called.
}

void KisDoubleParseUnitSpinBox::setReturnUnit(const QString & symbol)
{
    d->outPutSymbol = symbol;
}

QString KisDoubleParseUnitSpinBox::returnUnit() const
{
    return d->outPutSymbol;
}

void KisDoubleParseUnitSpinBox::prepareUnitChange() {

    d->previousValueInPoint = d->unitManager->getReferenceValue(KisDoubleParseSpinBox::value());
    d->previousSymbol = d->unitManager->getApparentUnitSymbol();
}

void KisDoubleParseUnitSpinBox::internalUnitChange(const QString &symbol) {
    //d->unitManager->setApparentUnitFromSymbol(symbol);

    if (d->unitManager->getApparentUnitSymbol() == d->previousSymbol) { //the setApparentUnitFromSymbol is a bit clever, for example in regard of Casesensitivity. So better check like this.
        return;
    }

    // decimals must be updated before value/step/min/max otherwise they'll rounded with previous unit decimal value
    if (d->allowResetDecimals) {
        setDecimals(d->unitManager->getApparentUnitRecommendedDecimals());
    }

    KisDoubleParseSpinBox::setMinimum( d->unitManager->getApparentValue( d->lowerInPoints ) );
    KisDoubleParseSpinBox::setMaximum( d->unitManager->getApparentValue( d->upperInPoints ) );

    qreal step = d->unitManager->getApparentValue( d->stepInPoints );

    if (symbol == KoUnit(KoUnit::Pixel).symbol()) {
        // limit the pixel step by 1.0
        step = 1.0;
    }

    setSingleStep( step );
    KisDoubleParseSpinBox::setValue( d->unitManager->getApparentValue( d->previousValueInPoint ) );

    d->unitHasBeenChangedFromOutSideOnce = true;
}

void KisDoubleParseUnitSpinBox::setDimensionType(int dim)
{
    if (!KisSpinBoxUnitManager::isUnitId(dim)) {
        return;
    }

    d->unitManager->setUnitDimension((KisSpinBoxUnitManager::UnitDimension) dim);
}

double KisDoubleParseUnitSpinBox::value( ) const
{
    if (d->outPutSymbol.isEmpty()) {
        return d->unitManager->getReferenceValue( KisDoubleParseSpinBox::value() );
    }

    double ref = d->unitManager->getReferenceValue( KisDoubleParseSpinBox::value() );
    double fact = d->unitManager->getConversionFactor(d->unitManager->getUnitDimensionType(), d->outPutSymbol);
    double cons = d->unitManager->getConversionConstant(d->unitManager->getUnitDimensionType(), d->outPutSymbol);

    return fact*ref + cons;
}

double KisDoubleParseUnitSpinBox::valuePt( ) const
{
    return d->unitManager->getReferenceValue( KisDoubleParseSpinBox::value() );
}

void KisDoubleParseUnitSpinBox::setMinimum(double min)
{
    d->lowerInPoints = d->unitManager->getReferenceValue(min);
    KisDoubleParseSpinBox::setMinimum( min );
}

void KisDoubleParseUnitSpinBox::setMinimumPt(double min)
{
    d->lowerInPoints = min;
    KisDoubleParseSpinBox::setMinimum( d->unitManager->getApparentValue( min ) );
}


void KisDoubleParseUnitSpinBox::setMaximum(double max)
{
    d->upperInPoints = d->unitManager->getReferenceValue(max);
    KisDoubleParseSpinBox::setMaximum( max );
}

void KisDoubleParseUnitSpinBox::setMaximumPt(double max)
{
    d->upperInPoints = max;
    KisDoubleParseSpinBox::setMaximum( d->unitManager->getApparentValue( max ) );
}

void KisDoubleParseUnitSpinBox::setLineStep(double step)
{
    d->stepInPoints = d->unitManager->getReferenceValue(step);
    KisDoubleParseSpinBox::setSingleStep( step );
}

void KisDoubleParseUnitSpinBox::setLineStepPt(double step)
{
    d->stepInPoints = step;
    KisDoubleParseSpinBox::setSingleStep( d->unitManager->getApparentValue( step ) );
}

void KisDoubleParseUnitSpinBox::setMinMaxStep( double min, double max, double step )
{
    setMinimum( min );
    setMaximum( max );
    setLineStep( step );
}

void KisDoubleParseUnitSpinBox::setMinMaxStepPt( double min, double max, double step )
{
    setMinimumPt( min );
    setMaximumPt( max );
    setLineStepPt( step );
}


QString KisDoubleParseUnitSpinBox::textFromValue( double value ) const
{
    // Just return the current value (for example when the user is editing)
    if (d->mustUsePreviousText) {
        return cleanText();
    }
    // Construct a new value
    QString txt = KisDoubleParseSpinBox::textFromValue(value);
    if (d->displayUnit) {
        if (!txt.endsWith(d->unitManager->getApparentUnitSymbol())) {
            txt += " " + d->unitManager->getApparentUnitSymbol();
        }
    }
    return txt;
}

QString KisDoubleParseUnitSpinBox::veryCleanText() const
{
    return makeTextClean(cleanText());
}

double KisDoubleParseUnitSpinBox::valueFromText( const QString& str ) const
{
    QString txt = makeTextClean(str);
    //this function will take care of prefix (and don't mind if suffix has been removed.
    return KisDoubleParseSpinBox::valueFromText(txt);
}

void KisDoubleParseUnitSpinBox::setUnitChangeFromOutsideBehavior(bool toggle)
{
    d->letUnitBeChangedFromOutsideMoreThanOnce = toggle;
}

void KisDoubleParseUnitSpinBox::setDisplayUnit(bool toggle)
{
    d->displayUnit = toggle;
}

void KisDoubleParseUnitSpinBox::preventDecimalsChangeFromUnitManager(bool prevent)
{
    d->allowResetDecimals = !prevent;
}

void KisDoubleParseUnitSpinBox::privateValueChanged()
{
    Q_EMIT valueChangedPt( value() );
}

QString KisDoubleParseUnitSpinBox::detectUnit()
{
    QString str = veryCleanText().trimmed(); //text with the new unit but not the old one.

    QRegExp regexp ("([ ]*[a-zA-Z]+[ ]*)$"); // Letters or spaces at end
    int res = str.indexOf( regexp );

    if (res > -1) {
        QString expr ( str.right( str.size() - res ) );
        expr = expr.trimmed();
        return expr;
    }

    return "";
}

void KisDoubleParseUnitSpinBox::detectUnitChanges()
{
    QString unitSymb = detectUnit();

    if (unitSymb.isEmpty()) {
        return;
    }

    QString oldUnitSymb = d->unitManager->getApparentUnitSymbol();

    setUnit(unitSymb);
    // Quick hack
    // This function is called when the user changed the text and the call to
    // setValue will provoke a call to textFromValue which will return a new
    // text different from the current one. Since the following setValue is
    // called because of a user change, we use a flag to prevent the text from
    // changing
    d->mustUsePreviousText = true;
    // Change value keep the old value, but converted to new unit... which is
    // different from the value the user entered in the new unit. So we need
    // to set the new value.
    setValue(valueFromText(cleanText()));
    d->mustUsePreviousText = false;

    if (oldUnitSymb != d->unitManager->getApparentUnitSymbol()) {
        // the user has changed the unit, so we block changes from outside.
        setUnitChangeFromOutsideBehavior(false);
    }
}

QString KisDoubleParseUnitSpinBox::makeTextClean(QString const& txt) const
{
    QString expr = txt;
    QString symbol = d->unitManager->getApparentUnitSymbol();

    if ( expr.endsWith(suffix()) ) {
        expr.remove(expr.size()-suffix().size(), suffix().size());
    }

    expr = expr.trimmed();

    if ( expr.endsWith(symbol) ) {
        expr.remove(expr.size()-symbol.size(), symbol.size());
    }

    return expr.trimmed();
}

void KisDoubleParseUnitSpinBox::disconnectExternalUnitManager()
{
    // Internal disconnectExternalUnitManager() method is called when the current unit manager d->unitManager
    // has been destroyed
    // --> ensure the d->unitManager does not point to anything anymore
    d->unitManager = 0;

    if (!d->isDeleting)
    {
        setUnitManager(d->defaultUnitManager); //go back to default unit manager.
    }
}

void KisDoubleParseUnitSpinBox::setDecimals(int prec)
{
    KisDoubleParseSpinBox::setDecimals(prec);
    d->minStepForPrec = 1/qPow(10, prec);

    // fix current single step value is needed
    setSingleStep(singleStep());
}

void KisDoubleParseUnitSpinBox::setSingleStep(double val)
{
    // ensure step value is never below minimal value for precision
    KisDoubleParseSpinBox::setSingleStep(qMax(d->minStepForPrec, val));
}


void KisDoubleParseUnitSpinBox::contextMenuEvent(QContextMenuEvent *event)
{
    // default standard menu for line edit, not possible to get the default menu from a QSpinBox
    QMenu* menu = lineEdit()->createStandardContextMenu();
    if (!menu)
        return;

    // then need to recreate "Step Up" and "Step Down" actions
    menu->addSeparator();
    const uint se = stepEnabled();
    QAction *up = menu->addAction(tr("&Step up"));
    up->setEnabled(se & StepUpEnabled);
    QAction *down = menu->addAction(tr("Step &down"));
    down->setEnabled(se & StepDownEnabled);
    menu->addSeparator();

    // and add expected new entries: menu/submenu with Units
    QMenu* menuUnit = menu->addMenu(i18n("Unit"));
    QActionGroup* unitActions = new QActionGroup(this);
    Q_FOREACH(QString unitSymbol, d->unitManager->getsUnitSymbolList(false)) {
        QString unitLabel = KoUnit::unitDescription(KoUnit::fromSymbol(unitSymbol).type());

        // need to check symbol not managed by KoUnit (return "Points (pt)" in this case...)
        switch (d->unitManager->getUnitDimensionType()) {
            case KisSpinBoxUnitManager::UnitDimension::LENGTH:
            case KisSpinBoxUnitManager::UnitDimension::IMLENGTH:
                if (unitSymbol == "%") {
                    unitLabel = i18n("Percent (%)");
                } else if (unitSymbol == "vw") {
                    unitLabel = i18n("percent of view width (vw)");
                } else if (unitSymbol == "vh") {
                    unitLabel = i18n("percent of view height (vh)");
                }
                break;

            case KisSpinBoxUnitManager::ANGLE:
                if (unitSymbol == "°") {
                    unitLabel = i18n("degrees (°)");
                } else if (unitSymbol == "rad") {
                    unitLabel = i18n("radians (rad)");
                } else if (unitSymbol == "gon") {
                    unitLabel = i18n("gons (gon)");
                } else if (unitSymbol == "%") {
                    unitLabel = i18n("percent of circle (%)");
                }
                break;

            case KisSpinBoxUnitManager::TIME:
                if (unitSymbol == "f") {
                    unitLabel = i18n("frames (f)");
                } else if (unitSymbol == "s") {
                    unitLabel = i18n("seconds (s)");
                } else if (unitSymbol == "%") {
                    unitLabel = i18n("percent of animation (%)");
                }
                break;
        }

        QAction *unitAction = menuUnit->addAction(unitLabel);
        unitAction->setProperty("symbol", unitSymbol);
        unitAction->setCheckable(true);
        unitAction->setActionGroup(unitActions);
        unitAction->setChecked(unitSymbol == d->unitManager->getApparentUnitSymbol());
    }

    const QPoint pos = (event->reason() == QContextMenuEvent::Mouse)
        ? event->globalPos() : mapToGlobal(QPoint(event->pos().x(), 0)) + QPoint(width() / 2, height() / 2);
    const QAction *action = menu->exec(pos);

    if (action) {
        if (action == up) {
            stepBy(1);
        } else if (action == down) {
            stepBy(-1);
        } else {
            QVariant symbol = action->property("symbol");
            if (symbol.isValid()) {
                d->unitManager->setApparentUnitFromSymbol(symbol.toString());
            }
        }
    }

    delete static_cast<QMenu *>(menuUnit);
    delete static_cast<QMenu *>(menu);
    event->accept();
}

