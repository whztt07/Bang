#include "Bang/Mesh.h"

#include <GL/glew.h>

#include "Bang/IBO.h"
#include "Bang/VAO.h"
#include "Bang/VBO.h"
#include "Bang/Debug.h"
#include "Bang/ModelIO.h"
#include "Bang/Material.h"
#include "Bang/Triangle.h"
#include "Bang/MetaNode.h"
#include "Bang/Resources.h"
#include "Bang/MeshSimplifier.h"
#include "Bang/MetaFilesManager.h"

USING_NAMESPACE_BANG

Mesh::Mesh() : m_bBox(Vector3::Zero)
{
    m_vao = new VAO();
}

Mesh::~Mesh()
{
    if (m_vao)
    {
        delete m_vao;
    }

    if (m_vertexIndicesIBO)
    {
        delete m_vertexIndicesIBO;
    }

    if (m_vertexAttributesVBO)
    {
        delete m_vertexAttributesVBO;
    }
}

void Mesh::SetVertexIndices(const Array<Mesh::VertexId> &faceIndices)
{
    m_vertexIndices = faceIndices;

    if (m_vertexIndicesIBO)
    {
        delete m_vertexIndicesIBO;
    }

    m_vertexIndicesIBO = new IBO();
    m_vertexIndicesIBO->Fill((void*)(&m_vertexIndices[0]),
                             m_vertexIndices.Size() * sizeof(Mesh::VertexId));

    m_vao->SetIBO(m_vertexIndicesIBO); // Bind to VAO

    m_areLodsValid = false;
}

void Mesh::SetBonesIndices(const Map<String, uint> &bonesIndices)
{
    m_bonesIndices = bonesIndices;
    for (const auto &pair : bonesIndices)
    {
        m_indexToBone.Add(pair.second, pair.first);
    }
}

void Mesh::SetPositionsPool(const Array<Vector3>& positions)
{
    m_positionsPool = positions;
    m_bBox.CreateFromPositions(m_positionsPool);
    m_bSphere.FromBox(m_bBox);
    m_areLodsValid = false;
}

void Mesh::SetNormalsPool(const Array<Vector3> &normals)
{
    m_normalsPool = normals;
    m_areLodsValid = false;
}

void Mesh::SetUvsPool(const Array<Vector2> &uvs)
{
    m_uvsPool = uvs;
    m_areLodsValid = false;
}

void Mesh::SetTangentsPool(const Array<Vector3> &tangents)
{
    m_tangentsPool = tangents;
    m_areLodsValid = false;
}

void Mesh::SetBonesPool(const Map<String, Mesh::Bone> &bones)
{
    m_bonesPool = bones;
}

