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

#include <QJsonDocument>
#include <QList>

#include "EntityItemProperties.h"
#include "ModelEntityItem.h"
#include "ShapeEntityItem.h"
#include "TextEntityItem.h"

//#define AFRAME_DEBUG_NOTES //< Turns on debug note prints for this file.

#ifndef CALC_ELEMENTS_OF
#define CALC_ELEMENTS_OF(a)   (sizeof(a)/sizeof((a)[0]))
#endif

const char AFRAME_SCENE[] = "a-scene";
const char AFRAME_ASSETS[] = "a-assets";
const char AFRAME_ID[] = "id";
const char COMMON_ELEMENTS_KEY[] = "common_elements";
const char DIRECTIONAL_LIGHT_NAME[] = "directional";
const char SPOT_LIGHT_NAME[] = "spot";
const char POINT_LIGHT_NAME[] = "point";
const char AMBIENT_LIGHT_NAME[] = "ambient";
const char TEXT_SIDE_FRONT[] = "front";
const char TEXT_SIDE_BACK[] = "back";
const char TEXT_SIDE_DOUBLE[] = "double";
const char INLINE_URL_START[] = "url(";
const char PROTOCOL_NAME_HTTP[] = "http";
const char PROTOCOL_NAME_ATP[] = "atp";
const char SELECTOR_SYMBOL = '#';
const QVariant INVALID_PROPERTY_DEFAULT = QVariant();
const float DEFAULT_POSITION_VALUE = 0.0f;
const float DEFAULT_ROTATION_VALUE = 0.0f;
const float DEFAULT_GENERAL_VALUE = 1.0f;

const char * const IMAGE_EXTENSIONS[] = { // TODO:  Is this centralized somewhere?
    ".jpg",
    ".png"
};

const int NUM_IMAGE_EXTENSIONS = (int)CALC_ELEMENTS_OF(IMAGE_EXTENSIONS);

const char * const MODEL_EXTENSIONS[] = { // TODO:  Is this centralized somewhere?
    ".fbx",
    ".obj"
};
const int NUM_MODEL_EXTENSIONS = (int)CALC_ELEMENTS_OF(MODEL_EXTENSIONS);

const std::array< QString, AFrameReader::AFRAMETYPE_COUNT > AFRAME_ELEMENT_NAMES { {
    "a-box",
    "a-circle",
    "a-cone",
    "a-cylinder",
    "a-image",
    "a-light",
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
    "intensity",
    "lineHeight",
    "position",
    "radius",
    "radius-bottom",
    "rotation",
    "side",
    "src",
    "type",
    "value",
    "width"
    }
};

const std::array< QString, AFrameReader::ASSET_CONTROL_TYPE_COUNT > AFRAME_ASSET_CONTROL_NAMES { {
    "a-asset-image",
    "img"
    }
};

AFrameReader::ElementProcessors AFrameReader::elementProcessors = AFrameReader::ElementProcessors();
AFrameReader::ElementUnnamedCounts AFrameReader::elementUnnamedCounts = AFrameReader::ElementUnnamedCounts();
AFrameReader::SourceReferenceDictionary AFrameReader::entitySrcReferences = AFrameReader::SourceReferenceDictionary();

bool hasImageExtension( const QString &fileName ) {
    if ( fileName.isEmpty() ) {
        return false;
    }

    const QString &normalizedFileName = fileName.toLower();
    for (int extIndex = 0; extIndex < NUM_IMAGE_EXTENSIONS; ++extIndex) {
        if (normalizedFileName.endsWith(IMAGE_EXTENSIONS[extIndex])) {
            return true;
        }
    }

    return false;
}

bool hasModelExtension(const QString &fileName) {
    if (fileName.isEmpty()) {
        return false;
    }

    const QString &normalizedFileName = fileName.toLower();
    for (int extIndex = 0; extIndex < NUM_MODEL_EXTENSIONS; ++extIndex) {
        if (normalizedFileName.endsWith(MODEL_EXTENSIONS[extIndex])) {
            return true;
        }
    }

    return false;
}

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

