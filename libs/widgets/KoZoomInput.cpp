/* SPDX-FileCopyrightText: 2008 Peter Simonsson <peter.simonsson@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoZoomInput.h"

#include <WidgetsDebug.h>
#include <klocalizedstring.h>

#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QPalette>
#include <QAbstractItemView>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>

class KoZoomInput::Private
{
    public:
        QComboBox* combo;
        QLabel* label;
        bool inside;
        bool isFlat;
};

KoZoomInput::KoZoomInput(QWidget* parent)
    : QStackedWidget(parent), d(new Private)
{
#ifdef Q_OS_MACOS
    setAttribute(Qt::WA_MacMiniSize, true);
#endif

    QWidget* first = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(first);
    layout->setSpacing(0);
    layout->setMargin(0);
    d->label = new QLabel(first);
    d->label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(d->label, 10);
    QLabel* icon = new QLabel(first);
    QStyleOption option;
    option.state = QStyle::State_Enabled;
    QPixmap pixmap(16, 16);
    pixmap.fill(QColor(255, 255, 255, 0));
    QPainter painter(&pixmap);
    painter.translate(8, 8);
    style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &option, &painter);
    icon->setPixmap(pixmap);
    layout->addWidget(icon);
    addWidget(first);
    d->combo = new QComboBox(this);
    d->combo->setMaxVisibleItems(15);
    d->combo->setEditable(true);
    d->combo->installEventFilter(this);
    addWidget(d->combo);
    d->inside = false;
    d->isFlat = true;

    connect(d->combo, SIGNAL(activated(QString)), this, SIGNAL(zoomLevelChanged(QString)));
}

KoZoomInput::~KoZoomInput()
{
    delete d;
}

void KoZoomInput::enterEvent(QEvent* event)
{
    Q_UNUSED(event);

    d->inside = true;
    if (!isFlat()) {
        return;
    }

    if (d->combo->view())
    {
        d->combo->view()->removeEventFilter(this);
    }

    setCurrentIndex(1);
}

void KoZoomInput::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);

    d->inside = false;
    if (!isFlat()) {
        return;
    }

    if(d->combo->view() && d->combo->view()->isVisible())
    {
        d->combo->view()->installEventFilter(this);
        return;
    }

    if(!d->combo->hasFocus())
        setCurrentIndex(0);
}

void KoZoomInput::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
    {
        focusNextChild();
    }
}

bool KoZoomInput::isFlat() const
{
    return d->isFlat;
}

void KoZoomInput::setFlat(bool flat)
{
    if (flat == d->isFlat) {
        return;
    }

    d->isFlat = flat;

    if (flat) {
        d->combo->installEventFilter(this);
        if (d->inside) {
            enterEvent(nullptr);
        } else {
            leaveEvent(nullptr);
        }
        d->combo->setEditable(true);
    } else {
        d->combo->removeEventFilter(this);
        if (d->combo->view()) {
            d->combo->view()->removeEventFilter(this);
        }
        d->combo->setCurrentIndex(d->combo->findText(d->label->text()));
        d->combo->setEditable(false);
        setCurrentIndex(1);
    }
}

void KoZoomInput::setZoomLevels(const QStringList& levels)
{
    d->combo->clear();
    d->combo->addItems(levels);
}

void KoZoomInput::setCurrentZoomLevel(const QString& level)
{
    d->combo->setCurrentIndex(d->combo->findText(level));
    d->label->setText(level);
}

bool KoZoomInput::eventFilter(QObject* watched, QEvent* event)
{
    if(watched == d->combo->view() && event->type() == QEvent::Hide)
    {
        focusNextChild();
        setCurrentIndex(0);
    }
    else if (watched == d->combo && event->type() == QEvent::FocusOut &&
            (d->combo->view() && !d->combo->view()->hasFocus()) && !d->inside)
    {
        setCurrentIndex(0);
    }
    return false;
}
