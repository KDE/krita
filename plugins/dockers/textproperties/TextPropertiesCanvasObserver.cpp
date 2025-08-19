/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TextPropertiesCanvasObserver.h"

#include <KLocalizedContext>

#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <kis_signal_compressor.h>

#include <KisResourceModel.h>
#include <KisResourceUserOperations.h>

#include <lager/state.hpp>
#include <QApplication>

#include "TextPropertyConfigDialog.h"
#include "CssStylePresetEditDialog.h"

struct TextPropertiesCanvasObserver::Private {

    Private(QObject *parent = nullptr)
        : modelToProviderCompressor(KisSignalCompressor(100, KisSignalCompressor::FIRST_ACTIVE, parent)) {

    }

    KisSignalCompressor modelToProviderCompressor;

    KoSvgTextPropertiesModel *textModel {new KoSvgTextPropertiesModel()};
    KisCanvasResourceProvider *provider{nullptr};
    TextPropertyConfigModel *textPropertyConfigModel {nullptr};
    qreal currentDpi{72.0};
    QStringList locales;
};

TextPropertiesCanvasObserver::TextPropertiesCanvasObserver(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    d->textPropertyConfigModel = new TextPropertyConfigModel(this);
    QList<QLocale> locales;
    QStringList wellFormedBCPNames;
    Q_FOREACH (const QString langCode, KLocalizedString::languages()) {
        locales.append(QLocale(langCode));
        wellFormedBCPNames.append(langCode.split("_").join("-"));
    }
    d->locales = wellFormedBCPNames;

    connect(d->textModel, SIGNAL(textPropertyChanged()),
            &d->modelToProviderCompressor, SLOT(start()));
    connect(&d->modelToProviderCompressor, SIGNAL(timeout()), this, SLOT(slotTextPropertiesChanged()));
    connect(d->textModel, SIGNAL(textPropertyChanged()),
            this, SIGNAL(textPropertiesChanged()));
}

TextPropertiesCanvasObserver::~TextPropertiesCanvasObserver()
{

}

void TextPropertiesCanvasObserver::setViewManager(KisViewManager *kisview)
{
    d->provider = kisview->canvasResourceProvider();
    if (d->provider) {
        connect(d->provider, SIGNAL(sigTextPropertiesChanged()),
                this, SLOT(slotCanvasTextPropertiesChanged()));

        // This initializes the docker to an empty entry;
        KoSvgTextPropertyData textData;
        textData.inheritedProperties = KoSvgTextProperties();
        d->provider->setTextPropertyData(textData);
    }
}

void TextPropertiesCanvasObserver::setCanvas(KoCanvasBase *canvas)
{
    //setEnabled(true);

    if (m_canvas == canvas) {
        return;
    }

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
    }

    KIS_ASSERT(canvas);

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    if (m_canvas && m_canvas->currentImage()) {
        d->currentDpi = m_canvas->currentImage()->xRes() * 72.0;
        Q_EMIT dpiChanged();
    }
}

void TextPropertiesCanvasObserver::unsetCanvas()
{
    m_canvas = 0;
}

qreal TextPropertiesCanvasObserver::dpi() const
{
    return d->currentDpi;
}

QStringList TextPropertiesCanvasObserver::locales() const
{
    return d->locales;
}

KoSvgTextPropertiesModel *TextPropertiesCanvasObserver::textProperties() const
{
    return d->textModel;
}

TextPropertyConfigModel *TextPropertiesCanvasObserver::textPropertyConfig() const
{
    return d->textPropertyConfigModel;
}

void TextPropertiesCanvasObserver::slotCanvasTextPropertiesChanged()
{
    KoSvgTextPropertyData data = d->provider->textPropertyData();
    if (d->textModel->textData.get() != data && !d->modelToProviderCompressor.isActive()) {
        d->textModel->textData.set(data);
        Q_EMIT textPropertiesChanged();
    }
}

void TextPropertiesCanvasObserver::slotTextPropertiesChanged()
{
    KoSvgTextPropertyData textData = d->textModel->textData.get();
    debugFlake << Q_FUNC_INFO << textData;
    if (d->provider && d->provider->textPropertyData() != textData) {
        d->provider->setTextPropertyData(textData);
    }
}

void TextPropertiesCanvasObserver::callModalTextPropertyConfigDialog()
{
    TextPropertyConfigDialog dialog(qApp->activeWindow());
    dialog.setTextPropertyConfigModel(d->textPropertyConfigModel);
    if (dialog.exec() == QDialog::Accepted) {
        dialog.model()->saveConfiguration();
        Q_EMIT textPropertyConfigChanged();
    }
}

void TextPropertiesCanvasObserver::applyPreset(KoResourceSP resource)
{
    KoCssStylePresetSP preset = resource.staticCast<KoCssStylePreset>();
    if (!preset) return;
    KoSvgTextPropertyData textData = d->textModel->textData.get();
    KoSvgTextProperties properties = preset->properties(d->currentDpi, true);
    Q_FOREACH(KoSvgTextProperties::PropertyId p, properties.properties()) {
        textData.commonProperties.setProperty(p, properties.property(p));
    }
    d->textModel->textData.set(textData);
}

bool TextPropertiesCanvasObserver::createNewPresetFromSettings()
{
    KoCssStylePresetSP preset(new KoCssStylePreset(QString()));
    KoSvgTextPropertyData textData = d->textModel->textData.get();
    preset->setProperties(textData.commonProperties);
    preset->setName(i18nc("@info:placeholder", "New Style Preset"));
    preset->setStyleType("character");
    preset->updateThumbnail();

    CssStylePresetEditDialog *dialog = new CssStylePresetEditDialog(qApp->activeWindow());
    dialog->setDpi(d->currentDpi);
    dialog->setCurrentResource(preset);

    if (dialog->exec() == QDialog::Accepted) {
        return KisResourceUserOperations::addResourceWithUserInput(qApp->activeWindow(), dialog->currentResource());
    }

    return false;
}

void TextPropertiesCanvasObserver::editPreset(KoResourceSP resource)
{
    KoCssStylePresetSP preset = resource.staticCast<KoCssStylePreset>();
    if (!preset) return;

    CssStylePresetEditDialog *dialog = new CssStylePresetEditDialog(qApp->activeWindow());
    dialog->setCurrentResource(preset);

    if (dialog->exec() == QDialog::Accepted) {
        KisResourceUserOperations::updateResourceWithUserInput(qApp->activeWindow(), dialog->currentResource());
    } else {
        KisResourceModel model = KisResourceModel(ResourceType::CssStyles, qApp->activeWindow());
        model.reloadResource(preset);
    }
}

void TextPropertiesCanvasObserver::cloneAndEditPreset(KoResourceSP resource)
{
    KoResourceSP newResource = resource->clone();
    KoCssStylePresetSP preset = newResource.staticCast<KoCssStylePreset>();
    if (!preset) return;

    CssStylePresetEditDialog *dialog = new CssStylePresetEditDialog(qApp->activeWindow());
    dialog->setCurrentResource(preset);

    if (dialog->exec() == QDialog::Accepted) {
        preset = dialog->currentResource();
        preset->setFilename(preset->name().replace(' ', '_').replace('.', '_') + preset->defaultFileExtension());
        KisResourceUserOperations::addResourceWithUserInput(qApp->activeWindow(), preset);
    }
}
