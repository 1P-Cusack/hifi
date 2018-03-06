//
//  EntityTree.h
//  libraries/entities/src
//
//  Created by LaShonda Hopper 02/21/18.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

#include "AFrameReader.h"

#include <QList>

#include "EntityItemProperties.h"
#include "ShapeEntityItem.h"

AFrameReader::AFrameConversionTable AFrameReader::commonConversionTable = AFrameReader::AFrameConversionTable();
AFrameReader::TagList AFrameReader::supportAFrameTypes = AFrameReader::TagList();

void helper_parseVector(int numDimensions, const QStringList &dimList, float defaultVal, QList<float> &outList) {
    const int listSize = dimList.size();
    for (int index = 0; index < numDimensions; ++index) {
        if (listSize > index) {
            outList.push_back(dimList.at(index).toFloat());
        }
        else {
            outList.push_back(defaultVal);
        }
    }
}

void parseVec3(const QXmlStreamAttributes &attributes, const QString &attributeName, const QRegExp &splitExp, float defaultValue, QList<float> &valueList) {
    QStringList stringList = attributes.value(attributeName).toString().split(splitExp, QString::SkipEmptyParts);
    helper_parseVector(3, stringList, defaultValue, valueList);
}

void processPosition(const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    QList<float> values;
    parseVec3(elementAttributes, "position", QRegExp("\\s+"), 0.0f, values);
    properties.setPosition(glm::vec3(values.at(0), values.at(1), values.at(2)));
}

void processRotation(const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    QList<float> values;
    parseVec3(elementAttributes, "rotation", QRegExp("\\s+"), 0.0f, values);
    properties.setRotation(glm::vec3(values.at(0), values.at(1), values.at(2)));
}

void processRadius(const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (properties.dimensionsChanged()) {
        return;
    }

    QStringList stringList = elementAttributes.value("radius").toString().split(QRegExp("\\s+"), QString::SkipEmptyParts);
    float radius = 0.1f;
    if (stringList.size() > 0) {
        radius = stringList.at(0).toFloat();
    }

    const float sphericalDimension = radius * 2;
    properties.setDimensions(glm::vec3(sphericalDimension, sphericalDimension, sphericalDimension));
}

void processColor(const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (!elementAttributes.hasAttribute("color")) {
        return;
    }

    QString colorStr = elementAttributes.value("color").toString();
    const int hashIndex = colorStr.indexOf('#');
    if (hashIndex == 0) {
        colorStr = colorStr.mid(1);
    }
    else if (hashIndex > 0) {
        colorStr = colorStr.remove(hashIndex, 1);
    }
    const int hexValue = colorStr.toInt(Q_NULLPTR, 16);
    properties.setColor({ colorPart(hexValue >> 16),
        colorPart((hexValue & 0x00FF00) >> 8),
        colorPart(hexValue & 0x0000FF) });
}

void processDimensions(const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (properties.dimensionsChanged()) {
        return;
    }

    QRegExp splitExp("\\s+");
    const float defaultDim = 0.1f;
    float dimensionX = defaultDim;
    float dimensionY = defaultDim;
    float dimensionZ = defaultDim;

    // Note:  AFrame specifies the dimension components separately, so when we find one
    //        we're going to automatically test for the others so we have the information
    //        at the same time.
    QStringList stringList = elementAttributes.value("width").toString().split(splitExp, QString::SkipEmptyParts);
    if (stringList.size() > 0) {
        dimensionX = stringList.at(0).toFloat();
    }

    stringList = elementAttributes.value("height").toString().split(splitExp, QString::SkipEmptyParts);
    if (stringList.size() > 0) {
        dimensionY = stringList.at(0).toFloat();
    }

    stringList = elementAttributes.value("depth").toString().split(splitExp, QString::SkipEmptyParts);
    if (stringList.size() > 0) {
        dimensionZ = stringList.at(0).toFloat();
    }

    properties.setDimensions(glm::vec3(dimensionX, dimensionY, dimensionZ));
}

#define REGISTER_SUPPORTED_AFRAME_ELEMENT(aframeType) \
    QString normalizedTag = QString(aframeType).toLower(); \
    if (!supportAFrameTypes.contains(normalizedTag) { \
        supportAFrameTypes.insert(normalizedTag); \
    } else { \
        qWarning() << "AFrameReader Element " << normalizedTag << " already registered as supported."; \
    }

#define REGISTER_COMMON_ATTRIBUTE(name, handlerFunc) { \
    if (!commonConversionTable.contains("common_elements")) { \
        commonConversionTable.insert("common_elements", AFrameElementHandlerTable()); \
    } \
    \
    AFrameElementHandlerTable &elementHandlers = commonConversionTable["common_elements"]; \
    if (!elementHandlers.contains(name)) { \
        elementHandlers[name] = { name, AFrameProcessor::conversionHandler(&handlerFunc) }; \
    } else { \
        qWarning() << "AFrameReader already has handler registered for " << "common_elements" << " attribute: " << name; \
    } \
}

