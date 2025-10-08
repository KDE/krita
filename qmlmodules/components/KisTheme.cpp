/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisTheme.h"
#include <QApplication>
#include <kis_config_notifier.h>
#include <lager/state.hpp>

namespace {
auto getForeground = [](KColorScheme::ForegroundRole id) { return lager::lenses::getset(
                [id](const KColorScheme &value) -> QColor {
        return value.foreground(id).color();
    }, [id](KColorScheme value, const QColor &color) -> KColorScheme {return value;}
    );
};

auto getBackground = [](KColorScheme::BackgroundRole id) { return lager::lenses::getset(
                [id](const KColorScheme &value) -> QColor {
        return value.background(id).color();
    }, [id](KColorScheme value, const QColor &color) -> KColorScheme {return value;}
    );
};

auto getDecorations = [](KColorScheme::DecorationRole id) { return lager::lenses::getset(
                [id](const KColorScheme &value) -> QColor {
        return value.decoration(id).color();
    }, [id](KColorScheme value, const QColor &color) -> KColorScheme {return value;}
    );
};

auto getShade = [](KColorScheme::ShadeRole id) { return lager::lenses::getset(
                [id](const KColorScheme &value) -> QColor {
        return value.shade(id);
    }, [id](KColorScheme value, const QColor &color) -> KColorScheme {return value;}
    );
};
}


KisThemeColorGroup::KisThemeColorGroup(KColorScheme::ColorSet _set, QObject *parent)
    : QObject(parent)
    , m_set(_set)
    , m_active(KColorScheme(QPalette::Active, m_set))
    , m_inactive(KColorScheme(QPalette::Inactive, m_set))
    , m_disabled(KColorScheme(QPalette::Disabled, m_set))
    , scheme(lager::make_state(KColorScheme(QPalette::Active, _set), lager::automatic_tag{}))
    , LAGER_QT(textColor) {scheme.zoom(getForeground(KColorScheme::NormalText))}
    , LAGER_QT(disabledTextColor) {scheme.zoom(getForeground(KColorScheme::InactiveText))}
    , LAGER_QT(activeTextColor) {scheme.zoom(getForeground(KColorScheme::ActiveText))}
    , LAGER_QT(linkColor) {scheme.zoom(getForeground(KColorScheme::LinkText))}
    , LAGER_QT(visitedLinkColor) {scheme.zoom(getForeground(KColorScheme::VisitedText))}
    , LAGER_QT(negativeTextColor) {scheme.zoom(getForeground(KColorScheme::NegativeText))}
    , LAGER_QT(neutralTextColor) {scheme.zoom(getForeground(KColorScheme::NeutralText))}
    , LAGER_QT(positiveTextColor) {scheme.zoom(getForeground(KColorScheme::PositiveText))}
    , LAGER_QT(backgroundColor) {scheme.zoom(getBackground(KColorScheme::NormalBackground))}
    , LAGER_QT(activeBackgroundColor) {scheme.zoom(getBackground(KColorScheme::ActiveBackground))}
    , LAGER_QT(linkBackgroundColor) {scheme.zoom(getBackground(KColorScheme::LinkBackground))}
    , LAGER_QT(visitedLinkBackgroundColor) {scheme.zoom(getBackground(KColorScheme::VisitedBackground))}
    , LAGER_QT(negativeBackgroundColor) {scheme.zoom(getBackground(KColorScheme::NegativeBackground))}
    , LAGER_QT(neutralBackgroundColor) {scheme.zoom(getBackground(KColorScheme::NeutralBackground))}
    , LAGER_QT(positiveBackgroundColor) {scheme.zoom(getBackground(KColorScheme::PositiveBackground))}
    , LAGER_QT(alternateBackgroundColor) {scheme.zoom(getBackground(KColorScheme::AlternateBackground))}
    , LAGER_QT(focusColor) {scheme.zoom(getDecorations(KColorScheme::FocusColor))}
    , LAGER_QT(hoverColor) {scheme.zoom(getDecorations(KColorScheme::HoverColor))}
    , LAGER_QT(lightShadeColor) {scheme.zoom(getShade(KColorScheme::LightShade))}
    , LAGER_QT(midLightShadeColor) {scheme.zoom(getShade(KColorScheme::MidlightShade))}
    , LAGER_QT(midShadeColor) {scheme.zoom(getShade(KColorScheme::MidShade))}
    , LAGER_QT(darkShadeColor) {scheme.zoom(getShade(KColorScheme::DarkShade))}
    , LAGER_QT(shadowShadeColor) {scheme.zoom(getShade(KColorScheme::ShadowShade))}
{
    connect(this, SIGNAL(stateChanged()), this, SLOT(updateState()));
    lager::watch(scheme, std::bind(&KisThemeColorGroup::schemeChanged, this));
}

