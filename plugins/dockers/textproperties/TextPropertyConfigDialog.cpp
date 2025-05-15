/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KLocalizedContext>
#include <KLocalizedString>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <KoResourcePaths.h>

#include "TextPropertyConfigDialog.h"

TextPropertyConfigDialog::TextPropertyConfigDialog(QWidget *parent)
    : KoDialog(parent)

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
    this->setWindowTitle(i18nc("@title:window", "Text Property Configuration"));

    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setSource(QUrl("qrc:/TextPropertyConfigDialog.qml"));
    if (!m_quickWidget->errors().empty()) {
        qWarning() << "Errors in " << windowTitle() << ":" << m_quickWidget->errors();
    }

}

TextPropertyConfigDialog::~TextPropertyConfigDialog()
{
    /// Prevent accessing destroyed objects in QML engine
    /// See:
    ///   * https://invent.kde.org/graphics/krita/-/commit/d8676f4e9cac1a8728e73fec3ff1df1763c713b7
    ///   * https://bugreports.qt.io/browse/QTBUG-81247
    m_quickWidget->setParent(nullptr);
    delete m_quickWidget;
}

void TextPropertyConfigDialog::setTextPropertyConfigModel(TextPropertyConfigModel *model)
{
    if (m_quickWidget->rootObject() && model) {
        m_model = model;
        m_quickWidget->rootObject()->setProperty("configModel", QVariant::fromValue(m_model));
    }
}

TextPropertyConfigModel *TextPropertyConfigDialog::model()
{
    return m_model;
}
