/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 1999 Daniel M. Duley <mosfet@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "KoDualColorButton.h"
#include "KoColor.h"
#include "KoColorDisplayRendererInterface.h"
#include <kcolormimedata.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "dcolorarrow.xbm"
#include "dcolorreset.xpm"

#include <QColorDialog>
#include "KisDlgInternalColorSelector.h"

#include "kis_signals_blocker.h"

#include <QBrush>
#include <QDrag>
#include <QDragEnterEvent>
#include <QPainter>
#include <QPointer>
#include <qdrawutil.h>
#include <QApplication>

class Q_DECL_HIDDEN KoDualColorButton::Private
{
public:
    Private(const KoColor &fgColor,
            const KoColor &bgColor,
            QWidget *_dialogParent,
            const KoColorDisplayRendererInterface *_displayRenderer)
        : dialogParent(_dialogParent)
        , dragFlag( false )
        , miniCtlFlag( false )
        , foregroundColor(fgColor)
        , backgroundColor(bgColor)
        , displayRenderer(_displayRenderer)
    {
        updateArrows();
        resetPixmap = QPixmap( (const char **)dcolorreset_xpm );
    }

    void updateArrows() {
        arrowBitmap = QPixmap(12,12);
        arrowBitmap.fill(Qt::transparent);

        QPainter p(&arrowBitmap);
        p.setPen(dialogParent->palette().windowText().color());

        // arrow pointing left
        p.drawLine(0, 3, 7, 3);
        p.drawLine(1, 2, 1, 4);
        p.drawLine(2, 1, 2, 5);
        p.drawLine(3, 0, 3, 6);

        // arrow pointing down
        p.drawLine(8, 4, 8, 11);
        p.drawLine(5, 8, 11, 8);
        p.drawLine(6, 9, 10, 9);
        p.drawLine(7, 10, 9, 10);
    }

    QWidget* dialogParent;

    QPixmap arrowBitmap;
    QPixmap resetPixmap;
    bool dragFlag, miniCtlFlag;

    KoColor foregroundColor;
    KoColor backgroundColor;

    KisDlgInternalColorSelector *fgColorSelectorDialog;
    KisDlgInternalColorSelector *bgColorSelectorDialog;

    QPoint dragPosition;
    Selection tmpSelection;
    bool popDialog {true};
    QPointer<const KoColorDisplayRendererInterface> displayRenderer;

    void init(KoDualColorButton *q);
};

void KoDualColorButton::Private::init(KoDualColorButton *q)
{
    if ( q->sizeHint().isValid() )
        q->setMinimumSize( q->sizeHint() );

    q->setAcceptDrops( true );
    QString caption = i18n("Select a Color");
    KisDlgInternalColorSelector::Config config = KisDlgInternalColorSelector::Config();
    config.modal = false;
    fgColorSelectorDialog = new KisDlgInternalColorSelector(q, foregroundColor, config, caption, displayRenderer);
    fgColorSelectorDialog->setObjectName("Foreground");

    bgColorSelectorDialog = new KisDlgInternalColorSelector(q, foregroundColor, config, caption, displayRenderer);
    bgColorSelectorDialog->setObjectName("Background");

    connect(fgColorSelectorDialog, SIGNAL(colorChosen(KoColor)), q, SLOT(slotSetForeGroundColorFromDialog(KoColor)));
    connect(q, SIGNAL(foregroundColorChanged(KoColor)), q, SLOT(setForegroundColor(KoColor)));

    connect(bgColorSelectorDialog, SIGNAL(colorChosen(KoColor)), q, SLOT(slotSetBackGroundColorFromDialog(KoColor)));
    connect(q, SIGNAL(backgroundColorChanged(KoColor)), q, SLOT(setBackgroundColor(KoColor)));
}



KoDualColorButton::KoDualColorButton(const KoColor &foregroundColor, const KoColor &backgroundColor, QWidget *parent, QWidget* dialogParent )
    : QWidget( parent ),
      d( new Private(foregroundColor, backgroundColor,
                     dialogParent,
                     KoDumbColorDisplayRenderer::instance()) )
{
    d->init(this);
}

