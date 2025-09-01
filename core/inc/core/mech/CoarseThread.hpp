#pragma once

#include "geometry/Shape.hpp"

namespace mech {

/// Thread profile style.
enum class ThreadTip {
    Sharp,   ///< pure V
    Rounded  ///< V with a tangential fillet at the crest
};

/// Coarse thread parameters for 3D printing friendly threads.
struct CoarseThreadParams {
    double length = 10.0;         ///< overall axial length (mm)
    double depth = 0.8;           ///< radial thread depth (mm)
    int turns = 6;                ///< number of helical turns (pitch = length / turns)
    bool leftHand = false;        ///< false: right-hand thread
    double clearance = 0.20;      ///< extra radial clearance (mm) for 3D printing
    double flankAngleDeg = 60.0;  ///< included flank angle (deg), typical 60°

    // Helix & profile sampling / style
    int samplesPerTurn = 96;  ///< points per turn for BSpline helix (64..128 safe)
    ThreadTip tipStyle = ThreadTip::Sharp;
    double tipRadius = 0.15;  ///< crest fillet radius for Rounded tip

    // Safety clamps (optional): max depth ratio of pitch (avoid self-intersection)
    double maxDepthToPitch = 0.45;  ///< max allowed depth/pitch
};

/// A few presets to avoid “parameter soup”
struct ThreadPreset {
    double depth;
    double clearance;
    double flankAngleDeg;
    int samplesPerTurn;
    ThreadTip tipStyle;
    double tipRadius;
};

ThreadPreset PresetCoarse3D();  ///< coarse, forgiving, strong
ThreadPreset PresetFine3D();    ///< finer look, still printable

/// Create external (male) thread solid (shaft + ridges fused).
geometry::ShapePtr ThreadExternal(double outerDiameter, const CoarseThreadParams& p);

/// Create internal (female) thread cutter (subtract this from a bore).
geometry::ShapePtr ThreadInternalCutter(double boreDiameter, const CoarseThreadParams& p);

}  // namespace mech
