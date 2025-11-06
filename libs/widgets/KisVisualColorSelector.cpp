/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2022 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisVisualColorSelector.h"

#include <QVector4D>
#include <QList>
#include <QPointer>

#include <KSharedConfig>
#include <KConfigGroup>

#include "KoColorDisplayRendererInterface.h"
#include <KoColorModelStandardIds.h>
//#include <QPointer>
#include "kis_signal_compressor.h"
#include "kis_debug.h"

#include <KisScreenMigrationTracker.h>

#include "KisVisualColorSelectorShape.h"
#include "KisVisualDiamondSelectorShape.h"
#include "KisVisualRectangleSelectorShape.h"
#include "KisVisualTriangleSelectorShape.h"
#include "KisVisualEllipticalSelectorShape.h"

struct KisVisualColorSelector::Private
{
    QList<KisVisualColorSelectorShape*> widgetlist;
    bool acceptTabletEvents {false};
    bool circular {false};
    bool proofColors {false};
    bool initialized {false};
    bool useACSConfig {true};
    bool autoAdjustExposure {true};
    int colorChannelCount {0};
    int minimumSliderWidth {16};
    Qt::Edge sliderPosition {Qt::LeftEdge};
    qreal stretchLimit {1.5};
    QVector4D channelValues;
    KisVisualColorSelector::RenderMode renderMode {RenderMode::DynamicBackground};
    KisColorSelectorConfiguration acs_config;
    KisSignalCompressor *updateTimer {0};
    KisVisualColorModelSP selectorModel;
    QPointer<const KoColorDisplayRendererInterface> displayRenderer;
    KoGamutMaskSP gamutMask;
    QPointer<KisScreenMigrationTracker> screenMigrationTracker;
};

KisVisualColorSelector::KisVisualColorSelector(QWidget *parent, KisVisualColorModelSP model)
    : KisColorSelectorInterface(parent)
    , m_d(new Private)
{
    m_d->acs_config = validatedConfiguration(KisColorSelectorConfiguration());
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    loadACSConfig();
    if (model) {
        setSelectorModel(model);
    } else {
        setSelectorModel(KisVisualColorModelSP(new KisVisualColorModel));
        m_d->selectorModel->slotLoadACSConfig();
    }

    m_d->updateTimer = new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE);
    connect(m_d->updateTimer, SIGNAL(timeout()), SLOT(slotReloadConfiguration()), Qt::UniqueConnection);

    m_d->screenMigrationTracker = new KisScreenMigrationTracker(this, this);
    connect(m_d->screenMigrationTracker, &KisScreenMigrationTracker::sigScreenOrResolutionChanged,
            this, [this] (QScreen *screen) {
                Q_UNUSED(screen);
                Q_FOREACH(KisVisualColorSelectorShape *shape, m_d->widgetlist) {
                    shape->notifyDevicePixelRationChanged();
                }
            });
}

KisVisualColorSelector::~KisVisualColorSelector()
{
    delete m_d->updateTimer;
}

QSize KisVisualColorSelector::minimumSizeHint() const
{
    return QSize(75, 75);
}

void KisVisualColorSelector::setSelectorModel(KisVisualColorModelSP model)
{
    if (model == m_d->selectorModel) {
        return;
    }
    if (m_d->selectorModel) {
        m_d->selectorModel->disconnect(this);
    }
    connect(model.data(), SIGNAL(sigChannelValuesChanged(QVector4D,quint32)),
                          SLOT(slotChannelValuesChanged(QVector4D,quint32)));
    connect(model.data(), SIGNAL(sigColorModelChanged()), SLOT(slotColorModelChanged()));
    connect(model.data(), SIGNAL(sigColorSpaceChanged()), SLOT(slotColorSpaceChanged()));
    // to keep the KisColorSelectorInterface API functional:
    connect(model.data(), SIGNAL(sigNewColor(KoColor)), this, SIGNAL(sigNewColor(KoColor)));
    m_d->selectorModel = model;
    m_d->initialized = false;
    rebuildSelector();
}

