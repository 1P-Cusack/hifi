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

#define AFRAME_DEBUG_NOTES 0 //< Turns on debug note prints for this file.
#define AFRAME_DEBUG_STACK 1

#ifndef CALC_ELEMENTS_OF
#define CALC_ELEMENTS_OF(a)   (sizeof(a)/sizeof((a)[0]))
#endif

const char AFRAME_SCENE[] = "a-scene";
const char AFRAME_ASSETS[] = "a-assets";
const char AFRAME_ENTITY[] = "a-entity";
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

const int DEFAULT_PROPERTY_PAIR_RESERVE = 10;
const int DEFAULT_MIXIN_ATTRIBUTE_RESERVE = 25;

const char * const IMAGE_EXTENSIONS[] = { // TODO_WL21698:  Is this centralized somewhere?
    ".jpg",
    ".png"
};
const int NUM_IMAGE_EXTENSIONS = (int)CALC_ELEMENTS_OF(IMAGE_EXTENSIONS);

const char * const MODEL_EXTENSIONS[] = { // TODO_WL21698:  Is this centralized somewhere?
    ".fbx",
    ".obj"
};
const int NUM_MODEL_EXTENSIONS = (int)CALC_ELEMENTS_OF(MODEL_EXTENSIONS);

const char * const MATERIAL_EXTENSIONS[] = { // TODO_WL21698:  See a-obj-model TODO. What extensions are supported?
    ".mtl"
};
const int NUM_MATERIAL_EXTENSIONS = (int)CALC_ELEMENTS_OF(MATERIAL_EXTENSIONS);

const entity::Shape SUPPORTED_SHAPES[] = {
    entity::Circle,
    entity::Cone,
    entity::Cube,
    entity::Cylinder,
    entity::Quad,
    entity::Sphere,
    entity::Tetrahedron,
    entity::Triangle
};
const int NUM_SUPPORTED_SHAPES = CALC_ELEMENTS_OF(SUPPORTED_SHAPES);

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
    "id",
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
    "a-asset-item",
    "a-asset-image",
    "img",
    "a-mixin"
    }
};

const std::array<const AFrameReader::EntityComponentPair, AFrameReader::ENTITY_COMPONENT_COUNT> ENTITY_COMPONENTS = { {
        AFrameReader::EntityComponentPair{ AFrameReader::ENTITY_COMPONENT_GEOMETRY, "geometry" },
        AFrameReader::EntityComponentPair{ AFrameReader::ENTITY_COMPONENT_ID, "id" },
        AFrameReader::EntityComponentPair{ AFrameReader::ENTITY_COMPONENT_IMAGE, "image" },
        AFrameReader::EntityComponentPair{ AFrameReader::ENTITY_COMPONENT_LIGHT, "light" },
        AFrameReader::EntityComponentPair{ AFrameReader::ENTITY_COMPONENT_MATERIAL, "material" },
        AFrameReader::EntityComponentPair{ AFrameReader::ENTITY_COMPONENT_POSITION, "position" },
        AFrameReader::EntityComponentPair{ AFrameReader::ENTITY_COMPONENT_MIXIN, "mixin" },
        AFrameReader::EntityComponentPair{ AFrameReader::ENTITY_COMPONENT_MODEL_OBJ, "obj-model" },
        AFrameReader::EntityComponentPair{ AFrameReader::ENTITY_COMPONENT_ROTATION, "rotation" },
        AFrameReader::EntityComponentPair{ AFrameReader::ENTITY_COMPONENT_TEXT, "text" }
    }
};

const std::array<const AFrameReader::BaseEntityComponentPair, AFrameReader::BASE_ENTITY_COMPONENT_COUNT> BASE_ENTITY_COMPONENTS = { {
        AFrameReader::BaseEntityComponentPair{ AFrameReader::BASE_ENTITY_COMPONENT_MIXIN, "mixin" },
        AFrameReader::BaseEntityComponentPair{ AFrameReader::BASE_ENTITY_COMPONENT_POSITION, "position" },
        AFrameReader::BaseEntityComponentPair{ AFrameReader::BASE_ENTITY_COMPONENT_ROTATION, "rotation" },
        }
};

AFrameReader::ElementProcessors AFrameReader::elementProcessors = AFrameReader::ElementProcessors();
AFrameReader::ElementUnnamedCounts AFrameReader::elementUnnamedCounts = AFrameReader::ElementUnnamedCounts();
AFrameReader::SourceReferenceDictionary AFrameReader::entitySrcReferences = AFrameReader::SourceReferenceDictionary();

AFrameReader::AFrameType getElementTypeForShape(const entity::Shape shapeType) {
    switch (shapeType) {
        case entity::Circle:
            return AFrameReader::AFRAMETYPE_CIRCLE;
        case entity::Cone:
            return AFrameReader::AFRAMETYPE_CONE;
        case entity::Cube:
            return AFrameReader::AFRAMETYPE_BOX;
        case entity::Cylinder:
            return AFrameReader::AFRAMETYPE_CYLINDER;
        case entity::Quad:
            return AFrameReader::AFRAMETYPE_PLANE;
        case entity::Sphere:
            return AFrameReader::AFRAMETYPE_SPHERE;
        case entity::Tetrahedron:
            return AFrameReader::AFRAMETYPE_TETRAHEDRON;
        case entity::Triangle:
            return AFrameReader::AFRAMETYPE_TRIANGLE;
        default: {
            qWarning() << "AFrameReader::getElementTypeForShape - Encountered invalid/unknown shape: " << shapeType;
            return AFrameReader::AFRAMETYPE_COUNT;
        }
    }
}

