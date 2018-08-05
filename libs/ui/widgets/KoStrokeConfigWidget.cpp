/* This file is part of the KDE project
 * Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
 * Copyright (C) 2002 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
 * Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
 * Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * Copyright (C) 2005-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2005-2006, 2011 Inge Wallin <inge@lysator.liu.se>
 * Copyright (C) 2005-2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2006 Peter Simonsson <psn@linux.se>
 * Copyright (C) 2006 Laurent Montel <montel@kde.org>
 * Copyright (C) 2007,2011 Thorsten Zachmann <t.zachmann@zagge.de>
 * Copyright (C) 2011 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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

// Own
#include "KoStrokeConfigWidget.h"

// Qt
#include <QMenu>
#include <QToolButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QSignalMapper>

// KDE
#include <klocalizedstring.h>

// Calligra
#include <KoIcon.h>
#include <KoUnit.h>
#include <KoLineStyleSelector.h>
#include <KoUnitDoubleSpinBox.h>
#include <KoMarkerSelector.h>
#include <KoColorPopupAction.h>
#include <KoMarker.h>
#include <KoShapeStroke.h>
#include <KoPathShape.h>
#include <KoMarkerCollection.h>
#include <KoPathShapeMarkerCommand.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoCanvasResourceManager.h>
#include <KoDocumentResourceManager.h>
#include <KoSelection.h>
#include <KoShapeController.h>
#include <KoShapeStrokeCommand.h>
#include <KoShapeStrokeModel.h>
#include <KoSelectedShapesProxy.h>
#include "ui_KoStrokeConfigWidget.h"
#include <KoFlakeUtils.h>
#include <KoFillConfigWidget.h>
#include "kis_canvas_resource_provider.h"
#include "kis_acyclic_signal_connector.h"

// Krita
#include "kis_double_parse_unit_spin_box.h"

class CapNJoinMenu : public QMenu
{
public:
    CapNJoinMenu(QWidget *parent = 0);
    QSize sizeHint() const override;

    KisDoubleParseUnitSpinBox *miterLimit;
    QButtonGroup        *capGroup;
    QButtonGroup        *joinGroup;
};

CapNJoinMenu::CapNJoinMenu(QWidget *parent)
    : QMenu(parent)
{
    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->setMargin(2);

     // The cap group
    capGroup = new QButtonGroup(this);
    capGroup->setExclusive(true);

    QToolButton *button = 0;

    button = new QToolButton(this);
    button->setIcon(koIcon("stroke-cap-butt"));
    button->setCheckable(true);
    button->setToolTip(i18n("Butt cap"));
    capGroup->addButton(button, Qt::FlatCap);
    mainLayout->addWidget(button, 2, 0);

    button = new QToolButton(this);
    button->setIcon(koIcon("stroke-cap-round"));
    button->setCheckable(true);
    button->setToolTip(i18n("Round cap"));
    capGroup->addButton(button, Qt::RoundCap);
    mainLayout->addWidget(button, 2, 1);

    button = new QToolButton(this);
    button->setIcon(koIcon("stroke-cap-square"));
    button->setCheckable(true);
    button->setToolTip(i18n("Square cap"));
    capGroup->addButton(button, Qt::SquareCap);
    mainLayout->addWidget(button, 2, 2, Qt::AlignLeft);

    // The join group
    joinGroup = new QButtonGroup(this);
    joinGroup->setExclusive(true);

    button = new QToolButton(this);
    button->setIcon(koIcon("stroke-join-miter"));
    button->setCheckable(true);
    button->setToolTip(i18n("Miter join"));
    joinGroup->addButton(button, Qt::MiterJoin);
    mainLayout->addWidget(button, 3, 0);

    button = new QToolButton(this);
    button->setIcon(koIcon("stroke-join-round"));
    button->setCheckable(true);
    button->setToolTip(i18n("Round join"));
    joinGroup->addButton(button, Qt::RoundJoin);
    mainLayout->addWidget(button, 3, 1);

    button = new QToolButton(this);
    button->setIcon(koIcon("stroke-join-bevel"));
    button->setCheckable(true);
    button->setToolTip(i18n("Bevel join"));
    joinGroup->addButton(button, Qt::BevelJoin);
    mainLayout->addWidget(button, 3, 2, Qt::AlignLeft);

    // Miter limit
    // set min/max/step and value in points, then set actual unit
    miterLimit = new KisDoubleParseUnitSpinBox(this);
    miterLimit->setMinMaxStep(0.0, 1000.0, 0.5);
    miterLimit->setDecimals(2);
    miterLimit->setUnit(KoUnit(KoUnit::Point));
    miterLimit->setToolTip(i18n("Miter limit"));
    mainLayout->addWidget(miterLimit, 4, 0, 1, 3);

    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    setLayout(mainLayout);
}

QSize CapNJoinMenu::sizeHint() const
{
    return layout()->sizeHint();
}


class Q_DECL_HIDDEN KoStrokeConfigWidget::Private
{
public:
    Private()
        : canvas(0),
        active(true),
        allowLocalUnitManagement(true),
        fillConfigWidget(0),
        noSelectionTrackingMode(false)
    {
    }

    KoLineStyleSelector *lineStyle;
    KisDoubleParseUnitSpinBox *lineWidth;
    KoMarkerSelector    *startMarkerSelector;
    KoMarkerSelector    *midMarkerSelector;
    KoMarkerSelector    *endMarkerSelector;

   CapNJoinMenu *capNJoinMenu;

    QWidget *spacer;

    KoCanvasBase *canvas;

    bool active;
    bool allowLocalUnitManagement;

    KoFillConfigWidget *fillConfigWidget;
    bool noSelectionTrackingMode;

    KisAcyclicSignalConnector shapeChangedAcyclicConnector;
    KisAcyclicSignalConnector resourceManagerAcyclicConnector;

    std::vector<KisAcyclicSignalConnector::Blocker> deactivationLocks;

    Ui_KoStrokeConfigWidget *ui;
};


KoStrokeConfigWidget::KoStrokeConfigWidget(KoCanvasBase *canvas, QWidget * parent)
    : QWidget(parent)
    , d(new Private())
{
    // confure GUI
    d->ui = new Ui_KoStrokeConfigWidget();
    d->ui->setupUi(this);

    setObjectName("Stroke widget");

    { // connect the canvas
        d->shapeChangedAcyclicConnector.connectBackwardVoid(
            canvas->selectedShapesProxy(), SIGNAL(selectionChanged()),
            this, SLOT(selectionChanged()));

        d->shapeChangedAcyclicConnector.connectBackwardVoid(
            canvas->selectedShapesProxy(), SIGNAL(selectionContentChanged()),
            this, SLOT(selectionChanged()));

        d->resourceManagerAcyclicConnector.connectBackwardResourcePair(
            canvas->resourceManager(), SIGNAL(canvasResourceChanged(int, const QVariant&)),
            this, SLOT(canvasResourceChanged(int, const QVariant &)));

        d->canvas = canvas;
    }


    {

       d->fillConfigWidget = new KoFillConfigWidget(canvas, KoFlake::StrokeFill, false, this);
       d->fillConfigWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
       d->ui->fillConfigWidgetLayout->addWidget(d->fillConfigWidget);
       connect(d->fillConfigWidget, SIGNAL(sigFillChanged()), SIGNAL(sigStrokeChanged()));
    }

    d->ui->thicknessLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->ui->thicknessLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // set min/max/step and value in points, then set actual unit
    d->ui->lineWidth->setMinMaxStep(0.0, 1000.0, 0.5);
    d->ui->lineWidth->setDecimals(2);
    d->ui->lineWidth->setUnit(KoUnit(KoUnit::Point));
    d->ui->lineWidth->setToolTip(i18n("Set line width of actual selection"));

    d->ui->capNJoinButton->setMinimumHeight(25);
    d->capNJoinMenu = new CapNJoinMenu(this);
    d->ui->capNJoinButton->setMenu(d->capNJoinMenu);
    d->ui->capNJoinButton->setText("...");
    d->ui->capNJoinButton->setPopupMode(QToolButton::InstantPopup);


    {
        // Line style
        d->ui->strokeStyleLabel->setText(i18n("Line Style:"));
        d->ui->strokeStyleLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        d->ui->lineStyle->setToolTip(i18nc("@info:tooltip", "Line style"));
        d->ui->lineStyle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        d->ui->lineStyle->setLineStyle(Qt::SolidLine,  QVector<qreal>());
    }


    {
        QList<KoMarker*> emptyMarkers;


        d->startMarkerSelector = new KoMarkerSelector(KoFlake::StartMarker, this);
        d->startMarkerSelector->setToolTip(i18nc("@info:tooltip", "Start marker"));
        d->startMarkerSelector->updateMarkers(emptyMarkers);
        d->startMarkerSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred );
        d->ui->markerLayout->addWidget(d->startMarkerSelector);


        d->midMarkerSelector = new KoMarkerSelector(KoFlake::MidMarker, this);
        d->midMarkerSelector->setToolTip(i18nc("@info:tooltip", "Node marker"));
        d->midMarkerSelector->updateMarkers(emptyMarkers);
        d->midMarkerSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred );
        d->ui->markerLayout->addWidget(d->midMarkerSelector);


        d->endMarkerSelector = new KoMarkerSelector(KoFlake::EndMarker, this);
        d->endMarkerSelector->setToolTip(i18nc("@info:tooltip", "End marker"));
        d->endMarkerSelector->updateMarkers(emptyMarkers);
        d->endMarkerSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred );

        d->ui->markerLayout->addWidget(d->endMarkerSelector);
    }

    // Spacer
    d->spacer = new QWidget();
    d->spacer->setObjectName("SpecialSpacer");

    d->ui->markerLayout->addWidget(d->spacer);

    connect(d->ui->lineStyle,  SIGNAL(currentIndexChanged(int)), this, SLOT(applyDashStyleChanges()));
    connect(d->ui->lineWidth,  SIGNAL(valueChangedPt(qreal)),    this, SLOT(applyLineWidthChanges()));

    connect(d->capNJoinMenu->capGroup,   SIGNAL(buttonClicked(int)),       this, SLOT(applyJoinCapChanges()));
    connect(d->capNJoinMenu->joinGroup,  SIGNAL(buttonClicked(int)),       this, SLOT(applyJoinCapChanges()));
    connect(d->capNJoinMenu->miterLimit, SIGNAL(valueChangedPt(qreal)),    this, SLOT(applyJoinCapChanges()));

    { // Map the marker signals correctly
        QSignalMapper *mapper = new QSignalMapper(this);
        connect(mapper, SIGNAL(mapped(int)), SLOT(applyMarkerChanges(int)));

        connect(d->startMarkerSelector,  SIGNAL(currentIndexChanged(int)), mapper, SLOT(map()));
        connect(d->midMarkerSelector,  SIGNAL(currentIndexChanged(int)), mapper, SLOT(map()));
        connect(d->endMarkerSelector,  SIGNAL(currentIndexChanged(int)), mapper, SLOT(map()));

        mapper->setMapping(d->startMarkerSelector, KoFlake::StartMarker);
        mapper->setMapping(d->midMarkerSelector, KoFlake::MidMarker);
        mapper->setMapping(d->endMarkerSelector, KoFlake::EndMarker);
    }

    KoDocumentResourceManager *resourceManager = canvas->shapeController()->resourceManager();
    if (resourceManager) {
        KoMarkerCollection *collection = resourceManager->resource(KoDocumentResourceManager::MarkerCollection).value<KoMarkerCollection*>();
        if (collection) {
            updateMarkers(collection->markers());
        }
    }

    selectionChanged();

    d->fillConfigWidget->activate();
    deactivate();
}

KoStrokeConfigWidget::~KoStrokeConfigWidget()
{
    delete d;
}

void KoStrokeConfigWidget::setNoSelectionTrackingMode(bool value)
{
    d->fillConfigWidget->setNoSelectionTrackingMode(value);
    d->noSelectionTrackingMode = value;
    if (!d->noSelectionTrackingMode) {
        selectionChanged();
    }
}

// ----------------------------------------------------------------
//                         getters and setters


Qt::PenStyle KoStrokeConfigWidget::lineStyle() const
{
    return d->ui->lineStyle->lineStyle();
}

QVector<qreal> KoStrokeConfigWidget::lineDashes() const
{
    return d->ui->lineStyle->lineDashes();
}

qreal KoStrokeConfigWidget::lineWidth() const
{
    return d->ui->lineWidth->value();
}

qreal KoStrokeConfigWidget::miterLimit() const
{
    return d->capNJoinMenu->miterLimit->value();
}

KoMarker *KoStrokeConfigWidget::startMarker() const
{
    return d->startMarkerSelector->marker();
}

KoMarker *KoStrokeConfigWidget::endMarker() const
{
    return d->endMarkerSelector->marker();
}

Qt::PenCapStyle KoStrokeConfigWidget::capStyle() const
{
    return static_cast<Qt::PenCapStyle>(d->capNJoinMenu->capGroup->checkedId());
}

Qt::PenJoinStyle KoStrokeConfigWidget::joinStyle() const
{
    return static_cast<Qt::PenJoinStyle>(d->capNJoinMenu->joinGroup->checkedId());
}

KoShapeStrokeSP KoStrokeConfigWidget::createShapeStroke()
{
    KoShapeStrokeSP stroke(d->fillConfigWidget->createShapeStroke());

    stroke->setLineWidth(lineWidth());
    stroke->setCapStyle(capStyle());
    stroke->setJoinStyle(joinStyle());
    stroke->setMiterLimit(miterLimit());
    stroke->setLineStyle(lineStyle(), lineDashes());

    return stroke;
}

// ----------------------------------------------------------------
//                         Other public functions

void KoStrokeConfigWidget::updateStyleControlsAvailability(bool enabled)
{
    d->ui->lineWidth->setEnabled(enabled);
    d->capNJoinMenu->setEnabled(enabled);
    d->ui->lineStyle->setEnabled(enabled);

    d->startMarkerSelector->setEnabled(enabled);
    d->midMarkerSelector->setEnabled(enabled);
    d->endMarkerSelector->setEnabled(enabled);
}

void KoStrokeConfigWidget::setUnit(const KoUnit &unit, KoShape *representativeShape)
{
    if (!d->allowLocalUnitManagement) {
        return; //the unit management is completely transferred to the unitManagers.
    }

    blockChildSignals(true);

    /**
     * KoStrokeShape knows nothing about the transformations applied
     * to the shape, which doesn't prevent the shape to apply them and
     * display the stroke differently. So just take that into account
     * and show the user correct values using the multiplier in KoUnit.
     */
    KoUnit newUnit(unit);
    if (representativeShape) {
        newUnit.adjustByPixelTransform(representativeShape->absoluteTransformation(0));
    }

    d->ui->lineWidth->setUnit(newUnit);
    d->capNJoinMenu->miterLimit->setUnit(newUnit);

    d->ui->lineWidth->setLineStep(1.0);
    d->capNJoinMenu->miterLimit->setLineStep(1.0);

    blockChildSignals(false);
}