KisVisualColorModelSP KisVisualColorSelector::selectorModel() const
{
    return m_d->selectorModel;
}

void KisVisualColorSelector::setConfig(bool forceCircular, bool forceSelfUpdate)
{
    Q_UNUSED(forceSelfUpdate);
    if (forceCircular != m_d->circular) {
        m_d->circular = forceCircular;
        m_d->initialized = false;
        rebuildSelector();
    }
}

const KisColorSelectorConfiguration &KisVisualColorSelector::configuration() const
{
    return m_d->acs_config;
}

void KisVisualColorSelector::setConfiguration(const KisColorSelectorConfiguration *config)
{
    m_d->useACSConfig = !config;
    if (config) {
        // applies immediately, while signalled rebuilds from krita configuration changes
        // are queued, so make sure we cancel queued updates
        m_d->updateTimer->stop();
        KisColorSelectorConfiguration configNew = validatedConfiguration(*config);
        if (configNew != m_d->acs_config) {
            m_d->acs_config = configNew;
            m_d->initialized = false;
            rebuildSelector();
        }
    } else {
        m_d->initialized = false;
        m_d->updateTimer->start();
    }
}

void KisVisualColorSelector::setAcceptTabletEvents(bool on)
{
    m_d->acceptTabletEvents = on;
    for (KisVisualColorSelectorShape *shape : std::as_const(m_d->widgetlist)) {
        shape->setAcceptTabletEvents(on);
    }
}

KoColor KisVisualColorSelector::getCurrentColor() const
{
    if (m_d->selectorModel) {
        return m_d->selectorModel->currentColor();
    }
    return KoColor();
}

void KisVisualColorSelector::setMinimumSliderWidth(int width)
{
    int newWidth = qMax(5, width);
    if (newWidth != m_d->minimumSliderWidth) {
        m_d->minimumSliderWidth = width;
        KisVisualColorSelector::resizeEvent(0);
    }
}

const KoColorDisplayRendererInterface *KisVisualColorSelector::displayRenderer() const
{
    return m_d->displayRenderer ? m_d->displayRenderer : KoDumbColorDisplayRenderer::instance();
}

KisVisualColorSelector::RenderMode KisVisualColorSelector::renderMode() const
{
    return m_d->renderMode;
}

void KisVisualColorSelector::setRenderMode(KisVisualColorSelector::RenderMode mode)
{
    if (mode != m_d->renderMode) {
        m_d->renderMode = mode;
        for (KisVisualColorSelectorShape *shape : std::as_const(m_d->widgetlist)) {
            shape->forceImageUpdate();
            shape->update();
        }
    }
}

bool KisVisualColorSelector::autoAdjustExposure() const
{
    return m_d->autoAdjustExposure;
}

void KisVisualColorSelector::setAutoAdjustExposure(bool enabled)
{
    m_d->autoAdjustExposure = enabled;
}

bool KisVisualColorSelector::proofColors() const
{
    return m_d->proofColors;
}

void KisVisualColorSelector::setProofColors(bool enabled)
{
    if (enabled != m_d->proofColors) {
        m_d->proofColors = enabled;
        for (KisVisualColorSelectorShape *shape : std::as_const(m_d->widgetlist)) {
            shape->forceImageUpdate();
            shape->update();
        }
    }
}

void KisVisualColorSelector::setSliderPosition(Qt::Edge edge)
{
    if (edge != Qt::TopEdge && edge != Qt::LeftEdge) {
        return;
    }

    if (edge != m_d->sliderPosition) {
        m_d->sliderPosition = edge;
        rebuildSelector();
    }
}

KoGamutMask *KisVisualColorSelector::activeGamutMask() const
{
    return m_d->gamutMask.data();
}

void KisVisualColorSelector::slotSetColor(const KoColor &c)
{
    if (m_d->selectorModel) {
        m_d->selectorModel->slotSetColor(c);
    }
}

