
#include <ccad/base/PoissonDisk.hpp>

namespace ccad {

PoissonDiskGenerator::PoissonDiskGenerator(PoissonDiskSpec spec) : m_Spec(spec), m_Rng(spec.seed) {
}

double PoissonDiskGenerator::RadiusAtY(double y) const {
    double t = y / m_Spec.height;
    double f = std::pow(1.0 - t, m_Spec.radiusFalloff);
    return m_Spec.rMin + (m_Spec.rMax - m_Spec.rMin) * f;
}

double PoissonDiskGenerator::DensityAtY(double y) const {
    double t = y / m_Spec.height;
    return std::pow(1.0 - t, m_Spec.densityFalloff);
}

bool PoissonDiskGenerator::IsCollision(const std::vector<PoissonPoint>& pts, double x, double y, double r) const {
    for (const auto& p : pts) {
        double dx = p.x - x;
        double dy = p.y - y;
        double dist2 = dx * dx + dy * dy;

        double required = p.r + r + m_Spec.minGap;
        if (dist2 < required * required) {
            return true;
        }
    }
    return false;
}

PoissonResult PoissonDiskGenerator::Run() {
    std::uniform_real_distribution<double> distX(m_Spec.margin, m_Spec.width - m_Spec.margin);
    std::uniform_real_distribution<double> distY(m_Spec.margin, m_Spec.height - m_Spec.margin);
    std::uniform_real_distribution<double> distU(0.0, 1.0);

    std::vector<PoissonPoint> points;
    points.reserve(m_Spec.targetPoints);

    while (points.size() < m_Spec.targetPoints) {
        double x = distX(m_Rng);
        double y = distY(m_Rng);

        double r = RadiusAtY(y);
        double prob = DensityAtY(y);

        if (distU(m_Rng) > prob) continue;

        // Introduce jitter
        double jitter = distU(m_Rng) * 0.4 - 0.2;
        r = r * (1.0 + jitter);

        if (!IsCollision(points, x, y, r)) {
            points.push_back({x, y, r});
        }
    }

    return {points};
}

}  // namespace ccad
