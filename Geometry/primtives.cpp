#include "primtives.h"


#pragma region Ray

Ray::Ray()
{ }

Ray::Ray(const Ray& r) : origin(r.origin), direction(r.direction)
{ }

Ray::Ray(const Vector3f& o, const Vector3f& d) : origin(o), direction(d)
{ }

Ray& Ray::operator=(const Ray& r)
{
    this->origin = r.origin;
    this->direction = r.direction;
    return *this;
}

Vector3f Ray::GetPosAt(float t) const
{
    return origin + direction * t;
}

Ray Ray::Inversed() const
{
    return Ray(origin, -direction);
}

#pragma endregion

#pragma region Bounds

Bounds::Bounds() : center(0, 0, 0), extents(0, 0, 0)
{ }

Bounds::Bounds(const Bounds& b) : center(b.center), extents(b.extents)
{ }

Bounds::Bounds(const Vector3f& c, const Vector3f& e) : center(c), extents(e)
{ }

Bounds& Bounds::operator=(const Bounds& v)
{
    this->center = v.center;
    this->extents = v.extents;
    return *this;
}

bool Bounds::Intersect(const Ray& r) const
{
    // https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
    // r.dir is unit direction vector of ray
    Vector3f dir_frac = 1.0f / r.direction;
    // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    // r.org is origin of ray
    Vector3f smp = center - extents,
        bgp = center + extents;
    float    t1 = (smp.x - r.origin.x) * dir_frac.x;
    float    t2 = (bgp.x - r.origin.x) * dir_frac.x;
    float    t3 = (smp.y - r.origin.y) * dir_frac.y;
    float    t4 = (bgp.y - r.origin.y) * dir_frac.y;
    float    t5 = (smp.z - r.origin.z) * dir_frac.z;
    float    t6 = (bgp.z - r.origin.z) * dir_frac.z;

    float tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6));
    float tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6));

    // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
    // if tmin > tmax, ray doesn't intersect AABB
    return tmax >= 0 && tmin <= tmax;
}

bool Bounds::Intersect(const Ray& r, float& t) const
{
    // https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
    // r.dir is unit direction vector of ray
    Vector3f dir_frac = 1.0f / r.direction;
    // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    // r.org is origin of ray
    Vector3f smp = center - extents,
        bgp = center + extents;
    float    t1 = (smp.x - r.origin.x) * dir_frac.x;
    float    t2 = (bgp.x - r.origin.x) * dir_frac.x;
    float    t3 = (smp.y - r.origin.y) * dir_frac.y;
    float    t4 = (bgp.y - r.origin.y) * dir_frac.y;
    float    t5 = (smp.z - r.origin.z) * dir_frac.z;
    float    t6 = (bgp.z - r.origin.z) * dir_frac.z;

    float tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6));
    float tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6));

    // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
    if (tmax < 0) {
        t = tmax;
        return false;
    }

    // if tmin > tmax, ray doesn't intersect AABB
    if (tmin > tmax) {
        t = tmax;
        return false;
    }

    t = tmin;
    return true;
}

void Bounds::Union(const Bounds& b)
{
    Vector3f
        min = Min(b.center - b.extents, center - extents),
        max = Max(b.center + b.extents, center + extents);

    center = (max + min) / 2.f;
    extents = (max - min) / 2.f;
}

Bounds Bounds::Union(const Bounds& b0, const Bounds& b1)
{
    Vector3f
        min = Min(b0.center - b0.extents, b1.center - b1.extents),
        max = Max(b0.center + b0.extents, b1.center + b1.extents);

    return Bounds((min + max) / 2.f, (max - min) / 2.f);
}

void Bounds::Union(const Bounds& b0, const Bounds& b1, Bounds& b)
{
    Vector3f
        min = Min(b0.center - b0.extents, b1.center - b1.extents),
        max = Max(b0.center + b0.extents, b1.center + b1.extents);

    b = Bounds((max + min) / 2.f, (max - min) / 2.f);
}

#pragma endregion