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

#include <QXmlStreamReader>
#include <QMap>
#include <QVariant>

class EntityItemProperties;

class AFrameReader {
public:

    //! AFrameType enumeration specifies labels for each AFrame Element
    //! supported by this reader.  It's expected that when support for a new Element
    //! is added that a label be added to this enumeration _and_ that its respective
    //! AFrame element name be added to AFRAME_ELEMENT_NAMES within AFrameReader.cpp with
    //! respect to the enumeration order.
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

    //! AFrameComponent enumeration specifies labels for each AFrame Component supported
    //! by this reader with respect to AFrameType.  It's expected that when support for a 
    //! Component is added that a label be added to this enumeration _and_ that its respective
    //! AFrame component name be added to AFRAME_COMPONENT_NAMES within AFrameReader.cpp with respect
    //! to the enumeration order.
    enum AFrameComponent {
        AFRAMECOMPONENT_COLOR,
        AFRAMECOMPONENT_DEPTH,
        AFRAMECOMPONENT_HEIGHT,
        AFRAMECOMPONENT_POSITION,
        AFRAMECOMPONENT_RADIUS,
        AFRAMECOMPONENT_RADIUS_BOTTOM,
        AFRAMECOMPONENT_ROTATION,
        AFRAMECOMPONENT_SOURCE,
        AFRAMECOMPONENT_WIDTH,

        AFRAMECOMPONENT_COUNT
    };

    //! AFrameComponentProcessor represents component conversion information.  It contains
    //! the AFrameComponent which it represents along with a default value if the component
    //! processFunc doesn't detect the component's presence within the specified element attributes.
    //! AFrameElementProcessors owns instances of AFrameComponentProcessors; as many needed to
    //! describe the AFrame Entity in High Fidelity Entity context.
    struct AFrameComponentProcessor {
        typedef std::function<void(const AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties)> ProcessFunc;
        AFrameComponent componentType;
        QVariant componentDefault;
        ProcessFunc processFunc;
    };
    typedef QMap< AFrameComponent, AFrameComponentProcessor > ComponentProcessors;

    //! AFrameElementProcessor represents element conversion information.  It contains the
    //! AFrameType which it represents and owns an instance of ComponentProcessors which houses
    //! all AFrameComponentProcessors needed to translate the AFrame Element into its High Fidelity
    //! counterpart.  AFrameReader owns AFRAMETYPE_COUNT instances of these as there should be
    //! 1 AFrameElementProcessor for each supported AFrame Element type via AFrameReader::elementProcessors.
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

    //! Parses the specified ByteArray looking for supported AFrame Elements.
    //! @return: True, iff the data was read and parsed without error.
    //! @note:  If there's more than 1 a-scene found, _only_ the first encountered
    //!         is processed.
    bool read(const QByteArray &aframeData);
    QString getErrorString() const;

    //! Provides access to latest batch of composed entity property data
    //! @note:  This list will be empty if no AFrame elements were successfully parsed
    //!         via a read call _or_ if read hasn't been called yet.
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
