/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOSVGTEXTPROPERTIESINTERFACE_H
#define KOSVGTEXTPROPERTIESINTERFACE_H

#include <QObject>
#include <KoSvgTextProperties.h>
#include <kritaflake_export.h>

/**
 * @brief The KoSvgTextPropertiesInterface class
 *
 * This is an interface that can be used by tools to communicate
 * with the KisTextPropertiesManager.
 */
class KRITAFLAKE_EXPORT KoSvgTextPropertiesInterface : public QObject
{
    Q_OBJECT
public:
    explicit KoSvgTextPropertiesInterface(QObject *parent = nullptr): QObject(parent){}

    /**
     * @brief getSelectedProperties
     * @return all KoSvgTextProperties for the given selection.
     */
    virtual QList<KoSvgTextProperties> getSelectedProperties() = 0;

    /**
     * @brief getSelectedProperties
     * @return all KoSvgTextProperties for the given character selection.
     */
    virtual QList<KoSvgTextProperties> getCharacterProperties()  = 0;

    /**
     * @brief getInheritedProperties
     * The properties that should be visible when a given property
     * isn't available in common properties. This is typically the
     * paragraph properties.
     * @return what counts as the inherited properties for the given selection.
     */
    virtual KoSvgTextProperties getInheritedProperties() = 0;

    /**
     * @brief setPropertiesOnSelected
     * This sets the properties on the selection. The implementation is responsible
     * for handling the undo states.
     * @param properties -- the properties to set.
     * @param removeProperties -- properties to remove.
     */
    virtual void setPropertiesOnSelected(KoSvgTextProperties properties, QSet<KoSvgTextProperties::PropertyId> removeProperties = QSet<KoSvgTextProperties::PropertyId>()) = 0;

    /**
     * @brief setCharacterPropertiesOnSelected
     * This sets the properties for a character selection instead of the full
     * text shape. Typically the selection of characters.
     * The implementation is responsible for handling the undo states.
     * @param properties -- the properties to set.
     * @param removeProperties -- properties to remove.
     */
    virtual void setCharacterPropertiesOnSelected(KoSvgTextProperties properties, QSet<KoSvgTextProperties::PropertyId> removeProperties = QSet<KoSvgTextProperties::PropertyId>()) = 0;

    /// Whether the tool is currently selecting a set of characters instead of whole paragraphs.
    virtual bool spanSelection() = 0;

    /// Whether character selections are possible at all.
    virtual bool characterPropertiesEnabled() = 0;
Q_SIGNALS:
    /// Emit to signal to KisTextPropertiesManager to call getSelectedProperties
    void textSelectionChanged();
    /// Emit to signal to KisTextPropertiesManager to call getCharacterProperties
    /// and getInheritedProperties.
    void textCharacterSelectionChanged();
};


#endif // KOSVGTEXTPROPERTIESINTERFACE_H