void Mesh::UpdateVAOs()
{
    if (m_vertexAttributesVBO)
    {
        delete m_vertexAttributesVBO;
    }
    m_vertexAttributesVBO = new VBO();

    bool hasPos      = !GetPositionsPool().IsEmpty();
    bool hasNormals  = !GetNormalsPool().IsEmpty();
    bool hasUvs      = !GetUvsPool().IsEmpty();
    bool hasTangents = !GetTangentsPool().IsEmpty();
    bool hasBones    = !GetBonesPool().IsEmpty();

    if (hasBones)
    {
        // Pick 4 most relevant bones per vertex
        for (VertexId vid = 0; vid < GetVertexIndices().Size(); ++vid)
        {
            struct BoneExt : public IToString
            {
                float weight;
                uint id;
                String ToString() const override
                {
                    return "Bone(" + String::ToString(id) + ": " +
                                     String::ToString(weight) + ")";
                }
            };

            Array<BoneExt> vertexBonesExt;
            for (const auto &it : GetBonesPool())
            {
                const String &boneName = it.first;
                const Mesh::Bone &bone = it.second;
                if (bone.weights.ContainsKey(vid))
                {
                    ASSERT(GetBonesIndices().ContainsKey(boneName));

                    BoneExt boneExt;
                    boneExt.weight = bone.weights.Get(vid);
                    boneExt.id     = GetBonesIndices().Get(boneName);
                    vertexBonesExt.PushBack(boneExt);
                }
            }

            m_vertexIdToImportantBonesIndicesPool.Add(vid,
                                       std::array<int, 4>({{0,0,0,0}}));
            m_vertexIdToImportantBonesWeightsPool.Add(vid,
                                       // std::array<float, 4>({{0,0,0,0}}));
                                        std::array<float, 4>({{0,0,0,0}}));

            if (!vertexBonesExt.IsEmpty())
            {
                Containers::Sort(vertexBonesExt.Begin(),
                                 vertexBonesExt.End(),
                         [](const BoneExt &lhs, const BoneExt &rhs) -> bool
                         {
                             return lhs.weight > rhs.weight;
                         });

                int numImportantBones = Math::Min(4u, vertexBonesExt.Size());

                float weightSum = 0.0f;
                for (int i = 0; i < numImportantBones; ++i)
                {
                    weightSum += vertexBonesExt[i].weight; // To norm later
                }

                for (int i = 0; i < numImportantBones; ++i)
                {
                    m_vertexIdToImportantBonesIndicesPool.Get(vid)[i] =
                                        vertexBonesExt[i].id;
                    m_vertexIdToImportantBonesWeightsPool.Get(vid)[i] =
                                        vertexBonesExt[i].weight / weightSum;
                }

                // std::cerr << "ind: " << m_vertexIdToImportantBonesIndicesPool.Get(vid) << std::endl;
                // std::cerr << "wei: " << m_vertexIdToImportantBonesWeightsPool.Get(vid) << std::endl;
            }
        }
    }

    Array<float> interleavedAttributes;
    for (int i = 0; i < GetPositionsPool().Size(); ++i)
    {
        if (i < GetPositionsPool().Size())
        {
            const Vector3 &position = GetPositionsPool()[i];
            interleavedAttributes.PushBack(position.x);
            interleavedAttributes.PushBack(position.y);
            interleavedAttributes.PushBack(position.z);
        }

        if (i < GetNormalsPool().Size())
        {
            const Vector3 &normal = GetNormalsPool()[i];
            interleavedAttributes.PushBack(normal.x);
            interleavedAttributes.PushBack(normal.y);
            interleavedAttributes.PushBack(normal.z);
        }

        if (i < GetUvsPool().Size())
        {
            const Vector2 &uv = GetUvsPool()[i];
            interleavedAttributes.PushBack(uv.x);
            interleavedAttributes.PushBack(uv.y);
        }

        if (i < GetTangentsPool().Size())
        {
            const Vector3 &tangent = GetTangentsPool()[i];
            interleavedAttributes.PushBack(tangent.x);
            interleavedAttributes.PushBack(tangent.y);
            interleavedAttributes.PushBack(tangent.z);
        }

        if (i < m_vertexIdToImportantBonesIndicesPool.Size())
        {
            const auto &vertexIdToImportantBonesIndices =
                    m_vertexIdToImportantBonesIndicesPool.Get(i);
            interleavedAttributes.PushBack(vertexIdToImportantBonesIndices[0]);
            interleavedAttributes.PushBack(vertexIdToImportantBonesIndices[1]);
            interleavedAttributes.PushBack(vertexIdToImportantBonesIndices[2]);
            interleavedAttributes.PushBack(vertexIdToImportantBonesIndices[3]);
        }

        if (i < m_vertexIdToImportantBonesWeightsPool.Size())
        {
            const auto &vertexIdToImportantBonesWeights =
                    m_vertexIdToImportantBonesWeightsPool.Get(i);
            interleavedAttributes.PushBack(vertexIdToImportantBonesWeights[0]);
            interleavedAttributes.PushBack(vertexIdToImportantBonesWeights[1]);
            interleavedAttributes.PushBack(vertexIdToImportantBonesWeights[2]);
            interleavedAttributes.PushBack(vertexIdToImportantBonesWeights[3]);
        }
    }

    if (interleavedAttributes.Size() >= 1)
    {
        GetVertexAttributesVBO()->CreateAndFill(
                               SCAST<void*>(&interleavedAttributes[0]),
                               interleavedAttributes.Size() * sizeof(float));
    }

    int vboStride = GetVBOStride();

    if (hasPos)
    {
        GetVAO()->SetVBO(GetVertexAttributesVBO(),
                         Mesh::DefaultPositionsVBOLocation,
                         3,
                         GL::VertexAttribDataType::FLOAT,
                         false,
                         vboStride,
                         GetVBOPositionsOffset());
    }

    if (hasNormals)
    {
        GetVAO()->SetVBO(GetVertexAttributesVBO(),
                         Mesh::DefaultNormalsVBOLocation,
                         3,
                         GL::VertexAttribDataType::FLOAT,
                         true,
                         vboStride,
                         GetVBONormalsOffset());
    }

    if (hasUvs)
    {
        GetVAO()->SetVBO(GetVertexAttributesVBO(),
                         Mesh::DefaultUvsVBOLocation,
                         2,
                         GL::VertexAttribDataType::FLOAT,
                         false,
                         vboStride,
                         GetVBOUvsOffset());
    }

    if (hasTangents)
    {
        GetVAO()->SetVBO(GetVertexAttributesVBO(),
                         Mesh::DefaultTangentsVBOLocation,
                         3,
                         GL::VertexAttribDataType::FLOAT,
                         false,
                         vboStride,
                         GetVBOTangentsOffset());
    }

    if (hasBones)
    {
        GetVAO()->SetVBO(GetVertexAttributesVBO(),
                         Mesh::DefaultVertexToBonesIndicesVBOLocation,
                         4,
                         GL::VertexAttribDataType::FLOAT,
                         false,
                         vboStride,
                         GetVBOBonesIndicesOffset());

        GetVAO()->SetVBO(GetVertexAttributesVBO(),
                         Mesh::DefaultVertexToBonesWeightsVBOLocation,
                         4,
                         GL::VertexAttribDataType::FLOAT,
                         false,
                         vboStride,
                         GetVBOBonesWeightsOffset());
    }
}

