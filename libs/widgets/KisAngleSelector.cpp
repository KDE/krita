/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <cmath>
#include <functional>

#include <QHBoxLayout>
#include <QToolButton>
#include <QAction>
#include <QEvent>
#include <QMenu>
#include <QLineEdit>
#include <QStyleOptionSpinBox>
#include <QStyle>

#include <kis_icon_utils.h>
#include <kis_signals_blocker.h>

#include <klocalizedstring.h>

#include "KisAngleSelector.h"

struct KisAngleSelectorSpinBox::Private
{
    KisAngleSelectorSpinBox *q;
    bool isFlat;
    bool hasFocus;
    bool isHovered;
    QSize cachedSizeHint;

    void updateStyleSheet()
    {
        if (!isFlat || hasFocus || isHovered) {
            q->setStyleSheet("QDoubleSpinBox{}");
        } else {
            q->setStyleSheet(
                "QDoubleSpinBox{background:transparent; border:transparent;}"
                "QDoubleSpinBox::up-button{background:transparent; border:transparent;}"
                "QDoubleSpinBox::down-button{background:transparent; border:transparent;}"
            );
        }
        q->lineEdit()->setStyleSheet("QLineEdit{background:transparent;}");
    }
};

KisAngleSelectorSpinBox::KisAngleSelectorSpinBox(QWidget *parent)
    : KisDoubleParseSpinBox(parent)
    , m_d(new Private)
{
    m_d->q = this;
    m_d->isFlat = false;
    m_d->hasFocus = false;
    m_d->isHovered = false;
    m_d->updateStyleSheet();
}

KisAngleSelectorSpinBox::~KisAngleSelectorSpinBox()
{}

void KisAngleSelectorSpinBox::setRange(double min, double max)
{
    m_d->cachedSizeHint = QSize();
    KisDoubleParseSpinBox::setRange(min, max);
}

double KisAngleSelectorSpinBox::valueFromText(const QString & text) const
{
    const double v = KisDoubleParseSpinBox::valueFromText(text);
    return KisAngleSelector::closestCoterminalAngleInRange(v, minimum(), maximum());
}

bool KisAngleSelectorSpinBox::isFlat() const
{
    return m_d->isFlat;
}

void KisAngleSelectorSpinBox::setFlat(bool newFlat)
{
    m_d->isFlat = newFlat;
    m_d->updateStyleSheet();
}

void KisAngleSelectorSpinBox::enterEvent(QEvent *e)
{
    m_d->isHovered = true;
    m_d->updateStyleSheet();
    KisDoubleParseSpinBox::enterEvent(e);
}

void KisAngleSelectorSpinBox::leaveEvent(QEvent *e)
{
    m_d->isHovered = false;
    m_d->updateStyleSheet();
    KisDoubleParseSpinBox::leaveEvent(e);
}

void KisAngleSelectorSpinBox::focusInEvent(QFocusEvent *e)
{
    m_d->hasFocus = true;
    m_d->updateStyleSheet();
    KisDoubleParseSpinBox::focusInEvent(e);
}

void KisAngleSelectorSpinBox::focusOutEvent(QFocusEvent *e)
{
    m_d->hasFocus = false;
    m_d->updateStyleSheet();
    KisDoubleParseSpinBox::focusOutEvent(e);
}

QSize KisAngleSelectorSpinBox::minimumSizeHint() const
{
    if (m_d->cachedSizeHint.isEmpty()) {
        ensurePolished();

        const QFontMetrics fm(fontMetrics());
        int h = lineEdit()->minimumSizeHint().height();
        int w = 0;

        QString s;
        QString fixedContent =  prefix() + suffix() + QLatin1Char(' ');
        s = textFromValue(minimum());
        s.truncate(18);
        s += fixedContent;
        w = qMax(w, fm.horizontalAdvance(s));
        s = textFromValue(maximum());
        s.truncate(18);
        s += fixedContent;
        w = qMax(w, fm.horizontalAdvance(s));

        w += 2; // cursor blinking space

        QStyleOptionSpinBox option;
        initStyleOption(&option);

        QSize hint(w, h);

        KisDoubleParseSpinBox tmp;
        m_d->cachedSizeHint = style()->sizeFromContents(QStyle::CT_SpinBox, &option, hint, &tmp);
    }

    return m_d->cachedSizeHint;
}

QSize KisAngleSelectorSpinBox::sizeHint() const
{
    return minimumSizeHint();
}

void KisAngleSelectorSpinBox::refreshStyle()
{
    m_d->cachedSizeHint = QSize();
    updateGeometry();
    m_d->updateStyleSheet();
}

