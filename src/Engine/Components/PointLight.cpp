#include "Bang/PointLight.h"

#include "Bang/GL.h"
#include "Bang/AABox.h"
#include "Bang/Scene.h"
#include "Bang/AARect.h"
#include "Bang/Camera.h"
#include "Bang/Gizmos.h"
#include "Bang/Sphere.h"
#include "Bang/GEngine.h"
#include "Bang/XMLNode.h"
#include "Bang/Material.h"
#include "Bang/Texture2D.h"
#include "Bang/Transform.h"
#include "Bang/GLUniforms.h"
#include "Bang/GameObject.h"
#include "Bang/Framebuffer.h"
#include "Bang/MeshRenderer.h"
#include "Bang/TextureFactory.h"
#include "Bang/ShaderProgramFactory.h"

USING_NAMESPACE_BANG

PointLight::PointLight() : Light()
{
    m_shadowMapFramebuffer = new Framebuffer();
    m_shadowMapFramebuffer->CreateAttachmentTexCubeMap(GL::Attachment::DEPTH,
                                                       GL::ColorFormat::DEPTH16);

    GL::Push(GL::BindTarget::TEXTURE_2D);
    GetShadowMapTexture()->Bind();
    GetShadowMapTexture()->SetFilterMode(GL::FilterMode::BILINEAR);
    GL::TexParameteri( GetShadowMapTexture()->GetTextureTarget(),
                       GL::TexParameter::TEXTURE_COMPARE_MODE,
                       GL_COMPARE_REF_TO_TEXTURE );
    GL::Pop(GL::BindTarget::TEXTURE_2D);

    m_shadowMapMaterial = Resources::Create<Material>();
    m_shadowMapMaterial.Get()->SetShaderProgram(
                ShaderProgramFactory::GetPointLightShadowMap() );
    SetLightScreenPassShaderProgram(
                ShaderProgramFactory::GetPointLightScreenPass());
}

PointLight::~PointLight()
{
    delete m_shadowMapFramebuffer;
}

void PointLight::SetUniformsBeforeApplyingLight(ShaderProgram* sp) const
{
    Light::SetUniformsBeforeApplyingLight(sp);

    ASSERT(GL::IsBound(sp));
    sp->SetFloat("B_LightRange", GetRange(), false);
    sp->SetFloat("B_PointLightZFar", GetLightZFar(), false);
}

AARect PointLight::GetRenderRect(Camera *cam) const
{
    Transform *tr = GetGameObject()->GetTransform();
    Sphere sphere(tr->GetPosition(), Math::Pow(GetRange(), 2.0f) );
    AABox bbox = AABox::FromSphere(sphere);
    return cam->GetViewportBoundingAARectNDC(bbox);
}

void PointLight::SetRange(float range) { m_range = range; }
float PointLight::GetRange() const { return m_range; }

TextureCubeMap *PointLight::GetShadowMapTexture() const
{
    return m_shadowMapFramebuffer->GetAttachmentTexCubeMap(GL::Attachment::DEPTH);
}

void PointLight::OnRender(RenderPass rp)
{
    Component::OnRender(rp);
    if (rp == RenderPass::OVERLAY) // Render gizmos
    {
        Gizmos::Reset();
        Gizmos::SetColor(GetColor().WithAlpha(1.0f));
        Gizmos::SetSelectable(GetGameObject());
        Gizmos::SetPosition( GetGameObject()->GetTransform()->GetPosition() );
        Gizmos::SetScale( Vector3(0.1f) );
        Gizmos::RenderIcon( TextureFactory::GetLightBulbIcon().Get(), true );
    }
}