void Mesh::CalculateVertexNormals()
{
    UMap<VertexId, Array<TriangleId>> vertexIndexToTriIndices =
                                GetVertexIndicesToTriangleIndices();
    Array<Vector3> normalsPool;
    for (int vi = 0; vi < GetVertexIndices().Size(); ++vi)
    {
        Vector3 vNormal = Vector3::Zero;
        const Array<TriangleId> &vTriIds = vertexIndexToTriIndices.Get(vi);
        for (TriangleId vTriId : vTriIds)
        {
            std::array<Mesh::VertexId, 3> triVertexIds =
                                        GetTriangleVertexIndices(vTriId);
            Triangle tri = Triangle(GetPositionsPool()[triVertexIds[0]],
                                    GetPositionsPool()[triVertexIds[1]],
                                    GetPositionsPool()[triVertexIds[2]]);
            vNormal += tri.GetNormal();
        }
        vNormal /= float(vTriIds.Size());
        normalsPool.PushBack(vNormal);
    }
    SetNormalsPool(normalsPool);
}

void Mesh::CalculateLODs()
{
    if (!m_areLodsValid)
    {
        m_lodMeshes = MeshSimplifier::GetAllMeshLODs(this,
                                 // MeshSimplifier::Method::CLUSTERING);
                                 MeshSimplifier::Method::QUADRIC_ERROR_METRICS);
        m_areLodsValid = true;
    }
}

int Mesh::GetNumLODs() const
{
    return GetLODMeshes().Size();
}