void KoStrokeConfigWidget::setUnitManagers(KisSpinBoxUnitManager* managerLineWidth,
                                           KisSpinBoxUnitManager *managerMitterLimit)
{
    blockChildSignals(true);
    d->allowLocalUnitManagement = false;
    d->ui->lineWidth->setUnitManager(managerLineWidth);
    d->capNJoinMenu->miterLimit->setUnitManager(managerMitterLimit);
    blockChildSignals(false);
}

void KoStrokeConfigWidget::updateMarkers(const QList<KoMarker*> &markers)
{
    d->startMarkerSelector->updateMarkers(markers);
    d->midMarkerSelector->updateMarkers(markers);
    d->endMarkerSelector->updateMarkers(markers);
}

void KoStrokeConfigWidget::activate()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!d->deactivationLocks.empty());
    d->deactivationLocks.clear();
    d->fillConfigWidget->activate();

    if (!d->noSelectionTrackingMode) {
        selectionChanged();
    } else {
        loadCurrentStrokeFillFromResourceServer();
    }
}

void KoStrokeConfigWidget::deactivate()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->deactivationLocks.empty());

    d->deactivationLocks.push_back(KisAcyclicSignalConnector::Blocker(d->shapeChangedAcyclicConnector));
    d->deactivationLocks.push_back(KisAcyclicSignalConnector::Blocker(d->resourceManagerAcyclicConnector));
    d->fillConfigWidget->deactivate();
}

