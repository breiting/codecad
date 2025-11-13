
#include <ccad/base/PoissonDisk.hpp>

namespace ccad {

PoissonDiskGenerator::PoissonDiskGenerator(PoissonDiskSpec spec) : spec_(spec), rng_(spec.seed) {
}

double PoissonDiskGenerator::radiusAtY(double y) const {
    double t = y / spec_.height;
    double f = std::pow(1.0 - t, spec_.radiusFalloff);
    return spec_.rMin + (spec_.rMax - spec_.rMin) * f;
}

double PoissonDiskGenerator::densityAtY(double y) const {
    double t = y / spec_.height;
    return std::pow(1.0 - t, spec_.densityFalloff);
}

bool PoissonDiskGenerator::isCollision(const std::vector<PoissonPoint>& pts, double x, double y, double r) const {
    for (const auto& p : pts) {
        double dx = p.x - x;
        double dy = p.y - y;
        double dist2 = dx * dx + dy * dy;

        double required = p.r + r + spec_.minGap;
        if (dist2 < required * required) {
            return true;
        }
    }
    return false;
}

PoissonResult PoissonDiskGenerator::Run() {
    std::uniform_real_distribution<double> distX(spec_.margin, spec_.width - spec_.margin);
    std::uniform_real_distribution<double> distY(spec_.margin, spec_.height - spec_.margin);
    std::uniform_real_distribution<double> distU(0.0, 1.0);

    std::vector<PoissonPoint> points;
    points.reserve(spec_.targetPoints);

    while (points.size() < spec_.targetPoints) {
        double x = distX(rng_);
        double y = distY(rng_);

        double r = radiusAtY(y);
        double prob = densityAtY(y);

        if (distU(rng_) > prob) continue;

        if (!isCollision(points, x, y, r)) {
            points.push_back({x, y, r});
        }
    }

    return {points};
}

}  // namespace ccad
