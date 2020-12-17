/*
 *  SPDX-FileCopyrightText: 2015 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef LIBKIS_EXTENSION_H
#define LIBKIS_EXTENSION_H

#include "kritalibkis_export.h"

#include <QObject>
#include <Window.h>

/**
 * An Extension is the base for classes that extend Krita. An Extension
 * is loaded on startup, when the setup() method will be executed.
 *
 * The extension instance should be added to the Krita Application object
 * using Krita.instance().addViewExtension or Application.addViewExtension
 * or Scripter.addViewExtension.
 *
 * Example:
 *
 * @code
 * import sys
 * from PyQt5.QtGui import *
 * from PyQt5.QtWidgets import *
 * from krita import *
 * class HelloExtension(Extension):
 *
 * def __init__(self, parent):
 *     super().__init__(parent)
 *
 * def hello(self):
 *     QMessageBox.information(QWidget(), "Test", "Hello! This is Krita " + Application.version())
 *
 * def setup(self):
 *     qDebug("Hello Setup")
 *
 * def createActions(self, window)
 *     action = window.createAction("hello")
 *     action.triggered.connect(self.hello)
 *
 * Scripter.addExtension(HelloExtension(Krita.instance()))
 *
 * @endcode
 */
class KRITALIBKIS_EXPORT Extension : public QObject
{
    Q_OBJECT
public:

    /**
     * Create a new extension. The extension will be
     * owned by @p parent.
     */
    explicit Extension(QObject *parent = 0);
    ~Extension() override;

    /**
     * Override this function to setup your Extension. You can use it to integrate
     * with the Krita application instance.
     */
    virtual void setup() = 0;

    virtual void createActions(Window *window) = 0;

};




#endif
