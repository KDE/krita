#
# Simple script to take the template header/c++ file 
#
import sys
from string import Template

property_declaration_r = Template("""    Q_PROPERTY(${TYPE} ${PROPERTY} READ ${GETTER})
""")

property_declaration_rw = Template("""    Q_PROPERTY(${TYPE} ${PROPERTY} READ ${GETTER} WRITE ${SETTER})
""")

getter_declaration = Template("""    ${TYPE} ${GETTER}() const;
""")

setter_declaration = Template("""    void ${SETTER}(${TYPE} value);
""")

slot_declaration = Template("""    ${TYPE} ${SLOT};
""")

signal_declaration = Template("""    void ${SIGNAL};
""")

getter = Template("""${TYPE} ${CLASSNAME}::${GETTER}() const
{
    return ${RETURN_VALUE};
}

""")

setter = Template("""void ${CLASSNAME}::${SETTER}(${TYPE} value)
{
}

""")

slot_return = Template("""${TYPE} ${CLASSNAME}::${SLOT}
{
    return ${RETURN_VALUE};
}

""")

slot_no_return = Template("""${TYPE} ${CLASSNAME}::${SLOT}
{
}

""")


header = Template("""/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef LIBKIS_${HEADER_GUARD}_H
#define LIBKIS_${HEADER_GUARD}_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * ${CLASSNAME}
 */
class KRITALIBKIS_EXPORT ${CLASSNAME} : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(${CLASSNAME})
    
${PROPERTIES}
public:
    explicit ${CLASSNAME}(QObject *parent = 0);
    virtual ~${CLASSNAME}();

${GETTER_SETTER_DECLARATIONS}

public Q_SLOTS:
    
${SLOT_DECLARATIONS}
    
Q_SIGNALS:

${SIGNAL_DECLARATIONS}

private:
    struct Private;
    const Private *const d;

};

Q_DECLARE_METATYPE(${CLASSNAME}*)

#endif // LIBKIS_${HEADER_GUARD}_H
""")

source = Template("""/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
#include "${CLASSNAME}.h"

struct ${CLASSNAME}::Private {
    Private() {}
};

${CLASSNAME}::${CLASSNAME}(QObject *parent) 
    : QObject(parent)
    , d(new Private)
{
}

${CLASSNAME}::~${CLASSNAME}() 
{
    delete d;
}

${GETTER_SETTERS}

${SLOTS}

""")


sip = Template("""%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

class ${CLASSNAME} : public QObject
{
%TypeHeaderCode
#include "${CLASSNAME}.h"
%End

public:
    explicit ${CLASSNAME}(QObject *parent  /TransferThis/ = 0);
};
""")