bool isShapeSupported(const entity::Shape shape) {
    for (int shapeIndex = 0; shapeIndex < NUM_SUPPORTED_SHAPES; ++shapeIndex) {
        if (shape == SUPPORTED_SHAPES[shapeIndex]) {
            return true;
        }
    }

    return false;
}

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

void processPosition(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, 
    EntityItemProperties &properties, const AFrameReader::ParseNode * const parentNode) {
    QList<float> values;
    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_POSITION_VALUE);
    const QString &posComponentName = AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_POSITION);
    parseVec3(elementAttributes, posComponentName, QRegExp("\\s+"), defaultValue, values);
    
    glm::vec3 entityPos(values.at(0), values.at(1), values.at(2));
    if ((parentNode != nullptr) &&
        (parentNode->type == AFrameReader::ParseNode::NODE_TYPE_REFERENCE) &&
        !elementAttributes.hasAttribute(posComponentName)) {
            QList<float> parentValues;
            parseVec3(parentNode->attributes, posComponentName
                , QRegExp("\\s+"), DEFAULT_POSITION_VALUE, parentValues);
            glm::vec3 parentPos(parentValues.at(0), parentValues.at(1), parentValues.at(2));
            entityPos = parentPos;
    }
    
    properties.setPosition(entityPos);
}

void processRotation(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, 
    EntityItemProperties &properties, const AFrameReader::ParseNode * const parentNode) {
    QList<float> values;
    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_ROTATION_VALUE);
    const QString &rotComponentName = AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_ROTATION);
    parseVec3(elementAttributes, rotComponentName, QRegExp("\\s+"), defaultValue, values);

    glm::vec3 entityRot(values.at(0), values.at(1), values.at(2));
    if ((parentNode != nullptr) &&
        (parentNode->type == AFrameReader::ParseNode::NODE_TYPE_REFERENCE) &&
        !elementAttributes.hasAttribute(rotComponentName)) {
        QList<float> parentValues;
        parseVec3(parentNode->attributes, rotComponentName
            , QRegExp("\\s+"), DEFAULT_POSITION_VALUE, parentValues);
        glm::vec3 parentRot(parentValues.at(0), parentValues.at(1), parentValues.at(2));
        entityRot = parentRot;
    }

    properties.setRotation(entityRot);
}

void processSphereDimensions(const AFrameReader::AFrameComponentProcessor &component
    , const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties, const AFrameReader::ParseNode * const) {
    if (properties.dimensionsChanged()) {
        return;
    }

    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_GENERAL_VALUE);
    const float radius = parseFloat(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_RADIUS), elementAttributes, QRegExp("\\s+"), defaultValue);
    const float sphericalDimension = radius * 2;

    properties.setDimensions(glm::vec3(sphericalDimension, sphericalDimension, sphericalDimension));
}

void processCylinderDimensions(const AFrameReader::AFrameComponentProcessor &component, 
    const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties, const AFrameReader::ParseNode * const) {
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

void processCircleDimensions(const AFrameReader::AFrameComponentProcessor &component, 
    const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties, const AFrameReader::ParseNode * const) {
    if (properties.dimensionsChanged()) {
        return;
    }

    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_GENERAL_VALUE);
    const float radius = parseFloat(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_RADIUS), elementAttributes, QRegExp("\\s+"), defaultValue);
    const float diameter = radius * 2;
    
    // Circles are essentially flat cylinders, thus they shouldn't have any height.
    properties.setDimensions(glm::vec3(diameter, 0.0f, diameter));
}

void processConeDimensions(const AFrameReader::AFrameComponentProcessor &component
    , const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties, const AFrameReader::ParseNode * const) {
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
    
    // Types of color specifications
    //      #ffff
    //      rgb(0,0,0)
    //      0 0 0
    //      0, 0, 0
    //      color word (ie: "blue", "maroon") - this isn't supported so should trigger defaultColor return
    xColor color;
    const QString rgbPrefix("rgb(");
    const int hashIndex = colorStr.indexOf('#');
    if (hashIndex >= 0) {
        if (hashIndex != 0) {
            return defaultColor;
        }

        colorStr = colorStr.mid(1);
        const int hexValue = colorStr.toInt(Q_NULLPTR, 16);
        color.red = (colorPart)(hexValue >> 16);
        color.green = (colorPart)((hexValue & 0x00FF00) >> 8);
        color.blue = (colorPart)(hexValue & 0x0000FF);

        return color;
    }
    
    if (colorStr.startsWith(rgbPrefix,Qt::CaseInsensitive)) {
        colorStr = colorStr.mid(rgbPrefix.size(), (colorStr.size() - rgbPrefix.size())-1);
    } else if (!colorStr[0].isDigit()) {
        return defaultColor;
    }

    const QStringList &colorChannelValues = colorStr.split(QRegExp("\\s+|\\s*,"), QString::SkipEmptyParts);
    const int numColorChannels = colorChannelValues.size();
    if (numColorChannels == 0) {
        return defaultColor;
    }

    color.red = (numColorChannels >= 1 ? (colorPart)colorChannelValues[0].toInt() : defaultColor.red);
    color.green = (numColorChannels >= 2 ? (colorPart)colorChannelValues[1].toInt() : defaultColor.green);
    color.blue = (numColorChannels >= 3 ? (colorPart)colorChannelValues[2].toInt() : defaultColor.blue);

    return color;
}