float parseFloat(const QString &parseKey, const QXmlStreamAttributes &elementAttributes, const QRegExp &splitExp, const float defaultValue) {
    const QStringList stringList = elementAttributes.value(parseKey).toString().split(splitExp, QString::SkipEmptyParts);
    if (stringList.size() > 0) {
        return stringList.at(0).toFloat();
    }

    return defaultValue;
}

void processPosition(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    QList<float> values;
    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_POSITION_VALUE);
    parseVec3(elementAttributes, AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_POSITION), QRegExp("\\s+"), defaultValue, values);
    properties.setPosition(glm::vec3(values.at(0), values.at(1), values.at(2)));
}

void processRotation(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    QList<float> values;
    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_ROTATION_VALUE);
    parseVec3(elementAttributes, AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_ROTATION), QRegExp("\\s+"), defaultValue, values);
    properties.setRotation(glm::vec3(values.at(0), values.at(1), values.at(2)));
}

void processSphereDimensions(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (properties.dimensionsChanged()) {
        return;
    }

    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_GENERAL_VALUE);
    const float radius = parseFloat(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_RADIUS), elementAttributes, QRegExp("\\s+"), defaultValue);
    const float sphericalDimension = radius * 2;

    properties.setDimensions(glm::vec3(sphericalDimension, sphericalDimension, sphericalDimension));
}

void processCylinderDimensions(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (properties.dimensionsChanged()) {
        return;
    }

    // A-Frame cylinders are Y-Major, so height is the Y and radius*2 is the full extent for x & z.
    const QRegExp splitExp("\\s+");
    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_GENERAL_VALUE);

    const float radius = parseFloat(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_RADIUS), elementAttributes, splitExp, defaultValue);
    const float diameter = radius * 2;

    const float dimensionY = parseFloat(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_HEIGHT), elementAttributes, splitExp, DEFAULT_GENERAL_VALUE);

    properties.setDimensions(glm::vec3(diameter, dimensionY, diameter));
}

void processCircleDimensions(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (properties.dimensionsChanged()) {
        return;
    }

    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_GENERAL_VALUE);
    const float radius = parseFloat(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_RADIUS), elementAttributes, QRegExp("\\s+"), defaultValue);
    const float diameter = radius * 2;
    
    // Circles are essentially flat cylinders, thus they shouldn't have any height.
    properties.setDimensions(glm::vec3(diameter, 0.0f, diameter));
}

void processConeDimensions(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (properties.dimensionsChanged()) {
        return;
    }

    // A-Frame cones are Y-Major, so height is the Y and radius*2 is the full extent for x & z.
    const QRegExp splitExp("\\s+");
    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_GENERAL_VALUE);

    const float radius = parseFloat(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_RADIUS_BOTTOM), elementAttributes, splitExp, defaultValue);
    const float diameter = radius * 2;

    const float dimensionY = parseFloat(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_HEIGHT), elementAttributes, splitExp, DEFAULT_GENERAL_VALUE);

    properties.setDimensions(glm::vec3(diameter, dimensionY, diameter));
}

xColor helper_parseColor(const QXmlStreamAttributes &elementAttributes, const xColor &defaultColor = { (colorPart)255, (colorPart)255, (colorPart)255 }) {
    QString colorStr = elementAttributes.value(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_COLOR)).toString();
    if (colorStr.isEmpty()) {
        return defaultColor;
    }
    
    const int hashIndex = colorStr.indexOf('#');
    if (hashIndex == 0) {
        colorStr = colorStr.mid(1);
    }
    else if (hashIndex > 0) {
        colorStr = colorStr.remove(hashIndex, 1);
    }
    const int hexValue = colorStr.toInt(Q_NULLPTR, 16);
    xColor color;
    color.red = (colorPart)(hexValue >> 16);
    color.green = (colorPart)((hexValue & 0x00FF00) >> 8);
    color.blue = (colorPart)(hexValue & 0x0000FF);

    return color;
}

void processColor(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (!elementAttributes.hasAttribute(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_COLOR))) {
        return;
    }

    const xColor color = helper_parseColor(elementAttributes);
    properties.setColor(color);
}

