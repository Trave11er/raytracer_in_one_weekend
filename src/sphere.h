#include "hittable.h"

class sphere : public hittable {
  public:
    sphere(point3 _center, double _radius, material *_material) : center(_center), radius(_radius), mat(_material) {}

    bool hit(const ray& r, const interval ray_t, hit_record& rec) const override {
        vec3 oc = r.origin() - center;
        auto a = dot(r.direction(), r.direction());
        auto b = 2.0 * dot(oc, r.direction());
        auto c = dot(oc, oc) - radius*radius;
        auto discriminant = b*b - 4*a*c;

        if (discriminant < 0)
            return false;

        // Find the nearest root that lies in the acceptable range.
        auto sqrtd = sqrt(discriminant);
        auto root = (-b - sqrtd) / (2.0 * a);
        if (not ray_t.surrounds(root)) {
            root = (-b + sqrtd) / (2.0 * a);
            if (not ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        rec.normal = (rec.p - center) / radius;
        rec.mat = mat;

        return true;
    }
  private:
    point3 center;
    double radius;
    material *mat;
};
