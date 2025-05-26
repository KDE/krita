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
#include <KoResourcePaths.h>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KoFontRegistry.h>
#include "FontAxesModel.h"
#include "FontStyleModel.h"

CssStylePresetEditDialog::CssStylePresetEditDialog(QWidget *parent)
    :KoDialog(parent)
    , m_model(new KoSvgTextPropertiesModel())

{
    setMinimumSize(500, 300);
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
    if (resource == m_currentResource) return;
    m_currentResource = resource;
    KoSvgTextProperties properties = m_currentResource->properties();
    KoSvgTextPropertyData textData;
    textData.inheritedProperties = KoSvgTextProperties();
    textData.commonProperties = properties;
    m_model->textData.set(textData);
    m_quickWidget->rootObject()->setProperty("presetTitle", m_currentResource->name());
    m_quickWidget->rootObject()->setProperty("styleType", m_currentResource->styleType());
}

KoCssStylePresetSP CssStylePresetEditDialog::currentResource()
{
    KoSvgTextPropertyData textData = m_model->textData.get();
    m_currentResource->setProperties(textData.commonProperties);
    const QString title = m_quickWidget->rootObject()->property("presetTitle").toString();
    const QString styleType = m_quickWidget->rootObject()->property("styleType").toString();
    m_currentResource->setName(title);
    m_currentResource->setStyleType(styleType);
    return m_currentResource;
}

void CssStylePresetEditDialog::slotUpdateTextProperties()
{
    QMetaObject::invokeMethod(m_quickWidget->rootObject(), "setProperties");
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

void CssStylePresetEditDialog::connectAutoEnabler(QObject *watched)
{

}