KoDualColorButton::KoDualColorButton(const KoColor &foregroundColor, const KoColor &backgroundColor,
                                     const KoColorDisplayRendererInterface *displayRenderer,
                                     QWidget *parent, QWidget* dialogParent)
    : QWidget( parent ),
      d( new Private(foregroundColor, backgroundColor,
                     dialogParent,
                     displayRenderer) )
{
    d->init(this);
}

KoDualColorButton::~KoDualColorButton()
{
    delete d;
}

KoColor KoDualColorButton::foregroundColor() const
{
    return d->foregroundColor;
}

KoColor KoDualColorButton::backgroundColor() const
{
    return d->backgroundColor;
}

bool KoDualColorButton::popDialog() const
{
    return d->popDialog;
}

QSize KoDualColorButton::sizeHint() const
{
    return QSize(34, 34);
}

void KoDualColorButton::setForegroundColor(const KoColor &color)
{
    d->foregroundColor = color;
    {
        /**
         * The internal color selector might emit the color of a different profile, so
         * we should break this cycling dependency somehow.
         */
        KisSignalsBlocker b(d->fgColorSelectorDialog);
        d->fgColorSelectorDialog->setPreviousColor(color);
        d->fgColorSelectorDialog->slotColorUpdated(color);
    }
    update();
}

void KoDualColorButton::setBackgroundColor( const KoColor &color )
{
    d->backgroundColor = color;
    {
        /**
         * The internal color selector might emit the color of a different profile, so
         * we should break this cycling dependency somehow.
         */
        KisSignalsBlocker b(d->bgColorSelectorDialog);
        d->bgColorSelectorDialog->setPreviousColor(color);
        d->bgColorSelectorDialog->slotColorUpdated(color);
    }
    update();
}

void KoDualColorButton::setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer)
{
    if (d->displayRenderer && d->displayRenderer != KoDumbColorDisplayRenderer::instance()) {
        d->displayRenderer->disconnect(this);
    }
    if (displayRenderer) {
        d->displayRenderer = displayRenderer;
        KisSignalsBlocker b(this,
                            d->fgColorSelectorDialog,
                            d->bgColorSelectorDialog);
        d->fgColorSelectorDialog->setDisplayRenderer(displayRenderer);
        d->bgColorSelectorDialog->setDisplayRenderer(displayRenderer);
        connect(d->displayRenderer, SIGNAL(destroyed()), this, SLOT(setDisplayRenderer()), Qt::UniqueConnection);
        connect(d->displayRenderer, SIGNAL(displayConfigurationChanged()), this, SLOT(update()));
    } else {
        d->displayRenderer = KoDumbColorDisplayRenderer::instance();
    }
}

void KoDualColorButton::setColorSpace(const KoColorSpace *cs)
{
    KisSignalsBlocker b(this,
                        d->fgColorSelectorDialog,
                        d->bgColorSelectorDialog);
    d->bgColorSelectorDialog->setColorSpace(cs);
    d->fgColorSelectorDialog->setColorSpace(cs);
}

QColor KoDualColorButton::getColorFromDisplayRenderer(KoColor c)
{
    QColor col;
    if (d->displayRenderer) {
        c.convertTo(d->displayRenderer->getPaintingColorSpace());
        col = d->displayRenderer->toQColor(c);
    } else {
        col = c.toQColor();
    }
    return col;
}

void KoDualColorButton::setPopDialog( bool popDialog )
{
    d->popDialog = popDialog;
}

void KoDualColorButton::metrics( QRect &foregroundRect, QRect &backgroundRect )
{
    foregroundRect = QRect( 0, 0, width() - 14, height() - 14 );
    backgroundRect = QRect( 14, 14, width() - 14, height() - 14 );
}