void processColor(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, 
    EntityItemProperties &properties, const AFrameReader::ParseNode * const) {
    if (!elementAttributes.hasAttribute(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_COLOR))) {
        return;
    }

    const xColor color = helper_parseColor(elementAttributes);
    properties.setColor(color);
}

void processSkyColor(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, 
    EntityItemProperties &properties, const AFrameReader::ParseNode * const) {
    if (!elementAttributes.hasAttribute(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_COLOR))) {
        return;
    }

    const xColor color = helper_parseColor(elementAttributes);
    properties.getSkybox().setColor(color);
}

void processTextColor(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, 
    EntityItemProperties &properties, const AFrameReader::ParseNode * const) {
    const xColor color = helper_parseColor(elementAttributes, TextEntityItem::DEFAULT_TEXT_COLOR);
    properties.setTextColor(color);
}

void processDimensions(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, 
    EntityItemProperties &properties, const AFrameReader::ParseNode * const) {
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

void processIntensity(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, 
    EntityItemProperties &properties, const AFrameReader::ParseNode * const) {
    QRegExp splitExp("\\s+");
    const float defaultValue = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : DEFAULT_GENERAL_VALUE);
    const float intensity = helper_parseIntensity(elementAttributes, splitExp, defaultValue);

    properties.setIntensity(intensity);
}

void processLightType(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, 
    EntityItemProperties &properties, const AFrameReader::ParseNode * const) {
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

void processText(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, 
    EntityItemProperties &properties, const AFrameReader::ParseNode * const parentNode) {
    QString displayText = elementAttributes.value(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_VALUE)).toString();

    if (displayText.isEmpty()) {
        displayText = (component.componentDefault.isValid() ? component.componentDefault.toString() : TextEntityItem::DEFAULT_TEXT);
    }

    properties.setText(displayText);
    processDimensions({ component.componentType, component.elementType, QVariant(), nullptr }, 
        elementAttributes, properties, parentNode);
}

void processLineHeight(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, 
    EntityItemProperties &properties, const AFrameReader::ParseNode * const) {
    float lineHeight = (component.componentDefault.isValid() ? component.componentDefault.toFloat() : TextEntityItem::DEFAULT_LINE_HEIGHT);

    if (elementAttributes.hasAttribute(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_LINE_HEIGHT))) {
        lineHeight = elementAttributes.value(AFrameReader::getNameForComponent(AFrameReader::AFRAMECOMPONENT_LINE_HEIGHT)).toFloat();
    }

    properties.setLineHeight(lineHeight);
}

