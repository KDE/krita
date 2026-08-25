/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 1999 Daniel M. Duley <mosfet@kde.org>
   SPDX-FileCopyrightText: 2026 Arkady Flury <arkady.flury@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "KoDualColorButton.h"
#include "KoColor.h"
#include "KoColorDisplayRendererInterface.h"
#include <kcolormimedata.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "dcolorreset.xpm"

#include <QColorDialog>
#include "KisDlgInternalColorSelector.h"

#include "kis_signals_blocker.h"
#include <kis_icon.h>
#include "kis_canvas_resource_provider.h"

#include <QBrush>
#include <QDrag>
#include <QDragEnterEvent>
#include <QPainter>
#include <QPointer>
#include <qdrawutil.h>
#include <QApplication>
#include <kis_signal_auto_connection.h>

class KoDualColorButton::Private
{
public:
    Private(const KoColor &fgColor, const KoColor &bgColor,
            QWidget *_dialogParent,
            const KoColorDisplayRendererInterface *_displayRenderer)
        : dialogParent(_dialogParent)
        , dragFlag( false )
        , miniCtlFlag( false )
        , foregroundColor(fgColor)
        , backgroundColor(bgColor)
        , displayRenderer(_displayRenderer)
    {
        popDialog = true;
    }

    QWidget* dialogParent;

    bool dragFlag, miniCtlFlag;
    KoColor foregroundColor;
    KoColor backgroundColor;
    KisDlgInternalColorSelector *colorSelectorDialog;
    struct ColorDialogState {
        KisSignalAutoConnectionsStore connections;
        Selection selection = Foreground;
    };
    std::optional<ColorDialogState> colorDialogState;

    QPoint dragPosition;
    Selection tmpSelection;
    bool popDialog;
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
    colorSelectorDialog = new KisDlgInternalColorSelector(q, foregroundColor, config, caption, displayRenderer);
    connect(colorSelectorDialog, &KisDlgInternalColorSelector::finished, q, &KoDualColorButton::slotColorDialogClosed);

}

KoDualColorButton::KoDualColorButton(KisCanvasResourceProvider *canvasResourceProvider,
                                     const KoColorDisplayRendererInterface *displayRenderer,
                                     QWidget *parent, QWidget* dialogParent)
    : QWidget( parent ),
      d( new Private(canvasResourceProvider->fgColor(),canvasResourceProvider->bgColor(),
                     dialogParent,
                     displayRenderer) )
{
    d->init(this);
}

KoDualColorButton::~KoDualColorButton()
{
    delete d;
}

void KoDualColorButton::updateArrows() {
    resetPixmap = QPixmap( (const char **)dcolorreset_xpm );

    QColor windowText = d->dialogParent->palette().windowText().color();

    arrowBitmap = KisIconUtils::loadIcon("swap_arrow").pixmap(12,12);
    
    QPainter pen(&arrowBitmap);
    
    pen.setCompositionMode(QPainter::CompositionMode_SourceIn);
    pen.fillRect(arrowBitmap.rect(),windowText);
    pen.end();

    resetArrowPixmap = KisIconUtils::loadIcon("reset_arrow").pixmap(10,12);
    
    pen.begin(&resetArrowPixmap);

    pen.setCompositionMode(QPainter::CompositionMode_SourceIn);
    pen.fillRect(resetArrowPixmap.rect(),windowText);
    pen.end();
}

void KoDualColorButton::setColorDialogState(Selection state){
    if (d->colorDialogState.has_value()) {
        d->colorDialogState->selection = state;
    }
    d->tmpSelection = state;
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
    if (d->colorDialogState && d->colorDialogState->selection == Foreground) {
        /**
         * The internal color selector might Q_EMIT the color of a different profile, so
         * we should break this cycling dependency somehow.
         */
        KisSignalsBlocker b(d->colorSelectorDialog);
        d->colorSelectorDialog->slotColorUpdated(color);
    }
    update();
}

