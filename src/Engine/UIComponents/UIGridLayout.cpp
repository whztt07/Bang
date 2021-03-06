#include "Bang/UIGridLayout.h"

#include "Bang/Array.h"
#include "Bang/Array.tcc"
#include "Bang/ClassDB.h"
#include "Bang/GameObject.h"
#include "Bang/ILayoutController.h"
#include "BangMath/Math.h"
#include "BangMath/Rect.h"
#include "Bang/RectTransform.h"
#include "Bang/UILayoutManager.h"
#include "BangMath/Vector2.h"

using namespace Bang;

UIGridLayout::UIGridLayout()
{
    SET_INSTANCE_CLASS_ID(UIGridLayout)
}

UIGridLayout::~UIGridLayout()
{
}

void UIGridLayout::ApplyLayout(Axis axis)
{
    BANG_UNUSED(axis);
    Array<GameObject *> children =
        UILayoutManager::GetLayoutableChildrenList(GetGameObject());

    uint i = 0;
    // const int numRows = GetNumRows();
    const int numColumns = GetNumColumns();
    for (GameObject *child : children)
    {
        RectTransform *childRT = child->GetRectTransform();
        childRT->SetAnchorX(Vector2(-1));
        childRT->SetAnchorY(Vector2(1));

        Vector2i currentIndex((i % numColumns), (i / numColumns));
        Vector2i currentTopLeft = currentIndex * GetCellSize();
        currentTopLeft += currentIndex * GetSpacing();
        currentTopLeft += Vector2i(GetPaddingLeft(), GetPaddingTop());

        childRT->SetMarginLeft(currentTopLeft.x);
        childRT->SetMarginTop(currentTopLeft.y);
        childRT->SetMarginRight(-childRT->GetMarginLeft() - GetCellSize().x);
        childRT->SetMarginBot(-childRT->GetMarginTop() - GetCellSize().y);

        ++i;
    }
}

void UIGridLayout::CalculateLayout(Axis axis)
{
    Vector2i minSize = Vector2i::Zero();
    Vector2i prefSize = GetCellSize() * Vector2i(GetNumColumns(), GetNumRows());
    prefSize += GetTotalSpacing();
    prefSize += GetPaddingSize();
    SetCalculatedLayout(axis, minSize.GetAxis(axis), prefSize.GetAxis(axis));
}

void UIGridLayout::SetCellSize(const Vector2i &cellSize)
{
    if (cellSize != GetCellSize())
    {
        m_cellSize = cellSize;
        ILayoutController::Invalidate();
    }
}

const Vector2i &UIGridLayout::GetCellSize() const
{
    return m_cellSize;
}

int UIGridLayout::GetNumRows() const
{
    Array<GameObject *> children =
        UILayoutManager::GetLayoutableChildrenList(GetGameObject());
    const int numColumns = GetNumColumns();
    if (children.Size() == 0 || numColumns == 0)
    {
        return 0;
    }
    return numColumns > 0 ? ((children.Size() - 1) / numColumns + 1) : 0;
}

int UIGridLayout::GetNumColumns() const
{
    RectTransform *rt = GetGameObject()->GetRectTransform();
    float effectiveWidth =
        (rt->GetViewportRect().GetSize().x - GetPaddingSize().x);

    int cellSizeSpaced = (GetCellSize().x + GetSpacing());

    int numCols = cellSizeSpaced > 0
                      ? int(effectiveWidth + GetSpacing()) / cellSizeSpaced
                      : 0;
    return Math::Max(numCols, 1);
}

Vector2i UIGridLayout::GetTotalSpacing() const
{
    return GetSpacing() * Vector2i(GetNumColumns() - 1, GetNumRows() - 1);
}
