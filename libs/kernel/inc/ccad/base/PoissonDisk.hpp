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
    double margin = 20.0;

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

    PoissonResult Run();

   private:
    PoissonDiskSpec spec_;
    std::mt19937_64 rng_;

    double radiusAtY(double y) const;
    double densityAtY(double y) const;

    bool isCollision(const std::vector<PoissonPoint>& pts, double x, double y, double r) const;
};

}  // namespace ccad
