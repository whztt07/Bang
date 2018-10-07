#include "Bang/AnimatorStateMachineNode.h"

USING_NAMESPACE_BANG

AnimatorStateMachineNode::AnimatorStateMachineNode()
{
}

AnimatorStateMachineNode::~AnimatorStateMachineNode()
{
}

void AnimatorStateMachineNode::SetName(const String &name)
{
    m_name = name;
}

AnimatorStateMachineConnection*
AnimatorStateMachineNode::CreateConnection(uint nodeToIdx)
{

}

AnimatorStateMachineConnection*
AnimatorStateMachineNode::CreateConnection(AnimatorStateMachineNode *nodeTo)
{
    AnimatorStateMachineConnection newConnection;
    newConnection.SetNodeTo(nodeTo);
    return AddConnection(newConnection);
}

const AnimatorStateMachineConnection *AnimatorStateMachineNode::GetConnection(
                                                    uint connectionIdx) const
{
    return const_cast<AnimatorStateMachineNode*>(this)->
           GetConnection(connectionIdx);
}

AnimatorStateMachineConnection *AnimatorStateMachineNode::GetConnection(
                                                    uint connectionIdx)
{
    if (connectionIdx < GetConnections().Size())
    {
        return &(m_connections[connectionIdx]);
    }
    return nullptr;
}

void AnimatorStateMachineNode::RemoveConnection(uint connectionIdx)
{
    ASSERT(connectionIdx < GetConnections().Size());
    m_connections.RemoveByIndex(connectionIdx);
}

AnimatorStateMachineConnection *AnimatorStateMachineNode::AddConnection(
                                AnimatorStateMachineConnection newConnection)
{
    newConnection.SetAnimatorStateMachine( GetStateMachine() );
    ASSERT(newConnection.GetNodeFrom() == this);
    ASSERT(newConnection.GetNodeTo() != nullptr);
    ASSERT(newConnection.GetNodeTo() != this);
    m_connections.PushBack(newConnection);
    return &m_connections.Back();
}

void AnimatorStateMachineNode::SetAnimatorStateMachine(
                                    AnimatorStateMachine *stateMachine)
{
    p_stateMachine = stateMachine;
}

void AnimatorStateMachineNode::SetAnimation(Animation *animation)
{
    p_animation.Set( animation );
}

const String &AnimatorStateMachineNode::GetName() const
{
    return m_name;
}

Array<AnimatorStateMachineConnection*>
AnimatorStateMachineNode::GetConnectionsTo(AnimatorStateMachineNode *nodeTo)
{
    Array<AnimatorStateMachineConnection*> connectionsToNode;
    for (AnimatorStateMachineConnection connection : GetConnections())
    {
        if (connection.GetNodeTo() == nodeTo)
        {
            connectionsToNode.PushBack(&connection);
        }
    }
    return connectionsToNode;
}
Array<const AnimatorStateMachineConnection*>
AnimatorStateMachineNode::GetConnectionsTo(AnimatorStateMachineNode *nodeTo) const
{
    Array<AnimatorStateMachineConnection*> connectionsTo =
        const_cast<AnimatorStateMachineNode*>(this)->GetConnectionsTo(nodeTo);

    Array<const AnimatorStateMachineConnection*> connectionsToConst;
    for (AnimatorStateMachineConnection *conn : connectionsTo)
    {
        connectionsToConst.PushBack(conn);
    }

    return connectionsToConst;
}


Animation *AnimatorStateMachineNode::GetAnimation() const
{
    return p_animation.Get();
}

AnimatorStateMachine *AnimatorStateMachineNode::GetStateMachine() const
{
    return p_stateMachine;
}

const Array<AnimatorStateMachineConnection>&
AnimatorStateMachineNode::GetConnections() const
{
    return m_connections;
}