void processTextSide(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, 
    EntityItemProperties &properties, const AFrameReader::ParseNode * const) {
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
        url = resourceName.mid(inlineURLStart.size(), (inlineURLStart.size() - resourceName.size()) - 1);
    } else {
        url = resourceName;
    }

    // Vet the extension to ensure it's a supported type.
    const bool isImageExtension = hasImageExtension(url);
    const bool isModelExtension = hasModelExtension(url);

    if (!isImageExtension && !isModelExtension) {
        return QString();
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

void processSource(const AFrameReader::AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, 
    EntityItemProperties &properties, const AFrameReader::ParseNode * const) {

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
    if (propSource.isEmpty()) {
        return;
    }

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
      // TODO_WL21698: Material Entities were recently added.  Do they map to models? 
      //     If so, re-examine support for Materials on Models.
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

        const QString &sourceUrl = srcDictionary.value(sourceRef._srcReference);
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

QString AFrameReader::getNameForBaseComponent(const BaseEntityComponent componentType) {
    if (!isBaseEntityComponentValid(componentType)) {
        return QString();
    }

    return BASE_ENTITY_COMPONENTS[(int)componentType].second;
}

AFrameReader::BaseEntityComponent AFrameReader::getBaseComponentForName(const QString &componentName) {
    if (componentName.isEmpty()) {
        return BASE_ENTITY_COMPONENT_COUNT;
    }

    for each (const BaseEntityComponentPair &basePair in BASE_ENTITY_COMPONENTS) {
        if (componentName != basePair.second) {
            continue;
        }

        return basePair.first;
    }

    return BASE_ENTITY_COMPONENT_COUNT;
}

bool AFrameReader::isBaseEntityComponentValid(const BaseEntityComponent componentType) {
    return ((int)componentType >= 0) && ((int)componentType < (int)BASE_ENTITY_COMPONENT_COUNT);
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

    qDebug() << "AFrameReader::processScene ENTERED... ";

    m_propData.clear();
    m_propChildMap.clear();
    m_parseStack.clear();
    m_srcDictionary.clear();
    m_mixinDictionary.clear();
    bool success = true;
    QXmlStreamReader::TokenType currentTokenType = QXmlStreamReader::NoToken;
    auto handleEarlyIterationExit = [this]() {
        m_propData.pop_back();
    };

    while (!m_reader.atEnd()) {

        currentTokenType = m_reader.readNext();
        if (currentTokenType == QXmlStreamReader::TokenType::Invalid) {
            success = false;
            break;
        }

        if (m_reader.isStartElement()) {
            QString elementName = m_reader.name().toString();

            // Assets section is expected to be first section under
            // the scene; however, checking status here and allowing
            // for continued non-asset parsing should cover both cases
            //      1) Assets grouped at top
            //      2) Assets somewhere in between or weirdly divvied (why?)
            if (elementName == AFRAME_ASSETS) {
                processAssets();

                if (!m_reader.isStartElement()) {
                    // Early Iteration Exit -- likely reached the end, for loop check as opposed
                    // to breaking just in case.
                    continue;
                }

                elementName = m_reader.name().toString();
            }
            
            if (elementName == AFRAME_ENTITY) {
                processAFrameEntity(m_reader.attributes());
                // Early Iteration Exit -- entity properties, if valid, have been processed and noted
                continue;
            }

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
            // Placeholder name until actually idName is queried.  This allows for better
            // debug/warning/error statements than nameless entity would provide.
            hifiProps.setName(elementName);

            const EntityProcessExitReason typeAssignmentResult = assignEntityType(elementType, hifiProps);
            if (typeAssignmentResult != PROCESS_EXIT_NORMAL) {
                handleEarlyIterationExit();

                // Early Iteration Exit
                continue;
            }

            QXmlStreamAttributes attributes = m_reader.attributes();
            if (attributes.isEmpty()) {
                // Early Iteration Exit
                continue;
            }

            const EntityProcessExitReason attributeResult = processEntityAttributes(elementType, attributes, hifiProps);
            if (attributeResult != PROCESS_EXIT_NORMAL) {
                handleEarlyIterationExit();

                // Early Iteration Exit
                continue;
            }

        } else if (m_reader.isEndElement()) {
            if ( m_parseStack.isEmpty() ) {
                // Early Iteration Exit --
                continue;
            }

            const QString &endElementName = m_reader.name().toString();
            if (endElementName != AFRAME_ENTITY) {
                const AFrameType elementType = getTypeForElementName(endElementName);
                if (elementType == AFRAMETYPE_COUNT) {
                    // Early Iteration Exit
                    continue;
                }
                else if (!elementProcessors.contains(elementType)) {
                    // Early Iteration Exit
                    continue;
                }
            }

            const ParseNode &parseTop = m_parseStack.top();
            if (parseTop.type == ParseNode::NODE_TYPE_REFERENCE) {
                m_propData.removeAt(parseTop.propInfo.index);
                qDebug() << "AFrameReader::processScene - Removed ReferenceEntity: " << parseTop.name;
            }
            m_parseStack.pop();
            qDebug() << "AFrameReader::processScene - Popped ParseNode: " << parseTop.name << "(" << parseTop.propInfo.hifiProps << ")";
            
        }// End_if( startElement/endElement )
    } // End_while( !atEnd )

    processEntitySourceReferences(m_srcDictionary);
    clearEntitySourceReferences();

    if (m_reader.hasError()) {
        qWarning() << "AFrameReader::read encountered error: " << m_reader.errorString();
        success = false;
    }

    qDebug() << "AFrameReader::processScene EXITTED... ";

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
            const AssetControlType controlType = getTypeForAssetElementName(elementName);
            if (controlType == ASSET_CONTROL_TYPE_COUNT) {
                const AFrameType elementType = getTypeForElementName(elementName);
                if ((elementType != AFRAMETYPE_COUNT) || (elementName == AFRAME_ENTITY)) {
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
            const QString &assetId = attributes.value(getNameForComponent(AFRAMECOMPONENT_ID)).toString();
            if (assetId.isEmpty()) {
                // Early Iteration Exit -- All assets required to have an id specified.
                qWarning() << "AFrameReader::processAssets detected missing id component for asset!";
                continue;
            }

            switch (controlType) {
                // Intentional fall through of ASSET_CONTROL_TYPE_ASSET_IMAGE - ASSET_CONTROL_TYPE_IMG
                case ASSET_CONTROL_TYPE_ASSET_IMAGE:
                case ASSET_CONTROL_TYPE_ASSET_ITEM:
                case ASSET_CONTROL_TYPE_IMG: {
                    const QString &assetSrc = attributes.value(getNameForComponent(AFRAMECOMPONENT_SOURCE)).toString();
                    if (assetSrc.isEmpty()) {
                        // Early Iteration Exit -- All assets are required to have an src specified.
                        qWarning() << "AFrameReader::processAssets detected asset " << assetId << " without required src component!";
                        continue;
                    }

                    const QString &resourceUrl = helper_getResourceURL(assetSrc);
                    if (resourceUrl.isEmpty()) {
                        // Early Iteration Exit -- This isn't a supported type of resource.
                        qWarning() << "AFrameReader::processAssets detected unsupported/unknown resource type: " << assetSrc;
                        continue;
                    }

                    m_srcDictionary.insert(assetId, resourceUrl);

                    // TODO_WL21698:  Remove Debug Dump
                    qDebug() << "----------";
                    qDebug() << "AFrameReader::processAssets adding pair: " << assetId << " - " << assetSrc;
                    qDebug() << "**********";

                    break;
                }
                case ASSET_CONTROL_TYPE_MIXIN: {
                    m_mixinDictionary.insert(assetId, attributes);
                    break;
                }
            } // End_Switch( controlType )
        } // End_if( isStartElement )
    } // End_while( !atEnd )

    if (m_reader.hasError()) {
        qWarning() << "AFrameReader::read encountered error: " << m_reader.errorString();
        success = false;
    }

    qDebug() << "AFrameReader::processAssets EXITED... ";
    return success;
}

AFrameReader::EntityProcessExitReason AFrameReader::processAFrameEntity(const QXmlStreamAttributes &attributes) {
    if (attributes.isEmpty()) {
        return PROCESS_EXIT_INVALID_INPUT;
    }

    qDebug() << "*************************************************";
    qDebug() << "AFrameReader::processAelement ENTERED... ";

    //StringDictionary elementComponents;
    ComponentPropertiesTable elementComponents;
    QXmlStreamAttributes entityAttributes;
    int numProbableAttributes = 0;
    for each (const QXmlStreamAttribute &component in attributes) {
        const QString &componentName = component.name().toString();
        if (!isSupportedEntityComponent(componentName)) {
            // TODO_WL21698:  Should this be a warning?
            qDebug() << "AFrameReader::processAFrameEntity encountered unknown/unsupported component: " << componentName;
            continue;
        }

        if (elementComponents.contains(componentName)) {
            qWarning() << "AFrameReader::processAFrameEntity encountered dupe component: " << componentName;
        }

        const int numComponentPairs = populateComponentPropertiesTable(component, elementComponents);
        if (numComponentPairs <= 0) {
            // Early Iteration Exit -- encountered error with component population
            continue;
        }

        numProbableAttributes += numComponentPairs;

    } // End_ForEach( Original Entity Component )

    // Compose Attribute Vector
    entityAttributes.reserve(numProbableAttributes);
    const QString &mixinKey = ENTITY_COMPONENTS[ENTITY_COMPONENT_MIXIN].second;
    bool needToResolveMixins = false;
    IterComponentProperties iterComponentProperty = elementComponents.begin();
    IterComponentProperties iterEndElementComponents = elementComponents.end();
    for (; iterComponentProperty != iterEndElementComponents; ++iterComponentProperty) {
        const ComponentProperties &componentProperties = iterComponentProperty.value();
        if (iterComponentProperty.key() == mixinKey) {
            needToResolveMixins = true;
            // Early Iteration Exit -- mixins will be resolved after all direct attributes have been noted.
            continue;
        }

        qDebug() << "\t\tNumProperties for " << iterComponentProperty.key() << ": " << componentProperties.size();
        for each (const ComponentPropertyPair &propPair in componentProperties) {
            entityAttributes.append(propPair.first, propPair.second);
            qDebug() << "\t\t\tProperty: " << propPair.first << " Value: " << propPair.second;
        }
    }

    // Resolve mixin attributes if need be...
    if (needToResolveMixins) {
        
        if (m_mixinDictionary.isEmpty()) {
            qWarning() << "AFrameReader::processAFrameEntity - Mixin Dictionary is empty. Ensure that asset block is first child of the scene!";
            return PROCESS_EXIT_EMPTY_MIXIN_DICTIONARY;
        }

        const ComponentPropertyPair &mixinComponent = elementComponents[mixinKey].back();
        const QStringList &mixinIds = mixinComponent.second.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        QXmlStreamAttributes attributeContributions;
        attributeContributions.reserve(DEFAULT_MIXIN_ATTRIBUTE_RESERVE);
        ComponentPropertiesTable mixComponentTable;

        QStringList::const_reverse_iterator const rIterEndMixinId = mixinIds.rend();
        for (auto rIterMixinId = mixinIds.rbegin(); rIterMixinId != rIterEndMixinId; ++rIterMixinId) {
            const QString &mixinName = (*rIterMixinId);
            qDebug() << "MixinName: " << (*rIterMixinId);
            if (!m_mixinDictionary.contains(mixinName)) {
                qWarning() << "AFrameReader::processAFrameEntity - Couldn't find expected mixin: " << mixinName;
                // Early Iteration Exit
                continue;
            }

            const QXmlStreamAttributes &mixinAttributes = m_mixinDictionary[mixinName];
            const QString &aframeIdKey = ENTITY_COMPONENTS[ENTITY_COMPONENT_ID].second;
            for each (const QXmlStreamAttribute &attribute in mixinAttributes) {
                const QString &attributeName = attribute.name().toString();
                if (attributeName == aframeIdKey) {
                    // Early Iteration Exit -- Entities don't inherit the mixin id, so bypass
                    continue;
                }

                if (entityAttributes.hasAttribute(attributeName)) {
                    // Early Iteration Exit -- direct entity spec of property has supremacy
                    continue;
                }

                if (attributeContributions.hasAttribute(attributeName)) {
                    // Early Iteration Exit -- Reverse iteration means recent spec of the property has already been noted
                    continue;
                }

                const int numMixPairs = populateComponentPropertiesTable(attribute, mixComponentTable);
                if (numMixPairs <= 0) {
                    // Early Iteration Exit -- encountered error with component population
                    continue;
                }

                attributeContributions.push_back(attribute);
                qDebug() << "\t\t\tProperty(Mixin): " << attribute.name() << " Value: " << attribute.value();
            } // End_ForEach( mixinAttribute )
        } // End_For[Reverse]( mixinId )

        // Add mixin contributions to entityAttributes
        ComponentPropertiesTable::const_iterator const iterMixPropertyEnd = mixComponentTable.end();
        for (auto iterMixProperty = mixComponentTable.begin(); iterMixProperty != iterMixPropertyEnd; ++iterMixProperty) {
            const ComponentProperties &mixProperties = iterMixProperty.value();
            qDebug() << "\t\tNumProperties for Mix_" << iterMixProperty.key() << ": " << mixProperties.size();
            elementComponents[iterMixProperty.key()] = iterMixProperty.value();
            for each (const ComponentPropertyPair &propPair in mixProperties) {
                entityAttributes.append(propPair.first, propPair.second);
                qDebug() << "\t\t\tMix_Property: " << propPair.first << " Value: " << propPair.second;
            }
        }

        needToResolveMixins = false;
        for each (const QXmlStreamAttribute &finalAttribute in entityAttributes) {
            qDebug() << "Final_Attribute: " << finalAttribute.name() << " - " << finalAttribute.value();
        }
    } // End_If( needToResolveMixins )

    // Construct EntityItemProperty based on determined element type
    AFrameType elementType = AFRAMETYPE_COUNT;
    const QList<QString> &componentKeys = elementComponents.keys();
    for each (const QString &key in componentKeys) {
        QString queryKey("a-");
        queryKey.append(key.toLower());
        for each (const QString &convenienceType in AFRAME_ELEMENT_NAMES) {
            if (queryKey == convenienceType) {
                elementType = getTypeForElementName(convenienceType);
                m_propData.push_back(EntityItemProperties());
                EntityItemProperties &hifiProps = m_propData.back();
                // Placeholder name until actually idName is queried.  This allows for better
                // debug/warning/error statements than nameless entity would provide.
                hifiProps.setName(convenienceType);
                const EntityProcessExitReason typeAssignmentExitReason = assignEntityType(elementType, hifiProps);
                if (typeAssignmentExitReason != PROCESS_EXIT_NORMAL) {
                    m_propData.pop_back();
                    return typeAssignmentExitReason;
                }

                const EntityProcessExitReason attributeResult = processEntityAttributes(elementType, entityAttributes, hifiProps);
                if (attributeResult != PROCESS_EXIT_NORMAL) {
                    m_propData.pop_back();
                    return attributeResult;
                }

                qDebug() << "AFrameReader::processAelement EXITTED... ";
                qDebug() << "*************************************************";

                return PROCESS_EXIT_NORMAL;
            }
        }
    }

    // There wasn't a higher priority type found, thus check to see if it's just a geometry/shape
    // type and go from there.
    const QString &geometryKey = ENTITY_COMPONENTS[(int)ENTITY_COMPONENT_GEOMETRY].second;
    if (componentKeys.contains(geometryKey)) {
    //if (entityAttributes.hasAttribute(geometryKey)) {
        const ComponentProperties &geometryProps = elementComponents[geometryKey];
        for each (const ComponentPropertyPair &propPair in geometryProps) {
            if (propPair.first != "primitive") {
                continue;
            }

            QString queryVal = propPair.second.toLower();
            if (queryVal == "box") {
                queryVal = "cube";
            }
            else if (queryVal == "plane") {
                queryVal = "quad";
            }

            const entity::Shape shape = entity::shapeFromString(queryVal, entity::NO_DEFAULT_ON_ERROR);
            const bool isSupportedShape = isShapeSupported(shape);
            if (!isSupportedShape) {
                return PROCESS_EXIT_UNKNOWN_TYPE;
            }

            elementType = getElementTypeForShape(shape);
            if (elementType == AFRAMETYPE_COUNT) {
                return PROCESS_EXIT_UNKNOWN_TYPE;
            }

            m_propData.push_back(EntityItemProperties());
            EntityItemProperties &hifiProps = m_propData.back();
            // Placeholder name until actually idName is queried.  This allows for better
            // debug/warning/error statements than nameless entity would provide.
            hifiProps.setName(getElementNameForType(elementType));
            const EntityProcessExitReason typeAssignmentExitReason = assignEntityType(elementType, hifiProps);
            if (typeAssignmentExitReason != PROCESS_EXIT_NORMAL) {
                m_propData.pop_back();
                return typeAssignmentExitReason;
            }

            const EntityProcessExitReason attributeResult = processEntityAttributes(elementType, entityAttributes, hifiProps);
            if (attributeResult != PROCESS_EXIT_NORMAL) {
                m_propData.pop_back();
                return attributeResult;
            }

            qDebug() << "AFrameReader::processAelement EXITTED... ";
            qDebug() << "*************************************************";

            return PROCESS_EXIT_NORMAL;
        }
    }

    // See if this is a data parent entity, which A-Frame allows without a given type component.
    const QString &aframeIdKey = ENTITY_COMPONENTS[ENTITY_COMPONENT_ID].second;
    const int numComponentKeys = componentKeys.size();
    for each (const QString &key in componentKeys) {
        const BaseEntityComponent keyBaseComponentId = getBaseComponentForName(key);
        qDebug() << "\t\tEvaluating CompKey: " << key 
            << " which has a BaseComponentId of: " << getNameForBaseComponent(keyBaseComponentId);
        if (keyBaseComponentId == BASE_ENTITY_COMPONENT_COUNT) {
            if (key == aframeIdKey && (numComponentKeys > 1)) {
                // Id component shouldn't invalidate the entry as long as there's more data to validate with.
                continue;
            }

            qDebug() << "AFrameReader::processAFrameEntity - Encountered unknown/unsupported custom a-entity.";
            return PROCESS_EXIT_UNKNOWN_TYPE;
        }
    }

    // This is a parent data node, so set it up...
    m_propData.push_back(EntityItemProperties());
    EntityItemProperties &hifiProps = m_propData.back();
    if (entityAttributes.hasAttribute(aframeIdKey)) {
        hifiProps.setName(attributes.value(aframeIdKey).toString());
    } else {
        QString parentNodeName("ParentDataElement_");
        QTextStream(&parentNodeName) << &hifiProps;
        hifiProps.setName(parentNodeName);
    }

    const ParseNode * const parentNode = getParentNode();
    if ((parentNode != nullptr) && (parentNode->type != ParseNode::NODE_TYPE_REFERENCE)) {
        m_propChildMap[parentNode->propInfo.hifiProps].push_back(&hifiProps);
        qDebug() << "AFrameReader::processAFrameEntity - Noting ParseNode: " << parentNode->name
            << "(" << parentNode->propInfo.hifiProps << ") has ChildNode:" << &hifiProps;
    }

    for each (const QXmlStreamAttribute &nodeAttribute in entityAttributes) {
        const QString &nodeName = nodeAttribute.name().toString();
        const BaseEntityComponent baseComponentType = getBaseComponentForName(nodeName);

        switch (baseComponentType) {
            case BASE_ENTITY_COMPONENT_POSITION: {
                processPosition(AFrameComponentProcessor(), entityAttributes, hifiProps, parentNode);
                break;
            }
            case BASE_ENTITY_COMPONENT_ROTATION: {
                processRotation(AFrameComponentProcessor(), entityAttributes, hifiProps, parentNode);
                break;
            }
            case BASE_ENTITY_COMPONENT_MIXIN: {
                // Shouldn't here as this has been resolved prior to this point; however,
                // if this happens, then warn and continue...
                qWarning() << "AFrameReader::processAFrameEntity - ParentDataElement unexpectedly still has mixin component.";
                continue;
            }
            default: {
                if (baseComponentType == BASE_ENTITY_COMPONENT_COUNT) {
                    if (nodeName != aframeIdKey) {
                        qWarning() << "AFrameReader::processAFrameEntity - Encountered uknown/unsupported BaseEntityComponent: " << nodeName;
                        continue;
                    }

                    // Do Nothing - name is the first thing set up
                }
                break;
            }
        }
    }

    m_parseStack.push({ hifiProps.getName(), QXmlStreamAttributes(), {m_propData.size()-1, &hifiProps}, ParseNode::NODE_TYPE_REFERENCE });
    ParseNode &parseTop = m_parseStack.top();
    parseTop.attributes.swap(entityAttributes);
    qDebug() << "AFrameReader::processAFrameEntity - Pushing ParseNode: " << parseTop.name << "(" << parseTop.propInfo.hifiProps << ")";

    qDebug() << "AFrameReader::processAelement EXITTED... ";
    qDebug() << "*************************************************";

    return PROCESS_EXIT_UNKNOWN_TYPE;
}

AFrameReader::EntityProcessExitReason AFrameReader::assignEntityType(const AFrameType elementType, EntityItemProperties &hifiProps) {
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
            qWarning() << "AFrameReader::assignEntityType encountered unknown/invalid element: " << hifiProps.getName();
            return PROCESS_EXIT_UNKNOWN_TYPE;
        }
    }

    return PROCESS_EXIT_NORMAL;
}

