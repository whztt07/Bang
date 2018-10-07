#include "Bang/AnimatorStateMachineConnection.h"

#include "Bang/AnimatorStateMachine.h"
#include "Bang/AnimatorStateMachineNode.h"

USING_NAMESPACE_BANG

AnimatorStateMachineConnection::AnimatorStateMachineConnection()
{
}

AnimatorStateMachineConnection::~AnimatorStateMachineConnection()
{
}

void AnimatorStateMachineConnection::SetNodeToIndex(uint nodeToIdx)
{
    m_nodeToIndex = nodeToIdx;
}
void AnimatorStateMachineConnection::SetNodeFromIndex(uint nodeFromIdx)
{
    m_nodeFromIndex = nodeFromIdx;
}

void AnimatorStateMachineConnection::SetNodeTo(AnimatorStateMachineNode *nodeTo)
{
    SetNodeToIndex( GetSMNodeIdx(nodeTo) );
}
void AnimatorStateMachineConnection::SetNodeFrom(AnimatorStateMachineNode *nodeFrom)
{
    SetNodeFromIndex( GetSMNodeIdx(nodeFrom) );
}

uint AnimatorStateMachineConnection::GetNodeToIndex() const
{
    return m_nodeToIndex;
}

uint AnimatorStateMachineConnection::GetNodeFromIndex() const
{
    return m_nodeFromIndex;
}

AnimatorStateMachineNode *AnimatorStateMachineConnection::GetNodeTo() const
{
    return GetSMNode(GetNodeToIndex());
}

AnimatorStateMachineNode *AnimatorStateMachineConnection::GetNodeFrom() const
{
    return GetSMNode(GetNodeFromIndex());
}

AnimatorStateMachine *AnimatorStateMachineConnection::GetStateMachine() const
{
    return p_stateMachine;
}

AnimatorStateMachineNode*
AnimatorStateMachineConnection::GetSMNode(uint idx) const
{
    if (GetStateMachine())
    {
        if (idx < GetStateMachine()->GetNodes().Size())
        {
            return &(GetStateMachine()->GetNodes()[idx]);
        }
    }
    return nullptr;
}

uint AnimatorStateMachineConnection::GetSMNodeIdx(AnimatorStateMachineNode *node) const
{
    for (uint i = 0; i < GetStateMachine()->GetNodes().Size(); ++i)
    {
        if (node == &(GetStateMachine()->GetNodes()[i]))
        {
            return i;
        }
    }
    return -1u;
}

void AnimatorStateMachineConnection::SetAnimatorStateMachine(
                                            AnimatorStateMachine *stateMachine)
{
    p_stateMachine = stateMachine;
}
