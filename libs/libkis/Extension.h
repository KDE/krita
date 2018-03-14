/*
 *  Copyright (c) 2015 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
     * owned by @param parent.
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
