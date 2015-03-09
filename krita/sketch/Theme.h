/*
 * This file is part of the KDE project
 * Copyright (C) 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef THEME_H
#define THEME_H

#include <QObject>
#include <QVariantMap>

#include "krita_sketch_export.h"

class KRITA_SKETCH_EXPORT Theme : public QObject
{
    Q_OBJECT
    /**
     * \property id
     * \brief
     *
     * \get id() const
     * \set setId()
     * \notify idChanged()
     */
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    /**
     * \property name
     * \brief The user-visible name of this theme.
     *
     * \get name() const
     * \set setName()
     * \notify nameChanged()
     */
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    /**
     * \property inherits
     * \brief The id of the theme this theme inherits.
     *
     * If a certain property can not be found, the theme will try to get that property
     * from the inherited theme.
     *
     * \default None
     * \get inherits() const
     * \set setInherits()
     * \notify inheritsChanged()
     */
    Q_PROPERTY(QString inherits READ inherits WRITE setInherits NOTIFY inheritsChanged)
    /**
     * \property colors
     * \brief A JavaScript object describing the colors to be used by this theme.
     *
     * \get colors() const
     * \set setColors()
     * \notify colorsChanged()
     */
    Q_PROPERTY(QVariantMap colors READ colors WRITE setColors NOTIFY colorsChanged)
    /**
     * \property sizes
     * \brief A JavaScript object describing a number of sizes to be used by this theme.
     *
     * \get sizes() const
     * \set setSizes()
     * \notify sizesChanged()
     */
    Q_PROPERTY(QVariantMap sizes READ sizes WRITE setSizes NOTIFY sizesChanged)
    /**
     * \property fonts
     * \brief A JavaScript object describing the fonts to be used by this theme.
     *
     * \get fonts() const
     * \set setFonts()
     * \notify fontsChanged()
     */
    Q_PROPERTY(QVariantMap fonts READ fonts WRITE setFonts NOTIFY fontsChanged)
    /**
     * \property iconPath
     * \brief The path used to look up icons from the theme.
     *
     * Relative paths are relative to the theme directory.
     *
     * \default "icons/"
     * \get iconPath() const
     * \set setIconPath()
     * \notify iconPathChanged()
     */
    Q_PROPERTY(QString iconPath READ iconPath WRITE setIconPath NOTIFY iconPathChanged)
    /**
     * \property imagePath
     * \brief The path used to look up images from the theme.
     *
     * Relative paths are relative to the theme directory.
     *
     * \default "images/"
     * \get imagePath() const
     * \set setImagePath()
     * \notify imagePathChanged()
     */
    Q_PROPERTY(QString imagePath READ imagePath WRITE setImagePath NOTIFY imagePathChanged)
    /**
     * \property fontPath
     * \brief A path containing additional fonts to load.
     *
     * The fontPath specifies a directory that will be searched for font files. These
     * font files will then be made available for use in the theme.
     *
     * \default "fonts/"
     * \get fontPath() const
     * \set setFontPath()
     * \notify fontPathChanged()
     */
    Q_PROPERTY(QString fontPath READ fontPath WRITE setFontPath NOTIFY fontPathChanged)
public:
    explicit Theme(QObject* parent = 0);
    virtual ~Theme();

    /**
     * Getter for property #id.
     */
    QString id() const;
    /**
     * Setter for property #id.
     */
    void setId(const QString& newValue);

    /**
     * Getter for property #name.
     */
    QString name() const;
    /**
     * Setter for property #name.
     */
    void setName(const QString& newValue);

    /**
     * Getter for property #inherits.
     */
    QString inherits() const;
    /**
     * Setter for property #inherits.
     */
    void setInherits(const QString& newValue);

    /**
     * Getter for property #colors.
     */
    QVariantMap colors() const;
    /**
     * Setter for property #colors.
     */
    void setColors(const QVariantMap& newValue);

    /**
     * Getter for property #sizes.
     */
    QVariantMap sizes() const;
    /**
     * Setter for property #sizes.
     */
    void setSizes(const QVariantMap& newValue);

    /**
     * Getter for property #fonts.
     */
    QVariantMap fonts() const;
    /**
     * Setter for property #fonts.
     */
    void setFonts(const QVariantMap& newValue);

    /**
     * Getter for property #iconPath.
     */
    QString iconPath() const;
    /**
     * Setter for property #iconPath.
     */
    void setIconPath(const QString& newValue);

    /**
     * Getter for property #imagePath.
     */
    QString imagePath() const;
    /**
     * Setter for property #imagePath.
     */
    void setImagePath(const QString& newValue);

    /**
     * Getter for property #fontPath.
     */
    QString fontPath() const;
    /**
     * Setter for property #fontPath.
     */
    void setFontPath(const QString& newValue);

    /**
     * Get a single color from the theme.
     *
     * \param name The color to get.
     * \return The color asked for, or a default color if it is not defined in the theme.
     */
    Q_INVOKABLE QColor color(const QString& name);
    /**
     * Get a single size value from the theme.
     */
    Q_INVOKABLE float size(const QString& name);
    /**
     * Get an icon from the theme.
     */
    Q_INVOKABLE QUrl icon(const QString& name);
    /**
     * Get a font from the theme.
     */
    Q_INVOKABLE QFont font(const QString& name);
    /**
     * Get an image from the theme.
     */
    Q_INVOKABLE QUrl image(const QString& name);

    static Theme* load(const QString& id, QObject* parent = 0);

Q_SIGNALS:
    void idChanged();
    void nameChanged();
    void inheritsChanged();
    void colorsChanged();
    void sizesChanged();
    void fontsChanged();
    void iconPathChanged();
    void imagePathChanged();
    void fontPathChanged();
    void fontCacheRebuilt();

protected:
    virtual bool eventFilter(QObject*, QEvent*);

private:
    class Private;
    Private * const d;
};

#endif // THEME_H
