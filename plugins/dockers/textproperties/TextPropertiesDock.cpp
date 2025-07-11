/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TextPropertiesDock.h"

#include <lager/state.hpp>

#include <QQuickWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QStringListModel>
#include <QQuickStyle>
#include <QColorDialog>

#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <KisStaticInitializer.h>

#include <KLocalizedContext>

#include <KoResourcePaths.h>
#include <KisResourceModel.h>
#include <KisResourceModelProvider.h>
#include <KisTagModel.h>
#include <kis_signal_compressor.h>
#include <KisResourceUserOperations.h>

#include <KoCanvasResourcesIds.h>
#include <KoSvgTextPropertyData.h>
#include <text/lager/KoSvgTextPropertiesModel.h>
#include <text/lager/CssLengthPercentageModel.h>
#include <text/lager/LineHeightModel.h>
#include <text/lager/TextIndentModel.h>
#include <text/lager/TabSizeModel.h>
#include <text/lager/TextTransformModel.h>
#include <text/lager/CssFontStyleModel.h>
#include <text/lager/FontVariantLigaturesModel.h>
#include <text/lager/FontVariantNumericModel.h>
#include <text/lager/FontVariantEastAsianModel.h>
#include <resources/KoFontFamily.h>
#include <resources/KoCssStylePreset.h>
#include <lager/state.hpp>

#include "FontStyleModel.h"
#include "FontAxesModel.h"
#include "KoShapeQtQuickLabel.h"
#include "OpenTypeFeatureModel.h"
#include "TagFilterProxyModelQmlWrapper.h"
#include "LocaleHandler.h"
#include "CssQmlUnitConverter.h"
#include "TextPropertyConfigModel.h"
#include <KisSurfaceColorSpaceWrapper.h>

#include "TextPropertyConfigDialog.h"
#include "CssStylePresetEditDialog.h"

/// Strange place to put this, do we have a better spot?
KIS_DECLARE_STATIC_INITIALIZER {
    qmlRegisterType<KoSvgTextPropertiesModel>("org.krita.flake.text", 1, 0, "KoSvgTextPropertiesModel");
    qmlRegisterType<CssLengthPercentageModel>("org.krita.flake.text", 1, 0, "CssLengthPercentageModel");
    qmlRegisterType<LineHeightModel>("org.krita.flake.text", 1, 0, "LineHeightModel");
    qmlRegisterType<TextIndentModel>("org.krita.flake.text", 1, 0, "TextIndentModel");
    qmlRegisterType<TabSizeModel>("org.krita.flake.text", 1, 0, "TabSizeModel");
    qmlRegisterType<TextTransformModel>("org.krita.flake.text", 1, 0, "TextTransformModel");
    qmlRegisterType<CssFontStyleModel>("org.krita.flake.text", 1, 0, "CssFontStyleModel");
    qmlRegisterType<FontVariantLigaturesModel>("org.krita.flake.text", 1, 0, "FontVariantLigaturesModel");
    qmlRegisterType<FontVariantNumericModel>("org.krita.flake.text", 1, 0, "FontVariantNumericModel");
    qmlRegisterType<FontVariantEastAsianModel>("org.krita.flake.text", 1, 0, "FontVariantEastAsianModel");
    qmlRegisterUncreatableMetaObject(KoSvgText::staticMetaObject, "org.krita.flake.text", 1, 0, "KoSvgText", "Error: Namespace with enums");

    qmlRegisterType<FontStyleModel>("org.krita.flake.text", 1, 0, "FontStyleModel");
    qmlRegisterType<FontAxesModel>("org.krita.flake.text", 1, 0, "FontAxesModel");
    qmlRegisterType<OpenTypeFeatureFilterModel>("org.krita.flake.text", 1, 0, "OpenTypeFeatureFilterModel");
    qmlRegisterType<OpenTypeFeatureModel>("org.krita.flake.text", 1, 0, "OpenTypeFeatureModel");
    qmlRegisterType<KoShapeQtQuickLabel>("org.krita.flake.text", 1, 0, "KoShapeQtQuickLabel");
    qmlRegisterType<CssQmlUnitConverter>("org.krita.flake.text", 1, 0, "CssQmlUnitConverter");
    qmlRegisterType<TagFilterProxyModelQmlWrapper>("org.krita.flake.text", 1, 0, "TagFilterProxyModelQmlWrapper");
    qmlRegisterType<LocaleHandler>("org.krita.flake.text", 1, 0, "LocaleHandler");
    qmlRegisterType<TextPropertyConfigModel>("org.krita.flake.text", 1, 0, "TextPropertyConfigModel");
    qmlRegisterType<TextPropertyConfigFilterModel>("org.krita.flake.text", 1, 0, "TextPropertyConfigFilterModel");
}

