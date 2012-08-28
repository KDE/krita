/*  This file is part of the Calligra project, made within the KDE community.
 *
 * Copyright 2012  Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoPageNavigator.h"

#include "KoPageNavigatorButton_p.h"

#include <KoPAView.h>
#include <KoPADocument.h>
#include <KoPAPage.h>
#include <KoIcon.h>
// KDE
#include <KDebug>
#include <KLocale>
#include <KActionCollection>
// Qt
#include <QLabel>
#include <QHBoxLayout>
#include <QWheelEvent>
#include <QEvent>
#include <QLineEdit>
#include <QIntValidator>
#include <QAction>


static const int maxPageCountPattern = 999;

class KoPageNavigator::Private
{
    public:
        explicit Private(KoPAView *_view)
        : view(_view)
        {}

        // normal display
        QLabel* displayLabel;
        // interactive state
        KoPageNavigatorButton *gotoFirstPageButton;
        KoPageNavigatorButton *gotoPreviousPageButton;
        KoPageNavigatorButton *gotoNextPageButton;
        KoPageNavigatorButton *gotoLastPageButton;
        QLineEdit *pageNumberEdit;
        QIntValidator *pageNumberEditValidator;

        KoPAView *view;
};

static QString
displayText(bool isMaster, bool isSlideType, int pageNumber, int pageCount)
{
    return isSlideType ?
        (isMaster ? i18n("Master Slide %1/%2", pageNumber, pageCount)
                  : i18n("Slide %1/%2", pageNumber, pageCount)) :
        (isMaster ? i18n("Master Page %1/%2", pageNumber, pageCount)
                  : i18n("Page %1/%2", pageNumber, pageCount));
}


KoPageNavigator::KoPageNavigator(KoPAView *view)
  : QStackedWidget(view)
  , d(new Private(view))
{
    const bool isSlideType = (d->view->kopaDocument()->pageType() == KoPageApp::Slide);

#ifdef Q_WS_MAC
    setAttribute(Qt::WA_MacMiniSize, true);
#endif
    // normal display
    d->displayLabel = new QLabel(this);
    d->displayLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    addWidget(d->displayLabel);

    // add interactive variant
    QWidget* controlWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(controlWidget);
    layout->setSpacing(0);
    layout->setMargin(0);

    // the original go-*-view-page icons as set for the actions are not reused,
    // because they look too complex, at least with the Oxygen icons
    // also installing an event filter for all buttons, to get wheel events even
    // for disabled buttons
    d->gotoFirstPageButton = new KoPageNavigatorButton(koIconNameCStr("go-first-view"), this);
    d->gotoFirstPageButton->installEventFilter(this);
    d->gotoPreviousPageButton = new KoPageNavigatorButton(koIconNameCStr("go-previous-view"), this);
    d->gotoPreviousPageButton->installEventFilter(this);
    d->gotoNextPageButton = new KoPageNavigatorButton(koIconNameCStr("go-next-view"), this);
    d->gotoNextPageButton->installEventFilter(this);
    d->gotoLastPageButton = new KoPageNavigatorButton(koIconNameCStr("go-last-view"), this);
    d->gotoLastPageButton->installEventFilter(this);

    d->pageNumberEdit = new QLineEdit(this);
    d->pageNumberEdit->installEventFilter(this);
    d->pageNumberEditValidator = new QIntValidator(d->pageNumberEdit);
    d->pageNumberEditValidator->setBottom(1);
    d->pageNumberEdit->setValidator(d->pageNumberEditValidator);
    d->pageNumberEdit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    connect(d->pageNumberEdit, SIGNAL(returnPressed()), SLOT(onPageNumberEntered()));

    layout->addWidget(d->gotoFirstPageButton);
    layout->addWidget(d->gotoPreviousPageButton);
    layout->addWidget(d->pageNumberEdit);
    layout->addWidget(d->gotoNextPageButton);
    layout->addWidget(d->gotoLastPageButton);

    addWidget(controlWidget);

    KoPADocument *const kopaDocument = d->view->kopaDocument();
    connect(kopaDocument, SIGNAL(pageAdded(KoPAPageBase*)), SLOT(updateDisplayLabel()));
    connect(kopaDocument, SIGNAL(pageRemoved(KoPAPageBase*)), SLOT(updateDisplayLabel()));
    connect(d->view->proxyObject, SIGNAL(activePageChanged()), SLOT(updateDisplayLabel()));

    // Fix width by the largest needed
    QFontMetrics fontMetrics(font());
    d->pageNumberEdit->setMinimumWidth(fontMetrics.width(QString::number(maxPageCountPattern*10))); //one more
    const int editWidth = widget(Edit)->minimumWidth();
    const int normalWidth = fontMetrics.width(displayText(false, isSlideType, maxPageCountPattern, maxPageCountPattern));
    const int masterWidth = fontMetrics.width(displayText(true, isSlideType, maxPageCountPattern, maxPageCountPattern));
    setFixedWidth(qMax(editWidth, qMax(normalWidth, masterWidth)));

    updateDisplayLabel();
}

KoPageNavigator::~KoPageNavigator()
{
    delete d;
}

void KoPageNavigator::initActions()
{
    KActionCollection *actionCollection = d->view->actionCollection();

    d->gotoFirstPageButton->setAction(actionCollection->action(QLatin1String("page_first")));
    d->gotoPreviousPageButton->setAction(actionCollection->action(QLatin1String("page_previous")));
    d->gotoNextPageButton->setAction(actionCollection->action(QLatin1String("page_next")));
    d->gotoLastPageButton->setAction(actionCollection->action(QLatin1String("page_last")));

}

void KoPageNavigator::enterEvent(QEvent *event)
{
    Q_UNUSED(event);

    setCurrentIndex(Edit);
}

void KoPageNavigator::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

    if (! d->pageNumberEdit->hasFocus()) {
        setCurrentIndex(Display);
    }
}

bool KoPageNavigator::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusOut && watched == d->pageNumberEdit) {
        if (! underMouse()) {
            setCurrentIndex(Display);
        }

        // reset editor in any case
        KoPADocument *const kopaDocument = d->view->kopaDocument();
        KoPAPageBase *const activePage = d->view->activePage();
        const int pageNumber = kopaDocument->pageIndex(activePage) + 1;
        const QString text = (pageNumber > 0) ? QString::number(pageNumber) : QString();
        d->pageNumberEdit->setText(text);
    } else if (event->type() == QEvent::Wheel) {
        // Scroll the pages by the wheel
        // Because the numbers are representatives of the actual pages
        // and the list of pages is ordered by smaller number first,
        // here an increasing delta means going up in the list, so go to
        // smaller page numbers, and vice versa.
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        const int delta = wheelEvent->delta();

        // trigger the respective actions
        if (delta > 0) {
            QAction *gotoPreviousPageAction = d->gotoPreviousPageButton->action();
            if (gotoPreviousPageAction->isEnabled()) {
                gotoPreviousPageAction->activate(QAction::Trigger);
            }
        } else if (delta < 0) {
            QAction *gotoNextPageAction = d->gotoNextPageButton->action();
            if (gotoNextPageAction->isEnabled()) {
                gotoNextPageAction->activate(QAction::Trigger);
            }
        }

        // scroll wheel events also cancel the editing,
        // so move focus out of the pageNumberEdit
        if (d->pageNumberEdit->hasFocus()) {
            d->view->setFocus();
        }
    }

    return false;
}

void KoPageNavigator::updateDisplayLabel()
{
    KoPADocument *const kopaDocument = d->view->kopaDocument();
    KoPAPageBase *const activePage = d->view->activePage();
    const int pageNumber = kopaDocument->pageIndex(activePage) + 1;

    if (pageNumber > 0) {
        const bool isMasterPage = (dynamic_cast<KoPAPage*>(activePage) == 0);

        const int pageCount = d->view->kopaDocument()->pages(isMasterPage).size();

        const bool isSlideType = (d->view->kopaDocument()->pageType() == KoPageApp::Slide);

        d->displayLabel->setText(displayText(isMasterPage, isSlideType, pageNumber, pageCount));

        d->pageNumberEdit->setText(QString::number(pageNumber));
        d->pageNumberEditValidator->setTop(pageCount);
    }

    // also leave the editor if in it
    if (d->pageNumberEdit->hasFocus()) {
        d->view->setFocus();
    }
}

void KoPageNavigator::onPageNumberEntered()
{
    const int pageNumber = d->pageNumberEdit->text().toInt();

    KoPADocument *const kopaDocument = d->view->kopaDocument();
    KoPAPageBase *const activePage = d->view->activePage();

    const bool isMasterPage = (dynamic_cast<KoPAPage*>(activePage) == 0);

    const QList<KoPAPageBase*> pages = kopaDocument->pages(isMasterPage);

    KoPAPageBase* newPage = pages.value(pageNumber-1);
    if (newPage) {
        d->view->proxyObject->updateActivePage(newPage);
    }
}
