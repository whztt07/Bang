#include "Bang/Gizmos.h"

#include "Bang/GL.h"
#include "Bang/Mesh.h"
#include "Bang/Math.h"
#include "Bang/Rect.h"
#include "Bang/AABox.h"
#include "Bang/Color.h"
#include "Bang/Scene.h"
#include "Bang/AARect.h"
#include "Bang/Camera.h"
#include "Bang/GBuffer.h"
#include "Bang/GEngine.h"
#include "Bang/Vector3.h"
#include "Bang/Vector4.h"
#include "Bang/Renderer.h"
#include "Bang/Material.h"
#include "Bang/Transform.h"
#include "Bang/Texture2D.h"
#include "Bang/Quaternion.h"
#include "Bang/GameObject.h"
#include "Bang/MeshFactory.h"
#include "Bang/MeshRenderer.h"
#include "Bang/LineRenderer.h"
#include "Bang/SceneManager.h"
#include "Bang/ShaderProgram.h"
#include "Bang/MaterialFactory.h"
#include "Bang/GameObjectFactory.h"
#include "Bang/SelectionFramebuffer.h"
#include "Bang/ShaderProgramFactory.h"

USING_NAMESPACE_BANG

Gizmos::Gizmos()
{
    m_gizmosGo = GameObjectFactory::CreateGameObject();
    m_gizmosGo->SetName("Gizmos");

    m_boxMesh = Resources::Clone<Mesh>(MeshFactory::GetCube());
    m_planeMesh = Resources::Clone<Mesh>(MeshFactory::GetUIPlane());
    m_sphereMesh = Resources::Clone<Mesh>(MeshFactory::GetSphere());
    m_outlineShaderProgram = ShaderProgramFactory::Get(
                ShaderProgramFactory::GetScreenPassVertexShaderPath(),
                EPATH("Shaders/GizmosOutline.frag"));

    m_lineRenderer = m_gizmosGo->AddComponent<LineRenderer>();
    m_meshRenderer = m_gizmosGo->AddComponent<MeshRenderer>();
    m_renderers    = m_gizmosGo->GetComponents<Renderer>();

    for (Renderer *rend : m_renderers)
    {
        rend->SetMaterial(MaterialFactory::GetGizmosUnLightedOverlay().Get());
    }

    m_gizmosGo->Start();
    m_gizmosGo->GetHideFlags().SetOn(HideFlag::DontSerialize);
}

Gizmos::~Gizmos()
{
    GameObject::Destroy(m_gizmosGo);
}

void Gizmos::SetColor(const Color &color)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    for (Renderer *rend : g->m_renderers)
    {
        rend->GetMaterial()->SetAlbedoColor(color);
    }
}

void Gizmos::SetCulling(bool culling)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    for (Renderer *rend : g->m_renderers)
    {
        rend->SetCulling(culling);
    }
}

void Gizmos::SetPosition(const Vector3 &position)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    g->m_gizmosGo->GetTransform()->SetPosition(position);
}

void Gizmos::SetRotation(const Quaternion &rotation)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    g->m_gizmosGo->GetTransform()->SetRotation(rotation);
}

void Gizmos::SetScale(const Vector3 &scale)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    g->m_gizmosGo->GetTransform()->SetLocalScale(scale);
}

void Gizmos::SetRenderPass(RenderPass rp)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    for (Renderer *rend : g->m_renderers)
    {
        rend->GetActiveMaterial()->SetRenderPass(rp);
    }
}

void Gizmos::SetSelectable(GameObject *go)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    g->p_selectable = go;
}

void Gizmos::SetThickness(float thickness)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    for (Renderer *rend : g->m_renderers)
    {
        rend->SetLineWidth(thickness);
    }
}

void Gizmos::SetRenderWireframe(bool wireframe)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    for (Renderer *rend : g->m_renderers)
    {
        rend->SetRenderWireframe(wireframe);
    }
}

void Gizmos::SetReceivesLighting(bool receivesLighting)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    for (Renderer *rend : g->m_renderers)
    {
        rend->GetActiveMaterial()->SetReceivesLighting(receivesLighting);
    }
}

void Gizmos::RenderCustomMesh(Mesh *m)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    g->m_meshRenderer->SetMesh(m);
    Render(g->m_meshRenderer);
}


