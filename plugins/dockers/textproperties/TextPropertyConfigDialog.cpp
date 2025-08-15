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

    m_quickWidget = new KisQQuickWidget(this);
    this->setMainWidget(m_quickWidget);

    this->setWindowTitle(i18nc("@title:window", "Text Property Configuration"));

    m_quickWidget->setSource(QUrl("qrc:/TextPropertyConfigDialog.qml"));
    if (!m_quickWidget->errors().empty()) {
        qWarning() << "Errors in " << windowTitle() << ":" << m_quickWidget->errors();
    }
    m_quickWidget->setPalette(this->palette());
}

TextPropertyConfigDialog::~TextPropertyConfigDialog()
{
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
