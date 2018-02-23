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

class EntityItemProperties;

class AFrameReader {
public:

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
