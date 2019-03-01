/* This file is part of the KDE project
 * Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
 * Copyright (C) 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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

#include "KoFillConfigWidget.h"

#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QLabel>
#include <QSizePolicy>
#include <QBitmap>
#include <QAction>
#include <QSharedPointer>

#include <klocalizedstring.h>

#include <KoIcon.h>
#include <KoColor.h>
#include <KoColorPopupAction.h>
#include "KoResourceServerProvider.h"
#include "KoResourceServerAdapter.h"
#include <KoSelection.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoDocumentResourceManager.h>
#include <KoShape.h>
#include <KoShapeController.h>
#include <KoShapeBackground.h>
#include <KoShapeBackgroundCommand.h>
#include <KoShapeStrokeCommand.h>
#include <KoShapeStroke.h>
#include <KoSelectedShapesProxy.h>
#include <KoColorBackground.h>
#include <KoGradientBackground.h>
#include <KoPatternBackground.h>
#include <KoImageCollection.h>
#include <KoResourcePopupAction.h>
#include "KoZoomHandler.h"
#include "KoColorPopupButton.h"
#include "ui_KoFillConfigWidget.h"
#include <kis_signals_blocker.h>
#include <kis_signal_compressor.h>
#include <kis_acyclic_signal_connector.h>
#include <kis_assert.h>
#include "kis_canvas_resource_provider.h"
#include <KoStopGradient.h>
#include <QInputDialog>
#include <KoShapeFillWrapper.h>

#include "kis_global.h"
#include "kis_debug.h"

static const char* const buttonnone[]={
    "16 16 3 1",
    "# c #000000",
    "e c #ff0000",
    "- c #ffffff",
    "################",
    "#--------------#",
    "#-e----------e-#",
    "#--e--------e--#",
    "#---e------e---#",
    "#----e----e----#",
    "#-----e--e-----#",
    "#------ee------#",
    "#------ee------#",
    "#-----e--e-----#",
    "#----e----e----#",
    "#---e------e---#",
    "#--e--------e--#",
    "#-e----------e-#",
    "#--------------#",
    "################"};

static const char* const buttonsolid[]={
    "16 16 2 1",
    "# c #000000",
    ". c #969696",
    "################",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "################"};


// FIXME: Smoother gradient button.

static const char* const buttongradient[]={
    "16 16 15 1",
    "# c #000000",
    "n c #101010",
    "m c #202020",
    "l c #303030",
    "k c #404040",
    "j c #505050",
    "i c #606060",
    "h c #707070",
    "g c #808080",
    "f c #909090",
    "e c #a0a0a0",
    "d c #b0b0b0",
    "c c #c0c0c0",
    "b c #d0d0d0",
    "a c #e0e0e0",
    "################",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "################"};

static const char* const buttonpattern[]={
    "16 16 4 1",
    ". c #0a0a0a",
    "# c #333333",
    "a c #a0a0a0",
    "b c #ffffffff",
    "################",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#bbbbbaaaabbbbb#",
    "#bbbbbaaaabbbbb#",
    "#bbbbbaaaabbbbb#",
    "#bbbbbaaaabbbbb#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "################"};

class Q_DECL_HIDDEN KoFillConfigWidget::Private
{
public:
    Private(KoFlake::FillVariant _fillVariant)
    : canvas(0),
      colorChangedCompressor(100, KisSignalCompressor::FIRST_ACTIVE),
      gradientChangedCompressor(100, KisSignalCompressor::FIRST_ACTIVE),
      fillVariant(_fillVariant),
      noSelectionTrackingMode(false)
    {
    }

    KoColorPopupAction *colorAction;
    KoResourcePopupAction *gradientAction;
    KoResourcePopupAction *patternAction;
    QButtonGroup *group;

    KoCanvasBase *canvas;

    KisSignalCompressor colorChangedCompressor;
    KisAcyclicSignalConnector shapeChangedAcyclicConnector;
    KisAcyclicSignalConnector resourceManagerAcyclicConnector;
    KoFillConfigWidget::StyleButton selectedFillIndex;

    QSharedPointer<KoStopGradient> activeGradient;
    KisSignalCompressor gradientChangedCompressor;
    KoFlake::FillVariant fillVariant;

    bool noSelectionTrackingMode;

    Ui_KoFillConfigWidget *ui;

    std::vector<KisAcyclicSignalConnector::Blocker> deactivationLocks;
};

KoFillConfigWidget::KoFillConfigWidget(KoCanvasBase *canvas, KoFlake::FillVariant fillVariant, bool trackShapeSelection, QWidget *parent)
    :  QWidget(parent)
    , d(new Private(fillVariant))
{
    d->canvas = canvas;

    if (trackShapeSelection) {
        d->shapeChangedAcyclicConnector.connectBackwardVoid(
                    d->canvas->selectedShapesProxy(), SIGNAL(selectionChanged()),
                    this, SLOT(shapeChanged()));

        d->shapeChangedAcyclicConnector.connectBackwardVoid(
                    d->canvas->selectedShapesProxy(), SIGNAL(selectionContentChanged()),
                    this, SLOT(shapeChanged()));
    }

    d->resourceManagerAcyclicConnector.connectBackwardResourcePair(
            d->canvas->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
            this, SLOT(slotCanvasResourceChanged(int,QVariant)));

    d->resourceManagerAcyclicConnector.connectForwardVoid(
         this, SIGNAL(sigInternalRequestColorToResourceManager()),
         this, SLOT(slotProposeCurrentColorToResourceManager()));


    // configure GUI

    d->ui = new Ui_KoFillConfigWidget();
    d->ui->setupUi(this);

    d->group = new QButtonGroup(this);
    d->group->setExclusive(true);

    d->ui->btnNoFill->setIcon(QPixmap((const char **) buttonnone));
    d->group->addButton(d->ui->btnNoFill, None);

    d->ui->btnSolidFill->setIcon(QPixmap((const char **) buttonsolid));
    d->group->addButton(d->ui->btnSolidFill, Solid);

    d->ui->btnGradientFill->setIcon(QPixmap((const char **) buttongradient));
    d->group->addButton(d->ui->btnGradientFill, Gradient);

    d->ui->btnPatternFill->setIcon(QPixmap((const char **) buttonpattern));
    d->group->addButton(d->ui->btnPatternFill, Pattern);

    d->colorAction = new KoColorPopupAction(d->ui->btnChooseSolidColor);
    d->colorAction->setToolTip(i18n("Change the filling color"));
    d->colorAction->setCurrentColor(Qt::white);

    d->ui->btnChooseSolidColor->setDefaultAction(d->colorAction);
    d->ui->btnChooseSolidColor->setPopupMode(QToolButton::InstantPopup);
    d->ui->btnSolidColorPick->setIcon(KisIconUtils::loadIcon("krita_tool_color_picker"));

    // TODO: for now the color picking button is disabled!
    d->ui->btnSolidColorPick->setEnabled(false);
    d->ui->btnSolidColorPick->setVisible(false);

    connect(d->colorAction, SIGNAL(colorChanged(KoColor)), &d->colorChangedCompressor, SLOT(start()));
    connect(&d->colorChangedCompressor, SIGNAL(timeout()), SLOT(colorChanged()));

    connect(d->ui->btnChooseSolidColor, SIGNAL(iconSizeChanged()), d->colorAction, SLOT(updateIcon()));

    connect(d->group, SIGNAL(buttonClicked(int)), SLOT(styleButtonPressed(int)));

    connect(d->group, SIGNAL(buttonClicked(int)), SLOT(slotUpdateFillTitle()));

    slotUpdateFillTitle();
    styleButtonPressed(d->group->checkedId());


    // Gradient selector
    d->ui->wdgGradientEditor->setCompactMode(true);
    connect(d->ui->wdgGradientEditor, SIGNAL(sigGradientChanged()), &d->gradientChangedCompressor, SLOT(start()));
    connect(&d->gradientChangedCompressor, SIGNAL(timeout()), SLOT(activeGradientChanged()));

    KoResourceServerProvider *serverProvider = KoResourceServerProvider::instance();
    QSharedPointer<KoAbstractResourceServerAdapter> gradientResourceAdapter(
                new KoResourceServerAdapter<KoAbstractGradient>(serverProvider->gradientServer()));

    d->gradientAction = new KoResourcePopupAction(gradientResourceAdapter,
                                                  d->ui->btnChoosePredefinedGradient);

    d->gradientAction->setToolTip(i18n("Change filling gradient"));
    d->ui->btnChoosePredefinedGradient->setDefaultAction(d->gradientAction);
    d->ui->btnChoosePredefinedGradient->setPopupMode(QToolButton::InstantPopup);

    connect(d->gradientAction, SIGNAL(resourceSelected(QSharedPointer<KoShapeBackground>)),
            SLOT(gradientResourceChanged()));
    connect(d->ui->btnChoosePredefinedGradient, SIGNAL(iconSizeChanged()), d->gradientAction, SLOT(updateIcon()));

    d->ui->btnSaveGradient->setIcon(KisIconUtils::loadIcon("document-save"));
    connect(d->ui->btnSaveGradient, SIGNAL(clicked()), SLOT(slotSavePredefinedGradientClicked()));

    connect(d->ui->cmbGradientRepeat, SIGNAL(currentIndexChanged(int)), SLOT(slotGradientRepeatChanged()));
    connect(d->ui->cmbGradientType, SIGNAL(currentIndexChanged(int)), SLOT(slotGradientTypeChanged()));

    deactivate();

#if 0

    // Pattern selector
    QSharedPointer<KoAbstractResourceServerAdapter>patternResourceAdapter(new KoResourceServerAdapter<KoPattern>(serverProvider->patternServer()));
    d->patternAction = new KoResourcePopupAction(patternResourceAdapter, d->colorButton);
    d->patternAction->setToolTip(i18n("Change the filling pattern"));
    connect(d->patternAction, SIGNAL(resourceSelected(QSharedPointer<KoShapeBackground>)), this, SLOT(patternChanged(QSharedPointer<KoShapeBackground>)));
    connect(d->colorButton, SIGNAL(iconSizeChanged()), d->patternAction, SLOT(updateIcon()));

#endif

}

KoFillConfigWidget::~KoFillConfigWidget()
{
    delete d;
}

void KoFillConfigWidget::activate()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!d->deactivationLocks.empty());
    d->deactivationLocks.clear();

    if (!d->noSelectionTrackingMode) {
        shapeChanged();
    } else {
        loadCurrentFillFromResourceServer();
    }

    updateWidgetComponentVisbility();
}

void KoFillConfigWidget::deactivate()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->deactivationLocks.empty());
    d->deactivationLocks.push_back(KisAcyclicSignalConnector::Blocker(d->shapeChangedAcyclicConnector));
    d->deactivationLocks.push_back(KisAcyclicSignalConnector::Blocker(d->resourceManagerAcyclicConnector));
}

void KoFillConfigWidget::forceUpdateOnSelectionChanged()
{
    shapeChanged();
}

void KoFillConfigWidget::setNoSelectionTrackingMode(bool value)
{
    d->noSelectionTrackingMode = value;
    if (!d->noSelectionTrackingMode) {
        shapeChanged();
    }
}

void KoFillConfigWidget::slotUpdateFillTitle()
{
    QString text = d->group->checkedButton() ? d->group->checkedButton()->text() : QString();
    text.replace('&', QString());
    d->ui->lblFillTitle->setText(text);
}

void KoFillConfigWidget::slotCanvasResourceChanged(int key, const QVariant &value)
{
    if ((key == KoCanvasResourceProvider::ForegroundColor && d->fillVariant == KoFlake::Fill) ||
        (key == KoCanvasResourceProvider::BackgroundColor &&
         d->fillVariant == KoFlake::StrokeFill && !d->noSelectionTrackingMode) ||
        (key == KoCanvasResourceProvider::ForegroundColor && d->noSelectionTrackingMode)) {

        KoColor color = value.value<KoColor>();

        const int checkedId = d->group->checkedId();

        if ((checkedId < 0 || checkedId == None || checkedId == Solid) &&
            !(checkedId == Solid && d->colorAction->currentKoColor() == color)) {

            d->group->button(Solid)->setChecked(true);
            d->selectedFillIndex = Solid;

            d->colorAction->setCurrentColor(color);
            d->colorChangedCompressor.start();
        } else if (checkedId == Gradient && key == KoCanvasResourceProvider::ForegroundColor) {
            d->ui->wdgGradientEditor->notifyGlobalColorChanged(color);
        }
    } else if (key == KisCanvasResourceProvider::CurrentGradient) {
        KoResource *gradient = value.value<KoAbstractGradient*>();
        const int checkedId = d->group->checkedId();

        if (gradient && (checkedId < 0 || checkedId == None || checkedId == Gradient)) {
            d->group->button(Gradient)->setChecked(true);
            d->gradientAction->setCurrentResource(gradient);
        }
    }
}

QList<KoShape*> KoFillConfigWidget::currentShapes()
{
    return d->canvas->selectedShapesProxy()->selection()->selectedEditableShapes();
}

int KoFillConfigWidget::selectedFillIndex() {
    return d->selectedFillIndex;
}

void KoFillConfigWidget::styleButtonPressed(int buttonId)
{
    switch (buttonId) {
        case KoFillConfigWidget::None:
            noColorSelected();
            break;
        case KoFillConfigWidget::Solid:
            colorChanged();
            break;
        case KoFillConfigWidget::Gradient:
            if (d->activeGradient) {
                activeGradientChanged();
            } else {
                gradientResourceChanged();
            }
            break;
        case KoFillConfigWidget::Pattern:
            // Only select mode in the widget, don't set actual pattern :/
            //d->colorButton->setDefaultAction(d->patternAction);
            //patternChanged(d->patternAction->currentBackground());
            break;
    }

    if (buttonId >= None && buttonId <= Pattern) {
        d->selectedFillIndex = static_cast<KoFillConfigWidget::StyleButton>(buttonId);
        updateWidgetComponentVisbility();
    }
}

KoShapeStrokeSP KoFillConfigWidget::createShapeStroke()
{
    KoShapeStrokeSP stroke(new KoShapeStroke());
    KIS_ASSERT_RECOVER_RETURN_VALUE(d->fillVariant == KoFlake::StrokeFill, stroke);

    switch (d->group->checkedId()) {
    case KoFillConfigWidget::None:
        stroke->setColor(Qt::transparent);
        break;
    case KoFillConfigWidget::Solid:
        stroke->setColor(d->colorAction->currentColor());
        break;
    case KoFillConfigWidget::Gradient: {
        QScopedPointer<QGradient> g(d->activeGradient->toQGradient());
        QBrush newBrush = *g;
        stroke->setLineBrush(newBrush);
        stroke->setColor(Qt::transparent);
        break;
    }
    case KoFillConfigWidget::Pattern:
        break;
    }

    return stroke;
}

void KoFillConfigWidget::noColorSelected()
{
    KisAcyclicSignalConnector::Blocker b(d->shapeChangedAcyclicConnector);

    QList<KoShape*> selectedShapes = currentShapes();
    if (selectedShapes.isEmpty()) {
        emit sigFillChanged();
        return;
    }

    KoShapeFillWrapper wrapper(selectedShapes, d->fillVariant);
    KUndo2Command *command = wrapper.setColor(QColor());

    if (command) {
        d->canvas->addCommand(command);
    }

    emit sigFillChanged();
}

void KoFillConfigWidget::colorChanged()
{
    KisAcyclicSignalConnector::Blocker b(d->shapeChangedAcyclicConnector);

    QList<KoShape*> selectedShapes = currentShapes();
    if (selectedShapes.isEmpty()) {
        emit sigInternalRequestColorToResourceManager();
        emit sigFillChanged();
        return;
    }

    KoShapeFillWrapper wrapper(selectedShapes, d->fillVariant);
    KUndo2Command *command = wrapper.setColor(d->colorAction->currentColor());

    if (command) {
        d->canvas->addCommand(command);
    }

    emit sigInternalRequestColorToResourceManager();
    emit sigFillChanged();
}

void KoFillConfigWidget::slotProposeCurrentColorToResourceManager()
{
    const int checkedId = d->group->checkedId();

    bool hasColor = false;
    KoColor color;
    KoCanvasResourceProvider::CanvasResource colorSlot = KoCanvasResourceProvider::ForegroundColor;


    if (checkedId == Solid) {
        if (d->fillVariant == KoFlake::StrokeFill) {
            colorSlot = KoCanvasResourceProvider::BackgroundColor;
        }
        color = d->colorAction->currentKoColor();
        hasColor = true;
    } else if (checkedId == Gradient) {
        if (boost::optional<KoColor> gradientColor = d->ui->wdgGradientEditor->currentActiveStopColor()) {
            color = *gradientColor;
            hasColor = true;
        }
    }

    if (hasColor) {
        /**
         * Don't let opacity leak to our resource manager system
         *
         * NOTE: theoretically, we could guarantee it on a level of the
         * resource manager itself,
         */
        color.setOpacity(OPACITY_OPAQUE_U8);
        d->canvas->resourceManager()->setResource(colorSlot, QVariant::fromValue(color));
    }
}

