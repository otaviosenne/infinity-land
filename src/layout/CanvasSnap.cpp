#include "CanvasSnap.hpp"
#include <cmath>
#include <limits>

static constexpr int    OVERLAP_MAX_ITERATIONS = 10;
static constexpr double GRID_GAP               = 20.0;

static bool overlaps(const CBox& a, const CBox& b) {
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}

Vector2D resolveOverlap(const Vector2D& droppedPos, const Vector2D& droppedSize, const std::vector<CBox>& otherWindows) {
    Vector2D pos = droppedPos;

    for (int iter = 0; iter < OVERLAP_MAX_ITERATIONS; ++iter) {
        bool anyOverlap = false;
        CBox dropped    = {pos.x, pos.y, droppedSize.x, droppedSize.y};

        for (const auto& other : otherWindows) {
            if (!overlaps(dropped, other))
                continue;

            anyOverlap = true;

            const double pushLeft  = other.x - (dropped.x + dropped.w);
            const double pushRight = (other.x + other.w) - dropped.x;
            const double pushUp    = other.y - (dropped.y + dropped.h);
            const double pushDown  = (other.y + other.h) - dropped.y;

            const double absLeft  = std::abs(pushLeft);
            const double absRight = std::abs(pushRight);
            const double absUp    = std::abs(pushUp);
            const double absDown  = std::abs(pushDown);

            const double minPush = std::min({absLeft, absRight, absUp, absDown});

            if (minPush == absLeft)
                pos.x += pushLeft;
            else if (minPush == absRight)
                pos.x += pushRight;
            else if (minPush == absUp)
                pos.y += pushUp;
            else
                pos.y += pushDown;

            dropped = {pos.x, pos.y, droppedSize.x, droppedSize.y};
        }

        if (!anyOverlap)
            break;
    }

    return pos;
}

std::vector<Vector2D> computeGridLayout(int windowCount, const Vector2D& viewportCenter, const Vector2D& viewportSize, const Vector2D& cellSize) {
    if (windowCount <= 0)
        return {};

    const int cols = static_cast<int>(std::ceil(std::sqrt(windowCount)));
    const int rows = static_cast<int>(std::ceil(static_cast<double>(windowCount) / cols));

    const double totalWidth  = cols * cellSize.x + (cols - 1) * GRID_GAP;
    const double totalHeight = rows * cellSize.y + (rows - 1) * GRID_GAP;

    const Vector2D origin = viewportCenter - Vector2D{totalWidth / 2.0, totalHeight / 2.0};

    std::vector<Vector2D> positions;
    positions.reserve(windowCount);

    for (int i = 0; i < windowCount; ++i) {
        const int col = i % cols;
        const int row = i / cols;
        positions.emplace_back(origin.x + col * (cellSize.x + GRID_GAP), origin.y + row * (cellSize.y + GRID_GAP));
    }

    return positions;
}
