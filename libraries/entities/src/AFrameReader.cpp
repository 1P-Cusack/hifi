//
//  EntityTree.h
//  libraries/entities/src
//
//  Created by LaShonda Hopper 2018/02/21.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

#include "AFrameReader.h"

#include <QList>

#include "EntityItemProperties.h"
#include "ShapeEntityItem.h"

//#define AFRAME_DEBUG_NOTES //< Turns on debug note prints for this file.

const char AFRAME_SCENE[] = "a-scene";
const char COMMON_ELEMENTS_KEY[] = "common_elements";
const QVariant INVALID_PROPERTY_DEFAULT = QVariant();
const float DEFAULT_POSITION_VALUE = 0.0f;
const float DEFAULT_ROTATION_VALUE = 0.0f;
const float DEFAULT_GENERAL_VALUE = 1.0f;

const std::array< QString, AFrameReader::AFRAMETYPE_COUNT > AFRAME_ELEMENT_NAMES { {
    "a-box",
    "a-circle",
    "a-cone",
    "a-cylinder",
    "a-image",
    "a-light",
    "a-gltf-model",
    "a-obj-model",
    "a-plane",
    "a-sky",
    "a-sphere",
    "a-tetrahedron",
    "a-text",
    "a-triangle"
    }
};

const std::array< QString, AFrameReader::AFRAMECOMPONENT_COUNT > AFRAME_COMPONENT_NAMES { {
    "color",
    "depth",
    "height",
    "position",
    "radius",
    "rotation",
    "src",
    "width"
    }
};

AFrameReader::ElementProcessors AFrameReader::elementProcessors = AFrameReader::ElementProcessors();
AFrameReader::ElementUnnamedCounts AFrameReader::elementUnnamedCounts = AFrameReader::ElementUnnamedCounts();


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

void processPosition(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    QList<float> values;
    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_POSITION_VALUE);
    parseVec3(elementAttributes, "position", QRegExp("\\s+"), defaultValue, values);
    properties.setPosition(glm::vec3(values.at(0), values.at(1), values.at(2)));
}

void processRotation(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    QList<float> values;
    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_ROTATION_VALUE);
    parseVec3(elementAttributes, "rotation", QRegExp("\\s+"), defaultValue, values);
    properties.setRotation(glm::vec3(values.at(0), values.at(1), values.at(2)));
}

float helper_parseRadius(const QXmlStreamAttributes &elementAttributes, const float defaultValue ) {
 
    QStringList stringList = elementAttributes.value("radius").toString().split(QRegExp("\\s+"), QString::SkipEmptyParts);
    if (stringList.size() > 0) {
        return stringList.at(0).toFloat();
    }

    return defaultValue;
}

void processSphereRadius(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (properties.dimensionsChanged()) {
        return;
    }

    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_GENERAL_VALUE);
    const float radius = helper_parseRadius(elementAttributes, defaultValue);
    const float sphericalDimension = radius * 2;

    properties.setDimensions(glm::vec3(sphericalDimension, sphericalDimension, sphericalDimension));
}

void processCylinderRadius(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    // A-Frame cylinders are Y-Major, so height is the Y and radius*2 is the full extent for x & z.
    // Note:  We don't care if the dimensions were already set as this is expected to override it given
    //        the element specific nature of the parsing which is handle after common attributes
    //        like dimension(height/width/depth)
    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_GENERAL_VALUE);
    const float radius = helper_parseRadius(elementAttributes, defaultValue);
    const float diameter = radius * 2;
    float dimensionY = DEFAULT_GENERAL_VALUE;

    QStringList stringList = elementAttributes.value("height").toString().split(QRegExp("\\s+"), QString::SkipEmptyParts);
    if (stringList.size() > 0) {
        dimensionY = stringList.at(0).toFloat();
    }

    properties.setDimensions(glm::vec3(diameter, dimensionY, diameter));
}

void processCircleRadius(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (properties.dimensionsChanged()) {
        return;
    }

    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_GENERAL_VALUE);
    const float radius = helper_parseRadius(elementAttributes, defaultValue);
    const float diameter = radius * 2;
    
    // Circles are essentially flat cylinders, thus they shouldn't have any height.
    properties.setDimensions(glm::vec3(diameter, 0.0f, diameter));
}

