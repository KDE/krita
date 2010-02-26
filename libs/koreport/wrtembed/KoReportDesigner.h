/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef REPORTDESIGNER_H
#define REPORTDESIGNER_H

#include <qwidget.h>
#include <qstring.h>

#include <qcolor.h>
#include <qmap.h>
#include <QVBoxLayout>
#include <QCloseEvent>

#include <krreportdata.h>
#include <koproperty/Set.h>
#include <koproperty/Property.h>
#include <kdebug.h>
#include <krobjectdata.h>
#include "koreport_export.h"
#include "KoReportData.h"

class ReportGridOptions;
class QDomDocument;
class QGraphicsScene;
class KoRuler;
class KoZoomHandler;
class QGridLayout;
class ReportSectionDetail;
class ReportSection;
class KoUnit;
class ReportScene;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneContextMenuEvent;
class ReportSceneView;
class ReportWriterSectionData;
class KexiView;

//
// Class ReportDesigner
//     The ReportDesigner is the main widget for designing a report
//
class KOREPORT_EXPORT KoReportDesigner : public QWidget
{
    Q_OBJECT
public:

    /**
    @brief Constructor that create a blank designer
    @param widget QWidget parent
    */
    KoReportDesigner(QWidget *);

    /**
    @brief Constructor that create a designer, and loads the report described in the QDomElement
    @param widget QWidget parent
    @param element Report structure XML element
    */
    KoReportDesigner(QWidget *, QDomElement);

    /**
    @brief Desctructor
    */
    ~KoReportDesigner();

    /**
    @brief Sets the report data
    The report data interface contains functions to retrieve data
    and information about the fields.
    @param kodata Pointer to KoReportData instance
    */
    void setReportData(KoReportData* kodata);

    /**
    @brief Return a pointer to the reports data
    @return Pointer to report data
    */
    KoReportData *reportData(){return m_kordata;}

    /**
    @brief Return a pointer to the section specified
    @param section KRSectionData::Section enum value of the section to return
    @return Pointer to report section object, or 0 if no section exists
    */
    ReportSection* section(KRSectionData::Section) const;

    /**
    @brief Deletes the section specified
    @param section KRSectionData::Section enum value of the section to return
    */
    void removeSection(KRSectionData::Section);

    /**
    @brief Create a new section and insert it into the report
    @param section KRSectionData::Section enum value of the section to return
    */
    void insertSection(KRSectionData::Section);

    /**
    @brief Return a pointer to the detail section.
    The detail section contains the actual detail section and related group sections
    @return Pointer to detail section
    */
    ReportSectionDetail* detailSection() const {
        return m_detail;
    }

    /**
    @brief Sets the title of the reportData
    @param title Report Title
    */
    void setReportTitle(const QString &);

    /**
    @brief Sets the parameters for the display of the background gridpoints
    @param visible Grid visibility
    @param divisions Number of minor divisions between major points
    */
    void setGridOptions(bool visible, int divisions);

    /**
    @brief Return the title of the report
    */
    QString reportTitle() const;

    /**
    @brief Return an XML description of the report
    @return QDomElement describing the report definition
    */
    QDomElement document() const;

    /**
    @brief Return true if the design has been modified
    @return modified status
    */
    bool isModified() const;

    /**
    @return a list of field names in the selected KoReportData
    */
    QStringList fieldNames() const;
    
    /**
    @return a list of field keys in the selected KoReportData
    The keys can be used to reference the names
    */
    QStringList fieldKeys() const;

    /**
    @brief Calculate the width of the page in pixels given the paper size, orientation, dpi and margin
    @return integer value of width in pixels
    */
    int pageWidthPx() const;

    /**
    @return the scene (section) that is currently active
    */
    QGraphicsScene* activeScene() const;

    /**
    @brief Sets the active Scene
    @param scene The scene to make active
    */
    void setActiveScene(QGraphicsScene* scene);

    /**
    @return the property set for the general report properties
    */
    KoProperty::Set* propertySet() const {
        return m_set;
    }

    /**
    @brief Give a hint on the size of the widget
    */
    virtual QSize sizeHint() const;

    /**
    @brief Return a pointer to the zoom handler
    */
    KoZoomHandler* zoomHandler() const;

