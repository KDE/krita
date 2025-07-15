/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "CssStylePresetEditDialog.h"
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QColorDialog>
#include <QPushButton>
#include <KoResourcePaths.h>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KoFontRegistry.h>
#include "FontAxesModel.h"
#include "FontStyleModel.h"

QVariant variantFromAlignment(Qt::Alignment align) {
    return QVariant(static_cast<Qt::Alignment::Int>(align));
}

CssStylePresetEditDialog::CssStylePresetEditDialog(QWidget *parent)
    :KoDialog(parent)
    , m_model(new KoSvgTextPropertiesModel())

{
    setMinimumSize(300, 500);
    setModal(true);

    m_quickWidget = new QQuickWidget(this);
    this->setMainWidget(m_quickWidget);
    m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_quickWidget->engine()->rootContext()->setContextProperty("mainWindow", this);
    m_quickWidget->engine()->rootContext()->setContextObject(new KLocalizedContext(this));

    m_quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    m_quickWidget->engine()->addImportPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

    m_quickWidget->engine()->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib/qml/");
    m_quickWidget->engine()->addPluginPath(KoResourcePaths::getApplicationRoot() + "/lib64/qml/");

    m_quickWidget->setPalette(this->palette());
    this->setWindowTitle(i18nc("@title:window", "Edit Style Preset"));

    QStringList wellFormedBCPNames;
    Q_FOREACH (const QString langCode, KLocalizedString::languages()) {
        wellFormedBCPNames.append(langCode.split("_").join("-"));
    }

    m_quickWidget->rootContext()->setContextProperty("textPropertiesModel", m_model);
    m_quickWidget->rootContext()->setContextProperty("locales", QVariant::fromValue(wellFormedBCPNames));
    m_quickWidget->rootContext()->setContextProperty("canvasDPI", QVariant::fromValue(72));

    connect(m_model, SIGNAL(textPropertyChanged()),
            this, SLOT(slotUpdateTextProperties()));

    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setSource(QUrl("qrc:/CssStylePresetEdit.qml"));

    if (!m_quickWidget->errors().empty()) {
        qWarning() << "Errors in " << windowTitle() << ":" << m_quickWidget->errors();
    }
}

CssStylePresetEditDialog::~CssStylePresetEditDialog()
{
    /// Prevent accessing destroyed objects in QML engine
    /// See:
    ///   * https://invent.kde.org/graphics/krita/-/commit/d8676f4e9cac1a8728e73fec3ff1df1763c713b7
    ///   * https://bugreports.qt.io/browse/QTBUG-81247
    m_quickWidget->setParent(nullptr);
    delete m_quickWidget;

}

void CssStylePresetEditDialog::setCurrentResource(KoCssStylePresetSP resource)
{
    m_blockUpdates = true;
    m_currentResource = resource;
    KoSvgTextProperties properties = m_currentResource->properties();
    KoSvgTextPropertyData textData;
    textData.inheritedProperties = KoSvgTextProperties();
    textData.commonProperties = properties;
    m_model->textData.set(textData);
    m_quickWidget->rootObject()->setProperty("presetTitle", m_currentResource->name());
    m_quickWidget->rootObject()->setProperty("presetDescription", m_currentResource->description());
    m_quickWidget->rootObject()->setProperty("presetSample", m_currentResource->sampleSvg());
    m_quickWidget->rootObject()->setProperty("presetSampleAlignment", variantFromAlignment(m_currentResource->alignSample()));
    m_quickWidget->rootObject()->setProperty("styleType", m_currentResource->styleType());

    m_quickWidget->rootObject()->setProperty("beforeSample", m_currentResource->beforeText());
    m_quickWidget->rootObject()->setProperty("sampleText", m_currentResource->sampleText());
    m_quickWidget->rootObject()->setProperty("afterSample", m_currentResource->afterText());
    const int storedPPI = m_currentResource->storedPPIResolution();
    m_quickWidget->rootObject()->setProperty("makePixelRelative", storedPPI > 0);

    QMetaObject::invokeMethod(m_quickWidget->rootObject(), "setProperties");
    m_blockUpdates = false;
    slotUpdateDirty();
}