void processSkyColor(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (!elementAttributes.hasAttribute(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_COLOR))) {
        return;
    }

    const xColor color = helper_parseColor(elementAttributes);
    properties.getSkybox().setColor(color);
}

void processTextColor(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    const xColor color = helper_parseColor(elementAttributes, TextEntityItem::DEFAULT_TEXT_COLOR);
    properties.setTextColor(color);
}

void processDimensions(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (properties.dimensionsChanged()) {
        return;
    }

    QRegExp splitExp("\\s+");
    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_GENERAL_VALUE);

    // Note:  AFrame specifies the dimension components separately, so when we find one
    //        we're going to automatically test for the others so we have the information
    //        at the same time.
    float dimensionX = parseFloat(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_WIDTH), elementAttributes, splitExp, defaultValue);
    float dimensionY = parseFloat(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_HEIGHT), elementAttributes, splitExp, defaultValue);
    float dimensionZ = parseFloat(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_DEPTH), elementAttributes, splitExp, defaultValue);

    properties.setDimensions(glm::vec3(dimensionX, dimensionY, dimensionZ));
}

float helper_parseIntensity(const QXmlStreamAttributes &elementAttributes, const QRegExp &splitExp, const float defaultValue) {
    return parseFloat(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_INTENSITY), elementAttributes, splitExp, defaultValue);
}

void processIntensity(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    QRegExp splitExp("\\s+");
    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_GENERAL_VALUE);
    const float intensity = helper_parseIntensity(elementAttributes, splitExp, defaultValue);

    properties.setIntensity(intensity);
}

void processLightType(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (!elementAttributes.hasAttribute(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_TYPE))) {
        return;
    }

    const QString strLightType = elementAttributes.value(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_TYPE)).toString();
    if ((strLightType == DIRECTIONAL_LIGHT_NAME) || (strLightType == POINT_LIGHT_NAME)) {
        // Directional lights aren't currently supported, so convert them to point lights.
        properties.setIsSpotlight(false);
    } else if (strLightType == SPOT_LIGHT_NAME) {
        properties.setIsSpotlight(true);
    } else if (strLightType == AMBIENT_LIGHT_NAME) {
        properties.setAmbientLightMode(COMPONENT_MODE_ENABLED);
        const float intensity = helper_parseIntensity(elementAttributes, QRegExp("\\s+"), DEFAULT_GENERAL_VALUE);
        properties.getAmbientLight().setAmbientIntensity(intensity);
    } else {
        // ...this is an unknown light type, so default to treating it as a point light.
        qWarning() << "AFrameReader::processLightType detected invalid/unknown LightType: " << strLightType;
        properties.setIsSpotlight(false);
    }
}

void processText(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    QString displayText = elementAttributes.value(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_VALUE)).toString();

    if (displayText.isEmpty()) {
        displayText = (component.componentDefault.isValid() ? component.componentDefault.toString() : TextEntityItem::DEFAULT_TEXT);
    }

    properties.setText(displayText);
    processDimensions({ component.componentType, component.elementType, QVariant(), nullptr }, elementAttributes, properties);
}

void processLineHeight(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    float lineHeight = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : TextEntityItem::DEFAULT_LINE_HEIGHT);

    if (elementAttributes.hasAttribute(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_LINE_HEIGHT))) {
        lineHeight = elementAttributes.value(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_LINE_HEIGHT)).toFloat();
    }

    properties.setLineHeight(lineHeight);
}

void processTextSide(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {
    if (!elementAttributes.hasAttribute(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_SIDE))) {
        return;
    }

    const QString strTextSide = elementAttributes.value(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_SIDE)).toString();
    if ((strTextSide == TEXT_SIDE_FRONT) || (strTextSide == TEXT_SIDE_DOUBLE)) {
        properties.setFaceCamera(true);
    } else if (strTextSide == TEXT_SIDE_BACK) {
        properties.setFaceCamera(false);
    } else {
        // ...this is an unknown light type, so defaulting.
        qWarning() << "AFrameReader::processTextSide detected invalid/unknown SideType: " << strTextSide;
        properties.setFaceCamera(TextEntityItem::DEFAULT_FACE_CAMERA);
    }
}