template <class ResourceServer>
QString findFirstAvailableResourceName(const QString &baseName, ResourceServer *server)
{
    if (!server->resourceByName(baseName)) return baseName;

    int counter = 1;
    QString result;
    while ((result = QString("%1%2").arg(baseName).arg(counter)),
           server->resourceByName(result)) {

        counter++;
    }

    return result;
}


void KoFillConfigWidget::slotSavePredefinedGradientClicked()
{
    KoResourceServerProvider *serverProvider = KoResourceServerProvider::instance();
    auto server = serverProvider->gradientServer();

    const QString defaultGradientNamePrefix = i18nc("default prefix for the saved gradient", "gradient");

    QString name = d->activeGradient->name().isEmpty() ? defaultGradientNamePrefix : d->activeGradient->name();
    name = findFirstAvailableResourceName(name, server);
    name = QInputDialog::getText(this, i18nc("@title:window", "Save Gradient"), i18n("Enter gradient name:"), QLineEdit::Normal, name);

    // TODO: currently we do not allow the user to
    //       create two resources with the same name!
    //       Please add some feedback for it!
    name = findFirstAvailableResourceName(name, server);

    d->activeGradient->setName(name);

    const QString saveLocation = server->saveLocation();
    d->activeGradient->setFilename(saveLocation + d->activeGradient->name() + d->activeGradient->defaultFileExtension());

    KoAbstractGradient *newGradient = d->activeGradient->clone();
    server->addResource(newGradient);

    d->gradientAction->setCurrentResource(newGradient);
}