void KoStrokeConfigWidget::blockChildSignals(bool block)
{
    d->ui->lineWidth->blockSignals(block);
    d->capNJoinMenu->capGroup->blockSignals(block);
    d->capNJoinMenu->joinGroup->blockSignals(block);
    d->capNJoinMenu->miterLimit->blockSignals(block);
    d->ui->lineStyle->blockSignals(block);
    d->startMarkerSelector->blockSignals(block);
    d->midMarkerSelector->blockSignals(block);
    d->endMarkerSelector->blockSignals(block);
}

void KoStrokeConfigWidget::setActive(bool active)
{
    d->active = active;
}

//------------------------

template <typename ModifyFunction>
    auto applyChangeToStrokes(KoCanvasBase *canvas, ModifyFunction modifyFunction)
        -> decltype(modifyFunction(KoShapeStrokeSP()), void())
{
    KoSelection *selection = canvas->selectedShapesProxy()->selection();

    if (!selection) return;

    QList<KoShape*> shapes = selection->selectedEditableShapes();

    KUndo2Command *command = KoFlake::modifyShapesStrokes(shapes, modifyFunction);

    if (command) {
        canvas->addCommand(command);
    }
}

void KoStrokeConfigWidget::applyDashStyleChanges()
{
    applyChangeToStrokes(
        d->canvas,
        [this] (KoShapeStrokeSP stroke) {
            stroke->setLineStyle(lineStyle(), lineDashes());
        });

    emit sigStrokeChanged();
}

