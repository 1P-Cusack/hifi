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

#include "EntityItemProperties.h"

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
            if (elementName == "a-scene") {
                return processScene();
            }
        }
    }

    if (m_reader.hasError()) {
        qDebug() << "AFrameReader::read encountered error: " << m_reader.errorString();
    }

    return false;
}

QString AFrameReader::getErrorString() const {
    return m_reader.errorString();
}

bool AFrameReader::processScene() {

    if (!m_reader.isStartElement() || m_reader.name() != "a-scene"){

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
            QStringRef elementName = m_reader.name();
            static unsigned int sUnsubEntityCount = 0;
            if (elementName == "a-box") {
                EntityItemProperties hifiProps;
                hifiProps.setType(EntityTypes::Box);

                //EntityItemID entityItemID = EntityItemID(QUuid::createUuid());

                // Get Attributes
                QXmlStreamAttributes attributes = m_reader.attributes();
                // For each attribute, process and record for entity properties.
                if (attributes.hasAttribute("id")) {
                    hifiProps.setName(attributes.value("id").toString());
                } else {
                    hifiProps.setName(elementName + "_" + QString::number(sUnsubEntityCount++));
                }

                if (attributes.hasAttribute("color")) {
                    QString colorStr = attributes.value("color").toString();
                    const int hashIndex = colorStr.indexOf('#');
                    if (hashIndex == 0) {
                        colorStr = colorStr.mid(1);
                    } else if (hashIndex > 0) {
                        colorStr = colorStr.remove(hashIndex, 1);
                    }
                    const int hexValue = colorStr.toInt(Q_NULLPTR, 16);
                    hifiProps.setColor({colorPart(hexValue >> 16),
                        colorPart((hexValue & 0x00FF00) >> 8),
                        colorPart(hexValue & 0x0000FF)});
                }

                if (attributes.hasAttribute("position")) {
                    QStringList stringList = attributes.value("position").toString().split(QRegExp("\\s+"), QString::SkipEmptyParts);
                    const int numAttributeDimensions = stringList.size();
                    const float xPosition = numAttributeDimensions >= 1 ? stringList.at(0).toFloat() : 0.0f;
                    const float yPosition = numAttributeDimensions >= 2 ? stringList.at(1).toFloat() : 0.0f;
                    const float zPosition = numAttributeDimensions >= 3 ? stringList.at(2).toFloat() : 0.0f;
                    hifiProps.setPosition(glm::vec3( xPosition, yPosition, zPosition ));
                }

                if (attributes.hasAttribute("rotation")) {
                    qDebug() << "Box - Rotation String: " << attributes.value("rotation").toString();
                    QStringList stringList = attributes.value("rotation").toString().split(QRegExp("\\s+"), QString::SkipEmptyParts);
                    const int numAttributeDimensions = stringList.size();
                    const float xRotation = numAttributeDimensions >= 1 ? stringList.at(0).toFloat() : 0.0f;
                    const float yRotation = numAttributeDimensions >= 2 ? stringList.at(1).toFloat() : 0.0f;
                    const float zRotation = numAttributeDimensions >= 3 ? stringList.at(2).toFloat() : 0.0f;
                    hifiProps.setRotation(glm::quat(glm::vec3(xRotation,yRotation,zRotation)));
                }

                if (hifiProps.getClientOnly()) {
                    auto nodeList = DependencyManager::get<NodeList>();
                    const QUuid myNodeID = nodeList->getSessionUUID();
                    hifiProps.setOwningAvatarID(myNodeID);
                }

                qDebug() << "-------------------------------------------------";
                hifiProps.debugDump();
                qDebug() << "*************************************************";
                qDebug() << "*************************************************";
                qDebug() << hifiProps;
                qDebug() << "-------------------------------------------------";

                m_propData.push_back(hifiProps);
                return true;
            }
        }
    }

    if (m_reader.hasError()) {
        qDebug() << "AFrameReader::read encountered error: " << m_reader.errorString();
    }

    return false;
}