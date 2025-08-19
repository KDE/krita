/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "TextPropertiesDock.h"

#include <QQuickWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QStringListModel>
#include <QQuickStyle>
#include <QColorDialog>

#include <KisQQuickWidget.h>

#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>

#include <KLocalizedContext>

#include "TextPropertiesCanvasObserver.h"

struct TextPropertiesDock::Private
{
    Private(QObject *parent = nullptr)
        : canvasObserver(new TextPropertiesCanvasObserver(parent)) {

    }
    TextPropertiesCanvasObserver *canvasObserver{nullptr};
};

TextPropertiesDock::TextPropertiesDock()
    : QDockWidget(i18n("Text Properties"))
    , d(new Private)
{
    m_quickWidget = new KisQQuickWidget(this);

    setWidget(m_quickWidget);
    setEnabled(true);

    m_quickWidget->setMinimumHeight(100);

    m_quickWidget->setSource(QUrl("qrc:/TextProperties.qml"));

    if (!m_quickWidget->errors().empty()) {
        qWarning() << "Errors in " << windowTitle() << ":" << m_quickWidget->errors();
    } else {
        m_quickWidget->rootObject()->setProperty("canvasObserver", QVariant::fromValue(d->canvasObserver));
    }

    m_quickWidget->setPalette(this->palette());
}

TextPropertiesDock::~TextPropertiesDock()
{
    delete m_quickWidget;
}

QString TextPropertiesDock::observerName()
{
    return d->canvasObserver->observerName();
}

void TextPropertiesDock::setViewManager(KisViewManager *kisview)
{
    d->canvasObserver->setViewManager(kisview);
}

void TextPropertiesDock::setCanvas(KoCanvasBase *canvas)
{
    d->canvasObserver->setCanvas(canvas);
}

void TextPropertiesDock::unsetCanvas()
{
    d->canvasObserver->unsetCanvas();
}

QColor TextPropertiesDock::modalColorDialog(QColor oldColor)
{
    QColor c = QColorDialog::getColor(oldColor);
    return c.isValid()? c: oldColor;
}

#include <KoFontRegistry.h>
QString TextPropertiesDock::wwsFontFamilyName(QString familyName, bool returnEmptyWhenMissing)
{
    std::optional<QString> name = KoFontRegistry::instance()->wwsNameByFamilyName(familyName);
    if (!name) {
        return returnEmptyWhenMissing? QString(): familyName;
    }
    return name.value();
}