void KisVisualColorSelector::slotSetColorSpace(const KoColorSpace *cs)
{
    if (m_d->selectorModel) {
        m_d->selectorModel->slotSetColorSpace(cs);
    }
}

void KisVisualColorSelector::slotConfigurationChanged()
{
    if (m_d->updateTimer && m_d->useACSConfig) {
        // NOTE: this timer is because notifyConfigChanged() is only called
        // via KisConfig::setCustomColorSelectorColorSpace(), but at this point
        // Advanced Color Selector has not written the relevant config values yet.
        m_d->initialized = false;
        m_d->updateTimer->start();
    }
}

void KisVisualColorSelector::setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer)
{
    switchDisplayRenderer(displayRenderer);

    if (m_d->selectorModel) {
        slotDisplayConfigurationChanged();
    }
}

void KisVisualColorSelector::slotGamutMaskChanged(KoGamutMaskSP mask)
{
    // Note: KisCanvasResourceProvider currently does not distinguish
    // between activating, switching and property changes of a gamut mask
    m_d->gamutMask = mask;
    for (KisVisualColorSelectorShape *shape : std::as_const(m_d->widgetlist)) {
        shape->updateGamutMask();
    }
}

void KisVisualColorSelector::slotGamutMaskUnset()
{
    m_d->gamutMask.clear();
    for (KisVisualColorSelectorShape *shape : std::as_const(m_d->widgetlist)) {
        shape->updateGamutMask();
    }
}

void KisVisualColorSelector::slotGamutMaskPreviewUpdate()
{
    // Shapes currently always requests preview shapes if available, so more of the same...
    for (KisVisualColorSelectorShape *shape : std::as_const(m_d->widgetlist)) {
        shape->updateGamutMask();
    }
}

void KisVisualColorSelector::slotChannelValuesChanged(const QVector4D &values, quint32 channelFlags)
{
    // about to (re-)build selector, values will be fetched when done
    if (!m_d->initialized) {
        return;
    }
    m_d->channelValues = values;
    for (KisVisualColorSelectorShape *shape : std::as_const(m_d->widgetlist)) {
        shape->setChannelValues(m_d->channelValues, channelFlags);
    }
}

void KisVisualColorSelector::slotColorModelChanged()
{
    // TODO: triangle <=> diamond switch only happens on HSV <=> non-HSV, but
    // the previous color model is not accessible right now
    if (!m_d->initialized || m_d->selectorModel->colorChannelCount() != m_d->colorChannelCount
            || m_d->acs_config.mainType == KisColorSelectorConfiguration::Triangle) {
        m_d->initialized = false;
        rebuildSelector();
    } else {
        slotDisplayConfigurationChanged();
    }
}

void KisVisualColorSelector::slotColorSpaceChanged()
{
    if (m_d->autoAdjustExposure && m_d->selectorModel && m_d->selectorModel->supportsExposure()) {
        m_d->selectorModel->setMaxChannelValues(calculateMaxChannelValues());
    }
}

void KisVisualColorSelector::slotCursorMoved(QPointF pos)
{
    const KisVisualColorSelectorShape *shape = qobject_cast<KisVisualColorSelectorShape *>(sender());
    KIS_SAFE_ASSERT_RECOVER_RETURN(shape);

    m_d->channelValues[shape->channel(0)] = pos.x();
    if (shape->getDimensions() == KisVisualColorSelectorShape::twodimensional) {
        m_d->channelValues[shape->channel(1)] = pos.y();
    }

    for (KisVisualColorSelectorShape *widget : std::as_const(m_d->widgetlist)) {
        if (widget != shape){
            widget->setChannelValues(m_d->channelValues, shape->channelMask());
        }
    }
    m_d->selectorModel->slotSetChannelValues(m_d->channelValues);
}

