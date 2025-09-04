#pragma once
#include <ccad/base/Math.hpp>
#include <memory>

namespace ccad {

/** \brief Abstract base for all shapes. */
class IShape {
   public:
    virtual ~IShape() = default;

    /** \return Human-friendly type name (e.g. "Box", "Cylinder", "Union"). */
    virtual std::string TypeName() const = 0;

    /** \return Axis-aligned bounding box. Implementations may approximate. */
    virtual Bounds BoundingBox() const = 0;

    /** \brief Deep clone into a new object. */
    virtual std::unique_ptr<IShape> Clone() const = 0;

   protected:
    IShape() = default;
};

}  // namespace ccad
