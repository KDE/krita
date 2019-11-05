/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "FilterEffectEditWidget.h"
#include "FilterEffectResource.h"
#include "FilterResourceServerProvider.h"
#include "FilterInputChangeCommand.h"
#include "FilterAddCommand.h"
#include "FilterRemoveCommand.h"
#include "FilterStackSetCommand.h"

#include <KoGenericRegistryModel.h>
#include <KoFilterEffectRegistry.h>
#include <KoFilterEffect.h>
#include <KoFilterEffectStack.h>
#include <KoFilterEffectConfigWidgetBase.h>
#include <KoFilterEffectFactoryBase.h>
#include <KoShape.h>
#include <KoCanvasBase.h>

#include <KoIcon.h>

#include <QInputDialog>
#include <QDebug>

#include <QGraphicsItem>
#include <QSet>

FilterEffectEditWidget::FilterEffectEditWidget(QWidget *parent)
    : QWidget(parent)
    , m_scene(new FilterEffectScene(this))
    , m_shape(0)
    , m_canvas(0)
    , m_effects(0)
{
    setupUi(this);

    presets->setDisplayMode(KoResourceSelector::TextMode);
    presets->setSingleColumn(true);

    connect(presets, SIGNAL(resourceSelected(KoResourceSP )),
            this, SLOT(presetSelected(KoResourceSP )));

    connect(presets, SIGNAL(resourceApplied(KoResourceSP )),
            this, SLOT(presetSelected(KoResourceSP )));

    KoGenericRegistryModel<KoFilterEffectFactoryBase *> *filterEffectModel = new KoGenericRegistryModel<KoFilterEffectFactoryBase *>(KoFilterEffectRegistry::instance());

    effectSelector->setModel(filterEffectModel);
    removeEffect->setIcon(koIcon("list-remove"));
    connect(removeEffect, SIGNAL(clicked()), this, SLOT(removeSelectedItem()));
    addEffect->setIcon(koIcon("list-add"));
    addEffect->setToolTip(i18n("Add effect to current filter stack"));
    connect(addEffect, SIGNAL(clicked()), this, SLOT(addSelectedEffect()));

    // TODO: make these buttons do something useful
    raiseEffect->setIcon(koIcon("arrow-up"));
    raiseEffect->hide();
    lowerEffect->setIcon(koIcon("arrow-down"));
    lowerEffect->hide();

    addPreset->setIcon(koIcon("list-add"));
    addPreset->setToolTip(i18n("Add to filter presets"));
    connect(addPreset, SIGNAL(clicked()), this, SLOT(addToPresets()));

    removePreset->setIcon(koIcon("list-remove"));
    removePreset->setToolTip(i18n("Remove filter preset"));
    connect(removePreset, SIGNAL(clicked()), this, SLOT(removeFromPresets()));

    view->setScene(m_scene);
    view->setRenderHint(QPainter::Antialiasing, true);
    view->setResizeAnchor(QGraphicsView::AnchorViewCenter);

    connect(m_scene, SIGNAL(connectionCreated(ConnectionSource,ConnectionTarget)),
            this, SLOT(connectionCreated(ConnectionSource,ConnectionTarget)));
    connect(m_scene, SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()));

    QSet<ConnectionSource::SourceType> inputs;
    inputs << ConnectionSource::SourceGraphic;
    inputs << ConnectionSource::SourceAlpha;
    inputs << ConnectionSource::BackgroundImage;
    inputs << ConnectionSource::BackgroundAlpha;
    inputs << ConnectionSource::FillPaint;
    inputs << ConnectionSource::StrokePaint;

    m_defaultSourceSelector = new KComboBox(this);
    Q_FOREACH (ConnectionSource::SourceType source, inputs) {
        m_defaultSourceSelector->addItem(ConnectionSource::typeToString(source));
    }
    m_defaultSourceSelector->hide();
    m_defaultSourceSelector->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connect(m_defaultSourceSelector, SIGNAL(currentIndexChanged(int)),
            this, SLOT(defaultSourceChanged(int)));
}

FilterEffectEditWidget::~FilterEffectEditWidget()
{
    if (!m_shape) {
        delete m_effects;
    }
}

