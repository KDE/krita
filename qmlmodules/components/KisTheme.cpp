/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisTheme.h"
#include <QApplication>
#include <kis_config_notifier.h>

const KisThemeConfig *KisThemeConfig::instance()
{
    static const KisThemeConfig *kts;
    if (!kts) {
        kts = new KisThemeConfig;
    }
    return kts;
}

void KisThemeConfig::slotSetFilename(const QString &filename)
{
    if (filename != m_filename) {
        m_filename = filename;
        updateSchemes();
    }
}

KisThemeConfig::KisThemeConfig(QObject *parent)
    : QObject(parent)
    , m_filename(qApp->property("KDE_COLOR_SCHEME_PATH").toString())
{
    connect(KisConfigNotifier::instance(),
            &KisConfigNotifier::signalColorThemeChanged,
            this,
            &KisThemeConfig::slotSetFilename);
    updateSchemes();
}

void KisThemeConfig::updateSchemes()
{
    KSharedConfigPtr config = KSharedConfig::openConfig(m_filename);
    m_viewActive = KColorScheme(QPalette::Active, KColorScheme::View, config);
    m_viewInactive = KColorScheme(QPalette::Inactive, KColorScheme::View, config);
    m_viewDisabled = KColorScheme(QPalette::Disabled, KColorScheme::View, config);
    m_windowActive = KColorScheme(QPalette::Active, KColorScheme::Window, config);
    m_windowInactive = KColorScheme(QPalette::Inactive, KColorScheme::Window, config);
    m_windowDisabled = KColorScheme(QPalette::Disabled, KColorScheme::Window, config);
    m_buttonActive = KColorScheme(QPalette::Active, KColorScheme::Button, config);
    m_buttonInactive = KColorScheme(QPalette::Inactive, KColorScheme::Button, config);
    m_buttonDisabled = KColorScheme(QPalette::Disabled, KColorScheme::Button, config);
    m_selectionActive = KColorScheme(QPalette::Active, KColorScheme::Selection, config);
    m_selectionInactive = KColorScheme(QPalette::Inactive, KColorScheme::Selection, config);
    m_selectionDisabled = KColorScheme(QPalette::Disabled, KColorScheme::Selection, config);
    m_tooltipActive = KColorScheme(QPalette::Active, KColorScheme::Tooltip, config);
    m_tooltipInactive = KColorScheme(QPalette::Inactive, KColorScheme::Tooltip, config);
    m_tooltipDisabled = KColorScheme(QPalette::Disabled, KColorScheme::Tooltip, config);
    Q_EMIT sigThemeChanged();
}

KisThemeColorGroup::KisThemeColorGroup(KColorScheme::ColorSet set, QObject *parent)
    : QObject(parent)
    , m_set(set)
{
}

int KisThemeColorGroup::state() const
{
    return int(m_state);
}

