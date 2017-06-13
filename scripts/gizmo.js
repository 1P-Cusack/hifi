//Worklist Item #21391:  Parenting Tool
//Gizmo:  Parent one object to another.
(function () {
    var GizmoState = {
        IDLE: 0,                //!< The tool currently isn't active/in use.
        VET_CHILD: 1,           //!< The tool has touched an object and needs to ascertain if the object can be edited or parented. 
        HAS_CHILD: 2,           //!< The tool currently has a valid child object, and is awaiting potential parent.
        ATTEMPT_PARENTING: 3,   //!< The tool has a valid child object and has touched a potential parent. 
        PARENTED_CHILD: 4       //!< The tool has parented the child object and after its cool down period will return to Idle. 
    };

    //! The amount of time required prior to being able to parent another
    //! set of objects.
    var PARENTING_COOL_DOWN_SECONDS = 5;

    var current_state = GizmoState.IDLE;
    var target_childID;
    var target_parent;

    function isValidChild( childProps ) {
        switch (childProps.type) {
            case "Light":
            case "Zone":
            case "ParticleEffect":
                return false;
            default:
                return true;
        }

        // Should never get here, paranoia return
        return false;
    }

    function evalCurState() {
        switch (current_state) {
            case GizmoState.IDLE:
                if (target_childID !== undefined)
                {
                    //--EARLY EXIT--( Shouldn't be here with valid child object )
                    break;
                }
                else if ( (entityID === undefined) || entityID.isInvalidID() )
                {
                    //--EARLY EXIT--( Aren't called due to a viable reason )
                    break;
                }
                var candidateProps = Entities.getEntityProperties(entityID);
                // TODO: Detect if the user has edit rights? How?
                
                if ( isValidChild(candidateProps) ) {
                    target_childID = entityID;
                    current_state = GizmoState.HAS_CHILD
                }
                break;
            case GizmoState.VET_CHILD:
                break;
            case GizmoState.HAS_CHILD:
        }
    }

    function goToNextState() {

    }

    //! Responds to clicking on an entity
    //! @param entityID:  EntityItemID of item clicked on.
    //! @param mouseEvent: ?JavaScript MouseEvent?
    function handleClickReleaseOnEntity(entityID, mouseEvent) {
        // TODO: Do stuff with the clicked entity
    }

    //! @param colliderID:  EntityItemID of the owner of the collision event. (this.entityID)
    //! @param collideeID:  EntityItemID of the object which was collided with.
    //! @param collisionData: Collision object which houses the information of the collision event
    function handlCollisionWithEntity(colliderID, collideeID, collisionData) {
        // TODO: Do stuff with the data
    }

    //--------------------------------
    //! Map internal functions for use by client entity (self/this)
    //--------------------------------
    this.clickReleaseOnEntity = handleClickReleaseOnEntity;
    this.collisionWithEntity = handlCollisionWithEntity;
})