void KoDualColorButton::paintEvent(QPaintEvent *)
{
    QRect foregroundRect;
    QRect backgroundRect;

    QPainter painter( this );

    metrics( foregroundRect, backgroundRect );

    QBrush defBrush = palette().brush( QPalette::Button );
    QBrush foregroundBrush( getColorFromDisplayRenderer(d->foregroundColor), Qt::SolidPattern );
    QBrush backgroundBrush( getColorFromDisplayRenderer(d->backgroundColor), Qt::SolidPattern );

    qDrawShadeRect( &painter, backgroundRect, palette(), false, 1, 0,
                    isEnabled() ? &backgroundBrush : &defBrush );

    qDrawShadeRect( &painter, foregroundRect, palette(), false, 1, 0,
                    isEnabled() ? &foregroundBrush : &defBrush );

    painter.setPen( palette().color( QPalette::Shadow ) );

    painter.drawPixmap( foregroundRect.right() + 2, 1, d->arrowBitmap );
    painter.drawPixmap( 1, foregroundRect.bottom() + 2, d->resetPixmap );
}

void KoDualColorButton::dragEnterEvent( QDragEnterEvent *event )
{
    event->setAccepted( isEnabled() && KColorMimeData::canDecode( event->mimeData() ) );
}

void KoDualColorButton::dropEvent( QDropEvent *event )
{
    Q_UNUSED(event);
    /*  QColor color = KColorMimeData::fromMimeData( event->mimeData() );

  if ( color.isValid() ) {
    if ( d->selection == Foreground ) {
      d->foregroundColor = color;
      emit foregroundColorChanged( color );
    } else {
      d->backgroundColor = color;
      emit backgroundColorChanged( color );
    }

    update();
  }
*/
}

void KoDualColorButton::slotSetForeGroundColorFromDialog(const KoColor color)
{
    d->foregroundColor = color;
    update();
    emit foregroundColorChanged(d->foregroundColor);
}

void KoDualColorButton::slotSetBackGroundColorFromDialog(const KoColor color)
{
    d->backgroundColor = color;
    update();
    emit backgroundColorChanged(d->backgroundColor);
}


void KoDualColorButton::openForegroundDialog()
{
    KisSignalsBlocker b(this);
    KisSignalsBlocker b1(d->fgColorSelectorDialog);
    d->fgColorSelectorDialog->setPreviousColor(d->foregroundColor);
    d->fgColorSelectorDialog->show();
    update();
}

void KoDualColorButton::openBackgroundDialog()
{    KisSignalsBlocker b(this);
     KisSignalsBlocker b1(d->bgColorSelectorDialog);
    d->bgColorSelectorDialog->setPreviousColor(d->backgroundColor);
    d->bgColorSelectorDialog->show();
    update();
}

void KoDualColorButton::mousePressEvent( QMouseEvent *event )
{
    QRect foregroundRect;
    QRect backgroundRect;

    metrics( foregroundRect, backgroundRect );

    d->dragPosition = event->pos();

    d->dragFlag = false;

    if ( foregroundRect.contains( d->dragPosition ) ) {
        d->tmpSelection = Foreground;
        d->miniCtlFlag = false;
    }
    else if( backgroundRect.contains( d->dragPosition ) ) {
        d->tmpSelection = Background;
        d->miniCtlFlag = false;
    }
    else if ( event->pos().x() > foregroundRect.width() ) {
        // We handle the swap and reset controls as soon as the mouse is
        // is pressed and ignore further events on this click (mosfet).

        KoColor tmp = d->foregroundColor;
        d->foregroundColor = d->backgroundColor;
        d->backgroundColor = tmp;

        emit backgroundColorChanged( d->backgroundColor );
        emit foregroundColorChanged( d->foregroundColor );

        d->miniCtlFlag = true;
    }
    else if ( event->pos().x() < backgroundRect.x() ) {
        d->foregroundColor = d->displayRenderer->approximateFromRenderedQColor(Qt::black);
        d->backgroundColor = d->displayRenderer->approximateFromRenderedQColor(Qt::white);

        emit backgroundColorChanged( d->backgroundColor );
        emit foregroundColorChanged( d->foregroundColor );

        d->miniCtlFlag = true;
    }
    update();
}