    /**
    @brief Return the current unit assigned to the report
    */
    KoUnit pageUnit() const;

    /**
    @brief Handle the context menu event for a report section
    @param scene The associated scene (section)
    */
    void sectionContextMenuEvent(ReportScene *, QGraphicsSceneContextMenuEvent * e);

    /**
    @brief Handle the mouse release event for a report section
    */
    void sectionMouseReleaseEvent(ReportSceneView *, QMouseEvent * e);

    /**
    @brief Sets the property set for the currenty selected item
    @param set Property set of item
    */
    void changeSet(KoProperty::Set *);

    /**
    @brief Return the property set for the curently selected item
    */
    KoProperty::Set* itemPropertySet() const {
        kDebug(); return m_itmset;
    }

    /**
    @brief Sets the modified status, defaulting to true for modified
    @param modified Modified status
    */
    void setModified(bool = true);

    /**
    @brief Return a unique name that can be used by the entity
    @param entity Name of entity
    */
    QString suggestEntityName(const QString &) const;

    /**
    @brief Checks if the supplied name is unique among all entities
    */
    bool isEntityNameUnique(const QString &, KRObjectData* = 0) const;

    /**
    @brief Return a list of actions that represent the netities that can be inserted into the report
    @return QList of QActions
    */
    static QList<QAction*> actions();

public slots:

    void slotEditDelete();
    void slotEditCut();
    void slotEditCopy();
    void slotEditPaste();
    void slotEditPaste(QGraphicsScene *);

    void slotItem(KRObjectData::EntityTypes);
    void slotItem(const QString&);

    void slotSectionEditor();

    void slotRaiseSelected();
    void slotLowerSelected();

protected:

    ReportSection *m_reportHeader;
    ReportSection *m_pageHeaderFirst;
    ReportSection *m_pageHeaderOdd;
    ReportSection *m_pageHeaderEven;
    ReportSection *m_pageHeaderLast;
    ReportSection *m_pageHeaderAny;

    ReportSection *m_pageFooterFirst;
    ReportSection *m_pageFooterOdd;
    ReportSection *m_pageFooterEven;
    ReportSection *m_pageFooterLast;
    ReportSection *m_pageFooterAny;
    ReportSection *m_reportFooter;
    ReportSectionDetail *m_detail;

private:
    class Private;
    Private * const d;

    void init();
    bool m_modified; // true if this document has been modified, false otherwise

    KoReportData *m_kordata;

    /**
    @brief Return a list of supported page formats
    @return A QStringList of page formats
    */
    QStringList pageFormats() const;

    /**
    @brief Sets the detail section to the given section
    */
    void setDetail(ReportSectionDetail *rsd);

    /**
    @brief Deletes the detail section
    */
    void deleteDetail();
        
    virtual void resizeEvent(QResizeEvent * event);

    //Properties
    void createProperties();
    KoProperty::Set* m_set;
    KoProperty::Set* m_itmset;
    KoProperty::Property* m_title;
    KoProperty::Property* m_pageSize;
    KoProperty::Property* m_orientation;
    KoProperty::Property* m_unit;
    KoProperty::Property* m_customHeight;
    KoProperty::Property* m_customWidth;
    KoProperty::Property* m_leftMargin;
    KoProperty::Property* m_rightMargin;
    KoProperty::Property* m_topMargin;
    KoProperty::Property* m_bottomMargin;
    KoProperty::Property* m_showGrid;
    KoProperty::Property* m_gridDivisions;
    KoProperty::Property* m_gridSnap;
    KoProperty::Property* m_labelType;
    KoProperty::Property* m_interpreter;
    KoProperty::Property* m_script;

    ReportWriterSectionData *m_sectionData;
    unsigned int selectionCount() const;

private slots:
    void slotPropertyChanged(KoProperty::Set &s, KoProperty::Property &p);

    /**
    @brief When the 'page' button in the top left is pressed, change the property set to the reports properties.
    */
    void slotPageButton_Pressed();

signals:
    void pagePropertyChanged(KoProperty::Set &s);
    void propertySetChanged();
    void dirty();
    void reportDataChanged();
};

#endif