void KoStrokeConfigWidget::applyLineWidthChanges()
{
    applyChangeToStrokes(
        d->canvas,
        [this] (KoShapeStrokeSP stroke) {
            stroke->setLineWidth(lineWidth());
        });

    emit sigStrokeChanged();
}

void KoStrokeConfigWidget::applyJoinCapChanges()
{
    applyChangeToStrokes(
        d->canvas,
        [this] (KoShapeStrokeSP stroke) {

            stroke->setCapStyle(static_cast<Qt::PenCapStyle>(d->capNJoinMenu->capGroup->checkedId()));
            stroke->setJoinStyle(static_cast<Qt::PenJoinStyle>(d->capNJoinMenu->joinGroup->checkedId()));
            stroke->setMiterLimit(miterLimit());
        });

    emit sigStrokeChanged();
}

void KoStrokeConfigWidget::applyMarkerChanges(int rawPosition)
{
    KoSelection *selection = d->canvas->selectedShapesProxy()->selection();
    if (!selection) {
        emit sigStrokeChanged();
        return;
    }

    QList<KoShape*> shapes = selection->selectedEditableShapes();
    QList<KoPathShape*> pathShapes;
    Q_FOREACH (KoShape *shape, shapes) {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*>(shape);
        if (pathShape) {
            pathShapes << pathShape;
        }
    }

    if (pathShapes.isEmpty()) {
        emit sigStrokeChanged();
        return;
    }


    KoFlake::MarkerPosition position = KoFlake::MarkerPosition(rawPosition);
    QScopedPointer<KoMarker> marker;

    switch (position) {
    case KoFlake::StartMarker:
        if (d->startMarkerSelector->marker()) {
            marker.reset(new KoMarker(*d->startMarkerSelector->marker()));
        }
        break;
    case KoFlake::MidMarker:
        if (d->midMarkerSelector->marker()) {
            marker.reset(new KoMarker(*d->midMarkerSelector->marker()));
        }
        break;
    case KoFlake::EndMarker:
        if (d->endMarkerSelector->marker()) {
            marker.reset(new KoMarker(*d->endMarkerSelector->marker()));
        }
        break;
    }

    KUndo2Command* command = new KoPathShapeMarkerCommand(pathShapes, marker.take(), position);
    d->canvas->addCommand(command);

    emit sigStrokeChanged();
}

