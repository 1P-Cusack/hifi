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


    struct AFrameProcessor {
        QString propName;
        typedef std::function<void(const AFrameType elementType, const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties)> conversionHandler;
        conversionHandler processFunc;
    };

    typedef QList<EntityItemProperties> AFramePropList;
    typedef QVector<QString> TagList;
    // attribute name -> handler
    typedef QMap< QString, AFrameProcessor > AFrameElementHandlerTable;
    // element name to handler directory
    typedef QMap< QString, AFrameElementHandlerTable> AFrameConversionTable;

    static AFrameConversionTable commonConversionTable;
    static TagList supportedAFrameElements;

    static void registerAFrameConversionHandlers();
    static QString getElementNameForType(AFrameType elementType);
    static AFrameType getTypeForElementName(const QString &elementName);

    bool read(const QByteArray &aframeData);
    QString getErrorString() const;
    const AFramePropList & getPropData() const { return m_propData; }


protected:

    typedef QHash<QString, int> ElementUnnamedCounts;
    static ElementUnnamedCounts elementUnnamedCounts;

    bool processScene();

    QXmlStreamReader m_reader;
    AFramePropList m_propData;
};

#endif
