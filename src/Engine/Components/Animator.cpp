#include "Bang/Animator.h"

#include "Bang/Mesh.h"
#include "Bang/Material.h"
#include "Bang/MetaNode.h"
#include "Bang/Transform.h"
#include "Bang/GameObject.h"
#include "Bang/ShaderProgram.h"
#include "Bang/SkinnedMeshRenderer.h"

USING_NAMESPACE_BANG

Animator::Animator()
{
    CONSTRUCT_CLASS_ID(Animator)
}

Animator::~Animator()
{
}

void Animator::OnStart()
{
    Component::OnStart();

    m_prevFrameTimeMillis = Time::GetNow_Millis();
    m_animationTimeSeconds = 0.0f;

    if (GetPlayOnStart())
    {
        Play();
    }
}

void Animator::OnUpdate()
{
    Component::OnUpdate();

    Time::TimeT nowMillis = Time::GetNow_Millis();
    double nowSeconds = double(nowMillis / 1000.0);
    Time::TimeT passedTimeMillis = (nowMillis - m_prevFrameTimeMillis);
    m_prevFrameTimeMillis = Time::GetNow_Millis();

    if (GetCurrentAnimation() && IsPlaying())
    {
        double passedTimeSeconds = (passedTimeMillis / double(1e3));
        m_animationTimeSeconds += passedTimeSeconds *
                                  GetCurrentAnimation()->GetSpeed();

        if (nowSeconds >= m_endCrossFadeTime)
        {
            EndCrossFade();
        }

        Map< String, Matrix4 > boneNameToCurrentMatrices;
        if (GetCurrentAnimation())
        {
            if (GetCurrentTargetCrossFadeAnimation())
            {
                // Cross-fade
                boneNameToCurrentMatrices =
                     Animation::GetBoneCrossFadeAnimationMatrices(
                            GetCurrentAnimation(),
                            m_animationTimeSeconds,
                            GetCurrentTargetCrossFadeAnimation(),
                            (nowSeconds - m_initCrossFadeTime),
                            (m_endCrossFadeTime - m_initCrossFadeTime));
            }
            else
            {
                // Direct
                boneNameToCurrentMatrices = GetCurrentAnimation()->
                     GetBoneAnimationMatricesForSecond(m_animationTimeSeconds);
            }
        }
        SetSkinnedMeshRendererCurrentBoneMatrices(boneNameToCurrentMatrices);
    }
}

void Animator::AddAnimation(Animation *animation, uint index_)
{
    RH<Animation> animationRH(animation);

    uint index = Math::Clamp(index_, 0, GetAnimations().Size());
    p_animations.Insert(animationRH, index);

    const bool setCurrentAnimation = (GetAnimations().Size() == 1);
    if (setCurrentAnimation)
    {
        ChangeCurrentAnimation(0);
    }
}

void Animator::RemoveAnimationByIndex(Animation *animation)
{
    uint index = SCAST<uint>(GetAnimations().IndexOf(RH<Animation>(animation)));
    if (index != -1u)
    {
        RemoveAnimationByIndex(index);
    }
}

void Animator::RemoveAnimationByIndex(uint animationIndex)
{
    ASSERT(animationIndex < GetAnimations().Size());
    if (GetCurrentAnimationIndex() == animationIndex)
    {
        ClearCurrentAnimation();
    }
    p_animations.RemoveByIndex(animationIndex);
}

void Animator::SetAnimation(uint animationIndex, Animation *animation)
{
    ASSERT(animationIndex < GetAnimations().Size());
    p_animations[animationIndex] = RH<Animation>(animation);
}

void Animator::ChangeCurrentAnimation(uint animationIndex)
{
    ASSERT(animationIndex < GetAnimations().Size());
    m_currentAnimationIndex = animationIndex;
    m_currentTargetCrossFadeAnimationIndex = -1u;
}

void Animator::ChangeCurrentAnimationCrossFade(uint animationIndex,
                                               double crossFadeTimeSeconds)
{
    ASSERT(animationIndex < GetAnimations().Size());
    if (animationIndex != GetCurrentAnimationIndex() &&
        animationIndex != GetCurrentTargetCrossFadeAnimationIndex())
    {
        m_currentTargetCrossFadeAnimationIndex = animationIndex;
        double now = (Time::GetNow_Millis() / 1000.0);
        m_initCrossFadeTime = now;
        m_endCrossFadeTime  = now + crossFadeTimeSeconds;
    }
}

void Animator::ClearCurrentAnimation()
{
    m_currentAnimationIndex = -1u;
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
                                                        double animationSeconds)
{
    Map< String, Matrix4 > boneNameToAnimationMatrices;
    if (animation)
    {
        boneNameToAnimationMatrices =
                animation->GetBoneAnimationMatricesForSecond(animationSeconds);
    }
    return boneNameToAnimationMatrices;
}

