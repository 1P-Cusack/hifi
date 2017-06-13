//var SCRIPT_URL = "http://hifi-production.s3.amazonaws.com/tutorials/entity_scripts/flashlight.js";
var MODEL_URL = Script.resolvePath("Parent-Tool-Production.fbx");

var orientation = MyAvatar.orientation;
orientation = Quat.safeEulerAngles(orientation);
orientation.x = 0;
orientation = Quat.fromVec3Degrees(orientation);
var center = Vec3.sum(MyAvatar.getHeadPosition(), Vec3.multiply(2, Quat.getForward(orientation)));

var gizmo = Entities.addEntity({
    type: "Model",
    name: 'Gizmo',
    modelURL: MODEL_URL,
    position: center,
    dimensions: {
        x: 0.5,//0.739,
        y: 0.5,//1.613,
        z: 2.529
    },
    gravity: {
        x: 0,
        y: 0,
        z: 0
    },
    dynamic: false,
    shapeType: 'compound',
    collisionless: false,
    userData: JSON.stringify({
        grabbableKey: {
            grabbable: true
        }
    })
});

print("Ran Gizmo");
Script.stop();