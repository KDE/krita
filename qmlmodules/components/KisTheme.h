/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISTHEME_H
#define KISTHEME_H

#include <QObject>
#include <QQmlEngine>
#include <functional>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <kcolorscheme.h>
#else
#include <KColorScheme>
#endif

class KisThemeConfig : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(KisThemeConfig)
public:
    static const KisThemeConfig *instance();

    const KColorScheme &viewActive() const
    {
        return m_viewActive;
    }

    const KColorScheme &viewInactive() const
    {
        return m_viewInactive;
    }

    const KColorScheme &viewDisabled() const
    {
        return m_viewDisabled;
    }

    const KColorScheme &windowActive() const
    {
        return m_windowActive;
    }

    const KColorScheme &windowInactive() const
    {
        return m_windowInactive;
    }

    const KColorScheme &windowDisabled() const
    {
        return m_windowDisabled;
    }

    const KColorScheme &buttonActive() const
    {
        return m_buttonActive;
    }

    const KColorScheme &buttonInactive() const
    {
        return m_buttonInactive;
    }

    const KColorScheme &buttonDisabled() const
    {
        return m_buttonDisabled;
    }

    const KColorScheme &selectionActive() const
    {
        return m_selectionActive;
    }

    const KColorScheme &selectionInactive() const
    {
        return m_selectionInactive;
    }

    const KColorScheme &selectionDisabled() const
    {
        return m_selectionDisabled;
    }

    const KColorScheme &tooltipActive() const
    {
        return m_tooltipActive;
    }

    const KColorScheme &tooltipInactive() const
    {
        return m_tooltipInactive;
    }

    const KColorScheme &tooltipDisabled() const
    {
        return m_tooltipDisabled;
    }

Q_SIGNALS:
    void sigThemeChanged();

private Q_SLOTS:
    void slotSetFilename(const QString &filename);

private:
    explicit KisThemeConfig(QObject *parent = nullptr);

    void updateSchemes();

    QString m_filename;
    KColorScheme m_viewActive = KColorScheme(QPalette::Active, KColorScheme::View);
    KColorScheme m_viewInactive = KColorScheme(QPalette::Inactive, KColorScheme::View);
    KColorScheme m_viewDisabled = KColorScheme(QPalette::Disabled, KColorScheme::View);
    KColorScheme m_windowActive = KColorScheme(QPalette::Active, KColorScheme::Window);
    KColorScheme m_windowInactive = KColorScheme(QPalette::Inactive, KColorScheme::Window);
    KColorScheme m_windowDisabled = KColorScheme(QPalette::Disabled, KColorScheme::Window);
    KColorScheme m_buttonActive = KColorScheme(QPalette::Active, KColorScheme::Button);
    KColorScheme m_buttonInactive = KColorScheme(QPalette::Inactive, KColorScheme::Button);
    KColorScheme m_buttonDisabled = KColorScheme(QPalette::Disabled, KColorScheme::Button);
    KColorScheme m_selectionActive = KColorScheme(QPalette::Active, KColorScheme::Selection);
    KColorScheme m_selectionInactive = KColorScheme(QPalette::Inactive, KColorScheme::Selection);
    KColorScheme m_selectionDisabled = KColorScheme(QPalette::Disabled, KColorScheme::Selection);
    KColorScheme m_tooltipActive = KColorScheme(QPalette::Active, KColorScheme::Tooltip);
    KColorScheme m_tooltipInactive = KColorScheme(QPalette::Inactive, KColorScheme::Tooltip);
    KColorScheme m_tooltipDisabled = KColorScheme(QPalette::Disabled, KColorScheme::Tooltip);
};

class KisThemeColorGroup : public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("Use Theme instead")
    Q_PROPERTY(int state READ state WRITE setState NOTIFY stateChanged FINAL)

    // Foreground roles: used for text.
    Q_PROPERTY(QColor textColor READ textColor NOTIFY textColorChanged FINAL)
    Q_PROPERTY(QColor disabledTextColor READ disabledTextColor NOTIFY disabledTextColorChanged FINAL)
    Q_PROPERTY(QColor activeTextColor READ activeTextColor NOTIFY activeTextColorChanged FINAL)

    Q_PROPERTY(QColor linkColor READ linkColor NOTIFY linkColorChanged FINAL)
    Q_PROPERTY(QColor visitedLinkColor READ visitedLinkColor NOTIFY visitedLinkColorChanged FINAL)

    Q_PROPERTY(QColor negativeTextColor READ negativeTextColor NOTIFY negativeTextColorChanged FINAL)
    Q_PROPERTY(QColor neutralTextColor READ neutralTextColor NOTIFY neutralTextColorChanged FINAL)
    Q_PROPERTY(QColor positiveTextColor READ positiveTextColor NOTIFY positiveTextColorChanged FINAL)

    // Background roles: used for backgrounds.
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY backgroundColorChanged FINAL)
    Q_PROPERTY(QColor activeBackgroundColor READ activeBackgroundColor NOTIFY activeBackgroundColorChanged FINAL)

    Q_PROPERTY(QColor linkBackgroundColor READ linkBackgroundColor NOTIFY linkBackgroundColorChanged FINAL)
    Q_PROPERTY(QColor visitedLinkBackgroundColor READ visitedLinkBackgroundColor NOTIFY
                   visitedLinkBackgroundColorChanged FINAL)

    Q_PROPERTY(QColor negativeBackgroundColor READ negativeBackgroundColor NOTIFY negativeBackgroundColorChanged FINAL)
    Q_PROPERTY(QColor neutralBackgroundColor READ neutralBackgroundColor NOTIFY neutralBackgroundColorChanged FINAL)
    Q_PROPERTY(QColor positiveBackgroundColor READ positiveBackgroundColor NOTIFY positiveBackgroundColorChanged FINAL)

    Q_PROPERTY(
        QColor alternateBackgroundColor READ alternateBackgroundColor NOTIFY alternateBackgroundColorChanged FINAL)

    // Decoration roles: used for borders.
    Q_PROPERTY(QColor focusColor READ focusColor NOTIFY focusColorChanged FINAL)
    Q_PROPERTY(QColor hoverColor READ hoverColor NOTIFY hoverColorChanged FINAL)

    // Shade roles: used for 3d elements like bevels.

    Q_PROPERTY(QColor lightShadeColor READ lightShadeColor NOTIFY lightShadeColorChanged FINAL)
    Q_PROPERTY(QColor midLightShadeColor READ midLightShadeColor NOTIFY midLightShadeColorChanged FINAL)
    Q_PROPERTY(QColor midShadeColor READ midShadeColor NOTIFY midShadeColorChanged FINAL)
    Q_PROPERTY(QColor darkShadeColor READ darkShadeColor NOTIFY darkShadeColorChanged FINAL)
    Q_PROPERTY(QColor shadowShadeColor READ shadowShadeColor NOTIFY shadowShadeColorChanged FINAL)