void FilterEffectEditWidget::editShape(KoShape *shape, KoCanvasBase *canvas)
{
    if (!m_shape) {
        delete m_effects;
        m_effects = 0;
    }

    m_shape = shape;
    m_canvas = canvas;

    if (m_shape) {
        m_effects = m_shape->filterEffectStack();
    }
    if (!m_effects) {
        m_effects = new KoFilterEffectStack();
    }

    m_scene->initialize(m_effects);
    fitScene();
}

void FilterEffectEditWidget::fitScene()
{
    QRectF bbox = m_scene->itemsBoundingRect();
    m_scene->setSceneRect(bbox);
    bbox.adjust(-25, -25, 25, 25);
    view->centerOn(bbox.center());
    view->fitInView(bbox, Qt::KeepAspectRatio);
}

void FilterEffectEditWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    fitScene();
}

void FilterEffectEditWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    fitScene();
}

void FilterEffectEditWidget::addSelectedEffect()
{
    KoFilterEffectRegistry *registry = KoFilterEffectRegistry::instance();
    KoFilterEffectFactoryBase *factory = registry->values()[effectSelector->currentIndex()];
    if (!factory) {
        return;
    }

    KoFilterEffect *effect = factory->createFilterEffect();
    if (!effect) {
        return;
    }

    if (m_shape) {
        if (!m_shape->filterEffectStack()) {
            m_effects->appendFilterEffect(effect);
            m_canvas->addCommand(new FilterStackSetCommand(m_effects, m_shape));
        } else {
            m_canvas->addCommand(new FilterAddCommand(effect, m_shape));
        }
    } else {
        m_effects->appendFilterEffect(effect);
    }

    m_scene->initialize(m_effects);
    fitScene();
}

void FilterEffectEditWidget::removeSelectedItem()
{
    QList<ConnectionSource> selectedItems = m_scene->selectedEffectItems();
    if (!selectedItems.count()) {
        return;
    }

    QList<InputChangeData> changeData;
    QList<KoFilterEffect *> filterEffects = m_effects->filterEffects();
    int effectIndexToDelete = -1;

    const ConnectionSource &item  = selectedItems.first();
    KoFilterEffect *effect = item.effect();
    if (item.type() == ConnectionSource::Effect) {
        int effectIndex = filterEffects.indexOf(effect);
        // adjust inputs of all following effects in the stack
        for (int i = effectIndex + 1; i < filterEffects.count(); ++i) {
            KoFilterEffect *nextEffect = filterEffects[i];
            QList<QString> inputs = nextEffect->inputs();
            int inputIndex = 0;
            Q_FOREACH (const QString &input, inputs) {
                if (input == effect->output()) {
                    InputChangeData data(nextEffect, inputIndex, input, "");
                    changeData.append(data);
                }
            }
            // if one of the next effects has the same output name we stop
            if (nextEffect->output() == effect->output()) {
                break;
            }
        }
        effectIndexToDelete = effectIndex;
    } else {
        QString outputName = ConnectionSource::typeToString(item.type());
        QList<QString> inputs = effect->inputs();
        int inputIndex = 0;
        Q_FOREACH (const QString &input, inputs) {
            if (input == outputName) {
                InputChangeData data(effect, inputIndex, input, "");
                changeData.append(data);
            }
            inputIndex++;
        }
    }

    KUndo2Command *cmd = new KUndo2Command();
    if (changeData.count()) {
        KUndo2Command *subCmd = new FilterInputChangeCommand(changeData, m_shape, cmd);
        cmd->setText(subCmd->text());
    }
    if (effectIndexToDelete >= 0) {
        KUndo2Command *subCmd = new FilterRemoveCommand(effectIndexToDelete, m_effects, m_shape, cmd);
        cmd->setText(subCmd->text());
    }
    if (m_canvas && m_shape) {
        m_canvas->addCommand(cmd);
    } else {
        cmd->redo();
        delete cmd;
    }
    m_scene->initialize(m_effects);
    fitScene();
}