def main(args):
    if len(args) != 4:
        print("Usage: apigen.py api.txt libdir sipdir")
        sys.exit(0)

    api = open(args[1], 'r').readlines()
    
    libdir = args[2]
    sipdir = args[3]
    
    class_definition = {"PROPERTIES" : [],
                        "SLOTS" : [],
                        "SIGNALS" : []
                        }
    class_name = ""

    inProperties = False
    inSlots = False
    inSignals = False
    
    classes = {}
    
    line_number = 0;
    
    for line in api:
        line_number += 1
        
        if line.isspace() or line.startswith("#"):
            continue
        
        elif line.startswith("class"):
            
            if class_name != "":
                classes[class_name] = class_definition
               
            class_definition = {"PROPERTIES" : [],
                                "SLOTS" : [],
                                "SIGNALS" : []
                                }
            
            inProperties = False
            inSlots = False
            inSignals = False
    
            class_name = line.split(' ')[1]
            
            if class_name == "Dummy":
                break
        
        elif line.lstrip().startswith("Properties"):
            inProperties = True
            inSlots = False
            inSignals = False
    
        elif line.lstrip().startswith("Slots"):
            inProperties = False
            inSlots = True
            inSignals = False
                
        elif line.lstrip().startswith("Signals"):
            inProperties = False
            inSlots = False
            inSignals = True
            
        elif line.startswith("$$$$$ END $$$$$"):
            break
            
        else:
            if inProperties:
                property_definition = line.split(" : ")
                if len(property_definition) != 2:
                    print("Could not parse property" + line + "," + str(line_number))
                    continue
                property_name = property_definition[0].lstrip().rstrip()
                property_type =  property_definition[1].lstrip().rstrip()
                
                d = {"TYPE" : property_type ,
                     "PROPERTY" : property_name,
                     "GETTER" : property_name[0].lower() + property_name[1:],
                     "SETTER" : "set" + property_name,
                     "CLASSNAME" : class_name
                    }
                if property_type != "void":
                    if property_type == "QString":
                        d["RETURN_VALUE"] = "QString()"
                    if property_type == "QByteArray":
                        d["RETURN_VALUE"] = "QByteArray()"
                    elif property_type == "int" or property_type.endswith("*"):
                        d["RETURN_VALUE"] = "0"
                    elif property_type.startswith("QList") or property_type.startswith("QMap"):
                        d["RETURN_VALUE"] = property_type + "()"
                    elif property_type == "bool":
                        d["RETURN_VALUE"] = "false"
                    elif property_type == "QVariant":
                        d["RETURN_VALUE"] = "QVariant()"
                        
                class_definition["PROPERTIES"].append(d)
            elif inSlots:
                
                slot_definition = line.split(" : ")
                if len(slot_definition) != 2:
                    print("Could not parse slot:" + line + "," + str(line_number))
                    continue
                slot_name = slot_definition[0].lstrip().rstrip()
                slot_type =  slot_definition[1].lstrip().rstrip()
                d = {"TYPE" : slot_type ,
                     "SLOT" : slot_name,
                     "CLASSNAME" : class_name
                     }
                
                if slot_type != "void":
                    if slot_type == "QString":
                        d["RETURN_VALUE"] = "QString()"
                    if slot_type == "QByteArray":
                        d["RETURN_VALUE"] = "QByteArray()"
                    elif slot_type == "int" or slot_type.endswith("*"):
                        d["RETURN_VALUE"] = "0"
                    elif slot_type.startswith("QList") or slot_type.startswith("QMap"):
                        d["RETURN_VALUE"] = slot_type + "()"
                    elif slot_type == "bool":
                        d["RETURN_VALUE"] = "false"
                    elif slot_type == "QVariant":
                        d["RETURN_VALUE"] = "QVariant()"
                class_definition["SLOTS"].append(d)
                
                        
            elif inSignals:
                class_definition["SIGNALS"].append({"SIGNAL": line.lstrip().rstrip(),
                                                    "CLASSNAME" : class_name})
            
    for class_name in classes:
        unpacked_definition = {
            "CLASSNAME" : class_name,
            "HEADER_GUARD" : class_name.upper(),
            "PROPERTIES" : "",
            "SLOT_DECLARATIONS" : "",
            "SIGNAL_DECLARATIONS" : "",
            "GETTER_SETTER_DECLARATIONS" : "",
            "SLOTS" : "",
            "GETTER_SETTERS" : ""
                }
        for p in classes[class_name]["PROPERTIES"]:
            unpacked_definition["PROPERTIES"] += property_declaration_rw.safe_substitute(p)
            unpacked_definition["GETTER_SETTER_DECLARATIONS"] += getter_declaration.safe_substitute(p)
            unpacked_definition["GETTER_SETTER_DECLARATIONS"] += setter_declaration.safe_substitute(p)
            unpacked_definition["GETTER_SETTER_DECLARATIONS"] += "\n"
            unpacked_definition["GETTER_SETTERS"] += getter.safe_substitute(p)
            unpacked_definition["GETTER_SETTERS"] += setter.safe_substitute(p)
            unpacked_definition["GETTER_SETTERS"] += "\n"
            
        for s in classes[class_name]["SLOTS"]:
            if "RETURN_VALUE" in s:
                unpacked_definition["SLOT_DECLARATIONS"] += slot_declaration.safe_substitute(s)
                unpacked_definition["SLOT_DECLARATIONS"] += "\n"
                unpacked_definition["SLOTS"] += slot_return.safe_substitute(s)
            else:
                unpacked_definition["SLOT_DECLARATIONS"] += slot_declaration.safe_substitute(s)
                unpacked_definition["SLOT_DECLARATIONS"] += "\n"
                unpacked_definition["SLOTS"] += slot_no_return.safe_substitute(s)
    
        for s in classes[class_name]["SIGNALS"]:
            unpacked_definition["SIGNAL_DECLARATIONS"] += signal_declaration.safe_substitute(s)
            
        f = open(libdir + "/" + class_name + ".h", 'w')
        f.write(header.safe_substitute(unpacked_definition))
        f.close()
        
        f = open(libdir + "/" + class_name + ".cpp", 'w')
        f.write(source.safe_substitute(unpacked_definition))
        f.close()
        
        f = open(sipdir + class_name + ".sip", 'w')
        f.write(sip.safe_substitute(unpacked_definition))
        f.close()
                

if __name__ == "__main__":
    main(sys.argv)