void KoDualColorButton::mouseMoveEvent( QMouseEvent *event )
{
    if ( !d->miniCtlFlag ) {
        int delay = QApplication::startDragDistance();

        if ( event->x() >= d->dragPosition.x() + delay || event->x() <= d->dragPosition.x() - delay ||
             event->y() >= d->dragPosition.y() + delay || event->y() <= d->dragPosition.y() - delay ) {
            KColorMimeData::createDrag( d->tmpSelection == Foreground ?
                                            getColorFromDisplayRenderer(d->foregroundColor) :
                                            getColorFromDisplayRenderer(d->backgroundColor),
                                        this )->exec();
            d->dragFlag = true;
        }
    }
}

void KoDualColorButton::mouseReleaseEvent( QMouseEvent *event )
{
    d->dragFlag = false;

    if ( d->miniCtlFlag )
        return;

    d->miniCtlFlag = false;

    QRect foregroundRect;
    QRect backgroundRect;
    metrics( foregroundRect, backgroundRect );

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("colorselector");
    bool usePlatformDialog = cfg.readEntry("UsePlatformColorDialog", false);

    if (foregroundRect.contains( event->pos())) {
        if (d->tmpSelection == Foreground) {
            if (d->popDialog) {
                if (usePlatformDialog) {
                    QColor c = d->foregroundColor.toQColor();
                    c = QColorDialog::getColor(c, this);
                    if (c.isValid()) {
                        d->foregroundColor = d->displayRenderer->approximateFromRenderedQColor(c);
                        emit foregroundColorChanged(d->foregroundColor);
                    }
                }
                else {
                    d->fgColorSelectorDialog->setPreviousColor(d->foregroundColor);
                    d->fgColorSelectorDialog->show();
                }
            }
        }
        else {
            d->foregroundColor = d->backgroundColor;
            emit foregroundColorChanged( d->foregroundColor );
        }
    }
    else if (backgroundRect.contains( event->pos())) {
        if(d->tmpSelection == Background ) {
            if( d->popDialog) {
                if (usePlatformDialog) {
                    QColor c = d->backgroundColor.toQColor();
                    c = QColorDialog::getColor(c, this);
                    if (c.isValid()) {
                        d->backgroundColor = d->displayRenderer->approximateFromRenderedQColor(c);
                        emit backgroundColorChanged(d->backgroundColor);
                    }
                }
                else {
                    KoColor c = d->backgroundColor;
                    d->bgColorSelectorDialog->setPreviousColor(d->backgroundColor);
                    d->bgColorSelectorDialog->show();

                }
            }
        } else {
            d->backgroundColor = d->foregroundColor;
            emit backgroundColorChanged( d->backgroundColor );
        }
    }

    update();
}

void KoDualColorButton::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    switch (event->type()) {
    case QEvent::StyleChange:
    case QEvent::PaletteChange:
        d->updateArrows();
    default:
        break;
    }
}

bool KoDualColorButton::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QRect foregroundRect;
        QRect backgroundRect;
        metrics( foregroundRect, backgroundRect );

        if (this->mapFromGlobal(QCursor::pos()).x() < backgroundRect.x() ) {
            if (this->mapFromGlobal(QCursor::pos()).y() < backgroundRect.y()){
                this->setToolTip(i18n("Foreground color selector"));
            }
            else{
                this->setToolTip(i18n("Set foreground and background colors to black and white"));
            }
        }
        else {
            if (this->mapFromGlobal(QCursor::pos()).y() < backgroundRect.y() ) {
                this->setToolTip(i18n("Swap foreground and background colors"));
            }
            else{
                this->setToolTip(i18n("Background color selector"));
            }
        }
    }
    return QWidget::event(event);

}