#define REGISTER_ELEMENT_ATTRIBUTE(elementName, attributeName, handlerFunc) { \
    if (!commonConversionTable.contains(elementName)) { \
        commonConversionTable.insert(elementName, AFrameElementHandlerTable()); \
    } \
    \
    AFrameElementHandlerTable &elementHandlers = commonConversionTable[elementName]; \
    if (!elementHandlers.contains(attributeName)) { \
        elementHandlers[attributeName] = { attributeName, AFrameProcessor::conversionHandler(&handlerFunc) }; \
    } else { \
        qWarning() << "AFrameReader already has a handler registered for " << elementName << " attribute: " << attributeName; \
    } \
}
void AFrameReader::registerAFrameConversionHandlers() {

    //REGISTER_SUPPORTED_AFRAME_ELEMENT("a-box");
    //REGISTER_SUPPORTED_AFRAME_ELEMENT("a-cylinder");
    //REGISTER_SUPPORTED_AFRAME_ELEMENT("a-plane");

    REGISTER_COMMON_ATTRIBUTE("position", processPosition);
    REGISTER_COMMON_ATTRIBUTE("rotation", processRotation);
    REGISTER_COMMON_ATTRIBUTE("color", processColor);
    REGISTER_COMMON_ATTRIBUTE("width", processDimensions);
    REGISTER_COMMON_ATTRIBUTE("height", processDimensions);
    REGISTER_COMMON_ATTRIBUTE("depth", processDimensions);
    REGISTER_ELEMENT_ATTRIBUTE("a-sphere", "radius", processRadius);
}

bool AFrameReader::read(const QByteArray &aframeData) {
    m_reader.addData(aframeData);
    
    QXmlStreamReader::TokenType currentTokenType = QXmlStreamReader::NoToken;
    while ( !m_reader.atEnd() ){
        currentTokenType = m_reader.readNext();
        if (currentTokenType == QXmlStreamReader::TokenType::Invalid) {
            break;
        }

        if (m_reader.isStartElement())
        {
            QStringRef elementName = m_reader.name();
            if (elementName == "a-scene") {
                return processScene();
            }
        }
    }

    if (m_reader.hasError()) {
        qWarning() << "AFrameReader::read encountered error: " << m_reader.errorString();
    }

    return false;
}

QString AFrameReader::getErrorString() const {
    return m_reader.errorString();
}

bool AFrameReader::processScene() {

    if (!m_reader.isStartElement() || m_reader.name() != "a-scene") {

        qDebug() << "AFrameReader::processScene expects element name a-scene, but element name was: " << m_reader.name();
        return false;
    }

    m_propData.clear();
    bool success = true;
    QXmlStreamReader::TokenType currentTokenType = QXmlStreamReader::NoToken;
    while (!m_reader.atEnd()) {

        currentTokenType = m_reader.readNext();
        if (currentTokenType == QXmlStreamReader::TokenType::Invalid) {
            success = false;
            break;
        }

        if (m_reader.isStartElement())
        {
            const QString &elementName = m_reader.name().toString();
            static unsigned int sUnsubEntityCount = 0;
            EntityItemProperties hifiProps;

            if (elementName == "a-box") {
                hifiProps.setType(EntityTypes::Box);
            } else if (elementName == "a-plane") {
                hifiProps.setType(EntityTypes::Shape);
                hifiProps.setShape(entity::stringFromShape(entity::Shape::Quad));
            } else if (elementName == "a-cylinder") {
                hifiProps.setType(EntityTypes::Shape);
                hifiProps.setShape(entity::stringFromShape(entity::Shape::Cylinder));
            } else if (elementName == "a-sphere") {
                hifiProps.setType(EntityTypes::Shape);
                hifiProps.setShape(entity::stringFromShape(entity::Shape::Sphere));
            }

            if (hifiProps.getType() != EntityTypes::Unknown) {
                // Get Attributes
                QXmlStreamAttributes attributes = m_reader.attributes();
                // For each attribute, process and record for entity properties.
                if (attributes.hasAttribute("id")) {
                    hifiProps.setName(attributes.value("id").toString());
                }
                else {
                    hifiProps.setName(elementName + "_" + QString::number(sUnsubEntityCount++));
                }

                const AFrameElementHandlerTable &elementHandlers = commonConversionTable["common_elements"];
                QStringList uncommonElements;
                for each (QXmlStreamAttribute currentAttribute in attributes)
                {
                    const QString attributeName = currentAttribute.name().toString();
                    if (!elementHandlers.contains(attributeName)) {

                        if (attributeName != "id") {
                            qDebug() << "AFrameReader - Warning: Missing handler for " << elementName << " attribute: " << attributeName;
                            uncommonElements.push_back(attributeName);
                        }

                        continue;
                    }

                    elementHandlers[attributeName].processFunc(attributes, hifiProps);
                }

                if (commonConversionTable.contains(elementName)) {
                    const AFrameElementHandlerTable &elementSpecificHandlers = commonConversionTable[elementName];
                    for each (QString elementAttribute in uncommonElements) {
                        if (!elementHandlers.contains(elementAttribute)) {

                            qDebug() << "AFrameReader - Error: Missing handler for " << elementName << " attribute: " << elementAttribute;

                            continue;
                        }

                        elementHandlers[elementAttribute].processFunc(attributes, hifiProps);
                    }
                }

                if (hifiProps.getClientOnly()) {
                    auto nodeList = DependencyManager::get<NodeList>();
                    const QUuid myNodeID = nodeList->getSessionUUID();
                    hifiProps.setOwningAvatarID(myNodeID);
                }

                // TODO_WL21698:  Remove Debug Dump
                qDebug() << "-------------------------------------------------";
                hifiProps.debugDump();
                qDebug() << "*************************************************";
                qDebug() << "*************************************************";
                qDebug() << hifiProps;
                qDebug() << "-------------------------------------------------";

                m_propData.push_back(hifiProps);
            }
        }
    }

    if (m_reader.hasError()) {
        qWarning() << "AFrameReader::read encountered error: " << m_reader.errorString();
        success = false;
    }

    return success;
}