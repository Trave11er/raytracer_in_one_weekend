#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "interval.h"
#include "rtweekend.h"
#include "config.h"

class material;

class hit_record {
  public:
    point3 p;
    vec3 normal;
    material* mat;  // having this as shared_ptr made everything much slower when multithreaded
    double t;
};

class hittable {
  public:
    virtual ~hittable() = default;

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec, const double frac=1.0) const = 0;
    material *mat;
};

class implicit_surface : public hittable {
  public:
    virtual double get_potential(const point3 point) const = 0;
    virtual vec3 get_potential_gradient(const point3 point) const = 0;
    virtual bool get_ray_bounds(const ray& r, double& t0, double& t1) const = 0;
};

class material {
  public:
    virtual ~material() = default;

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const = 0;
};

class lambertian : public material {
  public:
    lambertian(const color& a) : albedo(a) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        // TODO is this lambertian?
        bool front_face = dot(r_in.direction(), rec.normal) < 0;
        auto scatter_normal = rec.normal;
        if (!front_face)
            scatter_normal *= -1;

        auto scatter_direction = scatter_normal + random_unit_vector();
        // Catch degenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = scatter_normal;
        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

  private:
    color albedo;
};


#endif