void KisVisualColorSelector::slotDisplayConfigurationChanged()
{
    if (m_d->autoAdjustExposure && m_d->selectorModel && m_d->selectorModel->supportsExposure()) {
        m_d->selectorModel->setMaxChannelValues(calculateMaxChannelValues());
    }
    // TODO: can we be smarter about forced updates?
    for (KisVisualColorSelectorShape *shape : std::as_const(m_d->widgetlist)) {
        shape->forceImageUpdate();
        shape->update();
    }
}

void KisVisualColorSelector::slotReloadConfiguration()
{
    if (m_d->useACSConfig) {
        loadACSConfig();
        // this may trigger slotColorModelChanged() so check afterwards if we already rebuild
        m_d->selectorModel->slotLoadACSConfig();
        if (!m_d->initialized) {
            rebuildSelector();
        }
    }
}

void KisVisualColorSelector::rebuildSelector()
{
    qDeleteAll(m_d->widgetlist);
    m_d->widgetlist.clear();

    if (!m_d->selectorModel || m_d->selectorModel->colorModel() == KisVisualColorModel::None) {
        return;
    }

    m_d->colorChannelCount = m_d->selectorModel->colorChannelCount();

    bool supportsGamutMask = false;

    //recreate all the widgets.

    if (m_d->colorChannelCount == 1) {

        KisVisualColorSelectorShape *bar;

        if (m_d->circular) {
            bar = new KisVisualEllipticalSelectorShape(this, KisVisualColorSelectorShape::onedimensional,
                                                       0, 0, 20, KisVisualEllipticalSelectorShape::borderMirrored);
        }
        else {
            bar = new KisVisualRectangleSelectorShape(this, KisVisualColorSelectorShape::onedimensional,
                                                      0, 0, 20);
        }

        connect(bar, SIGNAL(sigCursorMoved(QPointF)), SLOT(slotCursorMoved(QPointF)));
        m_d->widgetlist.append(bar);
    }
    else if (m_d->colorChannelCount == 3) {
        int channel1 = 0;
        int channel2 = 1;
        int channel3 = 2;

        switch(m_d->acs_config.subTypeParameter)
        {
        case KisColorSelectorConfiguration::H:
        case KisColorSelectorConfiguration::Hluma:
            channel1 = 0;
            break;
        case KisColorSelectorConfiguration::hsvS:
            channel1 = 1;
            break;
        case KisColorSelectorConfiguration::V:
            channel1 = 2;
            break;
        default:
            Q_ASSERT_X(false, "", "Invalid acs_config.subTypeParameter");
        }

        switch(m_d->acs_config.mainTypeParameter)
        {
        case KisColorSelectorConfiguration::hsvSH:
            channel2 = 0;
            channel3 = 1;
            break;
        case KisColorSelectorConfiguration::VH:
            channel2 = 0;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::SV:
            channel2 = 1;
            channel3 = 2;
            break;
        default:
            Q_ASSERT_X(false, "", "Invalid acs_config.mainTypeParameter");
        }

        KisVisualColorSelectorShape *bar;
        if (m_d->acs_config.subType == KisColorSelectorConfiguration::Ring) {
            bar = new KisVisualEllipticalSelectorShape(this,
                                                       KisVisualColorSelectorShape::onedimensional,
                                                       channel1, channel1, 20,
                                                       KisVisualEllipticalSelectorShape::border);
        }
        else if (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider && m_d->circular == false) {
            KisVisualRectangleSelectorShape::singelDTypes orientation = useHorizontalSlider() ?
                        KisVisualRectangleSelectorShape::horizontal : KisVisualRectangleSelectorShape::vertical;

            bar = new KisVisualRectangleSelectorShape(this,
                                                      KisVisualColorSelectorShape::onedimensional,
                                                      channel1, channel1, 20, orientation);
        }
        else if (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider && m_d->circular == true) {
            bar = new KisVisualEllipticalSelectorShape(this,
                                                       KisVisualColorSelectorShape::onedimensional,
                                                       channel1, channel1,
                                                       20, KisVisualEllipticalSelectorShape::borderMirrored);
        } else {
            // Accessing bar below would crash since it's not initialized.
            // Hopefully this can never happen.
            warnUI << "Invalid subType, cannot initialize KisVisualColorSelectorShape";
            Q_ASSERT_X(false, "", "Invalid subType, cannot initialize KisVisualColorSelectorShape");
            return;
        }

        m_d->widgetlist.append(bar);

        KisVisualColorSelectorShape *block;
        if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Triangle) {
            if (m_d->selectorModel->colorModel() == KisVisualColorModel::HSV) {
                block = new KisVisualTriangleSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                           channel2, channel3);
            } else {
                block = new KisVisualDiamondSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                           channel2, channel3);
            }
        }
        else if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Square) {
            block = new KisVisualRectangleSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                        channel2, channel3);
        }
        else {
            block = new KisVisualEllipticalSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                         channel2, channel3);
        }

        supportsGamutMask = block->supportsGamutMask();

        connect(bar, SIGNAL(sigCursorMoved(QPointF)), SLOT(slotCursorMoved(QPointF)));
        connect(block, SIGNAL(sigCursorMoved(QPointF)), SLOT(slotCursorMoved(QPointF)));
        m_d->widgetlist.append(block);
    }
    else if (m_d->colorChannelCount == 4) {
        KisVisualRectangleSelectorShape *block =  new KisVisualRectangleSelectorShape(this, KisVisualRectangleSelectorShape::twodimensional, 0, 1);
        KisVisualRectangleSelectorShape *block2 =  new KisVisualRectangleSelectorShape(this, KisVisualRectangleSelectorShape::twodimensional, 2, 3);
        connect(block, SIGNAL(sigCursorMoved(QPointF)), SLOT(slotCursorMoved(QPointF)));
        connect(block2, SIGNAL(sigCursorMoved(QPointF)), SLOT(slotCursorMoved(QPointF)));
        m_d->widgetlist.append(block);
        m_d->widgetlist.append(block2);
    }

    m_d->initialized = true;
    // make sure we call "our" resize function
    KisVisualColorSelector::resizeEvent(0);

    for (KisVisualColorSelectorShape *shape : std::as_const(m_d->widgetlist)) {
        shape->setAcceptTabletEvents(m_d->acceptTabletEvents);
        // if this widget is currently visible, new children are hidden by default
        shape->show();
    }

    // finally update widgets with new channel values
    slotChannelValuesChanged(m_d->selectorModel->channelValues(), (1u << m_d->colorChannelCount) - 1);
    emit sigGamutMaskSupportChanged(supportsGamutMask);
}