void KoFillConfigWidget::activeGradientChanged()
{
    setNewGradientBackgroundToShape();
    updateGradientSaveButtonAvailability();

    emit sigInternalRequestColorToResourceManager();
}

void KoFillConfigWidget::gradientResourceChanged()
{
    QSharedPointer<KoGradientBackground> bg =
        qSharedPointerDynamicCast<KoGradientBackground>(
            d->gradientAction->currentBackground());

    uploadNewGradientBackground(bg->gradient());

    setNewGradientBackgroundToShape();
    updateGradientSaveButtonAvailability();
}

void KoFillConfigWidget::slotGradientTypeChanged()
{
    QGradient::Type type =
        d->ui->cmbGradientType->currentIndex() == 0 ?
            QGradient::LinearGradient : QGradient::RadialGradient;

    d->activeGradient->setType(type);
    activeGradientChanged();
}

void KoFillConfigWidget::slotGradientRepeatChanged()
{
    QGradient::Spread spread =
        QGradient::Spread(d->ui->cmbGradientRepeat->currentIndex());

    d->activeGradient->setSpread(spread);
    activeGradientChanged();
}

void KoFillConfigWidget::uploadNewGradientBackground(const QGradient *gradient)
{
    KisSignalsBlocker b1(d->ui->wdgGradientEditor,
                         d->ui->cmbGradientType,
                         d->ui->cmbGradientRepeat);

    d->ui->wdgGradientEditor->setGradient(0);

    d->activeGradient.reset(KoStopGradient::fromQGradient(gradient));

    d->ui->wdgGradientEditor->setGradient(d->activeGradient.data());
    d->ui->cmbGradientType->setCurrentIndex(d->activeGradient->type() != QGradient::LinearGradient);
    d->ui->cmbGradientRepeat->setCurrentIndex(int(d->activeGradient->spread()));
}

