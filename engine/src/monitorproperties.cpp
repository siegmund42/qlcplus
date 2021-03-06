/*
  Q Light Controller Plus
  monitorproperties.cpp

  Copyright (c) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include <QFont>

#include "monitorproperties.h"
#include "doc.h"

#define KXMLQLCMonitorDisplay "DisplayMode"
#define KXMLQLCMonitorChannels "ChannelStyle"
#define KXMLQLCMonitorValues "ValueStyle"
#define KXMLQLCMonitorFont "Font"
#define KXMLQLCMonitorGrid "Grid"
#define KXMLQLCMonitorGridWidth "Width"
#define KXMLQLCMonitorGridHeight "Height"
#define KXMLQLCMonitorGridDepth "Depth"
#define KXMLQLCMonitorGridUnits "Units"
#define KXMLQLCMonitorPointOfView "POV"
#define KXMLQLCMonitorShowLabels "ShowLabels"
#define KXMLQLCMonitorCommonBackground "Background"

#define KXMLQLCMonitorCustomBgItem "BackgroundItem"
#define KXMLQLCMonitorCustomBgFuncID "ID"

#define KXMLQLCMonitorFixtureItem "FxItem"
#define KXMLQLCMonitorFixtureID "ID"
#define KXMLQLCMonitorFixtureXPos "XPos"
#define KXMLQLCMonitorFixtureYPos "YPos"
#define KXMLQLCMonitorFixtureZPos "ZPos"
#define KXMLQLCMonitorFixtureRotation "Rotation"
#define KXMLQLCMonitorFixtureXRotation "XRot"
#define KXMLQLCMonitorFixtureYRotation "YRot"
#define KXMLQLCMonitorFixtureZRotation "ZRot"
#define KXMLQLCMonitorFixtureGelColor "GelColor"

#define GRID_DEFAULT_WIDTH  5
#define GRID_DEFAULT_HEIGHT 3
#define GRID_DEFAULT_DEPTH  5

MonitorProperties::MonitorProperties()
    : m_displayMode(DMX)
    , m_channelStyle(DMXChannels)
    , m_valueStyle(DMXValues)
    , m_gridSize(QVector3D(GRID_DEFAULT_WIDTH, GRID_DEFAULT_HEIGHT, GRID_DEFAULT_DEPTH))
    , m_gridUnits(Meters)
    , m_pointOfView(Undefined)
    , m_showLabels(false)
{
    m_font = QFont("Arial", 12);
}

void MonitorProperties::setPointOfView(MonitorProperties::PointOfView pov)
{
    if (pov == m_pointOfView)
        return;

    if (m_pointOfView == Undefined)
    {
        QVector3D gSize = gridSize();
        float units = gridUnits() == MonitorProperties::Meters ? 1000.0 : 304.8;

        if (gSize.z() == 0)
        {
            // convert the grid size first
            switch (pov)
            {
                case TopView:
                    setGridSize(QVector3D(gSize.x(), GRID_DEFAULT_HEIGHT, gSize.y()));
                break;
                case RightSideView:
                case LeftSideView:
                    setGridSize(QVector3D(GRID_DEFAULT_WIDTH, gSize.x(), gSize.x()));
                break;
                default:
                break;
            }
        }

        foreach (quint32 fid, fixtureItemsID())
        {
            QVector3D pos = fixturePosition(fid);
            QVector3D newPos;

            switch (pov)
            {
                case TopView:
                {
                    newPos = QVector3D(pos.x(), 1000, pos.y());
                }
                break;
                case RightSideView:
                {
                    newPos = QVector3D(0, pos.y(), (gridSize().z() * units) - pos.x());
                }
                break;
                case LeftSideView:
                {
                    newPos = QVector3D(0, pos.y(), pos.x());
                }
                break;
                default:
                    newPos = QVector3D(pos.x(), (gridSize().y() * units) - pos.y(), 1000);
                break;
            }
            setFixturePosition(fid, newPos);
        }
    }
    m_pointOfView = pov;
}

void MonitorProperties::removeFixture(quint32 fid)
{
    if (m_fixtureItems.contains(fid))
        m_fixtureItems.take(fid);
}

void MonitorProperties::setFixturePosition(quint32 fid, QVector3D pos)
{
    qDebug() << Q_FUNC_INFO << "X:" << pos.x() << "Y:" << pos.y();
    m_fixtureItems[fid].m_position = pos;
}

void MonitorProperties::setFixtureRotation(quint32 fid, QVector3D degrees)
{
    m_fixtureItems[fid].m_rotation = degrees;
}

void MonitorProperties::setFixtureGelColor(quint32 fid, QColor col)
{
    qDebug() << Q_FUNC_INFO << "Gel color:" << col;
    m_fixtureItems[fid].m_gelColor = col;
}

QString MonitorProperties::customBackground(quint32 fid)
{
    return m_customBackgroundImages.value(fid, QString());
}

void MonitorProperties::reset()
{
    m_gridSize = QVector3D(GRID_DEFAULT_WIDTH, GRID_DEFAULT_HEIGHT, GRID_DEFAULT_DEPTH);
    m_gridUnits = Meters;
    m_showLabels = false;
    m_fixtureItems.clear();
    m_commonBackgroundImage = QString();
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool MonitorProperties::loadXML(QXmlStreamReader &root, const Doc *mainDocument)
{
    if (root.name() != KXMLQLCMonitorProperties)
    {
        qWarning() << Q_FUNC_INFO << "Monitor node not found";
        return false;
    }

    QXmlStreamAttributes attrs = root.attributes();

    if (attrs.hasAttribute(KXMLQLCMonitorDisplay) == false)
    {
        qWarning() << Q_FUNC_INFO << "Cannot determine Monitor display mode !";
        return false;
    }

    setDisplayMode(DisplayMode(attrs.value(KXMLQLCMonitorDisplay).toString().toInt()));
    if (attrs.hasAttribute(KXMLQLCMonitorShowLabels))
    {
        if (attrs.value(KXMLQLCMonitorShowLabels).toString() == "1")
            setLabelsVisible(true);
        else
            setLabelsVisible(false);
    }

    while (root.readNextStartElement())
    {
        QXmlStreamAttributes tAttrs = root.attributes();
        if (root.name() == KXMLQLCMonitorFont)
        {
            QFont fn;
            fn.fromString(root.readElementText());
            setFont(fn);
        }
        else if (root.name() == KXMLQLCMonitorChannels)
            setChannelStyle(ChannelStyle(root.readElementText().toInt()));
        else if (root.name() == KXMLQLCMonitorValues)
            setValueStyle(ValueStyle(root.readElementText().toInt()));
        else if (root.name() == KXMLQLCMonitorCommonBackground)
            setCommonBackgroundImage(mainDocument->denormalizeComponentPath(root.readElementText()));
        else if (root.name() == KXMLQLCMonitorCustomBgItem)
        {
            if (tAttrs.hasAttribute(KXMLQLCMonitorCustomBgFuncID))
            {
                quint32 fid = tAttrs.value(KXMLQLCMonitorCustomBgFuncID).toString().toUInt();
                setCustomBackgroundItem(fid, mainDocument->denormalizeComponentPath(root.readElementText()));
            }
        }
        else if (root.name() == KXMLQLCMonitorGrid)
        {
            int w = 5, h = 3, d = 5;
            if (tAttrs.hasAttribute(KXMLQLCMonitorGridWidth))
                w = tAttrs.value(KXMLQLCMonitorGridWidth).toString().toInt();
            if (tAttrs.hasAttribute(KXMLQLCMonitorGridHeight))
                h = tAttrs.value(KXMLQLCMonitorGridHeight).toString().toInt();
            if (tAttrs.hasAttribute(KXMLQLCMonitorGridDepth))
                d = tAttrs.value(KXMLQLCMonitorGridDepth).toString().toInt();
            else
                d = h; // backward compatibility

            if (tAttrs.hasAttribute(KXMLQLCMonitorGridUnits))
                setGridUnits(GridUnits(tAttrs.value(KXMLQLCMonitorGridUnits).toString().toInt()));

            if (tAttrs.hasAttribute(KXMLQLCMonitorPointOfView))
                setPointOfView(PointOfView(tAttrs.value(KXMLQLCMonitorPointOfView).toString().toInt()));
            else
                setPointOfView(Undefined);

            setGridSize(QVector3D(w, h, d));
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCMonitorFixtureItem)
        {
            if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureID))
            {
                quint32 fid = tAttrs.value(KXMLQLCMonitorFixtureID).toString().toUInt();
                QVector3D pos(0, 0, 0);
                QVector3D rot(0, 0, 0);
                if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureXPos))
                    pos.setX(tAttrs.value(KXMLQLCMonitorFixtureXPos).toString().toDouble());
                if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureYPos))
                    pos.setY(tAttrs.value(KXMLQLCMonitorFixtureYPos).toString().toDouble());
                if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureZPos))
                    pos.setZ(tAttrs.value(KXMLQLCMonitorFixtureZPos).toString().toDouble());
                setFixturePosition(fid, pos);

                if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureRotation)) // check legacy first
                {
                    rot.setY(tAttrs.value(KXMLQLCMonitorFixtureRotation).toString().toDouble());
                }
                else
                {
                    if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureXRotation))
                        rot.setX(tAttrs.value(KXMLQLCMonitorFixtureXRotation).toString().toDouble());
                    if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureYRotation))
                        rot.setY(tAttrs.value(KXMLQLCMonitorFixtureYRotation).toString().toDouble());
                    if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureZRotation))
                        rot.setZ(tAttrs.value(KXMLQLCMonitorFixtureZRotation).toString().toDouble());
                }
                setFixtureRotation(fid, rot);

                if (tAttrs.hasAttribute(KXMLQLCMonitorFixtureGelColor))
                    setFixtureGelColor(fid, QColor(tAttrs.value(KXMLQLCMonitorFixtureGelColor).toString()));
                root.skipCurrentElement();
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown MonitorProperties tag:" << root.name();
            root.skipCurrentElement();
        }
    }
    return true;
}

bool MonitorProperties::saveXML(QXmlStreamWriter *doc, const Doc *mainDocument) const
{
    Q_ASSERT(doc != NULL);

    /* Create the master Monitor node */
    doc->writeStartElement(KXMLQLCMonitorProperties);
    doc->writeAttribute(KXMLQLCMonitorDisplay, QString::number(displayMode()));
    doc->writeAttribute(KXMLQLCMonitorShowLabels, QString::number(labelsVisible()));

    /* Font */
    doc->writeTextElement(KXMLQLCMonitorFont, font().toString());
    /* Channels style */
    doc->writeTextElement(KXMLQLCMonitorChannels, QString::number(channelStyle()));
    /* Values style */
    doc->writeTextElement(KXMLQLCMonitorValues, QString::number(valueStyle()));

    /* Background */
    if (commonBackgroundImage().isEmpty() == false)
    {
        doc->writeTextElement(KXMLQLCMonitorCommonBackground,
                              mainDocument->normalizeComponentPath(commonBackgroundImage()));
    }
    else if(customBackgroundList().isEmpty() == false)
    {
        QHashIterator <quint32, QString> it(customBackgroundList());
        while (it.hasNext() == true)
        {
            it.next();
            doc->writeStartElement(KXMLQLCMonitorCustomBgItem);
            quint32 fid = it.key();
            doc->writeAttribute(KXMLQLCMonitorCustomBgFuncID, QString::number(fid));
            doc->writeCharacters(mainDocument->normalizeComponentPath(it.value()));
            doc->writeEndElement();
        }
    }

    doc->writeStartElement(KXMLQLCMonitorGrid);
    doc->writeAttribute(KXMLQLCMonitorGridWidth, QString::number(gridSize().x()));
    doc->writeAttribute(KXMLQLCMonitorGridHeight, QString::number(gridSize().y()));
    doc->writeAttribute(KXMLQLCMonitorGridDepth, QString::number(gridSize().z()));
    doc->writeAttribute(KXMLQLCMonitorGridUnits, QString::number(gridUnits()));
    if (m_pointOfView != Undefined)
        doc->writeAttribute(KXMLQLCMonitorPointOfView, QString::number(pointOfView()));

    doc->writeEndElement();

    foreach (quint32 fid, fixtureItemsID())
    {
        QVector3D pos = fixturePosition(fid);
        QVector3D rotation = fixtureRotation(fid);

        doc->writeStartElement(KXMLQLCMonitorFixtureItem);
        doc->writeAttribute(KXMLQLCMonitorFixtureID, QString::number(fid));
        doc->writeAttribute(KXMLQLCMonitorFixtureXPos, QString::number(pos.x()));
        doc->writeAttribute(KXMLQLCMonitorFixtureYPos, QString::number(pos.y()));
#ifdef QMLUI
        doc->writeAttribute(KXMLQLCMonitorFixtureZPos, QString::number(pos.z()));
        if (rotation.x() != 0)
            doc->writeAttribute(KXMLQLCMonitorFixtureXRotation, QString::number(rotation.x()));
        if (rotation.y() != 0)
            doc->writeAttribute(KXMLQLCMonitorFixtureYRotation, QString::number(rotation.y()));
        if (rotation.z() != 0)
            doc->writeAttribute(KXMLQLCMonitorFixtureZRotation, QString::number(rotation.z()));
#else
        if (fixtureRotation(fid) != QVector3D(0, 0, 0))
            doc->writeAttribute(KXMLQLCMonitorFixtureRotation, QString::number(rotation.y()));
#endif
        QColor col = fixtureGelColor(fid);
        if (col.isValid())
            doc->writeAttribute(KXMLQLCMonitorFixtureGelColor, col.name());
        doc->writeEndElement();
    }

    doc->writeEndElement();

    return true;
}