void FilterEffectEditWidget::connectionCreated(ConnectionSource source, ConnectionTarget target)
{
    QList<KoFilterEffect *> filterEffects = m_effects->filterEffects();

    int targetEffectIndex = filterEffects.indexOf(target.effect());
    if (targetEffectIndex < 0) {
        return;
    }

    QList<InputChangeData> changeData;
    QString sourceName;

    if (source.type() == ConnectionSource::Effect) {
        sourceName = source.effect()->output();
        int sourceEffectIndex = filterEffects.indexOf(source.effect());
        if (targetEffectIndex - sourceEffectIndex > 1) {
            // there are effects between source effect and target effect
            // so we have to take extra care
            bool renameOutput = false;
            if (sourceName.isEmpty()) {
                // output is not named so we have to rename the source output
                // and adjust the next effect in case it uses this output
                renameOutput = true;
            } else {
                // output is named but if there is an effect with the same
                // output name, we have to rename the source output
                for (int i = sourceEffectIndex + 1; i < targetEffectIndex; ++i) {
                    KoFilterEffect *effect = filterEffects[i];
                    if (effect->output() == sourceName) {
                        renameOutput = true;
                        break;
                    }
                }
            }
            if (renameOutput) {
                QSet<QString> uniqueOutputNames;
                Q_FOREACH (KoFilterEffect *effect, filterEffects) {
                    uniqueOutputNames.insert(effect->output());
                }
                int index = 0;
                QString newOutputName;
                do {
                    newOutputName = QString("result%1").arg(index);
                } while (uniqueOutputNames.contains(newOutputName));

                // rename source output
                source.effect()->setOutput(newOutputName);
                // adjust following effects
                for (int i = sourceEffectIndex + 1; i < targetEffectIndex; ++i) {
                    KoFilterEffect *effect = filterEffects[i];
                    int inputIndex = 0;
                    Q_FOREACH (const QString &input, effect->inputs()) {
                        if (input.isEmpty() && (i == sourceEffectIndex + 1 || input == sourceName)) {
                            InputChangeData data(effect, inputIndex, input, newOutputName);
                            changeData.append(data);
                        }
                        inputIndex++;
                    }
                    if (sourceName.isEmpty() || effect->output() == sourceName) {
                        break;
                    }
                }
                sourceName = newOutputName;
            }
        }
    } else {
        // source is an predefined input image
        sourceName = ConnectionSource::typeToString(source.type());
    }

    // finally set the input of the target
    if (target.inputIndex() >= target.effect()->inputs().count()) {
        // insert new input here
        target.effect()->addInput(sourceName);
    } else {
        QString oldInput = target.effect()->inputs()[target.inputIndex()];
        InputChangeData data(target.effect(), target.inputIndex(), oldInput, sourceName);
        changeData.append(data);
    }

    if (changeData.count()) {
        KUndo2Command *cmd = new FilterInputChangeCommand(changeData, m_shape);
        if (m_canvas) {
            m_canvas->addCommand(cmd);
        } else {
            cmd->redo();
            delete cmd;
        }
    }
    m_scene->initialize(m_effects);
    fitScene();
}

void FilterEffectEditWidget::addToPresets()
{
    if (!m_effects) {
        return;
    }

    bool ok = false;
    QString effectName = QInputDialog::getText(this, i18n("Effect name"),
                                               i18n("Please enter a name for the filter effect"),
                                               QLineEdit::Normal,
                                               QString(),
                                               &ok);
    if (!ok) {
        return;
    }

    QSharedPointer<FilterEffectResource> resource(FilterEffectResource::fromFilterEffectStack(m_effects));
    if (!resource) {
        return;
    }

    resource->setName(effectName);

    FilterResourceServerProvider *serverProvider = FilterResourceServerProvider::instance();
    KoResourceServer<FilterEffectResource> *server = serverProvider->filterEffectServer();

    QString savePath = server->saveLocation();

    int i = 1;
    QFileInfo fileInfo;

    do {
        fileInfo.setFile(savePath + QString("%1.svg").arg(i++, 4, 10, QChar('0')));
    } while (fileInfo.exists());

    resource->setFilename(fileInfo.filePath());
    resource->setValid(true);
}

void FilterEffectEditWidget::removeFromPresets()
{
    if (!presets->count()) {
        return;
    }

    FilterResourceServerProvider *serverProvider = FilterResourceServerProvider::instance();
    if (!serverProvider) {
        return;
    }

    KoResourceServer<FilterEffectResource> *server = serverProvider->filterEffectServer();
    if (!server) {
        return;
    }

    QSharedPointer<FilterEffectResource> resource = server->resources().at(presets->currentIndex());
    if (!resource) {
        return;
    }

    server->removeResourceFromServer(resource);
}