void KisThemeColorGroup::setState(int state)
{
    if (m_state != QPalette::ColorGroup(state)) {
        QColor oldTextColor = textColor();
        QColor oldDisabledTextColor = disabledTextColor();
        QColor oldActiveTextColor = activeTextColor();
        QColor oldLinkColor = linkColor();
        QColor oldVisitedLinkColor = visitedLinkColor();
        QColor oldNegativeTextColor = negativeTextColor();
        QColor oldNeutralTextColor = neutralTextColor();
        QColor oldPositiveTextColor = positiveTextColor();
        QColor oldBackgroundColor = backgroundColor();
        QColor oldActiveBackgroundColor = activeBackgroundColor();
        QColor oldLinkBackgroundColor = linkBackgroundColor();
        QColor oldVisitedLinkBackgroundColor = visitedLinkBackgroundColor();
        QColor oldNegativeBackgroundColor = negativeBackgroundColor();
        QColor oldNeutralBackgroundColor = neutralBackgroundColor();
        QColor oldPositiveBackgroundColor = positiveBackgroundColor();
        QColor oldAlternateBackgroundColor = alternateBackgroundColor();
        QColor oldFocusColor = focusColor();
        QColor oldHoverColor = hoverColor();
        QColor oldLightShadeColor = lightShadeColor();
        QColor oldMidLightShadeColor = midLightShadeColor();
        QColor oldMidShadeColor = midShadeColor();
        QColor oldDarkShadeColor = darkShadeColor();
        QColor oldShadowShadeColor = shadowShadeColor();

        m_state = QPalette::ColorGroup(state);
        Q_EMIT stateChanged();

        if (QColor newTextColor = textColor(); newTextColor != oldTextColor) {
            Q_EMIT textColorChanged();
        }
        if (QColor newDisabledTextColor = disabledTextColor(); newDisabledTextColor != oldDisabledTextColor) {
            Q_EMIT disabledTextColorChanged();
        }
        if (QColor newActiveTextColor = activeTextColor(); newActiveTextColor != oldActiveTextColor) {
            Q_EMIT activeTextColorChanged();
        }
        if (QColor newLinkColor = linkColor(); newLinkColor != oldLinkColor) {
            Q_EMIT linkColorChanged();
        }
        if (QColor newVisitedLinkColor = visitedLinkColor(); newVisitedLinkColor != oldVisitedLinkColor) {
            Q_EMIT visitedLinkColorChanged();
        }
        if (QColor newNegativeTextColor = negativeTextColor(); newNegativeTextColor != oldNegativeTextColor) {
            Q_EMIT negativeTextColorChanged();
        }
        if (QColor newNeutralTextColor = neutralTextColor(); newNeutralTextColor != oldNeutralTextColor) {
            Q_EMIT neutralTextColorChanged();
        }
        if (QColor newPositiveTextColor = positiveTextColor(); newPositiveTextColor != oldPositiveTextColor) {
            Q_EMIT positiveTextColorChanged();
        }
        if (QColor newBackgroundColor = backgroundColor(); newBackgroundColor != oldBackgroundColor) {
            Q_EMIT backgroundColorChanged();
        }
        if (QColor newActiveBackgroundColor = activeBackgroundColor();
            newActiveBackgroundColor != oldActiveBackgroundColor) {
            Q_EMIT activeBackgroundColorChanged();
        }
        if (QColor newLinkBackgroundColor = linkBackgroundColor(); newLinkBackgroundColor != oldLinkBackgroundColor) {
            Q_EMIT linkBackgroundColorChanged();
        }
        if (QColor newVisitedLinkBackgroundColor = visitedLinkBackgroundColor();
            newVisitedLinkBackgroundColor != oldVisitedLinkBackgroundColor) {
            Q_EMIT visitedLinkBackgroundColorChanged();
        }
        if (QColor newNegativeBackgroundColor = negativeBackgroundColor();
            newNegativeBackgroundColor != oldNegativeBackgroundColor) {
            Q_EMIT negativeBackgroundColorChanged();
        }
        if (QColor newNeutralBackgroundColor = neutralBackgroundColor();
            newNeutralBackgroundColor != oldNeutralBackgroundColor) {
            Q_EMIT neutralBackgroundColorChanged();
        }
        if (QColor newPositiveBackgroundColor = positiveBackgroundColor();
            newPositiveBackgroundColor != oldPositiveBackgroundColor) {
            Q_EMIT positiveBackgroundColorChanged();
        }
        if (QColor newAlternateBackgroundColor = alternateBackgroundColor();
            newAlternateBackgroundColor != oldAlternateBackgroundColor) {
            Q_EMIT alternateBackgroundColorChanged();
        }
        if (QColor newFocusColor = focusColor(); newFocusColor != oldFocusColor) {
            Q_EMIT focusColorChanged();
        }
        if (QColor newHoverColor = hoverColor(); newHoverColor != oldHoverColor) {
            Q_EMIT hoverColorChanged();
        }
        if (QColor newLightShadeColor = lightShadeColor(); newLightShadeColor != oldLightShadeColor) {
            Q_EMIT lightShadeColorChanged();
        }
        if (QColor newMidLightShadeColor = midLightShadeColor(); newMidLightShadeColor != oldMidLightShadeColor) {
            Q_EMIT midLightShadeColorChanged();
        }
        if (QColor newMidShadeColor = midShadeColor(); newMidShadeColor != oldMidShadeColor) {
            Q_EMIT midShadeColorChanged();
        }
        if (QColor newDarkShadeColor = darkShadeColor(); newDarkShadeColor != oldDarkShadeColor) {
            Q_EMIT darkShadeColorChanged();
        }
        if (QColor newShadowShadeColor = shadowShadeColor(); newShadowShadeColor != oldShadowShadeColor) {
            Q_EMIT shadowShadeColorChanged();
        }
    }
}