// ----------------------------------------------------------------

struct CheckShapeStrokeStyleBasePolicy
{
    typedef KoShapeStrokeSP PointerType;
    static PointerType getProperty(KoShape *shape) {
        return qSharedPointerDynamicCast<KoShapeStroke>(shape->stroke());
    }
};

struct CheckShapeStrokeDashesPolicy : public CheckShapeStrokeStyleBasePolicy
{
    static bool compareTo(PointerType p1, PointerType p2) {
        return p1->lineStyle() == p2->lineStyle() &&
                p1->lineDashes() == p2->lineDashes() &&
                p1->dashOffset() == p2->dashOffset();
    }
};

struct CheckShapeStrokeCapJoinPolicy : public CheckShapeStrokeStyleBasePolicy
{
    static bool compareTo(PointerType p1, PointerType p2) {
        return p1->capStyle() == p2->capStyle() &&
                p1->joinStyle() == p2->joinStyle() &&
                p1->miterLimit() == p2->miterLimit();
    }
};

struct CheckShapeStrokeWidthPolicy : public CheckShapeStrokeStyleBasePolicy
{
    static bool compareTo(PointerType p1, PointerType p2) {
        return p1->lineWidth() == p2->lineWidth();
    }
};

struct CheckShapeMarkerPolicy
{
    CheckShapeMarkerPolicy(KoFlake::MarkerPosition position)
        : m_position(position)
    {
    }