void KisVisualColorSelector::resizeEvent(QResizeEvent *)
{
    if (!m_d->selectorModel || !m_d->initialized) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->widgetlist.isEmpty() || !m_d->initialized);
        return;
    }
    int sizeValue = qMin(width(), height());
    // due to masking/antialiasing, the visible width is ~4 pixels less, so add them here
    const int margin = 4;
    const qreal sliderRatio = 0.09;
    int borderWidth = qMax(int(sizeValue * sliderRatio), m_d->minimumSliderWidth) + margin;
    QRect newrect(0,0, this->geometry().width(), this->geometry().height());

    if (m_d->colorChannelCount == 1) {
        if (m_d->circular) {
            m_d->widgetlist.at(0)->resize(sizeValue, sizeValue);
        }
        else {
            KisVisualRectangleSelectorShape *slider = qobject_cast<KisVisualRectangleSelectorShape *>(m_d->widgetlist.at(0));
            KIS_SAFE_ASSERT_RECOVER_RETURN(slider);
            if (useHorizontalSlider()) {
                int sliderWidth = qMax(width()/10, m_d->minimumSliderWidth);
                sliderWidth = qMin(sliderWidth, height());
                int y = (height() - sliderWidth)/2;
                slider->setOneDimensionalType(KisVisualRectangleSelectorShape::horizontal);
                slider->setGeometry(0, y, width(), sliderWidth);
            }
            else {
                // vertical slider
                int sliderWidth = qMax(height()/10, m_d->minimumSliderWidth);
                sliderWidth = qMin(sliderWidth, width());
                int x = (width() - sliderWidth)/2;
                slider->setOneDimensionalType(KisVisualRectangleSelectorShape::vertical);
                slider->setGeometry(x, 0, sliderWidth, height());
            }
        }
    }
    else if (m_d->colorChannelCount == 3) {
        m_d->widgetlist.at(0)->setBorderWidth(borderWidth);
        // Ring
        if (m_d->acs_config.subType == KisColorSelectorConfiguration::Ring ||
            (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider && m_d->circular)) {

            m_d->widgetlist.at(0)->setGeometry((width() - sizeValue)/2, (height() - sizeValue)/2,
                                               sizeValue, sizeValue);
        }
        // Slider Bar
        else if (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider) {
            // limit stretch; only vertical slider currently
            if (useHorizontalSlider()) {
                newrect.setWidth(qMin(newrect.width(), qRound((newrect.height() - borderWidth) * m_d->stretchLimit)));
                newrect.setHeight(qMin(newrect.height(), qRound(sizeValue * m_d->stretchLimit + borderWidth)));

                m_d->widgetlist.at(0)->setGeometry(0, 0, newrect.width(), borderWidth);
            }
            else {
                newrect.setWidth(qMin(newrect.width(), qRound(sizeValue * m_d->stretchLimit + borderWidth)));
                newrect.setHeight(qMin(newrect.height(), qRound((newrect.width() - borderWidth) * m_d->stretchLimit)));

                m_d->widgetlist.at(0)->setGeometry(0, 0, borderWidth, newrect.height());
            }

        }

        if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Triangle) {
            if (m_d->selectorModel->colorModel() == KisVisualColorModel::HSV) {
                m_d->widgetlist.at(1)->setGeometry(m_d->widgetlist.at(0)->getSpaceForTriangle(newrect));
            } else {
                m_d->widgetlist.at(1)->setGeometry(m_d->widgetlist.at(0)->getSpaceForCircle(newrect));
            }
        }
        else if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Square) {
            m_d->widgetlist.at(1)->setGeometry(m_d->widgetlist.at(0)->getSpaceForSquare(newrect));
        }
        else if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Wheel) {
            m_d->widgetlist.at(1)->setGeometry(m_d->widgetlist.at(0)->getSpaceForCircle(newrect));
        }
        // center horizontally
        QRect boundRect(m_d->widgetlist.at(0)->geometry() | m_d->widgetlist.at(1)->geometry());
        int offset = (width() - boundRect.width()) / 2 - boundRect.left();
        m_d->widgetlist.at(0)->move(m_d->widgetlist.at(0)->pos() + QPoint(offset, 0));
        m_d->widgetlist.at(1)->move(m_d->widgetlist.at(1)->pos() + QPoint(offset, 0));
    }
    else if (m_d->colorChannelCount == 4) {
        int sizeBlock = qMin(width()/2 - 8, height());
        m_d->widgetlist.at(0)->setGeometry(0, 0, sizeBlock, sizeBlock);
        m_d->widgetlist.at(1)->setGeometry(sizeBlock + 8, 0, sizeBlock, sizeBlock);
    }
}

