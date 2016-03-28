#pragma once

#include <unordered_map>

#include <glm/glm.hpp>

struct KeyHash {
    std::size_t operator()(const glm::ivec2& v) const {
        long x = v.x;
        long y = v.y;
        return std::hash<long>()(x + (y << 8));
    }
    std::size_t operator()(const glm::ivec4& v) const {
        long x = v.x;
        long y = v.y;
        long z = v.z;
        long w = v.w;
        return std::hash<long>()(x + (y << 8) + (z << 16) + (w << 24));
    }
};

struct KeyEqual {
    bool operator()(const glm::ivec2& lhs, const glm::ivec2& rhs) const
    {
        return lhs == rhs;
    }
    bool operator()(const glm::ivec4& lhs, const glm::ivec4& rhs) const
    {
        return lhs == rhs;
    }
};

template <typename V>
using ivec2_map = std::unordered_map<glm::ivec2, V, KeyHash, KeyEqual>;
template <typename V>
using ivec4_map = std::unordered_map<glm::ivec4, V, KeyHash, KeyEqual>;
