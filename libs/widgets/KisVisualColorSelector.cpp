/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisVisualColorSelector.h"

#include <QVector4D>
#include <QList>

#include <KSharedConfig>
#include <KConfigGroup>

#include "KoColorDisplayRendererInterface.h"
#include <KoColorModelStandardIds.h>
//#include <QPointer>
#include "kis_signal_compressor.h"
#include "kis_debug.h"

#include "KisVisualColorSelectorShape.h"
#include "KisVisualRectangleSelectorShape.h"
#include "KisVisualTriangleSelectorShape.h"
#include "KisVisualEllipticalSelectorShape.h"

struct KisVisualColorSelector::Private
{
    QList<KisVisualColorSelectorShape*> widgetlist;
    bool acceptTabletEvents {false};
    bool circular {false};
    //bool isRGBA {false};
    bool initialized {false};
    bool loadingConfig {false};
    int colorChannelCount {0};
    QVector4D channelValues;
    KisColorSelectorConfiguration acs_config;
    KisSignalCompressor *updateTimer {0};
    KisVisualColorModel *selector {0};
};

KisVisualColorSelector::KisVisualColorSelector(QWidget *parent)
    : KisColorSelectorInterface(parent)
    , m_d(new Private)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setSelectorModel(new KisVisualColorModel(this));
    m_d->selector->slotLoadACSConfig();
    m_d->updateTimer = new KisSignalCompressor(100 /* ms */, KisSignalCompressor::POSTPONE);
    connect(m_d->updateTimer, SIGNAL(timeout()), SLOT(slotRebuildSelectors()), Qt::UniqueConnection);
}

KisVisualColorSelector::~KisVisualColorSelector()
{
    delete m_d->updateTimer;
}

QSize KisVisualColorSelector::minimumSizeHint() const
{
    return QSize(75, 75);
}

void KisVisualColorSelector::setSelectorModel(KisVisualColorModel *model)
{
    if (model == m_d->selector) {
        return;
    }
    if (m_d->selector) {
        m_d->selector->disconnect(this);
    }
    connect(model, SIGNAL(sigChannelValuesChanged(QVector4D)), SLOT(slotChannelValuesChanged(QVector4D)));
    connect(model, SIGNAL(sigColorModelChanged()), SLOT(slotColorModelChanged()));
    // to keep the KisColorSelectorInterface API functional:
    connect(model, SIGNAL(sigNewColor(KoColor)), this, SIGNAL(sigNewColor(KoColor)));
    m_d->selector = model;
    if (m_d->updateTimer) {
        m_d->initialized = false;
        m_d->updateTimer->start();
    }
}

KisVisualColorModel *KisVisualColorSelector::selectorModel() const
{
    return m_d->selector;
}

void KisVisualColorSelector::setConfig(bool forceCircular, bool forceSelfUpdate)
{
    Q_UNUSED(forceSelfUpdate);
    if (forceCircular != m_d->circular) {
        m_d->circular = forceCircular;
        m_d->initialized = false;
        m_d->updateTimer->start();
    }
}

void KisVisualColorSelector::setAcceptTabletEvents(bool on)
{
    m_d->acceptTabletEvents = on;
    for (KisVisualColorSelectorShape *shape: m_d->widgetlist) {
        shape->setAcceptTabletEvents(on);
    }
}

KoColor KisVisualColorSelector::getCurrentColor() const
{
    if (m_d->selector) {
        return m_d->selector->currentColor();
    }
    return KoColor();
}

void KisVisualColorSelector::slotSetColor(const KoColor &c)
{
    if (m_d->selector) {
        m_d->selector->slotSetColor(c);
    }
}

void KisVisualColorSelector::slotSetColorSpace(const KoColorSpace *cs)
{
    if (m_d->selector) {
        m_d->selector->slotSetColorSpace(cs);
    }
}

void KisVisualColorSelector::slotConfigurationChanged()
{
    if (m_d->updateTimer) {
        m_d->loadingConfig = true;
        m_d->initialized = false;
        m_d->updateTimer->start();
    }
}

void KisVisualColorSelector::setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer)
{
    if (m_d->selector) {
        m_d->selector->setDisplayRenderer(displayRenderer);
    }
}

void KisVisualColorSelector::slotChannelValuesChanged(const QVector4D &values)
{
    // about to (re-)build selector, values will be fetched when done
    if (!m_d->initialized) {
        return;
    }
    m_d->channelValues = values;
    for (KisVisualColorSelectorShape *shape: m_d->widgetlist) {
        shape->setChannelValues(m_d->channelValues, true);
    }
}

void KisVisualColorSelector::slotColorModelChanged()
{
    // while reloading configuration the color model may change, but we
    // don't want to restart the timer here because the rebuild is happening now.
    // maybe delaying triggered rebuilds is not necessary anymore after decoupling
    // it from configuration reloads.
    if (!m_d->loadingConfig)
    {
        m_d->initialized = false;
        m_d->updateTimer->start();
    }
}

void KisVisualColorSelector::slotCursorMoved(QPointF pos)
{
    const KisVisualColorSelectorShape *shape = qobject_cast<KisVisualColorSelectorShape *>(sender());
    KIS_SAFE_ASSERT_RECOVER_RETURN(shape);

    QVector<int> channels = shape->getChannels();
    m_d->channelValues[channels.at(0)] = pos.x();
    if (shape->getDimensions() == KisVisualColorSelectorShape::twodimensional) {
        m_d->channelValues[channels.at(1)] = pos.y();
    }

    for (KisVisualColorSelectorShape *widget: m_d->widgetlist) {
        if (widget != shape){
            widget->setChannelValues(m_d->channelValues, false);
        }
    }
    m_d->selector->slotSetChannelValues(m_d->channelValues);
}