public:
    KisThemeColorGroup(KColorScheme::ColorSet set = KColorScheme::View, QObject *parent = nullptr);

    int state() const;
    void setState(int state);

    QColor textColor() const;
    QColor disabledTextColor() const;
    QColor activeTextColor() const;
    QColor linkColor() const;
    QColor visitedLinkColor() const;
    QColor negativeTextColor() const;
    QColor neutralTextColor() const;
    QColor positiveTextColor() const;
    QColor backgroundColor() const;
    QColor activeBackgroundColor() const;
    QColor linkBackgroundColor() const;
    QColor visitedLinkBackgroundColor() const;
    QColor negativeBackgroundColor() const;
    QColor neutralBackgroundColor() const;
    QColor positiveBackgroundColor() const;
    QColor alternateBackgroundColor() const;
    QColor focusColor() const;
    QColor hoverColor() const;
    QColor lightShadeColor() const;
    QColor midLightShadeColor() const;
    QColor midShadeColor() const;
    QColor darkShadeColor() const;
    QColor shadowShadeColor() const;

Q_SIGNALS:
    void stateChanged();
    void textColorChanged();
    void disabledTextColorChanged();
    void activeTextColorChanged();
    void linkColorChanged();
    void visitedLinkColorChanged();
    void negativeTextColorChanged();
    void neutralTextColorChanged();
    void positiveTextColorChanged();
    void backgroundColorChanged();
    void activeBackgroundColorChanged();
    void linkBackgroundColorChanged();
    void visitedLinkBackgroundColorChanged();
    void negativeBackgroundColorChanged();
    void neutralBackgroundColorChanged();
    void positiveBackgroundColorChanged();
    void alternateBackgroundColorChanged();
    void focusColorChanged();
    void hoverColorChanged();
    void lightShadeColorChanged();
    void midLightShadeColorChanged();
    void midShadeColorChanged();
    void darkShadeColorChanged();
    void shadowShadeColorChanged();

public Q_SLOTS:
    void slotUpdateTheme();

private:
    const KColorScheme &currentScheme() const
    {
        return scheme(m_set, m_state);
    }

    static const KColorScheme &scheme(KColorScheme::ColorSet set, QPalette::ColorGroup state);

    const KColorScheme::ColorSet m_set;
    QPalette::ColorGroup m_state = QPalette::Active;
};

/**
 * @brief The KisTheme class
 *
 * This class both handles retrieving colors from KColorScheme, as well
 * as listening to theme changes, simplifying theme updates.
 */

class KisTheme : public QObject
{
    Q_OBJECT

    Q_PROPERTY(KisThemeColorGroup *view READ view() CONSTANT FINAL)
    Q_PROPERTY(KisThemeColorGroup *window READ window() CONSTANT FINAL)
    Q_PROPERTY(KisThemeColorGroup *button READ button() CONSTANT FINAL)
    Q_PROPERTY(KisThemeColorGroup *selection READ selection() CONSTANT FINAL)
    Q_PROPERTY(KisThemeColorGroup *tooltip READ tooltip() CONSTANT FINAL)

    QML_NAMED_ELEMENT(Theme)
public:
    KisTheme(QObject *parent = nullptr);

    KisThemeColorGroup *view();
    KisThemeColorGroup *window();
    KisThemeColorGroup *button();
    KisThemeColorGroup *selection();
    KisThemeColorGroup *tooltip();

public Q_SLOTS:
    void slotUpdateThemes();

private:
    KisThemeColorGroup m_view;
    KisThemeColorGroup m_window;
    KisThemeColorGroup m_button;
    KisThemeColorGroup m_selection;
    KisThemeColorGroup m_tooltip;
};

#endif // KISTHEME_H