void KoFillConfigWidget::setNewGradientBackgroundToShape()
{
    QList<KoShape*> selectedShapes = currentShapes();
    if (selectedShapes.isEmpty()) {
        emit sigFillChanged();
        return;
    }

    KisAcyclicSignalConnector::Blocker b(d->shapeChangedAcyclicConnector);

    KoShapeFillWrapper wrapper(selectedShapes, d->fillVariant);
    QScopedPointer<QGradient> srcQGradient(d->activeGradient->toQGradient());
    KUndo2Command *command = wrapper.applyGradientStopsOnly(srcQGradient.data());

    if (command) {
        d->canvas->addCommand(command);
    }

    emit sigFillChanged();
}

void KoFillConfigWidget::updateGradientSaveButtonAvailability()
{
    bool savingEnabled = false;

    QScopedPointer<QGradient> currentGradient(d->activeGradient->toQGradient());
    QSharedPointer<KoShapeBackground> bg = d->gradientAction->currentBackground();
    if (bg) {
        QSharedPointer<KoGradientBackground> resourceBackground =
            qSharedPointerDynamicCast<KoGradientBackground>(bg);

        savingEnabled = resourceBackground->gradient()->stops() != currentGradient->stops();
        savingEnabled |= resourceBackground->gradient()->type() != currentGradient->type();
        savingEnabled |= resourceBackground->gradient()->spread() != currentGradient->spread();
    }

    d->ui->btnSaveGradient->setEnabled(savingEnabled);
}