struct KisAngleSelector::Private
{
    KisAngleSelector *q;
    KisAngleGauge *angleGauge;
    KisAngleSelectorSpinBox *spinBox;
    QToolButton *toolButtonFlipOptions;
    QToolButton *toolButtonFlipHorizontally;
    QToolButton *toolButtonFlipVertically;
    QToolButton *toolButtonFlipHorizontallyAndVertically;
    QAction *actionFlipHorizontally;
    QAction *actionFlipVertically;
    QAction *actionFlipHorizontallyAndVertically;
    QAction *actionResetAngle;
    QMenu *menuFlip;

    KisAngleSelector::FlipOptionsMode flipOptionsMode;
    int angleGaugeSize;

    void on_angleGauge_angleChanged(qreal angle);
    void on_angleGauge_customContextMenuRequested(const QPoint &point);
    void on_spinBox_valueChanged(double value);
    void on_actionFlipHorizontally_triggered();
    void on_actionFlipVertically_triggered();
    void on_actionFlipHorizontallyAndVertically_triggered();
    void on_actionResetAngle_triggered();

    void resizeAngleGauge();
};

KisAngleSelector::KisAngleSelector(QWidget* parent)
    : QWidget(parent)
    , m_d(new Private)
{
    m_d->q = this;

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_d->angleGauge = new KisAngleGauge(this);
    m_d->angleGauge->setContextMenuPolicy(Qt::CustomContextMenu);

    m_d->spinBox = new KisAngleSelectorSpinBox(this);
    m_d->spinBox->setSuffix(i18nc("Degrees symbol", "˚"));
    m_d->spinBox->setRange(0, 360);
    m_d->spinBox->setWrapping(true);

    m_d->actionFlipHorizontally = new QAction(this);
    m_d->actionFlipHorizontally->setText(
        i18nc(
            "Flips the angle horizontally, around the vertical axis",
            "Flip the angle horizontally"
        )
    );
    m_d->actionFlipVertically = new QAction(this);
    m_d->actionFlipVertically->setText(
        i18nc(
            "Flips the angle vertically, around the horizontal axis",
            "Flip the angle vertically"
        )
    );
    m_d->actionFlipHorizontallyAndVertically = new QAction(this);
    m_d->actionFlipHorizontallyAndVertically->setText(
        i18nc(
            "Flips the angle horizontally and vertically",
            "Flip the angle horizontally and vertically"
        )
    );
    QAction *menuSeparator = new QAction(this);
    menuSeparator->setSeparator(true);
    m_d->actionResetAngle = new QAction(this);
    m_d->actionResetAngle->setText(
        i18nc(
            "Reset the angle to a predefined value",
            "Reset angle"
        )
    );
    m_d->menuFlip = new QMenu(this);
    m_d->menuFlip->addAction(m_d->actionFlipHorizontally);
    m_d->menuFlip->addAction(m_d->actionFlipVertically);
    m_d->menuFlip->addAction(m_d->actionFlipHorizontallyAndVertically);
    m_d->menuFlip->addAction(menuSeparator);
    m_d->menuFlip->addAction(m_d->actionResetAngle);

    QHBoxLayout *layoutFlipButtons = new QHBoxLayout;
    layoutFlipButtons->setSpacing(1);
    layoutFlipButtons->setContentsMargins(0, 0, 0, 0);

    m_d->toolButtonFlipOptions = new QToolButton(this);
    m_d->toolButtonFlipOptions->setPopupMode(QToolButton::InstantPopup);
    m_d->toolButtonFlipOptions->setAutoRaise(true);
    m_d->toolButtonFlipOptions->setIcon(KisIconUtils::loadIcon("hamburger_menu_dots"));
    m_d->toolButtonFlipOptions->setStyleSheet("QToolButton::menu-indicator { image: none; }");
    m_d->toolButtonFlipOptions->setMenu(m_d->menuFlip);
    m_d->toolButtonFlipOptions->setFocusPolicy(Qt::StrongFocus);

    m_d->toolButtonFlipHorizontally = new QToolButton(this);
    m_d->toolButtonFlipHorizontally->setAutoRaise(true);
    m_d->toolButtonFlipHorizontally->setIcon(KisIconUtils::loadIcon("flip_angle_h"));
    m_d->toolButtonFlipHorizontally->setIconSize(QSize(20, 20));
    m_d->toolButtonFlipHorizontally->setToolTip(m_d->actionFlipHorizontally->text());
    m_d->toolButtonFlipHorizontally->setFocusPolicy(Qt::StrongFocus);

    m_d->toolButtonFlipVertically = new QToolButton(this);
    m_d->toolButtonFlipVertically->setAutoRaise(true);
    m_d->toolButtonFlipVertically->setIcon(KisIconUtils::loadIcon("flip_angle_v"));
    m_d->toolButtonFlipVertically->setIconSize(QSize(20, 20));
    m_d->toolButtonFlipVertically->setToolTip(m_d->actionFlipVertically->text());
    m_d->toolButtonFlipVertically->setFocusPolicy(Qt::StrongFocus);

    m_d->toolButtonFlipHorizontallyAndVertically = new QToolButton(this);
    m_d->toolButtonFlipHorizontallyAndVertically->setAutoRaise(true);
    m_d->toolButtonFlipHorizontallyAndVertically->setIcon(KisIconUtils::loadIcon("flip_angle_hv"));
    m_d->toolButtonFlipHorizontallyAndVertically->setIconSize(QSize(20, 20));
    m_d->toolButtonFlipHorizontallyAndVertically->setToolTip(m_d->actionFlipHorizontallyAndVertically->text());
    m_d->toolButtonFlipHorizontallyAndVertically->setFocusPolicy(Qt::StrongFocus);

    layoutFlipButtons->addWidget(m_d->toolButtonFlipOptions);
    layoutFlipButtons->addWidget(m_d->toolButtonFlipHorizontally);
    layoutFlipButtons->addWidget(m_d->toolButtonFlipVertically);
    layoutFlipButtons->addWidget(m_d->toolButtonFlipHorizontallyAndVertically);

    mainLayout->addWidget(m_d->angleGauge);
    mainLayout->addWidget(m_d->spinBox);
    mainLayout->addLayout(layoutFlipButtons);

    setLayout(mainLayout);

    setTabOrder(m_d->angleGauge, m_d->spinBox);
    setTabOrder(m_d->spinBox, m_d->toolButtonFlipOptions);
    setTabOrder(m_d->toolButtonFlipOptions, m_d->toolButtonFlipHorizontally);
    setTabOrder(m_d->toolButtonFlipHorizontally, m_d->toolButtonFlipVertically);
    setTabOrder(m_d->toolButtonFlipVertically, m_d->toolButtonFlipHorizontallyAndVertically);

    setFlipOptionsMode(FlipOptionsMode_Buttons);
    setGaugeSize(0);
    
    using namespace std::placeholders;
    connect(
        m_d->angleGauge,
        &KisAngleGauge::angleChanged,
        std::bind(&Private::on_angleGauge_angleChanged, m_d.data(), _1)
    );
    connect(
        m_d->angleGauge,
        &KisAngleGauge::customContextMenuRequested,
        std::bind(&Private::on_angleGauge_customContextMenuRequested, m_d.data(), _1)
    );
    connect(
        m_d->spinBox,
        QOverload<double>::of(&KisDoubleParseSpinBox::valueChanged),
        std::bind(&Private::on_spinBox_valueChanged, m_d.data(), _1)
    );
    connect(
        m_d->actionFlipHorizontally,
        &QAction::triggered,
        std::bind(&Private::on_actionFlipHorizontally_triggered, m_d.data())
    );
    connect(
        m_d->actionFlipVertically,
        &QAction::triggered,
        std::bind(&Private::on_actionFlipVertically_triggered, m_d.data())
    );
    connect(
        m_d->actionFlipHorizontallyAndVertically,
        &QAction::triggered,
        std::bind(&Private::on_actionFlipHorizontallyAndVertically_triggered, m_d.data())
    );
    connect(m_d->actionResetAngle, SIGNAL(triggered()), SLOT(reset()));
    connect(m_d->toolButtonFlipHorizontally, SIGNAL(clicked()), m_d->actionFlipHorizontally, SLOT(trigger()));
    connect(m_d->toolButtonFlipVertically, SIGNAL(clicked()), m_d->actionFlipVertically, SLOT(trigger()));
    connect(m_d->toolButtonFlipHorizontallyAndVertically, SIGNAL(clicked()), m_d->actionFlipHorizontallyAndVertically, SLOT(trigger()));
}

