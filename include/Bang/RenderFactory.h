#ifndef RENDERFACTORY_H
#define RENDERFACTORY_H

#include "Bang/Texture2D.h"
#include "Bang/GameObject.h"
#include "Bang/EventEmitter.h"
#include "Bang/ShaderProgram.h"
#include "Bang/ResourceHandle.h"

NAMESPACE_BANG_BEGIN

FORWARD   class Material;
FORWARD   class Renderer;
FORWARD   class Texture2D;
FORWARD   class LineRenderer;
FORWARD   class MeshRenderer;

class RenderFactory
{
public:
    struct Parameters
    {
        RH<Texture2D> texture;
        bool wireframe = false;
        float thickness = 1.0f;
        Color color = Color::White;
        bool receivesLighting = false;
        Vector3 scale = Vector3::One;
        Vector3 position = Vector3::Zero;
        Quaternion rotation = Quaternion::Identity;
        GL::CullFaceExt cullFace = GL::CullFaceExt::BACK;
        GL::ViewProjMode viewProjMode = GL::ViewProjMode::WORLD;
    };

    static void RenderCustomMesh(Mesh *m,
                                 const RenderFactory::Parameters &params);
    static void RenderBox(const AABox &b,
                          const RenderFactory::Parameters &params);
    static void RenderSimpleBox(const AABox &b,
                                const RenderFactory::Parameters &params);
    static void RenderRectNDC(const AARect &r,
                              const RenderFactory::Parameters &params);
    static void RenderRectNDC(const RectPoints &rectPointsNDC,
                              const RenderFactory::Parameters &params);
    static void RenderRect(const RectPoints &rectPoints,
                           const RenderFactory::Parameters &params);
    static void RenderRect(const Rect &r,
                           const RenderFactory::Parameters &params);
    static void RenderFillRect(const AARect &r,
                               const RenderFactory::Parameters &params);
    static void RenderIcon(Texture2D *texture,
                           bool billboard,
                           const RenderFactory::Parameters &params);
    static void RenderViewportIcon(Texture2D *texture,
                                   const AARect &winRect,
                                   const RenderFactory::Parameters &params);
    static void RenderViewportLineNDC(const Vector2 &origin,
                                      const Vector2 &destiny,
                                      const RenderFactory::Parameters &params);
    static void RenderLine(const Vector3 &origin,
                           const Vector3 &destiny,
                           const RenderFactory::Parameters &params);
    static void RenderBillboardCircle(float radius,
                                      const RenderFactory::Parameters &params,
                                      int numSegments = 32);
    static void RenderCircle(float radius,
                             const RenderFactory::Parameters &params,
                             int numSegments = 32);
    static void RenderRay(const Vector3 &origin,
                          const Vector3 &rayDir,
                          const RenderFactory::Parameters &params);
    static void RenderSphere(float radius,
                             const RenderFactory::Parameters &params);
    static void RenderSimpleSphere(float radius,
                                   bool withOutline,
                                   const RenderFactory::Parameters &params,
                                   int numLoopsVertical = 4,
                                   int numLoopsHorizontal = 4,
                                   int numCircleSegments = 32);
    static void RenderOutline(GameObject *gameObject,
                              const RenderFactory::Parameters &params,
                              float alphaDepthOnFade = 0.2f);
    static void RenderFrustum(const Vector3 &forward,
                              const Vector3 &up,
                              const Vector3 &origin,
                              float zNear,
                              float zFar,
                              float fovDegrees,
                              float aspectRatio,
                              const RenderFactory::Parameters &params);
    static void RenderPoint(const Vector3 &point,
                            const RenderFactory::Parameters &params);

    GameObject *GetGameObject() const;
    static RenderFactory *GetInstance();

private:
    GameObject *m_gizmosGo = nullptr;

    RH<Mesh> m_boxMesh;
    RH<Mesh> m_planeMesh;
    RH<Mesh> m_sphereMesh;
    RH<ShaderProgram> m_outlineShaderProgram;


    LineRenderer *m_lineRenderer = nullptr;
    MeshRenderer *m_meshRenderer = nullptr;
    List<Renderer*> m_renderers;

    RenderFactory();
    virtual ~RenderFactory();

    static void ApplyRenderParameters(Renderer *rend,
                                      const RenderFactory::Parameters &params);
    static void Render(Renderer *rend,
                       const RenderFactory::Parameters &params);

    friend class Scene;
};

NAMESPACE_BANG_END

#endif // RENDERFACTORY_H