bool KisVisualColorSelector::useHorizontalSlider()
{
    if (m_d->colorChannelCount == 1) {
        return width() > height();
    }
    else {
        return m_d->sliderPosition == Qt::TopEdge;
    }
}

void KisVisualColorSelector::switchDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer)
{
    if (displayRenderer != m_d->displayRenderer) {
        if (m_d->displayRenderer) {
            m_d->displayRenderer->disconnect(this);
        }
        if (displayRenderer) {
            connect(displayRenderer, SIGNAL(displayConfigurationChanged()),
                    SLOT(slotDisplayConfigurationChanged()), Qt::UniqueConnection);
        }
        m_d->displayRenderer = displayRenderer;
    }
}

QVector4D KisVisualColorSelector::calculateMaxChannelValues()
{
    // Note: This calculation only makes sense for HDR color spaces
    QVector4D maxChannelValues = QVector4D(1, 1, 1, 1);
    const QList<KoChannelInfo *> channels = m_d->selectorModel->colorSpace()->channels();

    for (int i = 0; i < channels.size(); i++) {
        const KoChannelInfo *channel = channels.at(i);
        if (channel->channelType() != KoChannelInfo::ALPHA) {
            quint32 logical = channel->displayPosition();
            if (logical > m_d->selectorModel->colorSpace()->alphaPos()) {
                --logical;
            }
            maxChannelValues[logical] = displayRenderer()->maxVisibleFloatValue(channel);
        }
    }

    return maxChannelValues;
}