void Gizmos::RenderSimpleBox(const AABox &b)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    const Quaternion &r = g->m_gizmosGo->GetTransform()->GetLocalRotation();
    const Vector3& bMin = b.GetMin();
    const Vector3& bMax = b.GetMax();

    RenderLine(r * Vector3(bMin.x, bMin.y, bMin.z), r * Vector3(bMax.x, bMin.y, bMin.z));
    RenderLine(r * Vector3(bMin.x, bMin.y, bMin.z), r * Vector3(bMin.x, bMax.y, bMin.z));
    RenderLine(r * Vector3(bMin.x, bMin.y, bMin.z), r * Vector3(bMin.x, bMin.y, bMax.z));

    RenderLine(r * Vector3(bMax.x, bMin.y, bMin.z), r * Vector3(bMax.x, bMax.y, bMin.z));
    RenderLine(r * Vector3(bMax.x, bMin.y, bMin.z), r * Vector3(bMax.x, bMin.y, bMax.z));

    RenderLine(r * Vector3(bMin.x, bMax.y, bMin.z), r * Vector3(bMax.x, bMax.y, bMin.z));
    RenderLine(r * Vector3(bMin.x, bMax.y, bMin.z), r * Vector3(bMin.x, bMax.y, bMax.z));

    RenderLine(r * Vector3(bMin.x, bMin.y, bMax.z), r * Vector3(bMax.x, bMin.y, bMax.z));
    RenderLine(r * Vector3(bMin.x, bMin.y, bMax.z), r * Vector3(bMin.x, bMax.y, bMax.z));

    RenderLine(r * Vector3(bMin.x, bMax.y, bMax.z), r * Vector3(bMax.x, bMax.y, bMax.z));
    RenderLine(r * Vector3(bMax.x, bMin.y, bMax.z), r * Vector3(bMax.x, bMax.y, bMax.z));
    RenderLine(r * Vector3(bMax.x, bMax.y, bMin.z), r * Vector3(bMax.x, bMax.y, bMax.z));
}

void Gizmos::RenderBox(const AABox &b)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    g->m_meshRenderer->SetMesh(g->m_boxMesh.Get());
    g->m_gizmosGo->GetTransform()->SetPosition(b.GetCenter());
    g->m_gizmosGo->GetTransform()->
       SetScale(g->m_gizmosGo->GetTransform()->GetScale() * b.GetSize());
    g->Render(g->m_meshRenderer);
}

void Gizmos::RenderRectNDC(const AARect &r)
{
    Gizmos::RenderRectNDC({{r.GetMinXMinY(), r.GetMaxXMinY(),
                            r.GetMinXMaxY(), r.GetMaxXMaxY()}});
}

void Gizmos::RenderRectNDC(const RectPoints &rectPointsNDC)
{
    RenderViewportLineNDC(rectPointsNDC[0], rectPointsNDC[1]);
    RenderViewportLineNDC(rectPointsNDC[1], rectPointsNDC[2]);
    RenderViewportLineNDC(rectPointsNDC[2], rectPointsNDC[3]);
    RenderViewportLineNDC(rectPointsNDC[3], rectPointsNDC[0]);
}

void Gizmos::RenderRect(const RectPoints &rectPoints)
{
    RenderRectNDC({{GL::FromViewportPointToViewportPointNDC(rectPoints[0]),
                    GL::FromViewportPointToViewportPointNDC(rectPoints[1]),
                    GL::FromViewportPointToViewportPointNDC(rectPoints[2]),
                    GL::FromViewportPointToViewportPointNDC(rectPoints[3])}});
}

void Gizmos::RenderRect(const Rect &r)
{
    RenderRect({{r.GetLeftBot(), r.GetRightBot(),
                 r.GetLeftTop(), r.GetRightTop()}});
}

void Gizmos::RenderFillRect(const AARect &r)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    g->m_meshRenderer->SetMesh(g->m_planeMesh.Get());

    Gizmos::SetPosition( Vector3(r.GetCenter(), 0) );
    Gizmos::SetScale( Vector3(r.GetSize(), 1) );

    g->m_meshRenderer->SetViewProjMode(GL::ViewProjMode::Canvas);
    g->Render(g->m_meshRenderer);
}

void Gizmos::RenderIcon(Texture2D *texture,
                        bool billboard)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    g->m_meshRenderer->SetMesh(g->m_planeMesh.Get());

    SetRenderWireframe(false);
    SetReceivesLighting(false);
    if (billboard)
    {
        Camera *cam = Camera::GetActive();

        Vector3 camPos = cam->GetGameObject()->GetTransform()->GetPosition();
        float distScale = 1.0f;
        if (cam->GetProjectionMode() == Camera::ProjectionMode::Perspective)
        {
           Vector3 pos = g->m_gizmosGo->GetTransform()->GetPosition();
           distScale = Vector3::Distance(camPos, pos);
        }
        distScale *= 0.5f;

        Vector3 scale = g->m_gizmosGo->GetTransform()->GetScale();
        g->m_gizmosGo->GetTransform()->SetScale(distScale * scale * 8.0f);

        g->m_gizmosGo->GetTransform()->LookInDirection(
                    cam->GetGameObject()->GetTransform()->GetForward(),
                    cam->GetGameObject()->GetTransform()->GetUp());
    }
    g->m_meshRenderer->GetActiveMaterial()->SetAlbedoTexture(texture);
    g->Render(g->m_meshRenderer);
}