KisAngleSelector::~KisAngleSelector()
{}

qreal KisAngleSelector::angle() const
{
    return m_d->spinBox->value();
}

qreal KisAngleSelector::snapAngle() const
{
    return m_d->angleGauge->snapAngle();
}

qreal KisAngleSelector::resetAngle() const
{
    return m_d->angleGauge->resetAngle();
}

int	KisAngleSelector::decimals() const
{
    return m_d->spinBox->decimals();
}

qreal KisAngleSelector::maximum() const
{
    return m_d->spinBox->maximum();
}

qreal KisAngleSelector::minimum() const
{
    return m_d->spinBox->minimum();
}

bool KisAngleSelector::wrapping() const
{
    return m_d->spinBox->wrapping();
}

KisAngleSelector::FlipOptionsMode KisAngleSelector::flipOptionsMode() const
{
    return m_d->flipOptionsMode;
}

int KisAngleSelector::gaugeSize() const
{
    return m_d->angleGaugeSize;
}

KisAngleGauge::IncreasingDirection KisAngleSelector::increasingDirection() const
{
    return m_d->angleGauge->increasingDirection();
}

bool KisAngleSelector::isUsingFlatSpinBox() const
{
    return m_d->spinBox->isFlat();
}

void KisAngleSelector::setAngle(qreal newAngle)
{
    KisSignalsBlocker angleGaugeSignalsBlocker(m_d->angleGauge);
    KisSignalsBlocker spinBoxSignalsBlocker(m_d->spinBox);

    const qreal oldAngle = m_d->spinBox->value();
    
    m_d->spinBox->setValue(newAngle);
    m_d->angleGauge->setAngle(m_d->spinBox->value());

    if (qFuzzyCompare(oldAngle, m_d->spinBox->value())) {
        return;
    }

    emit angleChanged(m_d->spinBox->value());
}