QString helper_getResourceURL(const QString &resourceName) {
    if (resourceName.isEmpty()) {
        return QString();
    }

    // Types of source specifications
    //      #name_ref
    //      url(file_path)
    //          url(assets/models/enemy0.json)
    //      url(net_path)
    //          url(https://blah.blah.png)
    //          url(atp:/blah.blah.jpg)
    const QString inlineURLStart(INLINE_URL_START);
    QString url;
    if (resourceName.startsWith(inlineURLStart, Qt::CaseInsensitive)) {
        url = resourceName.mid(inlineURLStart.size(), resourceName.size() - 1);
    } else {
        url = resourceName;
    }

    if (!url.startsWith(PROTOCOL_NAME_HTTP, Qt::CaseInsensitive) && !url.startsWith(PROTOCOL_NAME_ATP, Qt::CaseInsensitive)) {
        url = url.prepend(QString(PROTOCOL_NAME_ATP) + ":/");
    } else {
        url = resourceName;
    }

    return url;
}

bool helper_assignModelSourceUrl(const QString &sourceUrl, EntityItemProperties *entityPropData) {
    if (sourceUrl.isEmpty() || entityPropData == nullptr) {
        return false;
    }

    if (hasImageExtension(sourceUrl)) {
        // Note:  setTextures is based on Application::addAssetToWorldAddEntity's image case.
        QJsonObject textures{
            { "tex.picture", sourceUrl }
        };
        entityPropData->setModelURL(ModelEntityItem::DEFAULT_IMAGE_MODEL_URL);
        entityPropData->setTextures(QJsonDocument(textures).toJson(QJsonDocument::Compact));

        return true;
    }
    else if (hasModelExtension(sourceUrl)) {
        entityPropData->setModelURL(sourceUrl);

        return true;
    }

    return false;
}

void processSource(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties) {

    const QString sourceName = elementAttributes.value(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_SOURCE)).toString();
    if (sourceName.isEmpty()) {
        return;
    }

    QString propSource;
    if (sourceName.startsWith(SELECTOR_SYMBOL)) {
        propSource = sourceName.mid(1);
        AFrameReader::noteEntitySourceReference(propSource, properties);

        return;
    }

    propSource = helper_getResourceURL(sourceName);
    if ((component.elementType == AFrameReader::AFRAMETYPE_IMAGE) 
            || (component.elementType == AFrameReader::AFRAMETYPE_MODEL_OBJ)) {
        helper_assignModelSourceUrl(propSource, &properties);
    } else {
        properties.setSourceUrl(propSource);
    }
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
            pElementProcessor->_componentProcessors[componentType] = { componentType, pElementProcessor->_element, QVariant(defaultValue) \
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
            pElementProcessor->_componentProcessors[componentType] = { componentType, pElementProcessor->_element, QVariant() \
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
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_HEIGHT, processCylinderDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_RADIUS, processCylinderDimensions, DEFAULT_GENERAL_VALUE)
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
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_RADIUS, processSphereDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_COLOR, processColor)
    }

    //{ // a-sky -> Zone::SkyBox conversion setup
    //    CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_SKY)
    //        ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
    //        ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_ROTATION, processRotation, DEFAULT_ROTATION_VALUE)
    //        ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_RADIUS, processSphereDimensions, 5000)
    //        ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_COLOR, processSkyColor);
    //}

    { // a-circle -> Shape::Circle conversion setup
        CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_CIRCLE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_ROTATION, processRotation, DEFAULT_ROTATION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_RADIUS, processCircleDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_COLOR, processColor)
    }

    { // a-cone -> Shape::Cone conversion setup
        CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_CONE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_ROTATION, processRotation, DEFAULT_ROTATION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_HEIGHT, processConeDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_RADIUS, processConeDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_COLOR, processColor)
    }

    { // a-tetrahedron -> Shape::Tetrahedron conversion setup
        CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_TETRAHEDRON)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_ROTATION, processRotation, DEFAULT_ROTATION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_RADIUS, processSphereDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_COLOR, processColor)
    }

    { // a-light -> LightEntityItem conversion setup
        CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_LIGHT)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_ROTATION, processRotation, DEFAULT_ROTATION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_INTENSITY, processIntensity, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_TYPE, processLightType)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_COLOR, processColor)
    }

    { // a-text -> TextEntityItem conversion setup
        CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_TEXT)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_VALUE, processText, TextEntityItem::DEFAULT_TEXT)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_LINE_HEIGHT, processLineHeight, TextEntityItem::DEFAULT_LINE_HEIGHT)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_SIDE, processTextSide)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_COLOR, processTextColor)
    }

    { // a-image -> ModelEntityItem::Image conversion setup
        CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_IMAGE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_ROTATION, processRotation, DEFAULT_ROTATION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_WIDTH, processDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_HEIGHT, processDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_SOURCE, processSource)
    }

    { // a-obj-model -> ModelEntityItem conversion setup
        CREATE_ELEMENT_PROCESSOR(AFRAMETYPE_MODEL_OBJ)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_POSITION, processPosition, DEFAULT_POSITION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_ROTATION, processRotation, DEFAULT_ROTATION_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_WIDTH, processDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_HEIGHT, processDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER_WITH_DEFAULT(AFRAMECOMPONENT_DEPTH, processDimensions, DEFAULT_GENERAL_VALUE)
            ADD_COMPONENT_HANDLER(AFRAMECOMPONENT_SOURCE, processSource)
    }
}