void Gizmos::RenderViewportIcon(Texture2D *texture,
                                const AARect &winRect)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    g->m_meshRenderer->SetMesh(g->m_planeMesh.Get());

    Gizmos::SetPosition( Vector3(winRect.GetCenter(), 0) );
    Gizmos::SetScale( Vector3(winRect.GetSize(), 1) );

    g->m_meshRenderer->SetRenderWireframe(false);
    SetReceivesLighting(false);
    g->m_meshRenderer->GetActiveMaterial()->SetAlbedoTexture(texture);
    g->m_meshRenderer->SetViewProjMode(GL::ViewProjMode::Canvas);
    g->Render(g->m_meshRenderer);
}

void Gizmos::RenderLine(const Vector3 &origin, const Vector3 &destiny)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    g->m_lineRenderer->SetPoints( {origin, destiny} );

    g->m_gizmosGo->GetTransform()->SetPosition(Vector3::Zero);
    g->m_gizmosGo->GetTransform()->SetScale(Vector3::One);

    g->m_lineRenderer->SetViewProjMode(GL::ViewProjMode::World);
    g->Render(g->m_lineRenderer);
}

void Gizmos::RenderBillboardCircle(const Vector3 &origin, float radius,
                                   int numSegments)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }

    Camera *cam = Camera::GetActive();
    Vector3 camPos = cam ? cam->GetGameObject()->GetTransform()->GetPosition() :
                           Vector3::Zero;
    g->m_gizmosGo->GetTransform()->LookAt(camPos);
    Gizmos::RenderCircle(origin, radius, numSegments);
}

void Gizmos::RenderCircle(const Vector3 &origin, float radius, int numSegments)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    Gizmos::SetPosition(origin);

    Array<Vector3> circlePoints;
    for (int i = 0; i < numSegments; ++i)
    {
        float angle = ((2 * Math::Pi) / numSegments) * i;
        Vector3 point = Vector3(Math::Cos(angle) * radius,
                                Math::Sin(angle) * radius,
                                0.0f);
        circlePoints.PushBack(point);
    }

    Array<Vector3> circleLinePoints;
    for (int i = 0; i < numSegments; ++i)
    {
        Vector3 p0 = circlePoints[i];
        Vector3 p1 = circlePoints[(i+1) % numSegments];
        circleLinePoints.PushBack(p0);
        circleLinePoints.PushBack(p1);
    }

    g->m_lineRenderer->SetPoints(circleLinePoints);
    g->Render(g->m_lineRenderer);
}

void Gizmos::RenderViewportLineNDC(const Vector2 &origin, const Vector2 &destiny)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    Vector2 originVP  = GL::FromViewportPointNDCToViewportPoint(origin);
    Vector2 destinyVP = GL::FromViewportPointNDCToViewportPoint(destiny);
    g->m_lineRenderer->SetPoints( {Vector3(originVP.x,  originVP.y,  0),
                                   Vector3(destinyVP.x, destinyVP.y, 0)} );

    g->m_gizmosGo->GetTransform()->SetPosition(Vector3::Zero);
    g->m_gizmosGo->GetTransform()->SetScale(Vector3::One);

    g->m_lineRenderer->SetViewProjMode(GL::ViewProjMode::Canvas);
    g->Render(g->m_lineRenderer);
}

void Gizmos::RenderRay(const Vector3 &origin, const Vector3 &rayDir)
{
    Gizmos::RenderLine(origin, origin + rayDir);
}

void Gizmos::RenderSphere(const Vector3 &origin, float radius)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    g->m_meshRenderer->SetMesh(g->m_sphereMesh.Get());
    g->m_gizmosGo->GetTransform()->SetPosition(origin);
    g->m_gizmosGo->GetTransform()->SetScale(radius);
    g->Render(g->m_meshRenderer);
}