AFrameReader::EntityProcessExitReason AFrameReader::processEntityAttributes(const AFrameType elementType, QXmlStreamAttributes &attributes, EntityItemProperties &hifiProps) {

    if (!elementProcessors.contains(elementType)) {
        return PROCESS_EXIT_UNKNOWN_TYPE;
    }

    const AFrameElementProcessor &elementProcessor = elementProcessors[elementType];

    // For each attribute, process and record for entity properties.
    const QString &aframeIdKey = getNameForComponent(AFRAMECOMPONENT_ID);
    if (attributes.hasAttribute(aframeIdKey)) {
        hifiProps.setName(attributes.value(aframeIdKey).toString());
    } else {
        const QString elementName = hifiProps.getName();
        const int elementUnsubCount = elementUnnamedCounts[elementName] + 1; //Unnamed count should be 1-based
        hifiProps.setName(elementName + "_" + QString::number(elementUnsubCount));
        elementUnnamedCounts[elementName] = elementUnsubCount;
    }

    const ParseNode * const parentNode = getParentNode();
    if ((parentNode != nullptr) && (parentNode->type != ParseNode::NODE_TYPE_REFERENCE)) {
        m_propChildMap[parentNode->propInfo.hifiProps].push_back(&hifiProps);
        qDebug() << "AFrameReader::processEntityAttributes - Noting ParseNode: " << parentNode->name 
            << "(" << parentNode->propInfo.hifiProps << ") has ChildNode:" << &hifiProps;
    }

    for each (const AFrameComponentProcessor &componentProcessor in elementProcessor._componentProcessors) {
        componentProcessor.processFunc(componentProcessor, attributes, hifiProps, parentNode);
    }

    if (hifiProps.getClientOnly()) {
        auto nodeList = DependencyManager::get<NodeList>();
        const QUuid myNodeID = nodeList->getSessionUUID();
        hifiProps.setOwningAvatarID(myNodeID);
    }

    m_parseStack.push({ hifiProps.getName(), QXmlStreamAttributes(), { m_propData.size()-1, &hifiProps }, ParseNode::NODE_TYPE_FUNCTIONAL });
    ParseNode &parseTop = m_parseStack.top();
    parseTop.attributes.swap(attributes);
    qDebug() << "AFrameReader::processEntityAttributes - Pushing ParseNode: " << parseTop.name << "(" << parseTop.propInfo.hifiProps << ")";

    // TODO_WL21698:  Remove Debug Dump
#if AFRAME_DEBUG_NOTES >= 1
    qDebug() << "-------------------------------------------------";
    hifiProps.debugDump();
    qDebug() << "*************************************************";
    qDebug() << "*************************************************";
    qDebug() << hifiProps;
    qDebug() << "-------------------------------------------------";
#endif

    return PROCESS_EXIT_NORMAL;
}