KoCssStylePresetSP CssStylePresetEditDialog::currentResource()
{
    if (m_currentResource) {
        slotUpdateTextProperties();
        const QString title = m_quickWidget->rootObject()->property("presetTitle").toString();
        const QString description = m_quickWidget->rootObject()->property("presetDescription").toString();
        m_currentResource->setName(title);
        m_currentResource->setDescription(description);
    }
    return m_currentResource;
}

void CssStylePresetEditDialog::setDpi(const double dpi)
{
    m_quickWidget->rootObject()->setProperty("canvasDPI", dpi);
}

void CssStylePresetEditDialog::slotUpdateTextProperties()
{
    if (m_currentResource && !m_blockUpdates) {
        const QString before = m_quickWidget->rootObject()->property("beforeSample").toString();
        const QString sample = m_quickWidget->rootObject()->property("sampleText").toString();
        const QString after = m_quickWidget->rootObject()->property("afterSample").toString();
        const QString styleType = m_quickWidget->rootObject()->property("styleType").toString();

        bool shouldUpdateSample = (before != m_currentResource->beforeText()
                || sample != m_currentResource->sampleText()
                || after != m_currentResource->afterText());

        KoSvgTextPropertyData textData = m_model->textData.get();

        if (m_currentResource->properties() != textData.commonProperties || shouldUpdateSample) {
            m_currentResource->setStyleType(styleType);
            m_currentResource->setSampleText(sample,  textData.commonProperties, before, after);
            m_quickWidget->rootObject()->setProperty("presetSample", m_currentResource->sampleSvg());
            m_quickWidget->rootObject()->setProperty("presetSampleAlignment", variantFromAlignment(m_currentResource->alignSample()));
            QMetaObject::invokeMethod(m_quickWidget->rootObject(), "setProperties");
            slotUpdateDirty();
        }
    }
}

QColor CssStylePresetEditDialog::modalColorDialog(QColor oldColor)
{
    QColor c = QColorDialog::getColor(oldColor);
    return c.isValid()? c: oldColor;
}

QString CssStylePresetEditDialog::wwsFontFamilyName(QString familyName)
{
    std::optional<QString> name = KoFontRegistry::instance()->wwsNameByFamilyName(familyName);
    if (!name) {
        return familyName;
    }
    return name.value();
}

void CssStylePresetEditDialog::slotUpdateDirty() {
    bool isDirty = m_currentResource->isDirty();
    const QString title = m_quickWidget->rootObject()->property("presetTitle").toString();
    if (!isDirty) {
        const QString description = m_quickWidget->rootObject()->property("presetDescription").toString();
        if (title != m_currentResource->name() || description != m_currentResource->description()) {
            isDirty = true;
        }
    }
    this->button(KoDialog::Ok)->setEnabled(isDirty && !title.isEmpty());
}

void CssStylePresetEditDialog::slotUpdateStoreDPI() {
    //Whenever we make it pixel relative, we'll set the resolution to 72 dpi.
    if (m_blockUpdates) return;
    const bool makePixelRelative = m_quickWidget->rootObject()->property("makePixelRelative").toBool();
    const int storedPPI = m_currentResource->storedPPIResolution();
    const int canvasPPI = m_quickWidget->rootObject()->property("canvasDPI").toInt();
    KoSvgTextPropertyData textData = m_model->textData.get();
    if (storedPPI > 0 && !makePixelRelative) {
        const double scale = double(storedPPI)/double(canvasPPI);
        textData.commonProperties.scaleAbsoluteValues(scale, scale);
        m_currentResource->setStoredPPIResolution(0);
    } else if(storedPPI == 0 && makePixelRelative) {
        const double scale = double(canvasPPI)/double(72);
        textData.commonProperties.scaleAbsoluteValues(scale, scale);
        m_currentResource->setStoredPPIResolution(72);
    }
    m_model->textData.set(textData);
    slotUpdateTextProperties();
}
