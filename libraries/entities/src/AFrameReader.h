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
        AFRAMETYPE_MODEL_OBJ,
        AFRAMETYPE_PLANE,
        AFRAMETYPE_SKY,
        AFRAMETYPE_SPHERE,
        AFRAMETYPE_TETRAHEDRON,
        AFRAMETYPE_TEXT,
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
        AFRAMECOMPONENT_INTENSITY,
        AFRAMECOMPONENT_LINE_HEIGHT,
        AFRAMECOMPONENT_POSITION,
        AFRAMECOMPONENT_RADIUS,
        AFRAMECOMPONENT_RADIUS_BOTTOM,
        AFRAMECOMPONENT_ROTATION,
        AFRAMECOMPONENT_SIDE,
        AFRAMECOMPONENT_SOURCE,
        AFRAMECOMPONENT_TYPE,
        AFRAMECOMPONENT_VALUE,
        AFRAMECOMPONENT_WIDTH,

        AFRAMECOMPONENT_COUNT
    };

    //! AssetControlType enumeration specifies labels for each A-Frame Asset Management Element
    //! supported by this reader.  It's expected that when support for a new Asset Management Element
    //! is added that a label be added to this enumeration _and_ that its respective
    //! A-Frame element name be added to AFRAME_ASSET_CONTROL_NAMES within AFrameReader.cpp with
    //! respect to the enumeration order.
    enum AssetControlType {
        ASSET_CONTROL_TYPE_ASSET_ITEM,
        ASSET_CONTROL_TYPE_ASSET_IMAGE,
        ASSET_CONTROL_TYPE_IMG,
        ASSET_CONTROL_TYPE_MIXIN,

        ASSET_CONTROL_TYPE_COUNT
    };

    enum EntityComponent {
        ENTITY_COMPONENT_GEOMETRY,
        ENTITY_COMPONENT_IMAGE,
        ENTITY_COMPONENT_LIGHT,
        ENTITY_COMPONENT_MATERIAL,
        ENTITY_COMPONENT_POSITION,
        ENTITY_COMPONENT_MIXIN,
        ENTITY_COMPONENT_MODEL_OBJ,
        ENTITY_COMPONENT_ROTATION,
        ENTITY_COMPONENT_TEXT,

        ENTITY_COMPONENT_COUNT
    };

    //! AFrameComponentProcessor represents component conversion information.  It contains
    //! the AFrameComponent which it represents along with a default value if the component
    //! processFunc doesn't detect the component's presence within the specified element attributes.
    //! AFrameElementProcessors owns instances of AFrameComponentProcessors; as many needed to
    //! describe the AFrame Entity in High Fidelity Entity context.
    struct AFrameComponentProcessor {
        typedef std::function<void(const AFrameComponentProcessor &component, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties)> ProcessFunc;
        AFrameComponent componentType;
        AFrameType elementType;
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

    struct SourceReference {
        QString _srcReference;
        EntityItemProperties * _entityPropData;
    };

    typedef QList<EntityItemProperties> AFramePropList;
    typedef QHash<QString, QString> StringDictionary;
    typedef QHash<QString, SourceReference> SourceReferenceDictionary;
    typedef QHash<QString, QXmlStreamAttributes> MixinDictionary;

    typedef std::pair<const EntityComponent, const QString> EntityComponentPair;
    typedef std::pair<const QString, const QString> ComponentPropertyPair;
    typedef QVector< ComponentPropertyPair > ComponentProperties;
    typedef QHash<QString, ComponentProperties> ComponentPropertiesTable;
    typedef ComponentPropertiesTable::iterator IterComponentProperties;

    static void registerAFrameConversionHandlers();
    static void noteEntitySourceReference(const QString &srcReference, EntityItemProperties &entityPropData);
    static inline void clearEntitySourceReferences() { entitySrcReferences.clear(); }

    static QString getElementNameForType(AFrameType elementType);
    static AFrameType getTypeForElementName(const QString &elementName);
    static bool isElementTypeValid(AFrameType elementType);

    static QString getNameForComponent(AFrameComponent componentType);
    static AFrameComponent getComponentForName(const QString &componentName);
    static bool isComponentValid(AFrameComponent componentType);

    static QString getNameForAssetElement(AssetControlType elementType);
    static AssetControlType getTypeForAssetElementName(const QString &elementName);
    static bool isAssetElementTypeValid(AssetControlType elementType);

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
    static SourceReferenceDictionary entitySrcReferences;

    static void processEntitySourceReferences(const StringDictionary &srcDictionary);

    bool processScene();
    bool processAssets();

    enum EntityProcessExitReason {
        PROCESS_EXIT_NORMAL,
        PROCESS_EXIT_INVALID_INPUT,
        PROCESS_EXIT_EMPTY_MIXIN_DICTIONARY,
        PROCESS_EXIT_EMPTY_SRC_DICTIONARY,
        PROCESS_EXIT_UNKNOWN_TYPE
    };

    EntityProcessExitReason processAFrameEntity(const QXmlStreamAttributes &attributes);

    EntityProcessExitReason assignEntityType(AFrameType elementType, EntityItemProperties &hifiProps);
    EntityProcessExitReason processEntityAttributes(AFrameType elementType, const QXmlStreamAttributes &attributes, EntityItemProperties &hifiProps);

    bool isSupportedEntityComponent(const QString &componentName) const;
    bool isSupportedEntityComponent(EntityComponent componentType) const;
    int populateComponentPropertiesTable(const QXmlStreamAttribute &component, ComponentPropertiesTable &componentsTable);

    QXmlStreamReader m_reader;
    AFramePropList m_propData;
    StringDictionary m_srcDictionary;
    MixinDictionary m_mixinDictionary;
};

#endif
