//
//  EntityTree.h
//  libraries/entities/src
//
//  Created by LaShonda Hopper 2018/02/21.
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
#ifndef hifi_AFrameReader_h
#define hifi_AFrameReader_h

#include <bitset>

#include <QXmlStreamReader>
#include <QMap>
#include <QVariant>

class EntityItemProperties;

class AFrameReader {
public:

    enum AFrameType {
        AFRAMETYPE_BOX,
        AFRAMETYPE_CIRCLE,
        AFRAMETYPE_CONE,
        AFRAMETYPE_CYLINDER,
        AFRAMETYPE_IMAGE,
        AFRAMETYPE_LIGHT,
        AFRAMETYPE_MODEL_GLTF,
        AFRAMETYPE_MODEL_OBJ,
        AFRAMETYPE_PLANE,
        AFRAMETYPE_SKY,
        AFRAMETYPE_SPHERE,
        AFRAMETYPE_TETRAHEDRON,
        AFRAMTYPE_TEXT,
        AFRAMETYPE_TRIANGLE,

        AFRAMETYPE_COUNT
    };

    enum AFrameComponent {
        AFRAMECOMPONENT_COLOR,
        AFRAMECOMPONENT_DEPTH,
        AFRAMECOMPONENT_HEIGHT,
        AFRAMECOMPONENT_POSITION,
        AFRAMECOMPONENT_RADIUS,
        AFRAMECOMPONENT_ROTATION,
        AFRAMECOMPONENT_SOURCE,
        AFRAMECOMPONENT_WIDTH,

        AFRAMECOMPONENT_COUNT
    };

    struct AFrameComponentProcessor {
        typedef std::function<void(const AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties)> ProcessFunc;
        AFrameComponent componentType;
        QVariant componentDefault;
        ProcessFunc processFunc;
    };

    typedef QMap< AFrameComponent, AFrameComponentProcessor > ComponentProcessors;

    struct AFrameElementProcessor {
        AFrameType _element;
        ComponentProcessors _componentProcessors;
    };

    typedef QMap< AFrameType, AFrameElementProcessor > ElementProcessors;
    typedef QList<EntityItemProperties> AFramePropList;

    static void registerAFrameConversionHandlers();
    static QString getElementNameForType(AFrameType elementType);
    static AFrameType getTypeForElementName(const QString &elementName);
    static bool isElementTypeValid(AFrameType elementType);
    static QString getNameForComponent(AFrameComponent componentType);
    static AFrameComponent getComponentForName(const QString &componentName);
    static bool isComponentValid(AFrameComponent componentType);

    bool read(const QByteArray &aframeData);
    QString getErrorString() const;
    const AFramePropList & getPropData() const { return m_propData; }


protected:

    static ElementProcessors elementProcessors;

    typedef QHash<QString, int> ElementUnnamedCounts;
    static ElementUnnamedCounts elementUnnamedCounts;

    bool processScene();

    QXmlStreamReader m_reader;
    AFramePropList m_propData;
};

#endif
