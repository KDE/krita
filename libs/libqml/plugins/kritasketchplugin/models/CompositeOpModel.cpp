/*
    Copyright (C) 2012  Dan Leinir Turthra Jensen <admin@leinir.dk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "CompositeOpModel.h"
#include <kis_composite_ops_model.h>
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_tool.h>
#include <kis_canvas2.h>
#include <input/kis_input_manager.h>
#include <kis_node_manager.h>
#include <kis_node.h>
#include <kis_layer.h>
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_settings.h>
#include <brushengine/kis_paintop_registry.h>
#include <brushengine/kis_paintop_config_widget.h>
#include <KoCompositeOpRegistry.h>
#include <KoColorSpace.h>
#include <KoToolManager.h>

class CompositeOpModel::Private
{
public:
    Private(CompositeOpModel* qq)
        : q(qq)
        , model(new KisCompositeOpListModel())
        , view(0)
        , eraserMode(0)
        , opacity(0)
        , opacityEnabled(false)
        , flow(0)
        , flowEnabled(false)
        , size(0)
        , sizeEnabled(false)
        , presetsEnabled(true)
    {};

    CompositeOpModel* q;
    KisCompositeOpListModel* model;
    KisViewManager* view;
    QString currentCompositeOpID;
    QString prevCompositeOpID;
    bool eraserMode;
    QMap<KisPaintOpPreset*, KisPaintOpConfigWidget*> settingsWidgets;

    qreal opacity;
    bool opacityEnabled;
    qreal flow;
    bool flowEnabled;
    qreal size;
    bool sizeEnabled;
    bool presetsEnabled;
    KisPaintOpPresetSP currentPreset;

    void updateCompositeOp(QString compositeOpID)
    {
        if (!view)
            return;

        KisNodeSP node = view->canvasResourceProvider()->currentNode();

        if (node && node->paintDevice())
        {
            if (!node->paintDevice()->colorSpace()->hasCompositeOp(compositeOpID))
                compositeOpID = KoCompositeOpRegistry::instance().getDefaultCompositeOp().id();

            if (compositeOpID != currentCompositeOpID)
            {
                q->setEraserMode(compositeOpID == COMPOSITE_ERASE);
                currentPreset->settings()->setProperty("CompositeOp", compositeOpID);
                //m_optionWidget->setConfiguration(m_activePreset->settings().data());
                view->canvasResourceProvider()->setCurrentCompositeOp(compositeOpID);
                prevCompositeOpID = currentCompositeOpID;
                currentCompositeOpID = compositeOpID;
            }
        }
        emit q->currentCompositeOpIDChanged();
    }

    void ofsChanged()
    {
        if (presetsEnabled && !currentPreset.isNull() && !currentPreset->settings().isNull())
        {
            // IMPORTANT: set the PaintOp size before setting the other properties
            //            it wont work the other way
            //qreal sizeDiff = size - currentPreset->settings()->paintOpSize();
            //currentPreset->settings()->changePaintOpSize(sizeDiff, 0);

            if (currentPreset->settings()->hasProperty("OpacityValue"))
                currentPreset->settings()->setProperty("OpacityValue", opacity);

            if (currentPreset->settings()->hasProperty("FlowValue"))
                currentPreset->settings()->setProperty("FlowValue", flow);

            //m_optionWidget->setConfiguration(d->currentPreset->settings().data());
        }
        if (view)
        {
            view->canvasResourceProvider()->setOpacity(opacity);
        }
    }
};

CompositeOpModel::CompositeOpModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*,int)),
            this, SLOT(slotToolChanged(KoCanvasController*,int)));

}

CompositeOpModel::~CompositeOpModel()
{
    delete d;
}

QHash<int, QByteArray> CompositeOpModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[TextRole] = "text";
    roles[IsCategoryRole] = "isCategory";
    return roles;
}

QVariant CompositeOpModel::data(const QModelIndex& index, int role) const
{
    QVariant data;
    if (index.isValid())
    {
        QModelIndex otherIndex = d->model->index(index.row(), index.column(), QModelIndex());
        switch(role)
        {
            case TextRole:
                data = d->model->data(otherIndex, Qt::DisplayRole);
                break;
            case IsCategoryRole:
                data = d->model->data(otherIndex, __CategorizedListModelBase::IsHeaderRole);
                break;
            default:
                break;
        }
    }
    return data;
}

int CompositeOpModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return d->model->rowCount(QModelIndex());
}

void CompositeOpModel::activateItem(int index)
{
    if (index > -1 && index < d->model->rowCount(QModelIndex()))
    {
        KoID compositeOp;
        if (d->model->entryAt(compositeOp, d->model->index(index)))
            d->updateCompositeOp(compositeOp.id());
    }
}

QObject* CompositeOpModel::view() const
{
    return d->view;
}

void CompositeOpModel::setView(QObject* newView)
{
    if (d->view)
    {
        d->view->canvasBase()->disconnect(this);
        d->view->canvasBase()->globalInputManager()->disconnect(this);
        d->view->nodeManager()->disconnect(this);
    }
    d->view = qobject_cast<KisViewManager*>( newView );
    if (d->view)
    {
        if (d->view->canvasBase() && d->view->canvasBase()->resourceManager()) {
            connect(d->view->canvasBase()->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
                    this, SLOT(resourceChanged(int,QVariant)));
        }
        if (d->view->nodeManager()) {
            connect(d->view->nodeManager(), SIGNAL(sigLayerActivated(KisLayerSP)),
                    this, SLOT(currentNodeChanged(KisLayerSP)));
        }
        slotToolChanged(0, 0);
    }
    emit viewChanged();
}

bool CompositeOpModel::eraserMode() const
{
    return d->eraserMode;
}

void CompositeOpModel::setEraserMode(bool newEraserMode)
{
    if (d->eraserMode != newEraserMode)
    {
        d->eraserMode = newEraserMode;
        if (d->eraserMode)
            d->updateCompositeOp(COMPOSITE_ERASE);
        else
            d->updateCompositeOp(d->prevCompositeOpID);
        emit eraserModeChanged();
    }
}

qreal CompositeOpModel::flow() const
{
    return d->flow;
}

void CompositeOpModel::setFlow(qreal newFlow)
{
    if (d->flow != newFlow)
    {
        d->flow = newFlow;
        d->ofsChanged();
        emit flowChanged();
    }
}

bool CompositeOpModel::flowEnabled() const
{
    return d->flowEnabled;
}

void CompositeOpModel::setFlowEnabled(bool newFlowEnabled)
{
    d->flowEnabled = newFlowEnabled;
    emit flowEnabledChanged();
}

qreal CompositeOpModel::opacity() const
{
    return d->opacity;
}

void CompositeOpModel::setOpacity(qreal newOpacity)
{
    if (d->opacity != newOpacity)
    {
        d->opacity = newOpacity;
        d->ofsChanged();
        emit opacityChanged();
    }
}

bool CompositeOpModel::opacityEnabled() const
{
    return d->opacityEnabled;
}

void CompositeOpModel::setOpacityEnabled(bool newOpacityEnabled)
{
    d->opacityEnabled = newOpacityEnabled;
    emit opacityEnabledChanged();
}

qreal CompositeOpModel::size() const
{
    return d->size;
}

void CompositeOpModel::setSize(qreal newSize)
{
    if (d->size != newSize)
    {
        d->size = newSize;
        d->ofsChanged();
        emit sizeChanged();
    }
}

bool CompositeOpModel::sizeEnabled() const
{
    return d->sizeEnabled;
}

void CompositeOpModel::setSizeEnabled(bool newSizeEnabled)
{
    d->sizeEnabled = newSizeEnabled;
    emit sizeEnabledChanged();
}

void CompositeOpModel::changePaintopValue(QString propertyName, QVariant value)
{
    if (propertyName == "size" && value.toReal() != d->size)
        setSize(value.toReal());
    else if (propertyName == "opacity" && value.toReal() != d->opacity)
        setOpacity(value.toReal());
    else if (propertyName == "flow" && value.toReal() != d->flow)
        setFlow(value.toReal());
}

bool CompositeOpModel::mirrorHorizontally() const
{
    if (d->view)
        return d->view->canvasResourceProvider()->mirrorHorizontal();
    return false;
}

void CompositeOpModel::setMirrorHorizontally(bool newMirrorHorizontally)
{
    if (d->view && d->view->canvasResourceProvider()->mirrorHorizontal() != newMirrorHorizontally)
    {
        d->view->canvasResourceProvider()->setMirrorHorizontal(newMirrorHorizontally);
        emit mirrorHorizontallyChanged();
    }
}

bool CompositeOpModel::mirrorVertically() const
{
    if (d->view)
        return d->view->canvasResourceProvider()->mirrorVertical();
    return false;
}

void CompositeOpModel::setMirrorVertically(bool newMirrorVertically)
{
    if (d->view && d->view->canvasResourceProvider()->mirrorVertical() != newMirrorVertically)
    {
        d->view->canvasResourceProvider()->setMirrorVertical(newMirrorVertically);
        emit mirrorVerticallyChanged();
    }
}

void CompositeOpModel::slotToolChanged(KoCanvasController* canvas, int toolId)
{
    Q_UNUSED(canvas);
    Q_UNUSED(toolId);

    if (!d->view) return;
    if (!d->view->canvasBase()) return;

    QString  id   = KoToolManager::instance()->activeToolId();
    KisTool* tool = dynamic_cast<KisTool*>(KoToolManager::instance()->toolById(d->view->canvasBase(), id));

    if (tool) {
        int flags = tool->flags();

        if (flags & KisTool::FLAG_USES_CUSTOM_COMPOSITEOP) {
            //setWidgetState(ENABLE_COMPOSITEOP|ENABLE_OPACITY);
            d->opacityEnabled = true;
        }
        else {
            //setWidgetState(DISABLE_COMPOSITEOP|DISABLE_OPACITY);
            d->opacityEnabled = false;
        }

        if (flags & KisTool::FLAG_USES_CUSTOM_PRESET) {
            d->flowEnabled = true;
            d->sizeEnabled = true;
            d->presetsEnabled = true;
        }
        else {
            d->flowEnabled = false;
            d->sizeEnabled = false;
            d->presetsEnabled = false;
        }
    }
    else {
        d->opacityEnabled = false;
        d->flowEnabled = false;
        d->sizeEnabled = false;
    }
    emit opacityEnabledChanged();
    emit flowEnabledChanged();
    emit sizeEnabledChanged();
}

void CompositeOpModel::resourceChanged(int key, const QVariant& /*v*/)
{
    if (d->view && d->view->canvasBase() && d->view->canvasBase()->resourceManager() && d->view->canvasResourceProvider()) {

        if (key == KisCanvasResourceProvider::MirrorHorizontal) {
            emit mirrorHorizontallyChanged();
            return;
        }
        else if(key == KisCanvasResourceProvider::MirrorVertical) {
            emit mirrorVerticallyChanged();
            return;
        }

        KisPaintOpPresetSP preset = d->view->canvasBase()->resourceManager()->resource(KisCanvasResourceProvider::CurrentPaintOpPreset).value<KisPaintOpPresetSP>();

        if (preset && d->currentPreset.data() != preset.data()) {
            d->currentPreset = preset;
            if (!d->settingsWidgets.contains(preset.data())) {
                d->settingsWidgets[preset.data()] = KisPaintOpRegistry::instance()->get(preset->paintOp().id())->createConfigWidget(0);
                d->settingsWidgets[preset.data()]->setImage(d->view->image());
                d->settingsWidgets[preset.data()]->setConfiguration(preset->settings());
            }

            if (d->settingsWidgets[preset.data()]) {
                preset->settings()->setOptionsWidget(d->settingsWidgets[preset.data()]);
            }

            d->size = preset->settings()->paintOpSize();
            emit sizeChanged();

            if (preset->settings()->hasProperty("OpacityValue"))  {
                d->opacityEnabled = true;
                d->opacity = preset->settings()->getProperty("OpacityValue").toReal();
            }
            else {
                d->opacityEnabled = false;
                d->opacity = 1;
            }

            d->view->canvasResourceProvider()->setOpacity(d->opacity);
            emit opacityChanged();
            emit opacityEnabledChanged();

            if (preset->settings()->hasProperty("FlowValue")) {
                d->flowEnabled = true;
                d->flow = preset->settings()->getProperty("FlowValue").toReal();
            }
            else {
                d->flowEnabled = false;
                d->flow = 1;
            }
            emit flowChanged();
            emit flowEnabledChanged();

            QString compositeOp = preset->settings()->getString("CompositeOp");

            // This is a little odd, but the logic here is that the opposite of an eraser is a normal composite op (so we just select over, aka normal)
            // This means that you can switch your eraser over to being a painting tool by turning off the eraser again.
            if (compositeOp == COMPOSITE_ERASE) {
                d->currentCompositeOpID = COMPOSITE_OVER;
                d->eraserMode = true;
            }
            else {
                d->eraserMode = false;
            }
            emit eraserModeChanged();
            d->updateCompositeOp(compositeOp);
        }
    }
}

void CompositeOpModel::currentNodeChanged(KisLayerSP newNode)
{
    Q_UNUSED(newNode);
    if (d->eraserMode) {
        d->eraserMode = false;
        d->updateCompositeOp(d->prevCompositeOpID);
        emit eraserModeChanged();
    }
}

int CompositeOpModel::indexOf(QString compositeOpId)
{
    return d->model->indexOf(KoID(compositeOpId)).row();
}

QString CompositeOpModel::currentCompositeOpID() const
{
    return d->currentCompositeOpID;
}