bool AFrameReader::isParseTop(const EntityItemProperties * const hifiProps) const {
    if (hifiProps == nullptr) {
        return false;
    }

    if ( m_parseStack.isEmpty() ) {
        return false;
    }

    return (hifiProps == &m_propData.back()) && (m_parseStack.top().propInfo.hifiProps == hifiProps);
}

const AFrameReader::ParseNode * AFrameReader::getParentNode() const {
    const int parseDepth = m_parseStack.size();
    return ((parseDepth >= 1) ? &(m_parseStack[(parseDepth - 1)]) : nullptr);
}

bool AFrameReader::isSupportedEntityComponent(const QString &componentName) const {
    if (componentName.isEmpty()) {
        return false;
    }

    const QString &queryName = componentName.toLower();
    for each (const EntityComponentPair &componentPair in ENTITY_COMPONENTS) {
        if (queryName != componentPair.second) {
            continue;
        }

        return true;
    }

    return false;
}

bool AFrameReader::isSupportedEntityComponent(const EntityComponent componentType) const {
    if (((int)componentType < 0) || ((int)componentType >= (int)ENTITY_COMPONENT_COUNT)) {
        return false;
    }

    for each (const EntityComponentPair &componentPair in ENTITY_COMPONENTS) {
        if (componentType != componentPair.first) {
            continue;
        }

        return true;
    }

    return false;
}