xColor helper_parseColor(const QXmlStreamAttributes &elementAttributes) {
    xColor color = { (colorPart)255, (colorPart)255, (colorPart)255 };
    QString colorStr = elementAttributes.value("color").toString();
    const int hashIndex = colorStr.indexOf('#');
    if (hashIndex == 0) {
        colorStr = colorStr.mid(1);
    }
    else if (hashIndex > 0) {
        colorStr = colorStr.remove(hashIndex, 1);
    }
    const int hexValue = colorStr.toInt(Q_NULLPTR, 16);
    color.red = (colorPart)(hexValue >> 16);
    color.green = (colorPart)((hexValue & 0x00FF00) >> 8);
    color.blue = (colorPart)(hexValue & 0x0000FF);

    return color;
}

void processColor(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (!elementAttributes.hasAttribute("color")) {
        return;
    }

    const xColor color = helper_parseColor(elementAttributes);
    properties.setColor(color);
}

void processSkyColor(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (!elementAttributes.hasAttribute("color")) {
        return;
    }

    const xColor color = helper_parseColor(elementAttributes);
    properties.getSkybox().setColor(color);
}

void processDimensions(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (properties.dimensionsChanged()) {
        return;
    }

    QRegExp splitExp("\\s+");
    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_GENERAL_VALUE);
    float dimensionX = defaultValue;
    float dimensionY = defaultValue;
    float dimensionZ = defaultValue;

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

#define CREATE_ELEMENT_PROCESSOR( elementType ) \
    AFrameElementProcessor * pElementProcessor = nullptr; \
    if (isElementTypeValid(elementType)) { \
        if (!elementProcessors.contains(elementType)) { \
            elementProcessors[elementType] = { elementType, ComponentProcessors() }; \
        } \
        pElementProcessor = &(elementProcessors[elementType]); \
    } else { \
        qWarning() << "AFrameReader detected attempt to create processor for invalid/unknown elementType: " << elementType; \
    }

#define ADD_COMPONENT_HANDLER_WITH_DEFAULT( componentType, handlerFunc, defaultValue ) \
    if (pElementProcessor) { \
        if (isComponentValid(componentType)) { \
            pElementProcessor->_componentProcessors[componentType] = { componentType, QVariant(defaultValue) \
                , AFrameComponentProcessor::ProcessFunc(&handlerFunc) }; \
        } else { \
            qWarning() << "AFrameReader Warning - attempted to create processor for invalid/unknown ComponentType: " << componentType; \
        } \
    } else { \
        qWarning() << "AFrameReader Warning - orphaned ADD_COMPONENT_HANDLER call for ComponentType " << componentType; \
    }

#define ADD_COMPONENT_HANDLER( componentType, handlerFunc ) \
    if (pElementProcessor) { \
        if (isComponentValid(componentType)) { \
            pElementProcessor->_componentProcessors[componentType] = { componentType, QVariant() \
                , AFrameComponentProcessor::ProcessFunc(&handlerFunc) }; \
        } else { \
            qWarning() << "AFrameReader Warning - attempted to create processor for invalid/unknown ComponentType: " << componentType; \
        } \
    } else { \
        qWarning() << "AFrameReader Warning - orphaned ADD_COMPONENT_HANDLER call for ComponentType " << componentType; \
    }

void AFrameReader::registerAFrameConversionHandlers() {

    { // a-box -> Shape::Box conversion setup
        CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_BOX)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_ROTATION, processRotation, DEFAULT_ROTATION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_WIDTH, processDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_HEIGHT, processDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_DEPTH, processDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_COLOR, processColor)
    }

    { // a-cylinder -> Shape::Cylinder converstion setup
        CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_CYLINDER)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_ROTATION, processRotation, DEFAULT_ROTATION_VALUE)
            // TODO_WL21698: Cylinder only cares about height, should have a singular handler as opposed to dimensions
            //  for cases like this....
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_HEIGHT, processDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_RADIUS, processCylinderRadius, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_COLOR, processColor)
    }

    { // a-plane -> Shape::Quad conversion setup
        CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_PLANE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_ROTATION, processRotation, DEFAULT_ROTATION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_WIDTH, processDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_HEIGHT, processDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_COLOR, processColor)
    }

    { // a-sphere -> Shape::Sphere conversion setup
        CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_SPHERE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_ROTATION, processRotation, DEFAULT_ROTATION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_RADIUS, processSphereRadius, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_COLOR, processColor)
    }

    //{ // a-sky -> Zone::SkyBox conversion setup
    //    CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_SKY)
    //        ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
    //        ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_ROTATION, processRotation, DEFAULT_ROTATION_VALUE)
    //        ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_RADIUS, processSphereRadius, 5000)
    //        ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_COLOR, processSkyColor);
    //}

    { // a-circle -> Shape::Circle conversion setup
        CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_CIRCLE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_ROTATION, processRotation, DEFAULT_ROTATION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_RADIUS, processCircleRadius, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_COLOR, processColor)
    }
}