void AFrameReader::noteEntitySourceReference(const QString &srcReference, EntityItemProperties &entityPropData) {
    if (srcReference.isEmpty()) {
        qWarning() << "AFrameReader::noteEntitySourceReference - Invalid data for key: " << entityPropData.getName();
        return;
    }

    const QString &entityName = entityPropData.getName();
    if (entityName.isEmpty()) {
        qWarning() << "AFrameReader::noteEntitySourceReference - Invalid key for srcReference: " << srcReference;
        return;
    }

    if (entitySrcReferences.contains(entityName)) {
        qWarning() << "AFrameReader::noteEntitySourceReference - Registry Keys should be unique.  Multiple registry attempts for " << entityName;
        return;
    }

    entitySrcReferences.insert(entityName, { srcReference, &entityPropData });
}

void AFrameReader::processEntitySourceReferences(const AFrameReader::StringDictionary &srcDictionary) {
    if (srcDictionary.isEmpty()) {
        qWarning() << "AFrameReader::processEntitySourceReferences - Received empty source dictionary!";
        return;
    }

    qDebug() << "AFrameReader::processEntitySourceReferences ENTERED... ";

    SourceReferenceDictionary::iterator iterEntitySrcRef = entitySrcReferences.begin();
    const SourceReferenceDictionary::const_iterator iterEntitySrcRefEnd = entitySrcReferences.end();
    for (; iterEntitySrcRef != iterEntitySrcRefEnd; ++iterEntitySrcRef) {

        SourceReference &sourceRef = iterEntitySrcRef.value();
        if (!srcDictionary.contains(sourceRef._srcReference)) {

            qDebug() << "Processing skipped EntityProp - " << sourceRef._entityPropData->getName() << 
                ", couldn't find source: " << sourceRef._srcReference;

            // Early Iteration Exit -- Source wasn't found within the look up table
            continue;
        }

        EntityItemProperties * const entityPropData = sourceRef._entityPropData;
        if (entityPropData->getType() != EntityTypes::Model) {
            qWarning() << "Processing skipped EntityProp - " << entityPropData->getName() <<
                "; there's no source ref support for type: " << EntityTypes::getEntityTypeName(entityPropData->getType());

            // Early Iteration Exit -- Currently don't support src references for non-Models
            continue;
        }

        const QString &sourceUrl = helper_getResourceURL(srcDictionary.value(sourceRef._srcReference));
        qDebug() << "Processing EntityProp - " << sourceRef._entityPropData->getName() << " -> Source: " << sourceUrl;
        
        const bool processedSource = helper_assignModelSourceUrl(sourceUrl, entityPropData);
        if (!processedSource) {
            qWarning() << "Processing terminated for EntityProp - " << entityPropData->getName() <<
                "; it has an invalid/unsupported source: " << sourceUrl;

            // Early Iteration Exit -- Currently don't support src type
            continue;
        }

        // TODO_WL21698:  Remove Debug Dump
        qDebug() << "---------------";
        entityPropData->debugDump();
        qDebug() << "***************";
    }

    qDebug() << "AFrameReader::processEntitySourceReferences EXITED... ";

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

    const int numElementNames = (int)AFRAME_ELEMENT_NAMES.size();
    for (int elementIndex =0; elementIndex < numElementNames; ++elementIndex) {

        if ( AFRAME_ELEMENT_NAMES[elementIndex] == elementName ) {
            return (AFrameType)elementIndex;
        }
    }

    return AFRAMETYPE_COUNT;
}