struct TextPropertiesDock::Private
{
    Private(QObject *parent = nullptr)
        : modelToProviderCompressor(KisSignalCompressor(200, KisSignalCompressor::FIRST_ACTIVE, parent))
        , providerToModelCompressor(KisSignalCompressor(200, KisSignalCompressor::FIRST_ACTIVE, parent)){

    }
    KoSvgTextPropertiesModel *textModel {new KoSvgTextPropertiesModel()};
    KisResourceModel *fontModel{nullptr};
    KisCanvasResourceProvider *provider{nullptr};
    TextPropertyConfigModel *textPropertyConfigModel {nullptr};
    qreal currentDpi{72.0};

    KisSignalCompressor modelToProviderCompressor;
    KisSignalCompressor providerToModelCompressor;
};

TextPropertiesDock::TextPropertiesDock()
    : QDockWidget(i18n("Text Properties"))
    , d(new Private(this))
{
    m_quickWidget = new QQuickWidget(this);

#if !defined(Q_OS_MACOS) || QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QSurfaceFormat format;

    {
        /**
         * Our version of Qt has a special handling of the color
         * space passed to the surface of the QQuickWidget. It will
         * allow it to render correctly on a Rec2020PQ window.
         */
        format.setRedBufferSize(8);
        format.setGreenBufferSize(8);
        format.setBlueBufferSize(8);
        format.setAlphaBufferSize(8);
        format.setColorSpace(KisSurfaceColorSpaceWrapper::makeSRGBColorSpace());
    }

    m_quickWidget->setFormat(format);
#endif
    setWidget(m_quickWidget);
    setEnabled(true);

    m_quickWidget->engine()->rootContext()->setContextProperty("mainWindow", this);
    m_quickWidget->engine()->rootContext()->setContextObject(new KLocalizedContext(this));

    // Default to fusion style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
         QQuickStyle::setStyle(QStringLiteral("Fusion"));
    }

    m_quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    m_quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

    m_quickWidget->engine()->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    m_quickWidget->engine()->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

    m_quickWidget->setMinimumHeight(100);

    d->textPropertyConfigModel = new TextPropertyConfigModel(this);

    QList<QLocale> locales;
    QStringList wellFormedBCPNames;
    Q_FOREACH (const QString langCode, KLocalizedString::languages()) {
        locales.append(QLocale(langCode));
        wellFormedBCPNames.append(langCode.split("_").join("-"));
    }


    m_quickWidget->rootContext()->setContextProperty("textPropertiesModel", d->textModel);
    m_quickWidget->rootContext()->setContextProperty("textPropertyConfigModel", QVariant::fromValue(d->textPropertyConfigModel));
    m_quickWidget->rootContext()->setContextProperty("locales", QVariant::fromValue(wellFormedBCPNames));
    connect(d->textModel, SIGNAL(textPropertyChanged()),
            &d->modelToProviderCompressor, SLOT(start()));
    connect(&d->modelToProviderCompressor, SIGNAL(timeout()), this, SLOT(slotTextPropertiesChanged()));
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setSource(QUrl("qrc:/TextProperties.qml"));

    m_quickWidget->setPalette(this->palette());
}

TextPropertiesDock::~TextPropertiesDock()
{
    /// Prevent accessing destroyed objects in QML engine
    /// See:
    ///   * https://invent.kde.org/graphics/krita/-/commit/d8676f4e9cac1a8728e73fec3ff1df1763c713b7
    ///   * https://bugreports.qt.io/browse/QTBUG-81247
    m_quickWidget->setParent(nullptr);
    delete m_quickWidget;
}

void TextPropertiesDock::setViewManager(KisViewManager *kisview)
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

void TextPropertiesDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(true);

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
        m_quickWidget->rootObject()->setProperty("canvasDPI", QVariant::fromValue(d->currentDpi));
    }
}

void TextPropertiesDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = 0;
}

QColor TextPropertiesDock::modalColorDialog(QColor oldColor)
{
    QColor c = QColorDialog::getColor(oldColor);
    return c.isValid()? c: oldColor;
}

void TextPropertiesDock::callModalTextPropertyConfigDialog()
{
    TextPropertyConfigDialog dialog(this);
    dialog.setTextPropertyConfigModel(d->textPropertyConfigModel);
    if (dialog.exec() == QDialog::Accepted) {
        dialog.model()->saveConfiguration();
        QMetaObject::invokeMethod(m_quickWidget->rootObject(), "updatePropertyVisibilityState");
    }
}

void TextPropertiesDock::slotCanvasTextPropertiesChanged()
{
    KoSvgTextPropertyData data = d->provider->textPropertyData();
    if (d->textModel->textData.get() != data) {
        d->textModel->textData.set(data);
    }
}

void TextPropertiesDock::slotTextPropertiesChanged()
{
    KoSvgTextPropertyData textData = d->textModel->textData.get();
    debugFlake << Q_FUNC_INFO << textData;
    if (d->provider && d->provider->textPropertyData() != textData) {
        d->provider->setTextPropertyData(textData);
    }
    QMetaObject::invokeMethod(m_quickWidget->rootObject(), "setProperties");
}

#include <KoFontRegistry.h>
QString TextPropertiesDock::wwsFontFamilyName(QString familyName)
{
    std::optional<QString> name = KoFontRegistry::instance()->wwsNameByFamilyName(familyName);
    if (!name) {
        return familyName;
    }
    return name.value();
}

void TextPropertiesDock::applyPreset(KoResourceSP resource)
{
    KoCssStylePresetSP preset = resource.staticCast<KoCssStylePreset>();
    if (!preset) return;
    KoSvgTextPropertyData textData = d->textModel->textData.get();
    KoSvgTextProperties properties = preset->properties();
    Q_FOREACH(KoSvgTextProperties::PropertyId p, properties.properties()) {
        textData.commonProperties.setProperty(p, properties.property(p));
    }
    qDebug() << "applied preset" << resource->name();
    d->textModel->textData.set(textData);
}

bool TextPropertiesDock::createNewPresetFromSettings()
{
    KoCssStylePresetSP preset(new KoCssStylePreset(QString()));
    KoSvgTextPropertyData textData = d->textModel->textData.get();
    preset->setProperties(textData.commonProperties);
    preset->setName(i18nc("@info:placeholder", "New Style Preset"));

    CssStylePresetEditDialog *dialog = new CssStylePresetEditDialog(this);
    dialog->setDpi(d->currentDpi);
    dialog->setCurrentResource(preset);

    if (dialog->exec() == QDialog::Accepted) {
        return KisResourceUserOperations::addResourceWithUserInput(this, dialog->currentResource());
    }

    return false;
}

void TextPropertiesDock::editPreset(KoResourceSP resource)
{
    KoCssStylePresetSP preset = resource.staticCast<KoCssStylePreset>();
    if (!preset) return;

    CssStylePresetEditDialog *dialog = new CssStylePresetEditDialog(this);
    dialog->setCurrentResource(preset);

    if (dialog->exec() == QDialog::Accepted) {
        KisResourceUserOperations::updateResourceWithUserInput(this, dialog->currentResource());
    }
}

void TextPropertiesDock::cloneAndEditPreset(KoResourceSP resource)
{
    KoResourceSP newResource = resource->clone();
    KoCssStylePresetSP preset = newResource.staticCast<KoCssStylePreset>();
    if (!preset) return;

    CssStylePresetEditDialog *dialog = new CssStylePresetEditDialog(this);
    dialog->setCurrentResource(preset);

    if (dialog->exec() == QDialog::Accepted) {
        preset = dialog->currentResource();
        preset->setFilename(preset->name().replace(' ', '_').replace('.', '_') + preset->defaultFileExtension());
        KisResourceUserOperations::addResourceWithUserInput(this, preset);
    }
}