void FilterEffectEditWidget::presetSelected(KoResourceSP resource)
{
    QSharedPointer<FilterEffectResource> effectResource = resource.dynamicCast<FilterEffectResource>();
    if (!effectResource) {
        return;
    }

    KoFilterEffectStack *filterStack = effectResource->toFilterStack();
    if (!filterStack) {
        return;
    }

    if (m_shape) {
        KUndo2Command *cmd = new FilterStackSetCommand(filterStack, m_shape);
        if (m_canvas) {
            m_canvas->addCommand(cmd);
        } else {
            cmd->redo();
            delete cmd;
        }
    } else {
        delete m_effects;
    }
    m_effects = filterStack;

    m_scene->initialize(m_effects);
    fitScene();
}

void FilterEffectEditWidget::addWidgetForItem(ConnectionSource item)
{
    // get the filter effect from the item
    KoFilterEffect *filterEffect = item.effect();
    if (item.type() != ConnectionSource::Effect) {
        filterEffect = 0;
    }

    KoFilterEffect *currentEffect = m_currentItem.effect();
    if (m_currentItem.type() != ConnectionSource::Effect) {
        currentEffect = 0;
    }

    m_defaultSourceSelector->hide();

    // remove current widget if new effect is zero or effect type has changed
    if (!filterEffect || !currentEffect || (filterEffect->id() != currentEffect->id())) {
        while (configStack->count()) {
            configStack->removeWidget(configStack->widget(0));
        }
    }

    m_currentItem = item;

    KoFilterEffectConfigWidgetBase *currentPanel = 0;

    if (!filterEffect) {
        if (item.type() != ConnectionSource::Effect) {
            configStack->insertWidget(0, m_defaultSourceSelector);
            m_defaultSourceSelector->blockSignals(true);
            m_defaultSourceSelector->setCurrentIndex(item.type() - 1);
            m_defaultSourceSelector->blockSignals(false);
            m_defaultSourceSelector->show();
        }
    }  else if (!currentEffect || currentEffect->id() != filterEffect->id()) {
        // when a shape is set and is differs from the previous one
        // get the config widget and insert it into the option widget

        KoFilterEffectRegistry *registry = KoFilterEffectRegistry::instance();
        KoFilterEffectFactoryBase *factory = registry->value(filterEffect->id());
        if (!factory) {
            return;
        }

        currentPanel = factory->createConfigWidget();
        if (!currentPanel) {
            return;
        }

        configStack->insertWidget(0, currentPanel);
        connect(currentPanel, SIGNAL(filterChanged()), this, SLOT(filterChanged()));
    }

    currentPanel = qobject_cast<KoFilterEffectConfigWidgetBase *>(configStack->widget(0));
    if (currentPanel) {
        currentPanel->editFilterEffect(filterEffect);
    }
}

void FilterEffectEditWidget::filterChanged()
{
    if (m_shape) {
        m_shape->update();
    }
}

void FilterEffectEditWidget::sceneSelectionChanged()
{
    QList<ConnectionSource> selectedItems = m_scene->selectedEffectItems();
    if (!selectedItems.count()) {
        addWidgetForItem(ConnectionSource());
    } else {
        addWidgetForItem(selectedItems.first());
    }
}

void FilterEffectEditWidget::defaultSourceChanged(int index)
{
    if (m_currentItem.type() == ConnectionSource::Effect) {
        return;
    }

    KoFilterEffect *filterEffect = m_currentItem.effect();
    if (!filterEffect) {
        return;
    }

    QString oldInput = ConnectionSource::typeToString(m_currentItem.type());
    QString newInput = m_defaultSourceSelector->itemText(index);

    const QString defInput = "SourceGraphic";
    int effectIndex = m_effects->filterEffects().indexOf(filterEffect);

    InputChangeData data;
    int inputIndex = 0;
    Q_FOREACH (const QString &input, filterEffect->inputs()) {
        if (input == oldInput || (effectIndex == 0 && oldInput == defInput)) {
            data = InputChangeData(filterEffect, inputIndex, input, newInput);
            break;
        }
        inputIndex++;
    }
    KUndo2Command *cmd = new FilterInputChangeCommand(data, m_shape);
    if (m_canvas && m_shape) {
        m_canvas->addCommand(cmd);
    } else {
        cmd->redo();
        delete cmd;
    }

    m_scene->initialize(m_effects);
    fitScene();
}