KisThemeColorGroup::~KisThemeColorGroup()
{
}

void KisThemeColorGroup::updateColorScheme(const QString filename)
{
    KSharedConfigPtr config = KSharedConfig::openConfig(filename);
    m_active = KColorScheme(QPalette::Active, m_set, config);
    m_inactive = KColorScheme(QPalette::Inactive, m_set, config);
    m_disabled = KColorScheme(QPalette::Disabled, m_set, config);

    updateState();
}

void KisThemeColorGroup::setState(int newState)
{
    if (m_state == QPalette::ColorGroup(newState)) return;
    m_state = QPalette::ColorGroup(newState);
    Q_EMIT stateChanged();
}

int KisThemeColorGroup::state()
{
    return int(m_state);
}

void KisThemeColorGroup::updateState()
{
    if (m_state == QPalette::Inactive) {
        scheme.set(m_inactive);
    } else if (m_state == QPalette::Disabled) {
        scheme.set(m_disabled);
    } else {
        scheme.set(m_active);
    }
}

KisTheme::KisTheme(QObject *parent)
    : QObject(parent)
    , viewModel(KisThemeColorGroup(KColorScheme::View, this))
    , windowModel(KisThemeColorGroup(KColorScheme::Window, this))
    , buttonModel(KisThemeColorGroup(KColorScheme::Button, this))
    , selectionModel(KisThemeColorGroup(KColorScheme::Selection, this))
    , tooltipModel(KisThemeColorGroup(KColorScheme::Tooltip, this))
{
    connect(KisConfigNotifier::instance(), SIGNAL(signalColorThemeChanged(QString)), this, SLOT(slotUpdateThemes()));

    connect(&viewModel, SIGNAL(schemeChanged()), this, SIGNAL(viewChanged()));
    connect(&windowModel, SIGNAL(schemeChanged()), this, SIGNAL(windowChanged()));
    connect(&buttonModel, SIGNAL(schemeChanged()), this, SIGNAL(buttonChanged()));
    connect(&selectionModel, SIGNAL(schemeChanged()), this, SIGNAL(selectionChanged()));
    connect(&tooltipModel, SIGNAL(schemeChanged()), this, SIGNAL(tooltipChanged()));
    slotUpdateThemes();
}

KisThemeColorGroup *KisTheme::view()
{
    return &this->viewModel;
}

KisThemeColorGroup *KisTheme::window()
{
    return &this->windowModel;
}

KisThemeColorGroup *KisTheme::button()
{
    return &this->buttonModel;
}

KisThemeColorGroup *KisTheme::selection()
{
    return &this->selectionModel;
}

KisThemeColorGroup *KisTheme::tooltip()
{
    return &this->tooltipModel;
}

void KisTheme::slotUpdateThemes()
{
    const QString filename = qApp->property("KDE_COLOR_SCHEME_PATH").toString();

    viewModel.updateColorScheme(filename);
    windowModel.updateColorScheme(filename);
    buttonModel.updateColorScheme(filename);
    selectionModel.updateColorScheme(filename);
    tooltipModel.updateColorScheme(filename);
}


