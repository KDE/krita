/* This file is part of the KDE project
   Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KARBONCALLIGRAPHYOPTIONWIDGET_H
#define KARBONCALLIGRAPHYOPTIONWIDGET_H

#include <QWidget>
#include <QMap>
#include <kis_properties_configuration.h>

class KComboBox;
class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;
class QToolButton;
class KarbonCalligraphyToolOptions;
class KisCurveOptionWidget;

class KarbonCalligraphyOptionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KarbonCalligraphyOptionWidget();
    ~KarbonCalligraphyOptionWidget();

    // emits all signals with the appropriate values
    // called once the signals are connected inside KarbonCalligraphyTool
    // to make sure all parameters are uptodate
    void emitAll();

Q_SIGNALS:
    // all the following signals emit user friendly values, not the internal
    // values which are instead computed directly by KarbonCalligraphyTool
    void usePathChanged(bool);
    void useAssistantChanged(bool);
    void useNoAdjustChanged(bool);
    void settingsChanged(KisPropertiesConfigurationSP settings);
    void massChanged(double);
    void dragChanged(double);
    void smoothTimeChanged(double);
    void smoothDistanceChanged(double);

public Q_SLOTS:
    // needed for the shortcuts
    //void increaseWidth();
    //void decreaseWidth();
    //void increaseAngle();
    //void decreaseAngle();

private Q_SLOTS:
    void loadProfile(const QString &name);
    //void toggleUseAngle(bool checked);
    void updateCurrentProfile();
    void saveProfileAs();
    void removeProfile();
    void generateSettings();

private:
    // TODO: maybe make it a hash?? <QString, QVariant>
    //       is it needed al all??
    struct Profile {
        QString name;
        int index; // index in the config file
        bool usePath;
        bool useAssistants;
        qreal caps;
        qreal timeInterval;
        qreal distanceInterval;
        KisPropertiesConfigurationSP curveConfig;
    };

    // convenience functions:

    // connects signals and slots
    void createConnections();

    // if they aren't already added adds the default profiles
    // called by the ctor
    void addDefaultProfiles();

    // laod the profiles from the configuration file
    void loadProfiles();

    // loads the profile set as current profile in the configuration file
    void loadCurrentProfile();

    // save a new profile using the values of the input boxes
    // if a profile with the same name already exists it will be overwritten
    void saveProfile(const QString &name);

    // removes the profile from the configuration file, from profiles
    // and from the combobox.
    // if the profile doesn't exist the function does nothing
    void removeProfile(const QString &name);

    // returns the position inside profiles of a certain profile
    // returns -1 if the profile is not found
    int profilePosition(const QString &profileName);

private:
    typedef QMap<QString, Profile *> ProfileMap;
    ProfileMap m_profiles;

    KarbonCalligraphyToolOptions *m_options;
    KisCurveOptionWidget *m_sizeOption;
    KisCurveOptionWidget *m_rotationOption;
    KisCurveOptionWidget *m_ratioOption;

    // when true updateCurrentProfile() doesn't do anything
    bool m_changingProfile;
};

#endif // KARBONCALLIGRAPHYOPTIONWIDGET_H