void KisVisualColorSelector::loadACSConfig()
{
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    KisColorSelectorConfiguration raw_config = KisColorSelectorConfiguration::fromString(
                cfg.readEntry("colorSelectorConfiguration", KisColorSelectorConfiguration().toString()));
    m_d->acs_config = validatedConfiguration(raw_config);
}

KisColorSelectorConfiguration KisVisualColorSelector::validatedConfiguration(const KisColorSelectorConfiguration &cfg)
{
    KisColorSelectorConfiguration validated(cfg);
    bool ok = true;

    switch (validated.mainType) {
    case KisColorSelectorConfiguration::Triangle:
    case KisColorSelectorConfiguration::Square:
    case KisColorSelectorConfiguration::Wheel:
        break;
    default:
        ok = false;
    }

    switch (validated.subType) {
    case KisColorSelectorConfiguration::Ring:
    case KisColorSelectorConfiguration::Slider:
        break;
    default:
        ok = false;
    }

    switch(validated.subTypeParameter)
    {
    case KisColorSelectorConfiguration::H:
    case KisColorSelectorConfiguration::hsvS:
    case KisColorSelectorConfiguration::V:
        break;
    // translate to HSV
    case KisColorSelectorConfiguration::hsyS:
    case KisColorSelectorConfiguration::hsiS:
    case KisColorSelectorConfiguration::hslS:
        validated.subTypeParameter = KisColorSelectorConfiguration::hsvS;
        break;
    case KisColorSelectorConfiguration::L:
    case KisColorSelectorConfiguration::I:
    case KisColorSelectorConfiguration::Y:
        validated.subTypeParameter = KisColorSelectorConfiguration::V;
        break;
    default:
        ok = false;
    }

    switch(validated.mainTypeParameter)
    {
    case KisColorSelectorConfiguration::SV:
    case KisColorSelectorConfiguration::hsvSH:
    case KisColorSelectorConfiguration::VH:
        break;
    // translate to HSV
    case KisColorSelectorConfiguration::SL:
    case KisColorSelectorConfiguration::SV2:
    case KisColorSelectorConfiguration::SI:
    case KisColorSelectorConfiguration::SY:
        validated.mainTypeParameter = KisColorSelectorConfiguration::SV;
        break;
    case KisColorSelectorConfiguration::hslSH:
    case KisColorSelectorConfiguration::hsiSH:
    case KisColorSelectorConfiguration::hsySH:
        validated.mainTypeParameter = KisColorSelectorConfiguration::hsvSH;
        break;
    case KisColorSelectorConfiguration::LH:
    case KisColorSelectorConfiguration::IH:
    case KisColorSelectorConfiguration::YH:
        validated.mainTypeParameter = KisColorSelectorConfiguration::VH;
        break;
    default:
        ok = false;
    }

    if (ok) {
        return validated;
    }
    return KisColorSelectorConfiguration(KisColorSelectorConfiguration::Triangle,
                                         KisColorSelectorConfiguration::Ring,
                                         KisColorSelectorConfiguration::SV,
                                         KisColorSelectorConfiguration::H);
}
