//
//  Created by Bradley Austin Davis on 2016/05/09
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "RenderableShapeEntityItem.h"

#include <glm/gtx/quaternion.hpp>

#include <gpu/Batch.h>
#include <DependencyManager.h>
#include <StencilMaskPass.h>
#include <GeometryCache.h>
#include <PerfStat.h>

#include <render-utils/simple_vert.h>
#include <render-utils/simple_frag.h>

//#define SHAPE_ENTITY_USE_FADE_EFFECT
#ifdef SHAPE_ENTITY_USE_FADE_EFFECT
#include <FadeEffect.h>
#endif
using namespace render;
using namespace render::entities;

// Sphere entities should fit inside a cube entity of the same size, so a sphere that has dimensions 1x1x1 
// is a half unit sphere.  However, the geometry cache renders a UNIT sphere, so we need to scale down.
static const float SPHERE_ENTITY_SCALE = 0.5f;


ShapeEntityRenderer::ShapeEntityRenderer(const EntityItemPointer& entity) : Parent(entity) {
    _procedural._vertexSource = simple_vert;
    _procedural._fragmentSource = simple_frag;
    _procedural._opaqueState->setCullMode(gpu::State::CULL_NONE);
    _procedural._opaqueState->setDepthTest(true, true, gpu::LESS_EQUAL);
    PrepareStencil::testMaskDrawShape(*_procedural._opaqueState);
    _procedural._opaqueState->setBlendFunction(false,
        gpu::State::SRC_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::INV_SRC_ALPHA,
        gpu::State::FACTOR_ALPHA, gpu::State::BLEND_OP_ADD, gpu::State::ONE);
}

bool ShapeEntityRenderer::needsRenderUpdate() const {
    if (_procedural.isEnabled() && _procedural.isFading()) {
        return true;
    }

    return Parent::needsRenderUpdate();
}

bool ShapeEntityRenderer::needsRenderUpdateFromTypedEntity(const TypedEntityPointer& entity) const {
    if (_lastUserData != entity->getUserData()) {
        return true;
    }
    glm::vec4 newColor(toGlm(entity->getXColor()), entity->getLocalRenderAlpha());
    if (newColor != _color) {
        return true;
    }

    return false;
}

void ShapeEntityRenderer::doRenderUpdateSynchronousTyped(const ScenePointer& scene, Transaction& transaction, const TypedEntityPointer& entity) {
    withWriteLock([&] {
        auto userData = entity->getUserData();
        if (_lastUserData != userData) {
            _lastUserData = userData;
            _procedural.setProceduralData(ProceduralData::parse(_lastUserData));
        }

        _color = vec4(toGlm(entity->getXColor()), entity->getLocalRenderAlpha());
    });
}

void ShapeEntityRenderer::doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) {
    if (_procedural.isEnabled() && _procedural.isFading()) {
        float isFading = Interpolate::calculateFadeRatio(_procedural.getFadeStartTime()) < 1.0f;
        _procedural.setIsFading(isFading);
    }

    _shape = entity->getShape();
    _position = entity->getPosition();
    _dimensions = entity->getDimensions();
    _orientation = entity->getOrientation();

    if (_shape == entity::Sphere) {
        _modelTransform.postScale(SPHERE_ENTITY_SCALE);
    }

    _modelTransform.postScale(_dimensions);
}

bool ShapeEntityRenderer::isTransparent() const {
    if (_procedural.isEnabled() && _procedural.isFading()) {
        return Interpolate::calculateFadeRatio(_procedural.getFadeStartTime()) < 1.0f;
    } 
    
    //        return _entity->getLocalRenderAlpha() < 1.0f || Parent::isTransparent();
    return Parent::isTransparent();
}



void ShapeEntityRenderer::doRender(RenderArgs* args) {
    PerformanceTimer perfTimer("RenderableShapeEntityItem::render");
    Q_ASSERT(args->_batch);

    gpu::Batch& batch = *args->_batch;

    auto geometryCache = DependencyManager::get<GeometryCache>();
    auto geometryShape = geometryCache->getShapeForEntityShape(_shape);
    batch.setModelTransform(_modelTransform); // use a transform with scale, rotation, registration point and translation

    bool proceduralRender = false;
    glm::vec4 outColor = _color;
    withReadLock([&] {
        if (_procedural.isReady()) {
            _procedural.prepare(batch, _position, _dimensions, _orientation);
            auto outColor = _procedural.getColor(_color);
            outColor.a *= _procedural.isFading() ? Interpolate::calculateFadeRatio(_procedural.getFadeStartTime()) : 1.0f;
            proceduralRender = true;
        }
    });

    if (proceduralRender) {
        batch._glColor4f(outColor.r, outColor.g, outColor.b, outColor.a);
        if (render::ShapeKey(args->_globalShapeKey).isWireframe()) {
            geometryCache->renderWireShape(batch, geometryShape);
        } else {
            geometryCache->renderShape(batch, geometryShape);
        }
    } else {
        // FIXME, support instanced multi-shape rendering using multidraw indirect
        _color.a *= _isFading ? Interpolate::calculateFadeRatio(_fadeStartTime) : 1.0f;
        auto pipeline = _color.a < 1.0f ? geometryCache->getTransparentShapePipeline() : geometryCache->getOpaqueShapePipeline();
        if (render::ShapeKey(args->_globalShapeKey).isWireframe()) {
            geometryCache->renderWireShapeInstance(args, batch, geometryShape, _color, pipeline);
        } else {
            geometryCache->renderSolidShapeInstance(args, batch, geometryShape, _color, pipeline);
        }
    }

    static const auto triCount = geometryCache->getShapeTriangleCount(geometryShape);
    args->_details._trianglesRendered += (int)triCount;
}