void Gizmos::RenderSimpleSphere(const Vector3 &origin,
                                float radius,
                                bool withOutline,
                                int numLoopsVertical,
                                int numLoopsHorizontal,
                                int numCircleSegments)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }

    const float angleAdvVertical = (Math::Pi / numLoopsVertical);
    for (int i = 0; i < numLoopsVertical; ++i)
    {
        Gizmos::SetRotation(
            Quaternion::AngleAxis(Math::Pi / 2, Vector3::Right) *
            Quaternion::AngleAxis(angleAdvVertical * i, Vector3::Right));
        Gizmos::RenderCircle(origin, radius, numCircleSegments);
    }

    const float angleAdvHorizontal = (Math::Pi / numLoopsHorizontal);
    for (int i = 0; i < numLoopsHorizontal; ++i)
    {
        Gizmos::SetRotation(
            Quaternion::AngleAxis(angleAdvHorizontal * i, Vector3::Up));
        Gizmos::RenderCircle(origin, radius, numCircleSegments);
    }

    if (withOutline)
    {
        for (Renderer *r : g->m_renderers) { r->SetEnabled(false); }
        Gizmos::SetPosition( origin );
        Gizmos::SetRotation( Quaternion::Identity );
        Gizmos::SetScale( Vector3(radius) );
        g->m_meshRenderer->SetEnabled(true);
        g->m_meshRenderer->SetMesh( g->m_sphereMesh.Get() );
        Gizmos::RenderOutline( g->m_gizmosGo );
        for (Renderer *r : g->m_renderers) { r->SetEnabled(true); }
    }
}

void Gizmos::RenderOutline(GameObject *gameObject,
                           float alphaDepthOnFade)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }

    // Save state
    bool prevDepthMask                        = GL::GetDepthMask();
    GL::Function prevDepthFunc                = GL::GetDepthFunc();
    Byte prevStencilValue                     = GL::GetStencilValue();
    GL::Function prevStencilFunc              = GL::GetStencilFunc();
    GL::StencilOperation prevStencilOperation = GL::GetStencilOp();
    GLId prevBoundSP = GL::GetBoundId(GL::BindTarget::ShaderProgram);
    GLId prevBoundFB = GL::GetBoundId(GL::BindTarget::Framebuffer);

    GBuffer *gbuffer = GEngine::GetActiveGBuffer();
    if (gbuffer)
    {
        // Save before drawing the first pass, because it could happen that it
        // draws to gizmos and change the gizmos color and thickness.
        const Color outlineColor = g->m_meshRenderer->GetActiveMaterial()->
                                   GetAlbedoColor();
        const float outlineThickness = g->m_meshRenderer->GetLineWidth();

        gbuffer->PushDrawAttachments();
        gbuffer->Bind();

        // Render depth
        gbuffer->SetDrawBuffers({});
        GL::ClearDepthBuffer(1);
        GL::SetDepthMask(true);
        GL::SetDepthFunc(GL::Function::Always);
        GL::SetStencilOp(GL::StencilOperation::Keep);
        GL::SetColorMask(false, false, false, false);
        GEngine::GetActive()->RenderWithAllPasses(gameObject);

        // Render outline
        GL::SetDepthMask(false);
        GL::SetDepthFunc(GL::Function::Always);
        GL::SetColorMask(true, true, true, true);

        ShaderProgram *sp = g->m_outlineShaderProgram.Get();
        sp->Bind();
        sp->SetColor("B_OutlineColor", outlineColor);
        sp->SetInt("B_OutlineThickness", outlineThickness);
        sp->SetFloat("B_AlphaFadeOnDepth", alphaDepthOnFade);
        GBuffer *gbuffer = GEngine::GetActiveGBuffer();
        if (gbuffer)
        {
            sp->SetTexture("B_SceneDepthTexture",
                           gbuffer->GetSceneDepthStencilTexture(), false);
        }

        gbuffer->SetColorDrawBuffer();
        gbuffer->ApplyPass(sp, false);

        GL::SetDepthMask(true);
        GL::ClearDepthBuffer(1);

        gbuffer->PopDrawAttachments();
    }

    // Restore state
    GL::SetDepthMask(prevDepthMask);
    GL::SetDepthFunc(prevDepthFunc);
    GL::SetStencilOp(prevStencilOperation);
    GL::SetStencilFunc(prevStencilFunc);
    GL::SetStencilValue(prevStencilValue);
    GL::Bind(GL::BindTarget::ShaderProgram, prevBoundSP);
    GL::Bind(GL::BindTarget::Framebuffer, prevBoundFB);
}

