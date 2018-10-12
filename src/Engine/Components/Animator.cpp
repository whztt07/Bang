#include "Bang/Animator.h"

#include "Bang/Mesh.h"
#include "Bang/Material.h"
#include "Bang/MetaNode.h"
#include "Bang/Transform.h"
#include "Bang/GameObject.h"
#include "Bang/ShaderProgram.h"
#include "Bang/SkinnedMeshRenderer.h"
#include "Bang/AnimatorStateMachineNode.h"
#include "Bang/AnimatorStateMachinePlayer.h"
#include "Bang/AnimatorStateMachineConnection.h"

USING_NAMESPACE_BANG

Animator::Animator()
{
    CONSTRUCT_CLASS_ID(Animator);
    m_animationStateMachinePlayer = new AnimatorStateMachinePlayer();
}

Animator::~Animator()
{
    delete m_animationStateMachinePlayer;
}

void Animator::OnStart()
{
    Component::OnStart();

    m_prevFrameTime = Time::GetNow();
    m_animationTime.SetNanos(0);

    if (GetPlayOnStart())
    {
        Play();
    }
}

void Animator::OnUpdate()
{
    Component::OnUpdate();

    Time now = Time::GetNow();
    Time passedTime = (now - m_prevFrameTime);
    m_prevFrameTime = now;

    AnimatorStateMachine *sm = GetStateMachine();
    if (sm && IsPlaying())
    {
        ASSERT( GetPlayer() );
        GetPlayer()->Step(passedTime);

        if (AnimatorStateMachineNode *currentNode = GetPlayer()->GetCurrentNode())
        {
            Map< String, Matrix4 > boneNameToCurrentMatrices;
            Animation *currentAnim = currentNode->GetAnimation();
            if (currentAnim)
            {
                Time currentNodeTime = GetPlayer()->GetCurrentNodeTime();
                boneNameToCurrentMatrices = currentAnim->
                     GetBoneAnimationMatricesForTime(currentNodeTime);
            }
            SetSkinnedMeshRendererCurrentBoneMatrices(boneNameToCurrentMatrices);
        }
    }
}

void Animator::SetStateMachine(AnimatorStateMachine *stateMachine)
{
    if (stateMachine != GetStateMachine())
    {
        m_stateMachine.Set(stateMachine);
        GetPlayer()->SetStateMachine( GetStateMachine() );
    }
}

void Animator::SetPlayOnStart(bool playOnStart)
{
    if (playOnStart != GetPlayOnStart())
    {
        m_playOnStart = playOnStart;
    }
}

void Animator::SetSkinnedMeshRendererCurrentBoneMatrices(
                                const Map<String, Matrix4> &boneAnimMatrices)
{
    Array<SkinnedMeshRenderer*> smrs =
                        GetGameObject()->GetComponents<SkinnedMeshRenderer>();
    for (SkinnedMeshRenderer *smr : smrs)
    {
        for (const auto &pair : boneAnimMatrices)
        {
            const String &boneName = pair.first;
            const Matrix4 &boneAnimMatrix = pair.second;
            GameObject *boneGo = smr->GetBoneGameObject(boneName);
            if (boneGo && smr->GetActiveMesh()->
                          GetBonesPool().ContainsKey(boneName))
            {
                boneGo->GetTransform()->FillFromMatrix( boneAnimMatrix );
            }
        }
    }

    for (SkinnedMeshRenderer *smr : smrs)
    {
        smr->UpdateBonesMatricesFromTransformMatrices();
    }
}

Map<String, Matrix4> Animator::GetBoneAnimationMatrices(Animation *animation,
                                                        Time animationTime)
{
    Map< String, Matrix4 > boneNameToAnimationMatrices;
    if (animation)
    {
        boneNameToAnimationMatrices =
                animation->GetBoneAnimationMatricesForTime(animationTime);
    }
    return boneNameToAnimationMatrices;
}

Map<String, Matrix4> Animator::GetBoneCrossFadeAnimationMatrices(
                                                Animation *prevAnimation,
                                                Time prevAnimationTime,
                                                Animation *nextAnimation,
                                                Time currentCrossFadeTime,
                                                Time totalCrossFadeTime)
{
    Map<String, Matrix4> boneCrossFadeAnimationMatrices =
            Animation::GetBoneCrossFadeAnimationMatrices(prevAnimation,
                                                         prevAnimationTime,
                                                         nextAnimation,
                                                         currentCrossFadeTime,
                                                         totalCrossFadeTime);
    return boneCrossFadeAnimationMatrices;
}

void Animator::Play()
{
    m_playing = true;
}

void Animator::Stop()
{
    m_playing = false;
    m_animationTime.SetNanos(0);
}

void Animator::Pause()
{
    m_playing = false;
}

bool Animator::IsPlaying() const
{
    return m_playing;
}

bool Animator::GetPlayOnStart() const
{
    return m_playOnStart;
}

AnimatorStateMachine* Animator::GetStateMachine() const
{
    return m_stateMachine.Get();
}

void Animator::CloneInto(ICloneable *clone) const
{
    Component::CloneInto(clone);

    Animator *animatorClone = SCAST<Animator*>(clone);
    animatorClone->SetPlayOnStart( GetPlayOnStart() );
    animatorClone->SetStateMachine( GetStateMachine() );
}

void Animator::ImportMeta(const MetaNode &metaNode)
{
    Component::ImportMeta(metaNode);

    if (metaNode.Contains("PlayOnStart"))
    {
        SetPlayOnStart( metaNode.Get<bool>("PlayOnStart") );
    }

    if (metaNode.Contains("StateMachine"))
    {
        SetStateMachine( Resources::Load<AnimatorStateMachine>(
                             metaNode.Get<GUID>("StateMachine") ).Get() );
    }
}

void Animator::ExportMeta(MetaNode *metaNode) const
{
    Component::ExportMeta(metaNode);

    metaNode->Set("StateMachine",
                  GetStateMachine() ? GetStateMachine()->GetGUID() :
                                      GUID::Empty());
    metaNode->Set("PlayOnStart", GetPlayOnStart());
}

AnimatorStateMachinePlayer *Animator::GetPlayer() const
{
    return m_animationStateMachinePlayer;
}