bool AFrameReader::isElementTypeValid(const AFrameType elementType) {
    return ((int)elementType >= 0) && ((int)elementType < (int)AFRAMETYPE_COUNT);
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

    const int numComponentNames = (int)AFRAME_COMPONENT_NAMES.size();
    for (int componentIndex = 0; componentIndex < numComponentNames; ++componentIndex) {

        if (AFRAME_COMPONENT_NAMES[componentIndex] == componentName) {
            return (AFrameComponent)componentIndex;
        }
    }

    return AFRAMECOMPONENT_COUNT;
}

bool AFrameReader::isComponentValid(const AFrameComponent componentType) {
    return ((int)componentType >= 0) && ((int)componentType < (int)AFRAMECOMPONENT_COUNT);
}

QString AFrameReader::getNameForAssetElement(const AssetControlType elementType) {
    if (!isAssetElementTypeValid(elementType)) {
        return QString();
    }

    return AFRAME_ASSET_CONTROL_NAMES[(int)elementType];
}

AFrameReader::AssetControlType AFrameReader::getTypeForAssetElementName(const QString &elementName) {
    if (elementName.isEmpty()) {
        return ASSET_CONTROL_TYPE_COUNT;
    }

    const int numAssetElementTypes = (int)AFRAME_ASSET_CONTROL_NAMES.size();
    for (int assetTypeIndex = 0; assetTypeIndex < numAssetElementTypes; ++assetTypeIndex) {

        if (AFRAME_ASSET_CONTROL_NAMES[assetTypeIndex] == elementName) {
            return (AssetControlType)assetTypeIndex;
        }
    }

    return ASSET_CONTROL_TYPE_COUNT;
}

