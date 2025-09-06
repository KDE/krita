/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TEXTPROPERTIESMANAGER_H
#define TEXTPROPERTIESMANAGER_H

#include <QObject>
#include <kis_mainwindow_observer.h>

#include <kis_canvas2.h>
#include <KoResource.h>
#include <lager/KoSvgTextPropertiesModel.h>
#include "TextPropertyConfigModel.h"

/**
 * @brief The TextPropertiesCanvasObserver class
 *
 * This splits out the canvas observer tasks from the docker into
 * it's own QObject, so that it may be instantiated inside QML.
 *
 * The benefit of this is that we'll have to do way less manipulating
 * of the rootobject on the quickwidget if we want to communicate
 * properties between qml and cpp. Given that the rootobject only exists when
 * the qml successfully loads, this should require way less checks.
 */
class TextPropertiesCanvasObserver : public QObject, public KisMainwindowObserver
{
    Q_OBJECT
    Q_PROPERTY(qreal dpi READ dpi() NOTIFY dpiChanged)
    Q_PROPERTY(QStringList locales READ locales() NOTIFY localesChanged)
    Q_PROPERTY(KoSvgTextPropertiesModel *textProperties READ textProperties NOTIFY textPropertiesChanged)
    Q_PROPERTY(KoSvgTextPropertiesModel *characterProperties READ characterProperties NOTIFY characterPropertiesChanged)
    Q_PROPERTY(TextPropertyConfigModel *textPropertyConfig READ textPropertyConfig NOTIFY textPropertyConfigChanged)
    /*
     * \qmlproperty hasFocus
     * whether the related widget has foxus. Used to use to figure out whether
     * we want to let the current signal compressor go on(hasFocus = true), or
     * whether we want to retrieve the new text property data from the canvas
     *  resource provider (hasFocus = false).
     */
    Q_PROPERTY(bool hasFocus READ hasFocus WRITE setHasFocus NOTIFY hasFocusChanged)
public:
    explicit TextPropertiesCanvasObserver(QObject *parent = nullptr);
    ~TextPropertiesCanvasObserver();

    QString observerName() override { return "TextPropertiesCanvasObserver"; }
    void setViewManager(KisViewManager *kisview) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

    qreal dpi() const;
    QStringList locales() const;
    KoSvgTextPropertiesModel *textProperties() const;
    KoSvgTextPropertiesModel *characterProperties() const;
    TextPropertyConfigModel *textPropertyConfig() const;

    bool hasFocus() const;
    void setHasFocus(const bool focus);

public Q_SLOTS:
    void slotCanvasTextPropertiesChanged();
    void slotCanvasCharacterPropertiesChanged();
    void slotTextPropertiesChanged();
    void slotCharacterPropertiesChanged();

    void callModalTextPropertyConfigDialog();

    void applyPreset(KoResourceSP resource);
    bool createNewPresetFromSettings();
    void editPreset(KoResourceSP resource);
    void cloneAndEditPreset(KoResourceSP resource);
Q_SIGNALS:
    void dpiChanged();
    void localesChanged();
    void textPropertiesChanged();
    void textPropertyConfigChanged();
    void characterPropertiesChanged();
    void hasFocusChanged();
private:
    struct Private;
    const QScopedPointer<Private> d;
    QPointer<KisCanvas2> m_canvas;
};

#endif // TEXTPROPERTIESMANAGER_H
