/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef SVGTEXTTOOLOPTIONSOBJECT_H
#define SVGTEXTTOOLOPTIONSOBJECT_H

#include <QObject>
#include "SvgTextToolOptionsModel.h"
/**
 * @brief The SvgTextToolOptionsManager class
 *
 * Class that communicates state changes and signals between SvgTextTool.cpp
 * and the SvgTextToolOptions.qml file.
 */
class SvgTextToolOptionsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SvgTextToolOptionsModel *optionsModel READ optionsModel WRITE setOptionsModel NOTIFY optionsModelChanged)

    Q_PROPERTY(int textType READ textType WRITE convertToTextType NOTIFY convertTextType)
    Q_PROPERTY(bool textPropertiesOpen READ textPropertiesOpen WRITE setTextPropertiesOpen NOTIFY openTextPropertiesDocker)
    Q_PROPERTY(bool showTextPropertyButton READ showTextPropertyButton WRITE setShowTextPropertyButton NOTIFY showTextPropertyButtonChanged)

    /* Debug options */
    Q_PROPERTY(bool showDebug READ showDebug WRITE setShowDebug NOTIFY showDebugChanged)
    Q_PROPERTY(bool showCharacterDebug READ showCharacterDebug WRITE setShowCharacterDebug NOTIFY showDebugCharacterChanged)
    Q_PROPERTY(bool showLineDebug READ showLineDebug WRITE setShowLineDebug NOTIFY showLineDebugChanged)

public:
    explicit SvgTextToolOptionsManager(QObject *parent = nullptr);
    ~SvgTextToolOptionsManager();

    /// Sync with KoSvgTextShape::TextType
    enum TextType {
        PrePositionedText = 0,
        PreformattedText,
        InlineWrap,
        TextInShape
    };
    Q_ENUM(TextType)

    SvgTextToolOptionsModel* optionsModel() const;
    void setOptionsModel(SvgTextToolOptionsModel *model);

    /* Debug options */
    bool showDebug() const;
    void setShowDebug(const bool show);

    bool showCharacterDebug() const;
    void setShowCharacterDebug(const bool show);

    bool showLineDebug() const;
    void setShowLineDebug(const bool show);

    /*
     * typeType handles the conversion from textShapes
     * to different types, like wrapped, preformatted
     * and svg 1.1 style positioned text.
     */
    int textType() const;
    void convertToTextType(const int type);

    /**
     * @brief textPropertiesOpen
     * @return whether the text properties docker is open.
     */
    bool textPropertiesOpen() const;
    void setTextPropertiesOpen(const bool open);

    /**
     * @brief showTextPropertyButton
     * Whether to show the open text properties button.
     * It is hidden when somehow that docker is missing.
     */
    bool showTextPropertyButton() const;
    void setShowTextPropertyButton(const bool show);

public Q_SLOTS:
    /**
     * @brief emitOpenTextEditor
     * Emit the open text editor signal.
     */
    void emitOpenTextEditor();
    /**
     * @brief emitGlyphPalette
     * Emit the open glyph palette signal.
     */
    void emitGlyphPalette();

Q_SIGNALS:
    void optionsModelChanged();
    void convertTextType(int type);
    void openTextEditor();
    void openGlyphPalette();

    void openTextPropertiesDocker(bool open);
    void showTextPropertyButtonChanged();

    /* Debug options */
    void showDebugChanged();
    void showDebugCharacterChanged();
    void showLineDebugChanged();

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // SVGTEXTTOOLOPTIONSOBJECT_H