QColor KisThemeColorGroup::textColor() const
{
    return currentScheme().foreground(KColorScheme::NormalText).color();
}

QColor KisThemeColorGroup::disabledTextColor() const
{
    return currentScheme().foreground(KColorScheme::InactiveText).color();
}

QColor KisThemeColorGroup::activeTextColor() const
{
    return currentScheme().foreground(KColorScheme::ActiveText).color();
}

QColor KisThemeColorGroup::linkColor() const
{
    return currentScheme().foreground(KColorScheme::LinkText).color();
}

QColor KisThemeColorGroup::visitedLinkColor() const
{
    return currentScheme().foreground(KColorScheme::VisitedText).color();
}

QColor KisThemeColorGroup::negativeTextColor() const
{
    return currentScheme().foreground(KColorScheme::NegativeText).color();
}

QColor KisThemeColorGroup::neutralTextColor() const
{
    return currentScheme().foreground(KColorScheme::NeutralText).color();
}

QColor KisThemeColorGroup::positiveTextColor() const
{
    return currentScheme().foreground(KColorScheme::PositiveText).color();
}

QColor KisThemeColorGroup::backgroundColor() const
{
    return currentScheme().background(KColorScheme::NormalBackground).color();
}

QColor KisThemeColorGroup::activeBackgroundColor() const
{
    return currentScheme().background(KColorScheme::ActiveBackground).color();
}

QColor KisThemeColorGroup::linkBackgroundColor() const
{
    return currentScheme().background(KColorScheme::LinkBackground).color();
}

QColor KisThemeColorGroup::visitedLinkBackgroundColor() const
{
    return currentScheme().background(KColorScheme::VisitedBackground).color();
}

QColor KisThemeColorGroup::negativeBackgroundColor() const
{
    return currentScheme().background(KColorScheme::NegativeBackground).color();
}

QColor KisThemeColorGroup::neutralBackgroundColor() const
{
    return currentScheme().background(KColorScheme::NeutralBackground).color();
}

QColor KisThemeColorGroup::positiveBackgroundColor() const
{
    return currentScheme().background(KColorScheme::PositiveBackground).color();
}

QColor KisThemeColorGroup::alternateBackgroundColor() const
{
    return currentScheme().background(KColorScheme::AlternateBackground).color();
}

QColor KisThemeColorGroup::focusColor() const
{
    return currentScheme().decoration(KColorScheme::FocusColor).color();
}

QColor KisThemeColorGroup::hoverColor() const
{
    return currentScheme().decoration(KColorScheme::HoverColor).color();
}

QColor KisThemeColorGroup::lightShadeColor() const
{
    return currentScheme().shade(KColorScheme::LightShade);
}

QColor KisThemeColorGroup::midLightShadeColor() const
{
    return currentScheme().shade(KColorScheme::MidlightShade);
}

QColor KisThemeColorGroup::midShadeColor() const
{
    return currentScheme().shade(KColorScheme::MidShade);
}

QColor KisThemeColorGroup::darkShadeColor() const
{
    return currentScheme().shade(KColorScheme::DarkShade);
}

QColor KisThemeColorGroup::shadowShadeColor() const
{
    return currentScheme().shade(KColorScheme::ShadowShade);
}

