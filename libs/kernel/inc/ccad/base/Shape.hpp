#pragma once
#include <ccad/base/IShape.hpp>
#include <ccad/base/Math.hpp>
#include <memory>
#include <string>

namespace ccad {

/** \brief Thin value handle to own a shape with nice pass/return semantics. */
class Shape {
   public:
    Shape() = default;
    explicit Shape(std::unique_ptr<IShape> p) : m_Ptr(std::move(p)) {
    }

    // Deep-copy via Clone:
    Shape(const Shape& rhs) : m_Ptr(rhs.m_Ptr ? rhs.m_Ptr->Clone() : nullptr) {
    }
    Shape& operator=(const Shape& rhs) {
        if (this != &rhs) m_Ptr = rhs.m_Ptr ? rhs.m_Ptr->Clone() : nullptr;
        return *this;
    }

    Shape(Shape&&) noexcept = default;
    Shape& operator=(Shape&&) noexcept = default;

    explicit operator bool() const {
        return static_cast<bool>(m_Ptr);
    }

    std::string TypeName() const {
        return m_Ptr ? m_Ptr->TypeName() : "Null";
    }

    Bounds BBox() const {
        return m_Ptr ? m_Ptr->BoundingBox() : Bounds{};
    }

    IShape& Get() {
        return *m_Ptr;
    }
    const IShape& Get() const {
        return *m_Ptr;
    }

    std::unique_ptr<IShape> Release() {
        return std::move(m_Ptr);
    }

   private:
    std::unique_ptr<IShape> m_Ptr;
};
}  // namespace ccad