void PointLight::RenderShadowMaps_()
{
    GL::Push(GL::Pushable::VIEWPORT);
    GL::Push(GL::Pushable::COLOR_MASK);
    GL::Push(GL::Pushable::ALL_MATRICES);
    GL::Push(GL::Pushable::DEPTH_STATES);
    GL::Push(GL::BindTarget::SHADER_PROGRAM);
    GL::Push(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);

    // Resize stuff to fit the shadow map size
    const Vector2i shadowMapSize = GetShadowMapSize();
    m_shadowMapFramebuffer->Bind();
    m_shadowMapFramebuffer->Resize(shadowMapSize.x, shadowMapSize.y);
    m_shadowMapFramebuffer->SetAllDrawBuffers();

    // Set up viewport
    GL::SetViewport(0, 0, shadowMapSize.x, shadowMapSize.y);

    m_shadowMapMaterial.Get()->Bind();
    SetUniformsBeforeApplyingLight(m_shadowMapMaterial.Get()->GetShaderProgram());

    Array<Matrix4> cubeMapPVMMatrices = GetWorldToShadowMapMatrices();
    m_shadowMapMaterial.Get()->GetShaderProgram()->
            SetMatrix4Array("B_WorldToShadowMapMatrices",
                            cubeMapPVMMatrices, false);

    // Render shadow map into framebuffer
    GEngine::GetInstance()->SetReplacementMaterial( m_shadowMapMaterial.Get() );
    GL::SetColorMask(false, false, false, false);
    GL::ClearDepthBuffer(1.0f);
    GL::SetDepthFunc(GL::Function::LEQUAL);

    float rangeLimit = Math::Pow(GetRange(), 1.0f);
    const Vector3 pointLightPos = GetGameObject()->GetTransform()->GetPosition();
    const List<GameObject*> shadowCasters = GetActiveSceneShadowCasters();
    for (GameObject *shadowCaster : shadowCasters)
    {
        AABox shadowCasterAABoxWorld = shadowCaster->GetAABBoxWorld(false);
        Vector3 closestPointInAABox = shadowCasterAABoxWorld.
                                           GetClosestPointInAABB(pointLightPos);
        bool isCompletelyOutside = Vector3::Distance(closestPointInAABox,
                                                     pointLightPos) > rangeLimit;
        if (!isCompletelyOutside)
        {
            GEngine::GetInstance()->RenderWithPass(shadowCaster,
                                                 RenderPass::SCENE, false);
        }
    }

    GL::Pop(GL::Pushable::FRAMEBUFFER_AND_READ_DRAW_ATTACHMENTS);
    GL::Pop(GL::BindTarget::SHADER_PROGRAM);
    GL::Pop(GL::Pushable::DEPTH_STATES);
    GL::Pop(GL::Pushable::ALL_MATRICES);
    GL::Pop(GL::Pushable::COLOR_MASK);
    GL::Pop(GL::Pushable::VIEWPORT);
    GEngine::GetInstance()->SetReplacementMaterial(nullptr);
}

float PointLight::GetLightZFar() const
{
    return GetRange();
}

Array<Matrix4> PointLight::GetWorldToShadowMapMatrices() const
{
    Array<Matrix4> cubeMapPVMMatrices;

    const Transform *tr = GetGameObject()->GetTransform();
    const Vector3 pos  = tr->GetPosition();
    const Matrix4 pers = Matrix4::Perspective(Math::DegToRad(90.0f),
                                              1.0f,
                                              0.05f,
                                              GetLightZFar());
    const Vector3 up = Vector3::Up, down = Vector3::Down, left = Vector3::Left,
          right = Vector3::Right, fwd = Vector3::Forward, back = Vector3::Back;
    cubeMapPVMMatrices.Resize(6);
    cubeMapPVMMatrices[0] = pers * Matrix4::LookAt(pos, (pos + right), down);
    cubeMapPVMMatrices[1] = pers * Matrix4::LookAt(pos, (pos + left),  down);
    cubeMapPVMMatrices[2] = pers * Matrix4::LookAt(pos, (pos + up),    back);
    cubeMapPVMMatrices[3] = pers * Matrix4::LookAt(pos, (pos + down),  fwd);
    cubeMapPVMMatrices[4] = pers * Matrix4::LookAt(pos, (pos + back),  down);
    cubeMapPVMMatrices[5] = pers * Matrix4::LookAt(pos, (pos + fwd),   down);

    return cubeMapPVMMatrices;
}

void PointLight::CloneInto(ICloneable *clone) const
{
    Light::CloneInto(clone);
    PointLight *pl = Cast<PointLight*>(clone);
    pl->SetRange(GetRange());
}

void PointLight::ImportXML(const XMLNode &xmlInfo)
{
    Light::ImportXML(xmlInfo);
    if (xmlInfo.Contains("Range"))
    { SetRange(xmlInfo.Get<float>("Range")); }
}

void PointLight::ExportXML(XMLNode *xmlInfo) const
{
    Light::ExportXML(xmlInfo);
    xmlInfo->Set("Range", GetRange());
}