void KisAngleSelector::setSnapAngle(qreal newSnapAngle)
{
    m_d->angleGauge->setSnapAngle(newSnapAngle);
}

void KisAngleSelector::setResetAngle(qreal newResetAngle)
{
    m_d->angleGauge->setResetAngle(newResetAngle);
}

void KisAngleSelector::setDecimals(int newNumberOfDecimals)
{
    m_d->spinBox->setDecimals(newNumberOfDecimals);
}

void KisAngleSelector::setMaximum(qreal newMaximum)
{
    m_d->spinBox->setMaximum(newMaximum);
}

void KisAngleSelector::setMinimum(qreal newMinimum)
{
    m_d->spinBox->setMinimum(newMinimum);
}

void KisAngleSelector::setRange(qreal newMinimum, qreal newMaximum)
{
    m_d->spinBox->setRange(newMinimum, newMaximum);
}

void KisAngleSelector::setWrapping(bool newWrapping)
{
    m_d->spinBox->setWrapping(newWrapping);
}

void KisAngleSelector::setFlipOptionsMode(FlipOptionsMode newMode)
{
    m_d->flipOptionsMode = newMode;

    m_d->toolButtonFlipOptions->setVisible(newMode == FlipOptionsMode_MenuButton);

    bool useButtons = newMode == FlipOptionsMode_Buttons;
    m_d->toolButtonFlipHorizontally->setVisible(useButtons);
    m_d->toolButtonFlipVertically->setVisible(useButtons);
    m_d->toolButtonFlipHorizontallyAndVertically->setVisible(useButtons);

    bool showMenus = newMode != FlipOptionsMode_NoFlipOptions;
    m_d->actionFlipHorizontally->setVisible(showMenus);
    m_d->actionFlipVertically->setVisible(showMenus);
    m_d->actionFlipHorizontallyAndVertically->setVisible(showMenus);
}

void KisAngleSelector::setGaugeSize(int newGaugeSize)
{
    if (newGaugeSize < 0) {
        return;
    }
    m_d->angleGaugeSize = newGaugeSize;
    m_d->resizeAngleGauge();
}

void KisAngleSelector::setIncreasingDirection(KisAngleGauge::IncreasingDirection newIncreasingDirection)
{
    m_d->angleGauge->setIncreasingDirection(newIncreasingDirection);
}

void KisAngleSelector::useFlatSpinBox(bool newUseFlatSpinBox)
{
    m_d->spinBox->setFlat(newUseFlatSpinBox);
}

void KisAngleSelector::reset()
{
    m_d->angleGauge->reset();
}

qreal KisAngleSelector::closestCoterminalAngleInRange(qreal angle, qreal minimum, qreal maximum, bool *ok)
{
    bool hasCoterminalAngleInRange = true;

    if (angle < minimum) {
        const qreal d = minimum - angle;
        const qreal cycles = std::floor(d / 360.0) + 1;
        angle += cycles * 360.0;
        if (angle > maximum) {
            hasCoterminalAngleInRange = false;
            angle = minimum;
        }
    } else if (angle > maximum) {
        const qreal d = angle - maximum;
        const qreal cycles = std::floor(d / 360.0) + 1;
        angle -= cycles * 360.0;
        if (angle < minimum) {
            hasCoterminalAngleInRange = false;
            angle = maximum;
        }
    }

    if (ok) {
        *ok = hasCoterminalAngleInRange;
    }
    return angle;
}

