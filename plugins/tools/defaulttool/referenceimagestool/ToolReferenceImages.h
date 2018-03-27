/*
 * Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TOOL_REFERENCE_IMAGES_H
#define TOOL_REFERENCE_IMAGES_H

#include <QPointer>

#include <KoToolFactoryBase.h>
#include <KoIcon.h>

#include <kis_tool.h>
#include "kis_painting_assistant.h"
#include <kis_icon.h>
#include <kis_canvas2.h>

#include <defaulttool/DefaultTool.h>

class ToolReferenceImagesWidget;
class KisReferenceImagesLayer;

class ToolReferenceImages : public DefaultTool
{
    Q_OBJECT

public:
    ToolReferenceImages(KoCanvasBase * canvas);
    ~ToolReferenceImages() override;

    virtual quint32 priority() {
        return 3;
    }

    void mouseDoubleClickEvent(KoPointerEvent *event) override;

protected:
    QList<QPointer<QWidget>> createOptionWidgets() override;
    QWidget *createOptionWidget() override;

    bool isValidForCurrentLayer() const override;
    KoShapeManager *shapeManager() const override;
    KoSelection *koSelection() const override;

public Q_SLOTS:
    void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes) override;
    void deactivate() override;

    void addReferenceImage();
    void removeAllReferenceImages();
    void saveReferenceImages();
    void loadReferenceImages();

    void slotSelectionChanged();

private:
    friend class ToolReferenceImagesWidget;
    ToolReferenceImagesWidget *m_optionsWidget = nullptr;

    KisReferenceImagesLayer *referenceImagesLayer() const;
    KisReferenceImagesLayer *getOrCreteReferenceImagesLayer();
};


class ToolReferenceImagesFactory : public KoToolFactoryBase
{
public:
    ToolReferenceImagesFactory()
    : KoToolFactoryBase("ToolReferenceImages") {
        setToolTip(i18n("Reference Images Tool"));
        setSection(TOOL_TYPE_VIEW);
        setIconName(koIconNameCStr("krita_tool_reference_images"));
        setPriority(0);
        setActivationShapeId(KRITA_TOOL_ACTIVATION_ID);
    };


    ~ToolReferenceImagesFactory() override {}

    KoToolBase * createTool(KoCanvasBase * canvas) override {
        return new ToolReferenceImages(canvas);
    }

};


#endif

