/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISTHEME_H
#define KISTHEME_H

#include <QObject>
#include <QQmlEngine>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>
#include <kcolorscheme.h>

class KisThemeColorGroup : public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("Use Theme instead")
    Q_PROPERTY(int state READ state WRITE setState NOTIFY stateChanged)
public:
    KisThemeColorGroup(KColorScheme::ColorSet _set = KColorScheme::View, QObject *parent = nullptr);
    ~KisThemeColorGroup();

    lager::cursor<KColorScheme> scheme;
    LAGER_QT_READER(int, colorSet);

    // Foreground roles: used for text.
    LAGER_QT_READER(QColor, textColor);
    LAGER_QT_READER(QColor, disabledTextColor);
    LAGER_QT_READER(QColor, activeTextColor);

    LAGER_QT_READER(QColor, linkColor);
    LAGER_QT_READER(QColor, visitedLinkColor);

    LAGER_QT_READER(QColor, negativeTextColor);
    LAGER_QT_READER(QColor, neutralTextColor);
    LAGER_QT_READER(QColor, positiveTextColor);

    // Background roles: used for backgrounds.
    LAGER_QT_READER(QColor, backgroundColor);
    LAGER_QT_READER(QColor, activeBackgroundColor);

    LAGER_QT_READER(QColor, linkBackgroundColor);
    LAGER_QT_READER(QColor, visitedLinkBackgroundColor);

    LAGER_QT_READER(QColor, negativeBackgroundColor);
    LAGER_QT_READER(QColor, neutralBackgroundColor);
    LAGER_QT_READER(QColor, positiveBackgroundColor);

    LAGER_QT_READER(QColor, alternateBackgroundColor);

    // Decoration roles: used for borders.
    LAGER_QT_READER(QColor, focusColor);
    LAGER_QT_READER(QColor, hoverColor);

    // Shade roles: used for 3d elements like bevels.

    LAGER_QT_READER(QColor, lightShadeColor);
    LAGER_QT_READER(QColor, midLightShadeColor);
    LAGER_QT_READER(QColor, midShadeColor);
    LAGER_QT_READER(QColor, darkShadeColor);
    LAGER_QT_READER(QColor, shadowShadeColor);

    void updateColorScheme(const QString filename);
    void setState(int newState);
    int state();
private Q_SLOTS:
    void updateState();

Q_SIGNALS:
    void schemeChanged();
    void stateChanged();
private:
    KColorScheme::ColorSet m_set = KColorScheme::Window;
    QPalette::ColorGroup m_state = QPalette::Active;

    KColorScheme m_active;
    KColorScheme m_inactive;
    KColorScheme m_disabled;
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

    Q_PROPERTY(KisThemeColorGroup *view READ view() NOTIFY viewChanged())
    Q_PROPERTY(KisThemeColorGroup *window READ window() NOTIFY windowChanged())
    Q_PROPERTY(KisThemeColorGroup *button READ button() NOTIFY buttonChanged())
    Q_PROPERTY(KisThemeColorGroup *selection READ selection() NOTIFY selectionChanged())
    Q_PROPERTY(KisThemeColorGroup *tooltip READ tooltip() NOTIFY tooltipChanged())

    QML_NAMED_ELEMENT(Theme)
public:
    KisTheme(QObject *parent = nullptr);

    KisThemeColorGroup viewModel;
    KisThemeColorGroup windowModel;
    KisThemeColorGroup buttonModel;
    KisThemeColorGroup selectionModel;
    KisThemeColorGroup tooltipModel;

    KisThemeColorGroup *view();
    KisThemeColorGroup *window();
    KisThemeColorGroup *button();
    KisThemeColorGroup *selection();
    KisThemeColorGroup *tooltip();
private Q_SLOTS:
    void slotUpdateThemes();
Q_SIGNALS:
    void viewChanged();
    void windowChanged();
    void buttonChanged();
    void selectionChanged();
    void tooltipChanged();
};

#endif // KISTHEME_H
