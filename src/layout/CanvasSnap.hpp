#pragma once
#include "../defines.hpp"
#include <vector>

Vector2D resolveOverlap(const Vector2D& droppedPos, const Vector2D& droppedSize, const std::vector<CBox>& otherWindows);
std::vector<Vector2D> computeGridLayout(int windowCount, const Vector2D& viewportCenter, const Vector2D& viewportSize, const Vector2D& cellSize);