RH<Mesh> Mesh::GetLODMesh(int lod) const
{
    if (GetLODMeshes().IsEmpty())
    {
        return RH<Mesh>(const_cast<Mesh*>(this));
    }

    const int clampedLODLevel = Math::Clamp(lod, 0, GetNumLODs()-1);
    return GetLODMeshes()[clampedLODLevel];
}

const Array<RH<Mesh> > Mesh::GetLODMeshes() const
{
    return m_lodMeshes;
}

uint Mesh::GetNumTriangles() const
{
    return SCAST<uint>(GetVertexIndices().Size() / 3);
}

String Mesh::GetBoneName(uint boneIndex) const
{
    if (m_indexToBone.ContainsKey(boneIndex))
    {
        return m_indexToBone.Get(boneIndex);
    }
    return "";
}

uint Mesh::GetBoneIndex(const String &boneName) const
{
    if (m_bonesIndices.ContainsKey(boneName))
    {
        return m_bonesIndices.Get(boneName);
    }
    return SCAST<uint>(-1);
}

Triangle Mesh::GetTriangle(uint triIndex) const
{
    ASSERT(triIndex < GetNumTriangles());
    VertexId vId0 = GetVertexIndices()[triIndex * 3 + 0];
    VertexId vId1 = GetVertexIndices()[triIndex * 3 + 1];
    VertexId vId2 = GetVertexIndices()[triIndex * 3 + 2];

    Triangle tri;
    tri.SetPoint(0, GetPositionsPool()[vId0]);
    tri.SetPoint(1, GetPositionsPool()[vId1]);
    tri.SetPoint(2, GetPositionsPool()[vId2]);
    return tri;
}

std::array<Mesh::VertexId, 3> Mesh::GetTriangleVertexIndices(uint triIndex) const
{
    ASSERT(triIndex < GetNumTriangles());
    const VertexId triVertex0Index = GetVertexIndices()[triIndex * 3 + 0];
    const VertexId triVertex1Index = GetVertexIndices()[triIndex * 3 + 1];
    const VertexId triVertex2Index = GetVertexIndices()[triIndex * 3 + 2];
    return {{triVertex0Index, triVertex1Index, triVertex2Index}};
}

int Mesh::GetNumVertices() const
{
    return GetVertexIndicesIBO() ? GetVertexIndices().Size() :
                                   GetPositionsPool().Size();
}

uint Mesh::GetPositionsBytesSize() const
{
    return GetPositionsPool().Size() >= 1 ? (3 * sizeof(float)) : 0;
}

uint Mesh::GetNormalsBytesSize() const
{
    return GetNormalsPool().Size() >= 1 ? (3 * sizeof(float)) : 0;
}

uint Mesh::GetUvsBytesSize() const
{
    return GetUvsPool().Size() >= 1 ? (2 * sizeof(float)) : 0;
}

uint Mesh::GetTangentsBytesSize() const
{
    return GetTangentsPool().Size() >= 1 ? (3 * sizeof(float)) : 0;
}

uint Mesh::GetBonesIndicesBytesSize() const
{
    return GetBonesPool().Size() >= 1 ? (4 * sizeof(float)) : 0;
}

uint Mesh::GetBonesWeightsBytesSize() const
{
    return GetBonesPool().Size() >= 1 ? (4 * sizeof(float)) : 0;
}

uint Mesh::GetVBOPositionsOffset() const
{
    return 0;
}

uint Mesh::GetVBONormalsOffset() const
{
    return GetVBOPositionsOffset() + GetPositionsBytesSize();
}

uint Mesh::GetVBOUvsOffset() const
{
    return GetVBONormalsOffset() + GetNormalsBytesSize();
}

uint Mesh::GetVBOTangentsOffset() const
{
    return GetVBOUvsOffset() + GetUvsBytesSize();
}

