#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include <math.h>
#include <iostream>
#include <assert.h>

double calculate_potential(double dist, double influence_radius) {
    double r = dist / influence_radius;
    // https://www.geisswerks.com/ryan/BLOBS/blobs.html
    // rescaled to have cut-off at 1 not 0.707
    if (r > 1.01)
        return 0;
    else
        return pow(r, 4.0) / 4.0 - pow(r, 2.0) / 2.0 + 0.25;
}


class sphere : public implicit_surface {
  public:
    sphere(point3 _center, double _radius, material *_material)
        : center(_center), radius(_radius), influence_radius(_radius * INFLUENCE_RADIUS_MULT)
    { mat = _material; }

    bool hit(const ray& r, const interval ray_t, hit_record& rec, const double frac=1.0) const override {
        double t0, t1;
        if (!intersect_ray(r, radius * frac, t0, t1))
            return false;

        auto root = t0;
        if (not ray_t.surrounds(root)) {
            root = t1;
            if (not ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        rec.normal = (rec.p - center) / (radius * frac);
        rec.mat = mat;

        return true;
    }

    double get_potential(const point3 point) const override {
        double dist = (point - center).length();
        return calculate_potential(dist, influence_radius);
    }

    vec3 get_potential_gradient(const point3 p) const override {
        vec3 diff = p - center;
        double d = diff.length();
        double r_norm = d / influence_radius;
        if (r_norm > 1.01 || d < 1e-10) return vec3(0, 0, 0);
        double factor = (r_norm * r_norm - 1.0) / (influence_radius * influence_radius);
        return factor * diff;
    }

    bool get_ray_bounds(const ray& r, double& t0, double& t1) const override {
        return intersect_ray(r, influence_radius, t0, t1);
    }

  private:
    point3 center;
    double radius;
    double influence_radius;

    bool intersect_ray(const ray& r, double effective_radius, double& t0, double& t1) const {
        vec3 oc = r.origin() - center;
        auto a = dot(r.direction(), r.direction());
        auto b = 2.0 * dot(oc, r.direction());
        auto c = dot(oc, oc) - effective_radius * effective_radius;
        auto discriminant = b * b - 4 * a * c;
        if (discriminant < 0) return false;
        auto sqrtd = sqrt(discriminant);
        t0 = (-b - sqrtd) / (2.0 * a);
        t1 = (-b + sqrtd) / (2.0 * a);
        return true;
    }
};

#endif