void KisVisualColorSelector::slotDisplayConfigurationChanged()
{
    // TODO...
}

void KisVisualColorSelector::slotRebuildSelectors()
{
    if (m_d->loadingConfig) {
        m_d->selector->slotLoadACSConfig();
        m_d->loadingConfig = false;
    }
    // TODO: not all changes to color model (e.g. HSV->HSI) require and actual rebuild

    qDeleteAll(m_d->widgetlist);
    m_d->widgetlist.clear();

    if (!m_d->selector || m_d->selector->colorModel() == KisVisualColorModel::None) {
        return;
    }

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    m_d->acs_config = KisColorSelectorConfiguration::fromString(cfg.readEntry("colorSelectorConfiguration", KisColorSelectorConfiguration().toString()));

    m_d->colorChannelCount = m_d->selector->colorChannelCount();

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
        case KisColorSelectorConfiguration::hsyS:
        case KisColorSelectorConfiguration::hsiS:
        case KisColorSelectorConfiguration::hslS:
        case KisColorSelectorConfiguration::hsvS:
            channel1 = 1;
            break;
        case KisColorSelectorConfiguration::V:
        case KisColorSelectorConfiguration::L:
        case KisColorSelectorConfiguration::I:
        case KisColorSelectorConfiguration::Y:
            channel1 = 2;
            break;
        default:
            Q_ASSERT_X(false, "", "Invalid acs_config.subTypeParameter");
        }

        switch(m_d->acs_config.mainTypeParameter)
        {
        case KisColorSelectorConfiguration::hsvSH:
        case KisColorSelectorConfiguration::hslSH:
        case KisColorSelectorConfiguration::hsiSH:
        case KisColorSelectorConfiguration::hsySH:
            channel2 = 0;
            channel3 = 1;
            break;
        case KisColorSelectorConfiguration::VH:
        case KisColorSelectorConfiguration::LH:
        case KisColorSelectorConfiguration::IH:
        case KisColorSelectorConfiguration::YH:
            channel2 = 0;
            channel3 = 2;
            break;
        case KisColorSelectorConfiguration::SL:
        case KisColorSelectorConfiguration::SV:
        case KisColorSelectorConfiguration::SV2:
        case KisColorSelectorConfiguration::SI:
        case KisColorSelectorConfiguration::SY:
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
            bar = new KisVisualRectangleSelectorShape(this,
                                                      KisVisualColorSelectorShape::onedimensional,
                                                      channel1, channel1, 20);
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
            block = new KisVisualTriangleSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                       channel2, channel3);
        }
        else if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Square) {
            block = new KisVisualRectangleSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                        channel2, channel3);
        }
        else {
            block = new KisVisualEllipticalSelectorShape(this, KisVisualColorSelectorShape::twodimensional,
                                                         channel2, channel3);
        }

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

    for (KisVisualColorSelectorShape *shape: m_d->widgetlist) {
        shape->setAcceptTabletEvents(m_d->acceptTabletEvents);
        // if this widget is currently visible, new children are hidden by default
        shape->show();
    }

    // finally update widgets with new channel values
    slotChannelValuesChanged(m_d->selector->channelValues());
}

void KisVisualColorSelector::resizeEvent(QResizeEvent *)
{
    if (!m_d->selector || !m_d->initialized) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->widgetlist.isEmpty() || !m_d->initialized);
        return;
    }
    int sizeValue = qMin(width(), height());
    int borderWidth = qMax(sizeValue*0.1, 20.0);
    QRect newrect(0,0, this->geometry().width(), this->geometry().height());

    if (m_d->colorChannelCount == 1) {
        if (m_d->circular) {
            m_d->widgetlist.at(0)->resize(sizeValue, sizeValue);
        }
        else {
            // vertical only currently
            int sliderWidth = qMax(height()/10, 20);
            sliderWidth = qMin(sliderWidth, width());
            int x = (width() - sliderWidth)/2;
            m_d->widgetlist.at(0)->setGeometry(x, 0, sliderWidth, height());
        }
    }
    else if (m_d->colorChannelCount == 3) {
        m_d->widgetlist.at(0)->setBorderWidth(borderWidth);
        if (m_d->acs_config.subType == KisColorSelectorConfiguration::Ring) {
            m_d->widgetlist.at(0)->resize(sizeValue,sizeValue);
        }
        else if (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider && m_d->circular==false) {
            m_d->widgetlist.at(0)->resize(borderWidth, sizeValue);
        }
        else if (m_d->acs_config.subType == KisColorSelectorConfiguration::Slider && m_d->circular==true) {
            m_d->widgetlist.at(0)->resize(sizeValue,sizeValue);
        }

        if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Triangle) {
            m_d->widgetlist.at(1)->setGeometry(m_d->widgetlist.at(0)->getSpaceForTriangle(newrect));
        }
        else if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Square) {
            m_d->widgetlist.at(1)->setGeometry(m_d->widgetlist.at(0)->getSpaceForSquare(newrect));
        }
        else if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Wheel) {
            m_d->widgetlist.at(1)->setGeometry(m_d->widgetlist.at(0)->getSpaceForCircle(newrect));
        }
    }
    else if (m_d->colorChannelCount == 4) {
        int sizeBlock = qMin(width()/2 - 8, height());
        m_d->widgetlist.at(0)->setGeometry(0, 0, sizeBlock, sizeBlock);
        m_d->widgetlist.at(1)->setGeometry(sizeBlock + 8, 0, sizeBlock, sizeBlock);
    }
}