    typedef KoMarker* PointerType;
    PointerType getProperty(KoShape *shape) const {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*>(shape);
        return pathShape ? pathShape->marker(m_position) : 0;
    }
    bool compareTo(PointerType p1, PointerType p2) const {
        if ((!p1 || !p2) && p1 != p2) return false;
        if (!p1 && p1 == p2) return true;

        return p1 == p2 || *p1 == *p2;
    }

    KoFlake::MarkerPosition m_position;
};

void KoStrokeConfigWidget::selectionChanged()
{
    if (d->noSelectionTrackingMode) return;

    KoSelection *selection = d->canvas->selectedShapesProxy()->selection();
    if (!selection) return;

    // we need to linearize update order, and force the child widget to update
    // before we start doing it
    d->fillConfigWidget->forceUpdateOnSelectionChanged();


    QList<KoShape*> shapes = selection->selectedEditableShapes();

    KoShape *shape = !shapes.isEmpty() ? shapes.first() : 0;

    const KoShapeStrokeSP stroke = shape ? qSharedPointerDynamicCast<KoShapeStroke>(shape->stroke()) : KoShapeStrokeSP();

    // setUnit uses blockChildSignals() so take care not to use it inside the block
    setUnit(d->canvas->unit(), shape);

    blockChildSignals(true);

    // line width
    if (stroke && KoFlake::compareShapePropertiesEqual<CheckShapeStrokeWidthPolicy>(shapes)) {
        d->ui->lineWidth->changeValue(stroke->lineWidth());
    } else {
        d->ui->lineWidth->changeValue(0);
    }


    // caps & joins
    if (stroke && KoFlake::compareShapePropertiesEqual<CheckShapeStrokeCapJoinPolicy>(shapes)) {
        Qt::PenCapStyle capStyle = stroke->capStyle() >= 0 ? stroke->capStyle() : Qt::FlatCap;
        Qt::PenJoinStyle joinStyle = stroke->joinStyle() >= 0 ? stroke->joinStyle() : Qt::MiterJoin;

        {
            QAbstractButton *button = d->capNJoinMenu->capGroup->button(capStyle);
            KIS_SAFE_ASSERT_RECOVER_RETURN(button);
            button->setChecked(true);
        }

        {
            QAbstractButton *button = d->capNJoinMenu->joinGroup->button(joinStyle);
            KIS_SAFE_ASSERT_RECOVER_RETURN(button);
            button->setChecked(true);
        }

        d->capNJoinMenu->miterLimit->changeValue(stroke->miterLimit());
        d->capNJoinMenu->miterLimit->setEnabled(joinStyle == Qt::MiterJoin);
    } else {
        d->capNJoinMenu->capGroup->button(Qt::FlatCap)->setChecked(true);
        d->capNJoinMenu->joinGroup->button(Qt::MiterJoin)->setChecked(true);
        d->capNJoinMenu->miterLimit->changeValue(0.0);
        d->capNJoinMenu->miterLimit->setEnabled(true);
    }


    // dashes style
    if (stroke && KoFlake::compareShapePropertiesEqual<CheckShapeStrokeDashesPolicy>(shapes)) {
        d->ui->lineStyle->setLineStyle(stroke->lineStyle(), stroke->lineDashes());
    } else {
        d->ui->lineStyle->setLineStyle(Qt::SolidLine, QVector<qreal>());
    }

    // markers
    KoPathShape *pathShape = dynamic_cast<KoPathShape *>(shape);
    if (pathShape) {
        if (KoFlake::compareShapePropertiesEqual(shapes, CheckShapeMarkerPolicy(KoFlake::StartMarker))) {
            d->startMarkerSelector->setMarker(pathShape->marker(KoFlake::StartMarker));
        }
        if (KoFlake::compareShapePropertiesEqual(shapes, CheckShapeMarkerPolicy(KoFlake::MidMarker))) {
            d->midMarkerSelector->setMarker(pathShape->marker(KoFlake::MidMarker));
        }
        if (KoFlake::compareShapePropertiesEqual(shapes, CheckShapeMarkerPolicy(KoFlake::EndMarker))) {
            d->endMarkerSelector->setMarker(pathShape->marker(KoFlake::EndMarker));
        }
    }

    const bool lineOptionsVisible =  d->fillConfigWidget->selectedFillIndex() != 0;

    // This switch statement is to help the tab widget "pages" to be closer to the correct size
    // if we don't do this the internal widgets get rendered, then the tab page has to get resized to
    // fill up the space, then the internal widgets have to resize yet again...causing flicker
    switch(d->fillConfigWidget->selectedFillIndex()) {
        case 0: // no fill
            this->setMinimumHeight(130);
            break;
        case 1: // solid fill
             this->setMinimumHeight(200);
             break;
        case 2: // gradient fill
            this->setMinimumHeight(350);
        case 3: // pattern fill
            break;
    }


    d->ui->thicknessLineBreak->setVisible(lineOptionsVisible);
    d->ui->lineWidth->setVisible(lineOptionsVisible);
    d->ui->capNJoinButton->setVisible(lineOptionsVisible);
    d->ui->lineStyle->setVisible(lineOptionsVisible);
    d->startMarkerSelector->setVisible(lineOptionsVisible);
    d->midMarkerSelector->setVisible(lineOptionsVisible);
    d->endMarkerSelector->setVisible(lineOptionsVisible);
    d->ui->thicknessLabel->setVisible(lineOptionsVisible);
    d->ui->strokeStyleLabel->setVisible(lineOptionsVisible);



    blockChildSignals(false);

    updateStyleControlsAvailability(!shapes.isEmpty());

}

void KoStrokeConfigWidget::canvasResourceChanged(int key, const QVariant &value)
{
    switch (key) {
    case KoCanvasResourceManager::Unit:
        // we request the whole selection to reload because the
        // unit of the stroke width depends on the selected shape
        selectionChanged();
        break;
    case KisCanvasResourceProvider::Size:
        if (d->noSelectionTrackingMode) {
            d->ui->lineWidth->changeValue(d->canvas->unit().fromUserValue(value.toReal()));
        }
        break;
    }
}

void KoStrokeConfigWidget::loadCurrentStrokeFillFromResourceServer()
{
    if (d->canvas) {
        const QVariant value = d->canvas->resourceManager()->resource(KisCanvasResourceProvider::Size);
        canvasResourceChanged(KisCanvasResourceProvider::Size, value);

        updateStyleControlsAvailability(true);

        emit sigStrokeChanged();
    }
}