void KoDualColorButton::setBackgroundColor( const KoColor &color )
{
    d->backgroundColor = color;

    if (d->colorDialogState && d->colorDialogState->selection == Background) {
        /**
         * The internal color selector might Q_EMIT the color of a different profile, so
         * we should break this cycling dependency somehow.
         */
        KisSignalsBlocker b(d->colorSelectorDialog);
        d->colorSelectorDialog->slotColorUpdated(color);
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
        d->colorSelectorDialog->setDisplayRenderer(displayRenderer);
        connect(d->displayRenderer, SIGNAL(destroyed()), this, SLOT(setDisplayRenderer()), Qt::UniqueConnection);
        connect(d->displayRenderer, SIGNAL(displayConfigurationChanged()), this, SLOT(update()));
    } else {
        d->displayRenderer = KoDumbColorDisplayRenderer::instance();
    }
}

void KoDualColorButton::updateColorSpace()
{
    d->colorSelectorDialog->lockUsedColorSpace(d->displayRenderer->getPaintingColorSpace());
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

void KoDualColorButton::paintEvent(QPaintEvent *)
{

    QPainter painter( this );

    QBrush defBrush = palette().brush( QPalette::Button );

    QBrush foregroundBrush( getColorFromDisplayRenderer(d->foregroundColor), Qt::SolidPattern );
    QBrush backgroundBrush( getColorFromDisplayRenderer(d->backgroundColor), Qt::SolidPattern );

    if (resetColours) {
        foregroundBrush = Qt::black;
        backgroundBrush = Qt::white;
    }

    qDrawShadeRect( &painter, backgroundRect, palette(), false, 1, 0,
                    isEnabled() ? &backgroundBrush : &defBrush );

    qDrawShadeRect( &painter, foregroundRect, palette(), false, 1, 0,
                    isEnabled() ? &foregroundBrush : &defBrush );

    painter.setPen( palette().color( QPalette::Shadow ) );

    paint_icons(painter);
}

void KoDualColorButton::paint_icons(QPainter &painter){
    painter.drawPixmap( foregroundRect.right() + 2, 1, arrowBitmap );
    painter.drawPixmap( 1, foregroundRect.bottom() + 2, resetPixmap );
}


void KoDualColorButton::dragEnterEvent( QDragEnterEvent *event )
{
    event->setAccepted( isEnabled() && KColorMimeData::canDecode( event->mimeData() ) );
}

void KoDualColorButton::dropEvent( QDropEvent *event )
{
    Q_UNUSED(event);
}

void KoDualColorButton::slotSetForegroundColorFromDialog(const KoColor color)
{
    d->foregroundColor = color;
    update();
    Q_EMIT foregroundColorChanged(d->foregroundColor);
}

void KoDualColorButton::slotSetBackgroundColorFromDialog(const KoColor color)
{
    d->backgroundColor = color;
    update();
    Q_EMIT backgroundColorChanged(d->backgroundColor);
}

void KoDualColorButton::slotColorDialogClosed()
{
    d->colorDialogState = std::nullopt;
}

void KoDualColorButton::openForegroundDialog()
{
    d->colorDialogState.emplace();
    d->colorDialogState->selection = Foreground;
    // TODO: fix cyclic connections in a proper way
    d->colorDialogState->connections.addUniqueConnection(d->colorSelectorDialog, SIGNAL(signalForegroundColorChosen(KoColor)), this, SLOT(slotSetForegroundColorFromDialog(KoColor)));
    d->colorDialogState->connections.addUniqueConnection(this, SIGNAL(foregroundColorChanged(KoColor)), d->colorSelectorDialog, SLOT(slotColorUpdated(KoColor)));
    d->colorSelectorDialog->slotColorUpdated(d->foregroundColor);
    d->colorSelectorDialog->setPreviousColor(d->foregroundColor);
    d->colorSelectorDialog->show();
    update();
}

void KoDualColorButton::openBackgroundDialog()
{
    d->colorDialogState.emplace();
    d->colorDialogState->selection = Background;
    // TODO: fix cyclic connections in a proper way
    d->colorDialogState->connections.addUniqueConnection(d->colorSelectorDialog, SIGNAL(signalForegroundColorChosen(KoColor)), this, SLOT(slotSetBackgroundColorFromDialog(KoColor)));
    d->colorDialogState->connections.addUniqueConnection(this, SIGNAL(backgroundColorChanged(KoColor)), d->colorSelectorDialog, SLOT(slotColorUpdated(KoColor)));
    d->colorSelectorDialog->slotColorUpdated(d->backgroundColor);
    d->colorSelectorDialog->setPreviousColor(d->backgroundColor);
    d->colorSelectorDialog->show();
    update();
}

void KoDualColorButton::backgroundSelect(bool usePlatformDialog)
{
    if( d->popDialog) {
        if (usePlatformDialog) {
            QColor c = d->backgroundColor.toQColor();
            c = QColorDialog::getColor(c, this);
            if (c.isValid()) {
                d->backgroundColor = d->displayRenderer->approximateFromRenderedQColor(c);
                Q_EMIT backgroundColorChanged(d->backgroundColor);
            }
        }
        else {
            openBackgroundDialog();
        }
    }
}

void KoDualColorButton::foregroundSelect(bool usePlatformDialog)
{
    if( d->popDialog) {
        if (usePlatformDialog) {
            QColor c = d->foregroundColor.toQColor();
            c = QColorDialog::getColor(c, this);
            if (c.isValid()) {
                d->foregroundColor = d->displayRenderer->approximateFromRenderedQColor(c);
                Q_EMIT foregroundColorChanged(d->foregroundColor);
            }
        }
        else {
            openForegroundDialog();
        }
    }
}

void KoDualColorButton::mousePressEvent( QMouseEvent *event )
{

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
        swapColours();

        d->miniCtlFlag = true;
    }
    else if ( event->pos().x() < backgroundRect.x() ) {
        ResetColours();
        
        d->miniCtlFlag = true;
    }
    update();
}