void KoFillConfigWidget::patternChanged(QSharedPointer<KoShapeBackground>  background)
{
    Q_UNUSED(background);

#if 0
    QSharedPointer<KoPatternBackground> patternBackground = qSharedPointerDynamicCast<KoPatternBackground>(background);
    if (! patternBackground) {
        return;
    }

    QList<KoShape*> selectedShapes = currentShapes();
    if (selectedShapes.isEmpty()) {
        return;
    }

    KoImageCollection *imageCollection = d->canvas->shapeController()->resourceManager()->imageCollection();
    if (imageCollection) {
        QSharedPointer<KoPatternBackground> fill(new KoPatternBackground(imageCollection));
        fill->setPattern(patternBackground->pattern());
        d->canvas->addCommand(new KoShapeBackgroundCommand(selectedShapes, fill));
    }
#endif
}

void KoFillConfigWidget::loadCurrentFillFromResourceServer()
{
    {
        KoColor color = d->canvas->resourceManager()->backgroundColor();
        slotCanvasResourceChanged(KoCanvasResourceProvider::BackgroundColor, QVariant::fromValue(color));
    }

    {
        KoColor color = d->canvas->resourceManager()->foregroundColor();
        slotCanvasResourceChanged(KoCanvasResourceProvider::ForegroundColor, QVariant::fromValue(color));
    }

    Q_FOREACH (QAbstractButton *button, d->group->buttons()) {
        button->setEnabled(true);
    }

    emit sigFillChanged();
}