void KisThemeColorGroup::slotUpdateTheme()
{
    Q_EMIT textColorChanged();
    Q_EMIT disabledTextColorChanged();
    Q_EMIT activeTextColorChanged();
    Q_EMIT linkColorChanged();
    Q_EMIT visitedLinkColorChanged();
    Q_EMIT negativeTextColorChanged();
    Q_EMIT neutralTextColorChanged();
    Q_EMIT positiveTextColorChanged();
    Q_EMIT backgroundColorChanged();
    Q_EMIT activeBackgroundColorChanged();
    Q_EMIT linkBackgroundColorChanged();
    Q_EMIT visitedLinkBackgroundColorChanged();
    Q_EMIT negativeBackgroundColorChanged();
    Q_EMIT neutralBackgroundColorChanged();
    Q_EMIT positiveBackgroundColorChanged();
    Q_EMIT alternateBackgroundColorChanged();
    Q_EMIT focusColorChanged();
    Q_EMIT hoverColorChanged();
    Q_EMIT lightShadeColorChanged();
    Q_EMIT midLightShadeColorChanged();
    Q_EMIT midShadeColorChanged();
    Q_EMIT darkShadeColorChanged();
    Q_EMIT shadowShadeColorChanged();
}

const KColorScheme &KisThemeColorGroup::scheme(KColorScheme::ColorSet set, QPalette::ColorGroup state)
{
    switch (set) {
    case KColorScheme::Window:
        switch (state) {
        case QPalette::Inactive:
            return KisThemeConfig::instance()->windowInactive();
        case QPalette::Disabled:
            return KisThemeConfig::instance()->windowDisabled();
        default:
            return KisThemeConfig::instance()->windowActive();
        }
    case KColorScheme::Button:
        switch (state) {
        case QPalette::Inactive:
            return KisThemeConfig::instance()->buttonInactive();
        case QPalette::Disabled:
            return KisThemeConfig::instance()->buttonDisabled();
        default:
            return KisThemeConfig::instance()->buttonActive();
        }
    case KColorScheme::Selection:
        switch (state) {
        case QPalette::Inactive:
            return KisThemeConfig::instance()->selectionInactive();
        case QPalette::Disabled:
            return KisThemeConfig::instance()->selectionDisabled();
        default:
            return KisThemeConfig::instance()->selectionActive();
        }
    case KColorScheme::Tooltip:
        switch (state) {
        case QPalette::Inactive:
            return KisThemeConfig::instance()->tooltipInactive();
        case QPalette::Disabled:
            return KisThemeConfig::instance()->tooltipDisabled();
        default:
            return KisThemeConfig::instance()->tooltipActive();
        }
    default:
        switch (state) {
        case QPalette::Inactive:
            return KisThemeConfig::instance()->viewInactive();
        case QPalette::Disabled:
            return KisThemeConfig::instance()->viewDisabled();
        default:
            return KisThemeConfig::instance()->viewActive();
        }
    }
}

KisTheme::KisTheme(QObject *parent)
    : QObject(parent)
    , m_view(KisThemeColorGroup(KColorScheme::View, this))
    , m_window(KisThemeColorGroup(KColorScheme::Window, this))
    , m_button(KisThemeColorGroup(KColorScheme::Button, this))
    , m_selection(KisThemeColorGroup(KColorScheme::Selection, this))
    , m_tooltip(KisThemeColorGroup(KColorScheme::Tooltip, this))
{
    connect(KisThemeConfig::instance(), &KisThemeConfig::sigThemeChanged, this, &KisTheme::slotUpdateThemes);
}

KisThemeColorGroup *KisTheme::view()
{
    return &this->m_view;
}

KisThemeColorGroup *KisTheme::window()
{
    return &this->m_window;
}

KisThemeColorGroup *KisTheme::button()
{
    return &this->m_button;
}

KisThemeColorGroup *KisTheme::selection()
{
    return &this->m_selection;
}

KisThemeColorGroup *KisTheme::tooltip()
{
    return &this->m_tooltip;
}

void KisTheme::slotUpdateThemes()
{
    m_view.slotUpdateTheme();
    m_window.slotUpdateTheme();
    m_button.slotUpdateTheme();
    m_selection.slotUpdateTheme();
    m_tooltip.slotUpdateTheme();
}