void Gizmos::RenderFrustum(const Vector3 &forward,
                           const Vector3 &up,
                           const Vector3 &origin,
                           float zNear, float zFar,
                           float fovDegrees, float aspectRatio)
{
    const Vector3 &c = origin;
    const Vector3 right = Vector3::Cross(forward, up).Normalized();

    const float fovH = Math::DegToRad(fovDegrees) / 2.0f;

    Vector3 nearPlaneCenter = c + forward * zNear;
    Vector3 farPlaneCenter  = c + forward * zFar;

    float nearHeight2 = zNear * Math::Tan(fovH);
    float nearWidth2 = nearHeight2 * aspectRatio;
    float farHeight2  = zFar  * Math::Tan(fovH);
    float farWidth2  = farHeight2 * aspectRatio;

    Vector3 nearUpLeft    = nearPlaneCenter - right * nearWidth2 + up * nearHeight2;
    Vector3 nearUpRight   = nearPlaneCenter + right * nearWidth2 + up * nearHeight2;
    Vector3 nearDownRight = nearPlaneCenter + right * nearWidth2 - up * nearHeight2;
    Vector3 nearDownLeft  = nearPlaneCenter - right * nearWidth2 - up * nearHeight2;

    Vector3 farUpLeft     = farPlaneCenter - right * farWidth2 + up * farHeight2;
    Vector3 farUpRight    = farPlaneCenter + right * farWidth2 + up * farHeight2;
    Vector3 farDownRight  = farPlaneCenter + right * farWidth2 - up * farHeight2;
    Vector3 farDownLeft   = farPlaneCenter - right * farWidth2 - up * farHeight2;

    // Near plane
    Gizmos::RenderLine(nearUpLeft   , nearUpRight);
    Gizmos::RenderLine(nearUpRight  , nearDownRight);
    Gizmos::RenderLine(nearDownRight, nearDownLeft);
    Gizmos::RenderLine(nearDownLeft , nearUpLeft);

    // Far plane
    Gizmos::RenderLine(farUpLeft   , farUpRight);
    Gizmos::RenderLine(farUpRight  , farDownRight);
    Gizmos::RenderLine(farDownRight, farDownLeft);
    Gizmos::RenderLine(farDownLeft , farUpLeft);

    // Projection lines
    Gizmos::RenderLine(nearUpLeft   , farUpLeft);
    Gizmos::RenderLine(nearUpRight  , farUpRight);
    Gizmos::RenderLine(nearDownRight, farDownRight);
    Gizmos::RenderLine(nearDownLeft , farDownLeft);
}

void Gizmos::RenderPoint(const Vector3 &point)
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }

    RH<Mesh> rhm = Resources::Create<Mesh>();
    Mesh *m = rhm.Get();
    m->SetPositionsPool( {point} );

    g->m_gizmosGo->GetTransform()->SetPosition(Vector3::Zero);
    g->m_meshRenderer->SetMesh(m);
    g->m_meshRenderer->SetRenderPrimitive(GL::Primitive::Points);

    Render(g->m_meshRenderer);

    g->m_meshRenderer->SetRenderPrimitive(GL::Primitive::Triangles);
}

void Gizmos::Reset()
{
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }

    Gizmos::SetCulling(true);
    Gizmos::SetPosition(Vector3::Zero);
    Gizmos::SetRotation(Quaternion::Identity);
    Gizmos::SetScale(Vector3::One);
    Gizmos::SetColor(Color::White);
    Gizmos::SetThickness(1.0f);
    Gizmos::SetReceivesLighting(false);
    Gizmos::SetRenderWireframe(false);
    Gizmos::SetSelectable(nullptr);
    Gizmos::SetRenderPass(RenderPass::Overlay);

    List<Renderer*> rends = g->m_gizmosGo->GetComponents<Renderer>();
    for (Renderer *rend : rends)
    {
        rend->SetViewProjMode(GL::ViewProjMode::World);
    }

    g->m_meshRenderer->GetActiveMaterial()->SetAlbedoTexture(nullptr);
}

GameObject *Gizmos::GetGameObject() const
{
    return m_gizmosGo;
}

void Gizmos::Render(Renderer *rend)
{
    // Set selectable for SelectionFramebuffer if any was set
    Gizmos *g = Gizmos::GetInstance(); if (!g) { return; }
    SelectionFramebuffer *sfb = GEngine::GetActive()->GetActiveSelectionFramebuffer();
    if (sfb && GL::IsBound(sfb) && g->p_selectable)
    {
        sfb->SetNextRenderSelectable(g->p_selectable);
    }

    // Render!
    rend->OnRender( rend->GetActiveMaterial()->GetRenderPass() );
}

Gizmos* Gizmos::GetInstance()
{
    Scene *scene = SceneManager::GetActiveScene();
    return scene ? scene->GetGizmos() : nullptr;
}
