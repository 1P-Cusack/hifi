//Worklist Item #21391:  Parenting Tool
//Gizmo:  Parent one object to another.
(function () {
    var GizmoState = {
        IDLE: 0,                //!< The tool currently isn't active/in use.
        //TODO(vet child) may be able to drop this state
        VET_CHILD: 1,           //!< The tool has touched an object and needs to ascertain if the object can be edited or parented. 
        HAS_CHILD: 2,           //!< The tool currently has a valid child object, and is awaiting potential parent.
        //TODO:(attempt parenting) may be able to drop this state
        ATTEMPT_PARENTING: 3,   //!< The tool has a valid child object and has touched a potential parent. 
        PARENTED_CHILD: 4       //!< The tool has parented the child object and after its cool down period will return to Idle. 
    };

    //! The amount of time required prior to being able to parent another
    //! set of objects.
    var PARENTING_COOL_DOWN_SECONDS = 5;

    var current_state = GizmoState.IDLE;
    //TODO:  null || undefined, which to use as normalized invalid value...?
    var target_childID;
    var target_child_prevProps;
    var target_parentID;

    function isValidChild(childProps) {
        //! @note: Explicit fallthrough [Light,ParticleEffect]
        switch (childProps.type) {
            case "Light":
            case "Zone":
            case "ParticleEffect":
                return false;
            default:
                //It's a valid type; however, is it currently locked?
                if ( ! childProps.locked ) {
                    return true;
                }

                //..it was locked and thus not viable, inform the user
                //TODO: ^
        }

        // Should never get here, paranoia return
        return false;
    }

    function isValidParent(parentProps) {
        //! @note: Explicit fallthrough [Light,ParticleEffect]
        switch (parentProps.type) {
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
        var candidateProps;
        switch (current_state) {
            case GizmoState.IDLE:
                if (target_childID !== undefined) {
                    //--EARLY EXIT--( Shouldn't be here with valid child object )
                    break;
                }
                else if ((entityID === undefined) || entityID.isInvalidID()) {
                    //--EARLY EXIT--( Aren't called due to a viable reason )
                    break;
                }

                candidateProps = Entities.getEntityProperties(entityID);
                // TODO: Detect if the user has edit rights? How? If they
                // don't have rights notify user regarding this and ignore.

                if (isValidChild(candidateProps)) {
                    target_childID = entityID;

                    // Preserve old prop values in the event of a cancellation
                    target_child_prevProps.parent = candidateProps.parent;
                    target_child_prevProps.dynamic = candidateProps.dynamic;

                    // Unparent the object and disable dynamic property in preparation
                    // for re-parenting.
                    Entities.editEntity(target_childID, { parent: undefined });
                    Entities.editEntity(target_childID, { dynamic: false });

                    // TODO: Notify the user of the next step in the process (TAP THE PARENT)
                    current_state = GizmoState.HAS_CHILD;
                }
                break;
            case GizmoState.VET_CHILD:
                break;
            case GizmoState.HAS_CHILD:
                if (target_parentID !== undefined) {
                    //--EARLY EXIT--( Shouldn't be here with valid parent object )
                    break;
                }
                else if ((entityID === undefined) || entityID.isInvalidID()) {
                    //--EARLY EXIT--( Aren't called due to a viable reason )
                    break;
                }
                // ..is the user cancelling the parenting attempt for the child?
                if (entityID === target_childID) {
                    //TODO: Notify user of successful cancellation of parenting
                    target_childID = undefined;
                    current_state = GizmoState.IDLE;

                    //--EARLY EXIT--(User elected to cancel current parenting session)
                    break;
                }

                candidateProps = Entities.getEntityProperties(entityID);
                if (isValidParent(candidateProps)) {
                    target_parentID = entityID;
                    //TODO: Notifiy the user of the success of the parenting
                    //      Schedule cool down time to go back to idle.
                    Entities.editEntity(target_childID, { parent: target_parentID });
                    current_state = GizmoState.PARENTED_CHILD;
                }

                break;
        }
    }

    function goToNextState() {
        //TODO: Fill in/remove
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
});