bool AFrameReader::isBaseEntityComponent(const QString &componentName) const {
    if (componentName.isEmpty()) {
        return false;
    }

    const QString &queryName = componentName.toLower();
    for each (const BaseEntityComponentPair &componentPair in BASE_ENTITY_COMPONENTS) {
        if (queryName != componentPair.second) {
            continue;
        }

        return true;
    }

    return false;
}

bool AFrameReader::isBaseEntityComponent(const BaseEntityComponent componentType) const {
    if (((int)componentType < 0) || ((int)componentType >= (int)BASE_ENTITY_COMPONENT_COUNT)) {
        return false;
    }

    for each (const BaseEntityComponentPair &componentPair in BASE_ENTITY_COMPONENTS) {
        if (componentType != componentPair.first) {
            continue;
        }

        return true;
    }

    return false;
}

int AFrameReader::populateComponentPropertiesTable(const QXmlStreamAttribute &component, ComponentPropertiesTable &componentsTable) {
    const QString &componentName = component.name().toString();
    const QString &componentValue = component.value().toString();
    const QStringList &propertyPairs = componentValue.split(QRegExp(";\\s*"), QString::SkipEmptyParts);
    const int numPairs = propertyPairs.size();
    if (numPairs == 0) {
        qWarning() << "AFrameReader::populateComponentPropertiesTable - Encountered invalid Component Specification: " << componentValue;
        return 0;
    }

    // TODO_WL21698: Additional filtering to avoid adding components with no supported properties?
    componentsTable[componentName].reserve(numPairs);
    for each (const QString propertyPairString in propertyPairs) {
        const QStringList &propertyInfo = propertyPairString.split(QRegExp(":\\s*"), QString::SkipEmptyParts);
        const int numInfo = propertyInfo.size();
        switch (numInfo) {
            case 1: { // This occurs when the component has a direct assignment as opposed to a listing of property pairs.
                componentsTable[componentName].push_back(ComponentPropertyPair{ componentName, propertyInfo[0] });
                break;
            }
            case 2: {
                componentsTable[componentName].push_back(ComponentPropertyPair{ propertyInfo[0], propertyInfo[1] });
                break;
            }
            default: {
                qWarning() << "AFrameReader::processAFrameEntity - Encountered invalid propertyPair: " << propertyPairString;
                // Early Iteration Exit
                continue;
            }
        }

        // TODO_WL21698: Remove this as it's for debugging only
        const ComponentPropertyPair &propPair = componentsTable[componentName].back();
        qDebug() << "\tComponent: " << componentName << "- Property: " << propPair.first << " Value: " << propPair.second;
        // ----------------
    } // End_ForEach( Component Property Pair )

    return numPairs;
}