bool AFrameReader::isAssetElementTypeValid(const AssetControlType elementType) {
    return ((int)elementType >= 0) && ((int)elementType < (int)ASSET_CONTROL_TYPE_COUNT);
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

    if (!m_reader.isStartElement() || m_reader.name() != AFRAME_SCENE) {

        qDebug() << "AFrameReader::processScene expects element name " << AFRAME_SCENE << ", but element name was: " << m_reader.name();
        return false;
    }

    m_propData.clear();
    m_srcDictionary.clear();
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
            // Assets section is expected to be first section under
            // the scene; however, checking status here and allowing
            // for continued non-asset parsing should cover both cases
            //      1) Assets grouped at top
            //      2) Assets somewhere in between or weirdly divvied (why?)
            if (m_reader.name().toString() == AFRAME_ASSETS) {
                processAssets();

                if (!m_reader.isStartElement()) {
                    continue;
                }
            }

            const QString &elementName = m_reader.name().toString();
            const AFrameType elementType = getTypeForElementName(elementName);
            if (elementType == AFRAMETYPE_COUNT) {
                // Early Iteration Exit
                continue;
            } else if (!elementProcessors.contains(elementType)) {
                // Early Iteration Exit
                qWarning() << "AFrameReader::processScene - Error - No ElementProcessor for ElementType: " << elementName;
                continue;
            }

            m_propData.push_back(EntityItemProperties());
            EntityItemProperties &hifiProps = m_propData.back();

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
                case AFRAMETYPE_CONE: {
                    hifiProps.setType(EntityTypes::Shape);
                    hifiProps.setShape(entity::stringFromShape(entity::Shape::Cone));
                    break;
                }
                case AFRAMETYPE_TETRAHEDRON: {
                    hifiProps.setType(EntityTypes::Shape);
                    hifiProps.setShape(entity::stringFromShape(entity::Shape::Tetrahedron));
                    break;
                }
                case AFRAMETYPE_LIGHT: {
                    hifiProps.setType(EntityTypes::Light);
                    break;
                }
                case AFRAMETYPE_TEXT: {
                    hifiProps.setType(EntityTypes::Text);
                    break;
                }
                case AFRAMETYPE_IMAGE: {
                    hifiProps.setType(EntityTypes::Model);
                    hifiProps.setShapeType(SHAPE_TYPE_BOX);
                    hifiProps.setCollisionless(true);
                    hifiProps.setDynamic(false);
                    break;
                }
                case AFRAMETYPE_MODEL_OBJ: {
                    hifiProps.setType(EntityTypes::Model);
                    hifiProps.setShapeType(SHAPE_TYPE_SIMPLE_COMPOUND);
                    hifiProps.setCollisionless(true);   // <- In the event that the import lands on top of the user's avatar.
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
                if (attributes.hasAttribute(AFRAME_ID)) {
                    hifiProps.setName(attributes.value(AFRAME_ID).toString());
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

            } // End_if( getType != EntityTypes::Unknown )
        } // End_if( startElement )
    } // End_while( !atEnd )

    processEntitySourceReferences(m_srcDictionary);
    clearEntitySourceReferences();

    if (m_reader.hasError()) {
        qWarning() << "AFrameReader::read encountered error: " << m_reader.errorString();
        success = false;
    }

    return success;
}

bool AFrameReader::processAssets() {
    if (!m_reader.isStartElement() || m_reader.name() != AFRAME_ASSETS) {

        qDebug() << "AFrameReader::processAssets expects element name " << AFRAME_ASSETS << ", but element name was: " << m_reader.name();
        return false;
    }

    qDebug() << "AFrameReader::processAssets ENTERED... ";

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
            const AssetControlType elementType = getTypeForAssetElementName(elementName);
            if (elementType == ASSET_CONTROL_TYPE_COUNT) {
                const AFrameType elementType = getTypeForElementName(elementName);
                if (elementType != AFRAMETYPE_COUNT) {
                    qDebug() << "AFrameReader::processAssets EXITING due to - " << elementName;

                    // Early Loop Exit -- encountered primitive/non-asset type
                    break;
                }

                // Early Iteration Exit -- encountered unknown/unsupported asset management element
                qWarning() << "AFrameReader::processAssets detected unknown/unsupported assetElement: " << elementName;
                continue;
            }

            qDebug() << "AFrameReader::processAssets detected - " << elementName;
            const QXmlStreamAttributes attributes = m_reader.attributes();
            const QString &assetSrc = attributes.value(getNameForComponent(AFRAMECOMPONENT_SOURCE)).toString();
            const QString &assetId = attributes.value(AFRAME_ID).toString();
            if (assetId.isEmpty()) {
                // Early Iteration Exit -- All assets required to have an id specified.
                qWarning() << "AFrameReader::processAssets detected missing id component for asset " << assetSrc << "!";
                continue;
            }

            if (assetSrc.isEmpty()) {
                // Early Iteration Exit -- All assets are required to have an src specified.
                qDebug() << "AFrameReader::processAssets detected asset " << assetId << " without required src component!";
                continue;
            }

            m_srcDictionary.insert(assetId, assetSrc);

            // TODO_WL21698:  Remove Debug Dump
            qDebug() << "----------";
            qDebug() << "AFrameReader::processAssets adding pair: " << assetId << " - " << assetSrc;
            qDebug() << "**********";
        } // End_if( isStartElement )
    } // End_while( !atEnd )

    if (m_reader.hasError()) {
        qWarning() << "AFrameReader::read encountered error: " << m_reader.errorString();
        success = false;
    }

    qDebug() << "AFrameReader::processAssets EXITED... ";
    return success;
}