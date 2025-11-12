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
    setMinimumSize(600, 450);
    setModal(true);

    m_quickWidget = new KisQQuickWidget(this);
    this->setMainWidget(m_quickWidget);
    m_quickWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_quickWidget->setPalette(this->palette());
    this->setWindowTitle(i18nc("@title:window", "Edit Style Preset"));

    QStringList wellFormedBCPNames;
    Q_FOREACH (const QString langCode, KLocalizedString::languages()) {
        wellFormedBCPNames.append(langCode.split("_").join("-"));
    }

    connect(m_model, SIGNAL(textPropertyChanged()),
            this, SLOT(slotUpdateTextProperties()));

    m_quickWidget->setSource(QUrl("qrc:/CssStylePresetEdit.qml"));

    if (!m_quickWidget->errors().empty()) {
        qWarning() << "Errors in " << windowTitle() << ":" << m_quickWidget->errors();
    } else {
        m_quickWidget->rootObject()->setProperty("textProperties", QVariant::fromValue(m_model));
        m_quickWidget->rootObject()->setProperty("locales", QVariant::fromValue(wellFormedBCPNames));
    }
}

CssStylePresetEditDialog::~CssStylePresetEditDialog()
{
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
    if (m_quickWidget->rootObject()) {
        m_quickWidget->rootObject()->setProperty("presetTitle", m_currentResource->name());
        m_quickWidget->rootObject()->setProperty("presetDescription", m_currentResource->description());
        m_quickWidget->rootObject()->setProperty("presetSample", m_currentResource->sampleSvg());
        m_quickWidget->rootObject()->setProperty("presetSampleAlignment", variantFromAlignment(m_currentResource->alignSample()));
        m_quickWidget->rootObject()->setProperty("styleType", m_currentResource->styleType());

        m_quickWidget->rootObject()->setProperty("beforeSample", m_currentResource->beforeText());
        m_quickWidget->rootObject()->setProperty("sampleText", m_currentResource->sampleText());
        m_quickWidget->rootObject()->setProperty("afterSample", m_currentResource->afterText());
        const QSizeF paragraphSampleSize = m_currentResource->paragraphSampleSize();
        m_quickWidget->rootObject()->setProperty("sampleWidth", paragraphSampleSize.width());
        m_quickWidget->rootObject()->setProperty("sampleHeight", paragraphSampleSize.height());
        const int storedPPI = m_currentResource->storedPPIResolution();
        m_quickWidget->rootObject()->setProperty("makePixelRelative", storedPPI > 0);
    }
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
    if (m_quickWidget->rootObject()) {
        m_quickWidget->rootObject()->setProperty("canvasDPI", dpi);
    }
}

void CssStylePresetEditDialog::slotUpdateTextProperties()
{
    if (m_currentResource && !m_blockUpdates) {
        const QString before = m_quickWidget->rootObject()->property("beforeSample").toString();
        const QString sample = m_quickWidget->rootObject()->property("sampleText").toString();
        const QString after = m_quickWidget->rootObject()->property("afterSample").toString();
        const QString styleType = m_quickWidget->rootObject()->property("styleType").toString();

        const int sampleHeight = m_quickWidget->rootObject()->property("sampleHeight").toInt();
        const int sampleWidth = m_quickWidget->rootObject()->property("sampleWidth").toInt();
        const QSizeF sampleSize = QSizeF(sampleWidth, sampleHeight);


        bool shouldUpdateSample = (before != m_currentResource->beforeText()
                || sample != m_currentResource->sampleText()
                || after != m_currentResource->afterText())
                || sampleSize != m_currentResource->paragraphSampleSize();

        KoSvgTextPropertyData textData = m_model->textData.get();

        if (m_currentResource->properties() != textData.commonProperties || shouldUpdateSample) {
            m_currentResource->setStyleType(styleType);
            m_currentResource->setBeforeText(before);
            m_currentResource->setSampleText(sample);
            m_currentResource->setAfterText(after);

            if (styleType == "paragraph") {
                m_currentResource->setParagraphSampleSize(sampleSize);
            }

            m_currentResource->setProperties(textData.commonProperties);
            m_currentResource->updateThumbnail();
            m_quickWidget->rootObject()->setProperty("presetSample", m_currentResource->sampleSvg());
            m_quickWidget->rootObject()->setProperty("presetSampleAlignment", variantFromAlignment(m_currentResource->alignSample()));
            slotUpdateDirty();
        }
    }
}

QColor CssStylePresetEditDialog::modalColorDialog(QColor oldColor)
{
    QColor c = QColorDialog::getColor(oldColor);
    return c.isValid()? c: oldColor;
}

void CssStylePresetEditDialog::slotUpdateDirty() {
    if (!m_quickWidget->rootObject()) return;
    bool isDirty = m_currentResource->isDirty();
    const QString title = m_quickWidget->rootObject()->property("presetTitle").toString();
    if (!isDirty && m_quickWidget->rootObject()) {
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
    if (!m_quickWidget->rootObject()) return;
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