QString AFrameReader::getElementNameForType(const AFrameType elementType) {
    if (!isElementTypeValid(elementType)) {
        return QString();
    }

    return AFRAME_ELEMENT_NAMES[(int)elementType];
}

AFrameReader::AFrameType AFrameReader::getTypeForElementName(const QString &elementName) {
    if (elementName.isEmpty()) {
        return AFRAMETYPE_COUNT;
    }

    const int numElementNames = AFRAME_ELEMENT_NAMES.size();
    for (int elementIndex =0; elementIndex < numElementNames; ++elementIndex) {

        if ( AFRAME_ELEMENT_NAMES[elementIndex] == elementName ) {
            return (AFrameType)elementIndex;
        }
    }

    return AFRAMETYPE_COUNT;
}

bool AFrameReader::isElementTypeValid(const AFrameType elementType) {
    return !((int)elementType < 0 || (int)elementType >= (int)AFRAMETYPE_COUNT);
}

QString AFrameReader::getNameForComponent(AFrameComponent componentType) {
    if (!isComponentValid(componentType)) {
        return QString();
    }

    return AFRAME_COMPONENT_NAMES[(int)componentType];
}

AFrameReader::AFrameComponent AFrameReader::getComponentForName(const QString &componentName) {
    if (componentName.isEmpty()) {
        return AFRAMECOMPONENT_COUNT;
    }

    const int numComponentNames = AFRAME_COMPONENT_NAMES.size();
    for (int componentIndex = 0; componentIndex < numComponentNames; ++componentIndex) {

        if (AFRAME_COMPONENT_NAMES[componentIndex] == componentName) {
            return (AFrameComponent)componentIndex;
        }
    }

    return AFRAMECOMPONENT_COUNT;
}

bool AFrameReader::isComponentValid(const AFrameComponent componentType) {
    return !((int)componentType < 0 || (int)componentType >= (int)AFRAMECOMPONENT_COUNT);
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
            EntityItemProperties hifiProps;
            const AFrameType elementType = getTypeForElementName(elementName);
            if (elementType == AFRAMETYPE_COUNT) {
                // Early Iteration Exit
                continue;
            } else if (!elementProcessors.contains(elementType)) {
                // Early Iteration Exit
                qWarning() << "AFrameReader::processScene - Error - No ElementProcessor for ElementType: " << elementName;
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
                case AFRAMETYPE_SKY: {
                    hifiProps.setType(EntityTypes::Zone);
                    hifiProps.setSkyboxMode(COMPONENT_MODE_ENABLED);
                    hifiProps.setShapeType(SHAPE_TYPE_SPHERE);
                    break;
                }
                case AFRAMETYPE_CIRCLE: {
                    hifiProps.setType(EntityTypes::Shape);
                    hifiProps.setShape(entity::stringFromShape(entity::Shape::Circle));
                    break;
                }
                default: {
                    // EARLY ITERATION EXIT -- Unknown/invalid type encountered.
                    qWarning() << "AFrameReader::processScene encountered unknown/invalid element: " << elementName;
                    continue;
                }
            }

            if (hifiProps.getType() != EntityTypes::Unknown) {

                const AFrameElementProcessor &elementProcessor = elementProcessors[elementType];
                const QXmlStreamAttributes attributes = m_reader.attributes();
                
                // For each attribute, process and record for entity properties.
                if (attributes.hasAttribute("id")) {
                    hifiProps.setName(attributes.value("id").toString());
                } else {
                    const int elementUnsubCount = elementUnnamedCounts[elementName]+1; //Unnamed count should be 1-based
                    hifiProps.setName(elementName + "_" + QString::number(elementUnsubCount));
                    elementUnnamedCounts[elementName] = elementUnsubCount;
                }

                for each ( const AFrameComponentProcessor &componentProcessor in elementProcessor._componentProcessors) {
                    componentProcessor.processFunc(componentProcessor, attributes, hifiProps);
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