void KoDualColorButton::ResetColours()
{
    d->foregroundColor = d->displayRenderer->approximateFromRenderedQColor(Qt::black);
    d->backgroundColor = d->displayRenderer->approximateFromRenderedQColor(Qt::white);

    Q_EMIT backgroundColorChanged( d->backgroundColor );
    Q_EMIT foregroundColorChanged( d->foregroundColor );
}

void KoDualColorButton::swapColours()
{
    KoColor tmp = d->foregroundColor;
    d->foregroundColor = d->backgroundColor;
    d->backgroundColor = tmp;

    Q_EMIT backgroundColorChanged( d->backgroundColor );
    Q_EMIT foregroundColorChanged( d->foregroundColor );
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


    KConfigGroup cfg =  KSharedConfig::openConfig()->group("colorselector");
    bool usePlatformDialog = cfg.readEntry("UsePlatformColorDialog", false);

    if (foregroundRect.contains( event->pos())) {
        if (d->tmpSelection == Foreground) {
            foregroundSelect(usePlatformDialog);
        }
        else {
            d->foregroundColor = d->backgroundColor;
            Q_EMIT foregroundColorChanged( d->foregroundColor );
        }
    }
    else if (backgroundRect.contains( event->pos())) {
        if(d->tmpSelection == Background ) {
            backgroundSelect(usePlatformDialog);
        } else {
            d->backgroundColor = d->foregroundColor;
            Q_EMIT backgroundColorChanged( d->backgroundColor );
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
    updateArrows();
    default:
        break;
    }
}

bool KoDualColorButton::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {

        const QHelpEvent *helpEvent = static_cast<QHelpEvent*>(event);

        if (this->mapFromGlobal(helpEvent->globalPos()).x() < backgroundRect.x() ) {
            if (this->mapFromGlobal(helpEvent->globalPos()).y() < backgroundRect.y()){
                this->setToolTip(i18n("Foreground color selector"));
            }
            else{
                this->setToolTip(i18n("Set foreground and background colors to black and white"));
            }
        }
        else {
            if (this->mapFromGlobal(helpEvent->globalPos()).y() < backgroundRect.y() ) {
                this->setToolTip(i18n("Swap foreground and background colors"));
            }
            else{
                this->setToolTip(i18n("Background color selector"));
            }
        }
    }
    return QWidget::event(event);

}

