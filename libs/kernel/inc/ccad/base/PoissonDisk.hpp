#pragma once

#include <random>
#include <vector>

namespace ccad {

/**
 * 2D Poisson point with its radius
 */
struct PoissonPoint {
    double x;
    double y;
    double r;
};

struct PoissonResult {
    std::vector<PoissonPoint> points;
};

struct PoissonDiskSpec {
    double width = 500.0;
    double height = 1000.0;
    double margin = 2.0;

    std::size_t targetPoints = 300;

    double rMin = 3.0;
    double rMax = 35.0;
    double minGap = 3.0;

    uint64_t seed = 123456;

    // Exponents for radii and density distribution
    double radiusFalloff = 2.0;
    double densityFalloff = 3.0;
};

class PoissonDiskGenerator {
   public:
    explicit PoissonDiskGenerator(PoissonDiskSpec spec);

    /**
     * Calculate the distribution
     */
    PoissonResult Run();

   private:
    double RadiusAtY(double y) const;
    double DensityAtY(double y) const;
    bool IsCollision(const std::vector<PoissonPoint>& pts, double x, double y, double r) const;

   private:
    PoissonDiskSpec m_Spec;
    std::mt19937_64 m_Rng;
};

}  // namespace ccad
