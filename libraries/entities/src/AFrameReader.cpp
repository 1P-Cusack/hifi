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

const char AFRAME_SCENE[] = "a-scene";
const char COMMON_ELEMENTS_KEY[] = "common_elements";

AFrameReader::AFrameConversionTable AFrameReader::commonConversionTable = AFrameReader::AFrameConversionTable();
AFrameReader::TagList AFrameReader::supportedAFrameElements = AFrameReader::TagList();

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

void processPosition(const AFrameReader::AFrameType, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    QList<float> values;
    parseVec3(elementAttributes, "position", QRegExp("\\s+"), 0.0f, values);
    properties.setPosition(glm::vec3(values.at(0), values.at(1), values.at(2)));
}

void processRotation(const AFrameReader::AFrameType, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    QList<float> values;
    parseVec3(elementAttributes, "rotation", QRegExp("\\s+"), 0.0f, values);
    properties.setRotation(glm::vec3(values.at(0), values.at(1), values.at(2)));
}

float helper_parseRadius(const QXmlStreamAttributes &elementAttributes, const float defaultValue ) {
 
    QStringList stringList = elementAttributes.value("radius").toString().split(QRegExp("\\s+"), QString::SkipEmptyParts);
    if (stringList.size() > 0) {
        return stringList.at(0).toFloat();
    }

    return defaultValue;
}

void processSphereRadius(const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (properties.dimensionsChanged()) {
        return;
    }

    const float radius = helper_parseRadius(elementAttributes, 0.1f);
    const float sphericalDimension = radius * 2;

    properties.setDimensions(glm::vec3(sphericalDimension, sphericalDimension, sphericalDimension));
}

void processCylinderRadius(const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    // A-Frame cylinders are Y-Major, so height is the Y and radius*2 is the full extent for x & z.
    // Note:  We don't care if the dimensions were already set as this is expected to override it given
    //        the element specific nature of the parsing which is handle after common attributes
    //        like dimension(height/width/depth)
    const float radius = helper_parseRadius(elementAttributes, 0.1f);
    const float diameter = radius * 2;
    float dimensionY = 0.1f;

    QStringList stringList = elementAttributes.value("height").toString().split(QRegExp("\\s+"), QString::SkipEmptyParts);
    if (stringList.size() > 0) {
        dimensionY = stringList.at(0).toFloat();
    }

    properties.setDimensions(glm::vec3(diameter, dimensionY, diameter));
}

void processRadius(const AFrameReader::AFrameType elementType, 
    const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    switch (elementType) {
        case AFrameReader::AFRAMETYPE_CYLINDER: {
            processCylinderRadius(elementAttributes, properties);
            return;
        }
        case AFrameReader::AFRAMETYPE_SPHERE: {
            processSphereRadius(elementAttributes, properties);
            return;
        }
        default: {
            qWarning() << "AFrameReader triggered processRadius for unknown/invalid elementType: " << elementType;
            break;
        }
    }
}

void processColor(const AFrameReader::AFrameType, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
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

void processDimensions(const AFrameReader::AFrameType, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
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

#define REGISTER_COMMON_ATTRIBUTE(name, handlerFunc) { \
    if (!commonConversionTable.contains(COMMON_ELEMENTS_KEY)) { \
        commonConversionTable.insert(COMMON_ELEMENTS_KEY, AFrameElementHandlerTable()); \
    } \
    \
    AFrameElementHandlerTable &elementHandlers = commonConversionTable[COMMON_ELEMENTS_KEY]; \
    if (!elementHandlers.contains(name)) { \
        elementHandlers[name] = { name, AFrameProcessor::conversionHandler(&handlerFunc) }; \
    } else { \
        qWarning() << "AFrameReader already has handler registered for " << COMMON_ELEMENTS_KEY << " attribute: " << name; \
    } \
}

#define REGISTER_ELEMENT_ATTRIBUTE(elementType, attributeName, handlerFunc) { \
    const QString &elementName = getElementNameForType(elementType); \
    if (elementName != "") { \
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
    } else { \
        qWarning() << "AFrameReader detected registry of invalid/unknown elementType: " << elementType; \
    } \
}
void AFrameReader::registerAFrameConversionHandlers() {

    supportedAFrameElements.reserve(AFRAMETYPE_COUNT);
    supportedAFrameElements.push_back("a-box");
    supportedAFrameElements.push_back("a-circle");
    supportedAFrameElements.push_back("a-cone");
    supportedAFrameElements.push_back("a-cylinder");
    supportedAFrameElements.push_back("a-image");
    supportedAFrameElements.push_back("a-light");
    supportedAFrameElements.push_back("a-gltf-model");
    supportedAFrameElements.push_back("a-obj-model");
    supportedAFrameElements.push_back("a-plane");
    supportedAFrameElements.push_back("a-sky");
    supportedAFrameElements.push_back("a-sphere");
    supportedAFrameElements.push_back("a-tetrahedron");
    supportedAFrameElements.push_back("a-text");
    supportedAFrameElements.push_back("a-triangle");

    REGISTER_COMMON_ATTRIBUTE("position", processPosition);
    REGISTER_COMMON_ATTRIBUTE("rotation", processRotation);
    REGISTER_COMMON_ATTRIBUTE("color", processColor);
    REGISTER_COMMON_ATTRIBUTE("width", processDimensions);
    REGISTER_COMMON_ATTRIBUTE("height", processDimensions);
    REGISTER_COMMON_ATTRIBUTE("depth", processDimensions);
    REGISTER_ELEMENT_ATTRIBUTE(AFRAMETYPE_SPHERE, "radius", processRadius);
    REGISTER_ELEMENT_ATTRIBUTE(AFRAMETYPE_CYLINDER, "radius", processRadius);
}