uint Mesh::GetVBOBonesIndicesOffset() const
{
    return GetVBOTangentsOffset() + GetTangentsBytesSize();
}

uint Mesh::GetVBOBonesWeightsOffset() const
{
    return GetVBOBonesIndicesOffset() + GetBonesWeightsBytesSize();
}

uint Mesh::GetVBOStride() const
{
    return GetPositionsBytesSize() +
           GetNormalsBytesSize() +
           GetUvsBytesSize() +
           GetTangentsBytesSize() +
           GetBonesIndicesBytesSize() +
           GetBonesWeightsBytesSize();
}


VAO *Mesh::GetVAO() const
{
    return m_vao;
}
IBO *Mesh::GetVertexIndicesIBO() const
{
    return m_vertexIndicesIBO;
}
VBO *Mesh::GetVertexAttributesVBO() const
{
    return m_vertexAttributesVBO;
}
const AABox &Mesh::GetAABBox() const
{
    return m_bBox;
}
const Sphere &Mesh::GetBoundingSphere() const
{
    return m_bSphere;
}
const Map<String, Mesh::Bone> &Mesh::GetBonesPool() const
{
    return m_bonesPool;
}
const Array<Mesh::VertexId> &Mesh::GetVertexIndices() const
{
    return m_vertexIndices;
}
const Array<Vector3> &Mesh::GetPositionsPool() const
{
    return m_positionsPool;
}
const Array<Vector3> &Mesh::GetNormalsPool() const
{
    return m_normalsPool;
}
const Array<Vector2> &Mesh::GetUvsPool() const
{
    return m_uvsPool;
}
const Array<Vector3> &Mesh::GetTangentsPool() const
{
    return m_tangentsPool;
}
const Map<String, uint> &Mesh::GetBonesIndices() const
{
    return m_bonesIndices;
}

UMap<Mesh::VertexId, Array<Mesh::TriangleId> >
Mesh::GetVertexIndicesToTriangleIndices() const
{
    UMap<VertexId, Array<Mesh::TriangleId>> vertexIndicesToTriIndices;
    for (int ti = 0; ti < GetNumTriangles(); ++ti)
    {
        std::array<Mesh::VertexId, 3> tiVerticesIds = GetTriangleVertexIndices(ti);
        for (Mesh::VertexId tivi : tiVerticesIds)
        {
            if (!vertexIndicesToTriIndices.ContainsKey(tivi))
            {
                vertexIndicesToTriIndices.Add(tivi, {});
            }
            vertexIndicesToTriIndices.Get(tivi).PushBack(ti);
        }
    }
    return vertexIndicesToTriIndices;
}

void Mesh::CloneInto(ICloneable *clone) const
{
    Mesh *mClone = SCAST<Mesh*>(clone);

    mClone->m_bBox = m_bBox;
    mClone->m_bSphere = m_bSphere;

    if (mClone->m_vao)
    {
        delete mClone->m_vao;
    }

    mClone->m_vao = new VAO();
    mClone->SetPositionsPool( GetPositionsPool() );
    mClone->SetNormalsPool( GetNormalsPool() );
    mClone->SetUvsPool( GetUvsPool() );
    mClone->SetTangentsPool( GetTangentsPool() );
    mClone->SetBonesPool( GetBonesPool() );
    mClone->SetVertexIndices( GetVertexIndices() );
    mClone->SetBonesIndices( GetBonesIndices() );
    mClone->UpdateVAOs();
}

void Mesh::Import(const Path &meshFilepath)
{
    ASSERT_MSG(false, "Load the Model, and from there retrieve the mesh. "
                      "Or use MeshFactory.");
    BANG_UNUSED(meshFilepath);
}


void Mesh::ImportMeta(const MetaNode &metaNode)
{
    Asset::ImportMeta(metaNode);
}

void Mesh::ExportMeta(MetaNode *metaNode) const
{
    Asset::ExportMeta(metaNode);
}
