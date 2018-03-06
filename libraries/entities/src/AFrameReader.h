//
//  EntityTree.h
//  libraries/entities/src
//
//  Created by LaShonda Hopper 02/21/18.
//  Copyright 2013 High Fidelity, Inc.
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


    struct AFrameProcessor {
        QString propName;
        typedef std::function<void(const QXmlStreamAttributes &elementAttributes, EntityItemProperties &properties)> conversionHandler;
        conversionHandler processFunc;
    };

    typedef QSet<QString> TagList;

    // attribute name -> handler
    typedef QMap< QString, AFrameProcessor > AFrameElementHandlerTable;
    // element type to handler directory
    typedef QMap< QString, AFrameElementHandlerTable> AFrameConversionTable;

    static AFrameConversionTable commonConversionTable;
    static TagList supportAFrameTypes;
    static void registerAFrameConversionHandlers();

    typedef QList<EntityItemProperties> AFramePropList;

    bool read(const QByteArray &aframeData);
    QString getErrorString() const;
    const AFramePropList & getPropData() const { return m_propData; }


protected:

    bool processScene();

    QXmlStreamReader m_reader;
    AFramePropList m_propData;
};

#endif
