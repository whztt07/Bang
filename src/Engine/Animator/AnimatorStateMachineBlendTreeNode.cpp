#include "Bang/AnimatorStateMachineBlendTreeNode.h"

#include "Bang/AnimatorStateMachine.h"
#include "Bang/Resources.h"

using namespace Bang;

AnimatorStateMachineBlendTreeNode::AnimatorStateMachineBlendTreeNode()
{
}

AnimatorStateMachineBlendTreeNode::~AnimatorStateMachineBlendTreeNode()
{
}

void AnimatorStateMachineBlendTreeNode::SetSecondAnimation(
    Animation *secondAnimation)
{
    p_secondAnimation.Set(secondAnimation);
}

void AnimatorStateMachineBlendTreeNode::SetSecondAnimationSpeed(
    float secondAnimationSpeed)
{
    m_secondAnimationSpeed = secondAnimationSpeed;
}

void AnimatorStateMachineBlendTreeNode::SetBlendVariableName(
    const String &blendVariableName)
{
    m_blendVariableName = blendVariableName;
}

const String &AnimatorStateMachineBlendTreeNode::GetBlendVariableName() const
{
    return m_blendVariableName;
}

float AnimatorStateMachineBlendTreeNode::GetSecondAnimationSpeed() const
{
    return m_secondAnimationSpeed;
}

Map<String, Animation::BoneTransformation>
AnimatorStateMachineBlendTreeNode::GetBoneTransformations(
    Time animationTime) const
{
    float secondWeight =
        GetStateMachine()->GetVariableFloat(GetBlendVariableName());
    secondWeight = Math::Clamp(secondWeight, 0.0f, 1.0f);

    Map<String, Animation::BoneTransformation> firstBoneTransformations =
        Animation::GetBoneAnimationTransformations(GetAnimation(),
                                                   animationTime);
    float normalizedTime =
        animationTime.GetSeconds() /
        Math::Max(GetAnimation()->GetDurationInSeconds(), 0.01f);

    Map<String, Animation::BoneTransformation> blended;
    if (GetSecondAnimation())
    {
        Time secondAnimationTime = Time::Seconds(
            normalizedTime * GetSecondAnimation()->GetDurationInSeconds());
        secondAnimationTime *= GetSecondAnimationSpeed();
        Map<String, Animation::BoneTransformation> secondBoneTransformations =
            Animation::GetBoneAnimationTransformations(GetSecondAnimation(),
                                                       secondAnimationTime);
        blended = Animation::GetInterpolatedBoneTransformations(
            firstBoneTransformations, secondBoneTransformations, secondWeight);
    }
    else
    {
        blended = firstBoneTransformations;
    }
    return blended;
}

Animation *AnimatorStateMachineBlendTreeNode::GetSecondAnimation() const
{
    return p_secondAnimation.Get();
}

void AnimatorStateMachineBlendTreeNode::CloneInto(ICloneable *clone) const
{
    AnimatorStateMachineNode::CloneInto(clone);

    AnimatorStateMachineBlendTreeNode *btNode =
        SCAST<AnimatorStateMachineBlendTreeNode *>(clone);
    btNode->SetSecondAnimationSpeed(GetSecondAnimationSpeed());
    btNode->SetBlendVariableName(GetBlendVariableName());
    btNode->SetSecondAnimation(GetSecondAnimation());
}

void AnimatorStateMachineBlendTreeNode::ImportMeta(const MetaNode &metaNode)
{
    AnimatorStateMachineNode::ImportMeta(metaNode);

    if (metaNode.Contains("BlendVariableName"))
    {
        SetBlendVariableName(metaNode.Get<String>("BlendVariableName"));
    }

    if (metaNode.Contains("SecondAnimation"))
    {
        SetSecondAnimation(
            Resources::Load<Animation>(metaNode.Get<GUID>("SecondAnimation"))
                .Get());
    }

    if (metaNode.Contains("SecondAnimationSpeed"))
    {
        SetSecondAnimationSpeed(metaNode.Get<float>("SecondAnimationSpeed"));
    }
}

void AnimatorStateMachineBlendTreeNode::ExportMeta(MetaNode *metaNode) const
{
    AnimatorStateMachineNode::ExportMeta(metaNode);

    metaNode->Set("BlendVariableName", GetBlendVariableName());
    metaNode->Set("SecondAnimationSpeed", GetSecondAnimationSpeed());
    metaNode->Set(
        "SecondAnimation",
        GetSecondAnimation() ? GetSecondAnimation()->GetGUID() : GUID::Empty());
}