QString AFrameReader::getElementNameForType(const AFrameType elementType) {
    if ((int)elementType < 0 || (int)elementType >= (int)AFRAMETYPE_COUNT) {
        return "";
    }

    return supportedAFrameElements[(int)elementType];
}

AFrameReader::AFrameType AFrameReader::getTypeForElementName(const QString &elementName) {
    if (elementName.isEmpty()) {
        return AFRAMETYPE_COUNT;
    }

    const int elementIndex = supportedAFrameElements.indexOf(elementName);
    if (elementIndex != -1) {
        return (AFrameType)elementIndex;
    }

    return AFRAMETYPE_COUNT;
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
            if (elementName == AFRAME_SCENE) {
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
            const AFrameType elementType = getTypeForElementName(elementName);
            if (elementType == AFRAMETYPE_COUNT) {
                // Early Iteration Exit
                continue;
            }

            switch (elementType) {
                case AFRAMETYPE_BOX: {
                    hifiProps.setType(EntityTypes::Box);
                    break;
                }
                case AFRAMETYPE_PLANE: {
                    hifiProps.setType(EntityTypes::Shape);
                    hifiProps.setShape(entity::stringFromShape(entity::Shape::Quad));
                    break;
                }
                case AFRAMETYPE_CYLINDER: {
                    hifiProps.setType(EntityTypes::Shape);
                    hifiProps.setShape(entity::stringFromShape(entity::Shape::Cylinder));
                    break;
                }
                case AFRAMETYPE_SPHERE: {
                    hifiProps.setType(EntityTypes::Shape);
                    hifiProps.setShape(entity::stringFromShape(entity::Shape::Sphere));
                    break;
                }
                default: {
                    // EARLY ITERATION EXIT -- Unknown/invalid type encountered.
                    qWarning() << "AFrameReader::processScene encountered unknown/invalid element: " << elementName;
                    continue;
                }
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

                const AFrameElementHandlerTable &elementHandlers = commonConversionTable[COMMON_ELEMENTS_KEY];
                QStringList uncommonElements;
                for each (QXmlStreamAttribute currentAttribute in attributes) {
                    const QString attributeName = currentAttribute.name().toString();
                    if (!elementHandlers.contains(attributeName)) {

                        if (attributeName != "id") {
                            uncommonElements.push_back(attributeName);
                        }

                        continue;
                    }

                    elementHandlers[attributeName].processFunc(elementType, attributes, hifiProps);
                }

                if (commonConversionTable.contains(elementName)) {
                    const AFrameElementHandlerTable &elementSpecificHandlers = commonConversionTable[elementName];
                    for each (QString elementAttribute in uncommonElements) {
                        if (!elementSpecificHandlers.contains(elementAttribute)) {

                            qDebug() << "AFrameReader - Error: Missing handler for " << elementName << " attribute: " << elementAttribute;

                            continue;
                        }

                        elementSpecificHandlers[elementAttribute].processFunc(elementType, attributes, hifiProps);
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