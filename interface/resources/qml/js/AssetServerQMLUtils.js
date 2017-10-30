.pragma library //< Should only ever have a single instance of this script when imported with QML file


// @note: Entity Export Section
var SHAPE_TYPE_NONE = 0;
var SHAPE_TYPE_SIMPLE_HULL = 1;
var SHAPE_TYPE_SIMPLE_COMPOUND = 2;
var SHAPE_TYPE_STATIC_MESH = 3;
var SHAPE_TYPE_BOX = 4;
var SHAPE_TYPE_SPHERE = 5;

var SHAPE_TYPE_DEFAULT = SHAPE_TYPE_STATIC_MESH;
var DYNAMIC_DEFAULT = false;

var SHAPE_TYPES = [];
SHAPE_TYPES[SHAPE_TYPE_NONE] = "none";
SHAPE_TYPES[SHAPE_TYPE_SIMPLE_HULL] = "simple-hull";
SHAPE_TYPES[SHAPE_TYPE_SIMPLE_COMPOUND] = "simple-compound";
SHAPE_TYPES[SHAPE_TYPE_STATIC_MESH] = "static-mesh";
SHAPE_TYPES[SHAPE_TYPE_BOX] = "box";
SHAPE_TYPES[SHAPE_TYPE_SPHERE] = "sphere";

var COLLISION_TYPES = [];
COLLISION_TYPES[SHAPE_TYPE_NONE] = "No Collision";
COLLISION_TYPES[SHAPE_TYPE_SIMPLE_HULL] = "Basic - Whole model";
COLLISION_TYPES[SHAPE_TYPE_SIMPLE_COMPOUND] = "Good - Sub-meshes";
COLLISION_TYPES[SHAPE_TYPE_STATIC_MESH] = "Exact - All polygons";
COLLISION_TYPES[SHAPE_TYPE_BOX] = "Box";
COLLISION_TYPES[SHAPE_TYPE_SPHERE] = "Sphere";

function getDefaultShape() {
    return SHAPE_TYPES[SHAPE_TYPE_DEFAULT];
}

function getDefaultCollision() {
    return COLLISION_TYPES[SHAPE_TYPE_DEFAULT];
}

function getShapeType(shapeType) {
    if (shapeType === null || shapeType === undefined || typeof(shapeType) !== "number"){
        print("ERROR: AssetServerQMLUtils::getShapeType - Invalid arg specified for shapeType: " + shapeType);
        //--EARLY EXIT--
        return SHAPE_TYPES[SHAPE_TYPE_NONE];
    }

    if ((shapeType < 0) || (shapeType >= SHAPE_TYPES.length)) {
        print("ERROR: AssetServerQMLUtils::getShapeType - Specified invalid/unknown type: " + shapeType);
        //--EARLY EXIT--
        return SHAPE_TYPES[SHAPE_TYPE_NONE];
    }

    return SHAPE_TYPES[shapeType];
}

function getCollisionType(shapeType) {
    if (shapeType === null || shapeType === undefined || typeof(shapeType) !== "number"){
        print("ERROR: AssetServerQMLUtils::getCollisionType - Invalid arg specified for shapeType: " + shapeType);
        //--EARLY EXIT--
        return COLLISION_TYPES[SHAPE_TYPE_NONE];
    }

    if ((shapeType < 0) || (shapeType >= COLLISION_TYPES.length)) {
        print("ERROR: AssetServerQMLUtils::getCollisionType - Specified invalid/unknown type: " + shapeType);
        //--EARLY EXIT--
        return COLLISION_TYPES[SHAPE_TYPE_NONE];
    }

    return COLLISION_TYPES[shapeType];
}