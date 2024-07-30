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
#include <QFontDatabase>
#include <QStringListModel>
#include <QQuickStyle>

#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <KisStaticInitializer.h>

#include <KLocalizedContext>

#include <KoResourcePaths.h>
#include <KoCanvasResourcesIds.h>
#include <KoSvgTextPropertyData.h>
#include <text/lager/KoSvgTextPropertiesModel.h>
#include <text/lager/CssLengthPercentageModel.h>
#include <text/lager/LineHeightModel.h>
#include <text/lager/TextIndentModel.h>
#include <text/lager/TabSizeModel.h>
#include <text/lager/TextTransformModel.h>
#include <lager/state.hpp>

/// Strange place to put this, do we have a better spot?
KIS_DECLARE_STATIC_INITIALIZER {
    qmlRegisterType<KoSvgTextPropertiesModel>("org.krita.flake.text", 1, 0, "KoSvgTextPropertiesModel");
    qmlRegisterType<CssLengthPercentageModel>("org.krita.flake.text", 1, 0, "CssLengthPercentageModel");
    qmlRegisterType<LineHeightModel>("org.krita.flake.text", 1, 0, "LineHeightModel");
    qmlRegisterType<TextIndentModel>("org.krita.flake.text", 1, 0, "TextIndentModel");
    qmlRegisterType<TabSizeModel>("org.krita.flake.text", 1, 0, "TabSizeModel");
    qmlRegisterType<TextTransformModel>("org.krita.flake.text", 1, 0, "TextTransformModel");
    qmlRegisterUncreatableMetaObject(KoSvgText::staticMetaObject, "org.krita.flake.text", 1, 0, "KoSvgText", "Error: Namespace with enums");
}


/// This will call the 'autoEnable' function on any "watched" QObject.
/// Within QML, this should be used with a MouseArea that holds the disabled QtQuickControl
/// The autoEnable function on this MouseArea should enable the QtQuick control.
/// This is because disabled QtQuickControls never receive mouse events.
struct TextPropertyAutoEnabler : public QObject {
    Q_OBJECT
public:
    TextPropertyAutoEnabler (QObject *watched, QObject *parent)
        : QObject(parent), m_watched(watched) {
        watched->installEventFilter(this);
    }

    bool eventFilter(QObject *watched, QEvent *event) override {
        if (watched != m_watched) return false;

        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::TabletPress ||
            event->type() == QEvent::TouchBegin) {

            QMetaObject::invokeMethod(m_watched, "autoEnable");
        }

        return false;
    }
private:
    QObject *m_watched;
};

struct TextPropertiesDock::Private
{
    KoSvgTextPropertiesModel *textModel {new KoSvgTextPropertiesModel()};
    KisCanvasResourceProvider *provider{nullptr};
};

TextPropertiesDock::TextPropertiesDock()
    : QDockWidget(i18n("Text Properties"))
    , d(new Private())
{
    m_quickWidget = new QQuickWidget(this);
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

    m_quickWidget->setPalette(this->palette());
    m_quickWidget->setMinimumHeight(100);

    QFontDatabase fontDataBase = QFontDatabase();

    m_quickWidget->rootContext()->setContextProperty("textPropertiesModel", d->textModel);
    m_quickWidget->rootContext()->setContextProperty("fontFamiliesModel", QVariant::fromValue(fontDataBase.families()));
    connect(d->textModel, SIGNAL(textPropertyChanged()),
            this, SLOT(slotTextPropertiesChanged()));
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quickWidget->setSource(QUrl("qrc:/TextProperties.qml"));

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
}

void TextPropertiesDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = 0;
}

void TextPropertiesDock::connectAutoEnabler(QObject *watched)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(watched);

    new TextPropertyAutoEnabler(watched, watched);
}

void TextPropertiesDock::slotCanvasTextPropertiesChanged()
{
    KoSvgTextPropertyData data = d->provider->textPropertyData();
    if (d->textModel->textData.get() != data) {
        d->textModel->textData.set(data);
        QMetaObject::invokeMethod(m_quickWidget->rootObject(), "setProperties");
    }
}

void TextPropertiesDock::slotTextPropertiesChanged()
{

    KoSvgTextPropertyData textData = d->textModel->textData.get();
    debugFlake << Q_FUNC_INFO << textData;
    if (d->provider && d->provider->textPropertyData() != textData) {
        QMetaObject::invokeMethod(m_quickWidget->rootObject(), "setProperties");
        d->provider->setTextPropertyData(textData);
    }
}
#include "TextPropertiesDock.moc"
