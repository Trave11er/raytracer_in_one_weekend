#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "interval.h"
#include "rtweekend.h"

class material;

class hit_record {
  public:
    point3 p;
    vec3 normal;
    shared_ptr<material> mat;
    double t;
};

class hittable {
  public:
    virtual ~hittable() = default;

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
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
        // TODO TODO is this lambertian?
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

class metal : public material {
  public:
    metal(const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        bool front_face = dot(r_in.direction(), rec.normal) < 0;
        auto scatter_normal = rec.normal;
        if (!front_face)
            scatter_normal *= -1;

        vec3 reflected = reflect(unit_vector(r_in.direction()), scatter_normal);
        scattered = ray(rec.p, reflected + fuzz*random_unit_vector());
        attenuation = albedo;
        return (dot(scattered.direction(), scatter_normal) > 0);
    }

  private:
    color albedo;
    double fuzz;
};

class dielectric : public material {
  public:
    dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        attenuation = color(1.0, 1.0, 1.0);

        auto scatter_normal = rec.normal;
        bool front_face = dot(r_in.direction(), rec.normal) < 0;
        if (!front_face)
            scatter_normal *= -1;

        double refraction_ratio = front_face ? (1.0/ir) : ir;
        vec3 unit_direction = unit_vector(r_in.direction());
        vec3 refracted = refract(unit_direction, scatter_normal, refraction_ratio);

        scattered = ray(rec.p, refracted);
        return true;
    }

  private:
    double ir; // Index of Refraction
};

#endif