void KoFillConfigWidget::shapeChanged()
{
    if (d->noSelectionTrackingMode) return;

    QList<KoShape*> shapes = currentShapes();

    if (shapes.isEmpty() ||
        (shapes.size() > 1 && KoShapeFillWrapper(shapes, d->fillVariant).isMixedFill())) {

        Q_FOREACH (QAbstractButton *button, d->group->buttons()) {
            button->setEnabled(!shapes.isEmpty());
        }

        d->group->button(None)->setChecked(true);
        d->selectedFillIndex = None;

    } else {
        Q_FOREACH (QAbstractButton *button, d->group->buttons()) {
            button->setEnabled(true);
        }

        KoShape *shape = shapes.first();
        updateWidget(shape);
    }
}

bool KoFillConfigWidget::checkNewFillModeIsSame(const KoShapeFillWrapper &w) const
{
    bool retval = false;

    switch (w.type()) {
    case KoFlake::None:
        retval = d->selectedFillIndex == None;
        break;
    case KoFlake::Solid:
        retval = d->selectedFillIndex == Solid && w.color() == d->colorAction->currentColor();
        break;
    case KoFlake::Gradient: {
        QScopedPointer<KoStopGradient> newGradient(KoStopGradient::fromQGradient(w.gradient()));

        retval = d->selectedFillIndex == Gradient && *newGradient == *d->activeGradient;
        break;
    }
    case KoFlake::Pattern:
        // TODO: not implemented
        retval = d->selectedFillIndex == Pattern && false;
        break;
    }

    return retval;
}