qreal KisAngleSelector::closestCoterminalAngleInRange(qreal angle, bool *ok) const
{
    return closestCoterminalAngleInRange(angle, m_d->spinBox->minimum(), m_d->spinBox->maximum(), ok);
}

qreal KisAngleSelector::flipAngle(qreal angle, Qt::Orientations orientations)
{
    if ((orientations & Qt::Horizontal) && (orientations & Qt::Vertical)) {
        angle += 180.0;
    } else if (orientations & Qt::Horizontal) {
        qreal a = std::fmod(angle, 360.0);
        if (a < 0) {
            a += 360.0;
        }
        if (a > 270.0) {
            angle -= 2.0 * (a - 270.0);
        } else if (a > 180.0) {
            angle += 2.0 * (270.0 - a);
        } else if (a > 90.0) {
            angle -= 2.0 * (a - 90.0);
        } else {
            angle += 2.0 * (90.0 - a);
        }
    } else if (orientations & Qt::Vertical) {
        qreal a = std::fmod(angle, 360.0);
        if (a < 0) {
            a += 360.0;
        }
        if (a > 270.0) {
            angle += 2.0 * (360.0 - a);
        } else if (a > 180.0) {
            angle -= 2.0 * (a - 180.0);
        } else if (a > 90.0) {
            angle += 2.0 * (180.0 - a);
        } else {
            angle -= 2.0 * a;
        }
    }

    return angle;
}

qreal KisAngleSelector::flipAngle(qreal angle, qreal minimum, qreal maximum, Qt::Orientations orientations, bool *ok)
{
    return closestCoterminalAngleInRange(flipAngle(angle, orientations), minimum, maximum, ok);
}

void KisAngleSelector::flip(Qt::Orientations orientations)
{
    bool ok = false;
    qreal flippedAngle = flipAngle(angle(), minimum(), maximum(), orientations, &ok);
    if (ok) {
        setAngle(flippedAngle);
    }
}

bool KisAngleSelector::event(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange) {
        KisIconUtils::updateIcon(m_d->toolButtonFlipOptions);
        KisIconUtils::updateIcon(m_d->toolButtonFlipHorizontally);
        KisIconUtils::updateIcon(m_d->toolButtonFlipVertically);
        KisIconUtils::updateIcon(m_d->toolButtonFlipHorizontallyAndVertically);
        // For some reason the spinbox, that uses stylesheets, doesn't update
        // on palette changes, so we reset the stylesheet to force an update.
        // Calling m_d->spinBox->update() doesn't work
        m_d->spinBox->refreshStyle();
    } else if (e->type() == QEvent::StyleChange || e->type() == QEvent::FontChange) {
        // Temporarily reset the spin box style so that we can get its
        // height size hint
        m_d->spinBox->refreshStyle();
        m_d->resizeAngleGauge();
    }
    return false;
}

void KisAngleSelector::Private::on_angleGauge_angleChanged(qreal angle)
{
    q->setAngle(q->closestCoterminalAngleInRange(angle));
}

void KisAngleSelector::Private::on_angleGauge_customContextMenuRequested(const QPoint &point)
{
    menuFlip->exec(angleGauge->mapToGlobal(point));
}

void KisAngleSelector::Private::on_spinBox_valueChanged(double value)
{
    KisSignalsBlocker angleGaugeSignalsBlocker(angleGauge);

    angleGauge->setAngle(value);

    emit q->angleChanged(value);
}

void KisAngleSelector::Private::on_actionFlipHorizontally_triggered()
{
    q->flip(Qt::Horizontal);
}

void KisAngleSelector::Private::on_actionFlipVertically_triggered()
{
    q->flip(Qt::Vertical);
}

void KisAngleSelector::Private::on_actionFlipHorizontallyAndVertically_triggered()
{
    q->flip(Qt::Horizontal | Qt::Vertical);
}

void KisAngleSelector::Private::resizeAngleGauge()
{
    if (angleGaugeSize == 0) {
        angleGauge->setFixedSize(spinBox->sizeHint().height(), spinBox->sizeHint().height());
    } else {
        angleGauge->setFixedSize(angleGaugeSize, angleGaugeSize);
    }
}