Map<String, Matrix4> Animator::GetBoneCrossFadeAnimationMatrices(
                                                Animation *prevAnimation,
                                                double prevAnimationSeconds,
                                                Animation *nextAnimation,
                                                double currentCrossFadeSeconds,
                                                double totalCrossFadeSeconds)
{
    Map<String, Matrix4> boneCrossFadeAnimationMatrices =
            Animation::GetBoneCrossFadeAnimationMatrices(prevAnimation,
                                                         prevAnimationSeconds,
                                                         nextAnimation,
                                                         currentCrossFadeSeconds,
                                                         totalCrossFadeSeconds);
    return boneCrossFadeAnimationMatrices;
}

void Animator::Play()
{
    m_playing = true;
}

void Animator::Stop()
{
    m_playing = false;
    m_animationTimeSeconds = 0.0;
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

Animation* Animator::GetAnimation(uint animationIndex) const
{
    if (animationIndex < GetAnimations().Size())
    {
        return p_animations[animationIndex].Get();
    }
    return nullptr;
}

const Array<RH<Animation> > &Animator::GetAnimations() const
{
    return p_animations;
}

void Animator::CloneInto(ICloneable *clone) const
{
    Component::CloneInto(clone);
    Animator *animatorClone = SCAST<Animator*>(clone);
    for (const RH<Animation> &animationRH : GetAnimations())
    {
        animatorClone->AddAnimation( animationRH.Get() );
    }
}

void Animator::ImportMeta(const MetaNode &metaNode)
{
    Component::ImportMeta(metaNode);


    Array<GUID> newAnimationGUIDs = metaNode.GetArray<GUID>("Animations");
    bool someAnimationDiffers = (GetAnimations().Size() != newAnimationGUIDs.Size());
    if (!someAnimationDiffers)
    {
        for (int i = 0; i < GetAnimations().Size(); ++i)
        {
            const GUID &newAnimationGUID = newAnimationGUIDs[i];
            GUID currentAnimationGUID = GUID::Empty();
            if (Animation *currentAnimation = GetAnimations()[i].Get())
            {
                currentAnimationGUID = currentAnimation->GetGUID();
            }

            someAnimationDiffers = (newAnimationGUID != currentAnimationGUID);
            if (someAnimationDiffers)
            {
                break;
            }
        }
    }

    if (someAnimationDiffers)
    {
        // Clear animations
        while (!GetAnimations().IsEmpty())
        {
            RemoveAnimationByIndex(0u);
        }

        // Add all new animations
        for (const GUID &newAnimationGUID : newAnimationGUIDs)
        {
            RH<Animation> newAnimationRH = Resources::Load<Animation>(newAnimationGUID);
            AddAnimation(newAnimationRH.Get());
        }
    }

    if (metaNode.Contains("PlayOnStart"))
    {
        SetPlayOnStart( metaNode.Get<bool>("PlayOnStart") );
    }
}

void Animator::ExportMeta(MetaNode *metaNode) const
{
    Component::ExportMeta(metaNode);

    Array<GUID> animationGUIDs;
    for (const RH<Animation> &animationRH : GetAnimations())
    {
        Animation *animation = animationRH.Get();
        animationGUIDs.PushBack( animation ? animation->GetGUID() : GUID::Empty());
    }
    metaNode->SetArray("Animations", animationGUIDs);

    metaNode->Set("PlayOnStart", GetPlayOnStart());
}

void Animator::EndCrossFade()
{
    m_currentAnimationIndex = GetCurrentTargetCrossFadeAnimationIndex();
    m_currentTargetCrossFadeAnimationIndex = -1u;
    m_animationTimeSeconds = (m_endCrossFadeTime - m_initCrossFadeTime);
    m_initCrossFadeTime = Math::Infinity<double>();
    m_endCrossFadeTime  = Math::Infinity<double>();
}

uint Animator::GetCurrentAnimationIndex() const
{
    return m_currentAnimationIndex;
}

Animation *Animator::GetCurrentAnimation() const
{
    return GetAnimation( GetCurrentAnimationIndex() );
}

uint Animator::GetCurrentTargetCrossFadeAnimationIndex() const
{
    return m_currentTargetCrossFadeAnimationIndex;
}

Animation *Animator::GetCurrentTargetCrossFadeAnimation() const
{
    if (GetCurrentTargetCrossFadeAnimationIndex() <= GetAnimations().Size())
    {
        return GetAnimations()[GetCurrentTargetCrossFadeAnimationIndex()].Get();
    }
    return nullptr;
}