void KoFillConfigWidget::updateWidget(KoShape *shape)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(shape);

    StyleButton newActiveButton = None;
    KoShapeFillWrapper wrapper(shape, d->fillVariant);

    if (checkNewFillModeIsSame(wrapper)) return;

    switch (wrapper.type()) {
    case KoFlake::None:
        break;
    case KoFlake::Solid: {
        QColor color = wrapper.color();
        if (color.alpha() > 0) {
            newActiveButton = KoFillConfigWidget::Solid;
            d->colorAction->setCurrentColor(wrapper.color());
        }
        break;
    }
    case KoFlake::Gradient:
        newActiveButton = KoFillConfigWidget::Gradient;
        uploadNewGradientBackground(wrapper.gradient());
        updateGradientSaveButtonAvailability();
        break;
    case KoFlake::Pattern:
        newActiveButton = KoFillConfigWidget::Pattern;
        break;
    }

    d->group->button(newActiveButton)->setChecked(true);

    d->selectedFillIndex = newActiveButton;
    updateWidgetComponentVisbility();
}


void KoFillConfigWidget::updateWidgetComponentVisbility()
{
    // The UI is showing/hiding things like this because the 'stacked widget' isn't very flexible
    // and makes it difficult to put anything underneath it without a lot empty space

    // hide everything first
    d->ui->wdgGradientEditor->setVisible(false);
    d->ui->btnChoosePredefinedGradient->setVisible(false);
    d->ui->btnChooseSolidColor->setVisible(false);
    d->ui->typeLabel->setVisible(false);
    d->ui->repeatLabel->setVisible(false);
    d->ui->cmbGradientRepeat->setVisible(false);
    d->ui->cmbGradientType->setVisible(false);
    d->ui->btnSolidColorPick->setVisible(false);
    d->ui->btnSaveGradient->setVisible(false);
    d->ui->gradientTypeLine->setVisible(false);
    d->ui->soldStrokeColorLabel->setVisible(false);
    d->ui->presetLabel->setVisible(false);

    switch (d->selectedFillIndex) {
        case KoFillConfigWidget::None:
            break;
        case KoFillConfigWidget::Solid:
            d->ui->btnChooseSolidColor->setVisible(true);
            d->ui->btnSolidColorPick->setVisible(true);
            d->ui->soldStrokeColorLabel->setVisible(true);
            break;
        case KoFillConfigWidget::Gradient:
            d->ui->wdgGradientEditor->setVisible(true);
            d->ui->btnChoosePredefinedGradient->setVisible(true);
            d->ui->typeLabel->setVisible(true);
            d->ui->repeatLabel->setVisible(true);
            d->ui->cmbGradientRepeat->setVisible(true);
            d->ui->cmbGradientType->setVisible(true);
            d->ui->btnSaveGradient->setVisible(true);
            d->ui->gradientTypeLine->setVisible(true);
            d->ui->presetLabel->setVisible(true);
            break;
        case KoFillConfigWidget::Pattern:
            break;
    }

}
