// GPU Ray Tracer Compute Shader
// DirectX 11 HLSL

// Constants
#define MAX_SPHERES 32
#define MAX_TRIANGLES 4096
#define MAX_MATERIALS 64
#define MAX_BOUNCES 200
#define PI 3.14159265359f
#define MIN_RAY_COLOR_THRESHOLD 0.1f

// Material types
#define MATERIAL_UNIVERSAL 1
#define MATERIAL_LIGHT 2

// GPU-friendly structures
struct GPUMaterial
{
    float4 color;
    float4 emitted_color;
    float4 gloss_color;
    uint type;
    float smoothness;
    float gloss_probability;
    float refraction_probability;
    float refraction_index;
    float3 padding;
};

struct GPUSphere
{
    float4 origin;
    uint material_index;
    float radius;
    float2 padding;
};

struct GPUTriangle
{
    float4 v0;
    float4 v1;
    float4 v2;
    uint material_index;
    float3 padding;
};

struct GPUCamera
{
    float4 look_from;
    float4 lower_left_corner;
    float4 horizontal;
    float4 vertical;
    float4 u;
    float4 v;
    float4 w;
    float lens_radius;
    float viewport_width;
    float viewport_height;
    float dist_to_focus;
    float type;
    float3 padding;
};

struct GPUConfig
{
    uint width;
    uint height;
    uint rays_per_pixel;
    uint ray_bounces;
    float white_point;
    float3 padding;
};

// Constant buffers
cbuffer SceneData : register(b0)
{
    GPUSphere spheres[MAX_SPHERES];
    GPUMaterial materials[MAX_MATERIALS];
    uint sphere_count;
    uint material_count;
    uint triangle_count;
    float scene_padding;
};

cbuffer CameraData : register(b1)
{
    GPUCamera camera;
};

cbuffer ConfigData : register(b2)
{
    GPUConfig config;
};

// Structured buffer for triangles
StructuredBuffer<GPUTriangle> triangles : register(t0);

// Output texture
RWTexture2D<float4> OutputTexture : register(u0);

// Random number generation (PCG)
uint pcg_hash(uint seed)
{
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float rand_pcg(inout uint seed)
{
    seed = pcg_hash(seed);
    return float(seed) / float(0xffffffffu);
}

float3 random_in_unit_sphere(inout uint seed)
{
    // Box-Muller transform for normal distribution
    float theta = 2.0f * PI * rand_pcg(seed);
    float rho = sqrt(-2.0f * log(max(rand_pcg(seed), 1e-7f)));
    float x = rho * cos(theta);
    
    theta = 2.0f * PI * rand_pcg(seed);
    rho = sqrt(-2.0f * log(max(rand_pcg(seed), 1e-7f)));
    float y = rho * cos(theta);
    
    theta = 2.0f * PI * rand_pcg(seed);
    rho = sqrt(-2.0f * log(max(rand_pcg(seed), 1e-7f)));
    float z = rho * cos(theta);
    
    return normalize(float3(x, y, z));
}

float3 random_in_unit_disk(inout uint seed)
{
    // Generate random point in unit disk using rejection sampling
    float3 p;
    do
    {
        p = float3(2.0f * rand_pcg(seed) - 1.0f, 2.0f * rand_pcg(seed) - 1.0f, 0.0f);
    } while (dot(p, p) >= 1.0f);
    return p;
}

// Ray structure
struct Ray
{
    float3 origin;
    float3 direction;
};

// Hit record
struct HitRecord
{
    float3 p;
    float3 normal;
    float t;
    bool front_face;
    uint material_index;
};

// Sphere intersection
bool hit_sphere(GPUSphere sphere, Ray r, float t_min, float t_max, inout HitRecord rec)
{
    float3 oc = r.origin - sphere.origin.xyz;
    float a = dot(r.direction, r.direction);
    float half_b = dot(oc, r.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = half_b * half_b - a * c;
    
    if (discriminant < 0.0f)
        return false;
    
    float sqrtd = sqrt(discriminant);
    float root = (-half_b - sqrtd) / a;
    
    if (root < t_min || t_max < root)
    {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }
    
    rec.t = root;
    rec.p = r.origin + r.direction * rec.t;
    float3 outward_normal = (rec.p - sphere.origin.xyz) / sphere.radius;
    rec.front_face = dot(r.direction, outward_normal) < 0.0f;
    rec.normal = rec.front_face ? outward_normal : -outward_normal;
    rec.material_index = sphere.material_index;
    
    return true;
}

// Triangle intersection using Möller-Trumbore algorithm
bool hit_triangle(GPUTriangle tri, Ray r, float t_min, float t_max, inout HitRecord rec)
{
    const float EPSILON = 0.0000001f;
    
    float3 v0 = tri.v0.xyz;
    float3 v1 = tri.v1.xyz;
    float3 v2 = tri.v2.xyz;
    
    float3 edge1 = v1 - v0;
    float3 edge2 = v2 - v0;
    
    float3 h = cross(r.direction, edge2);
    float a = dot(edge1, h);
    
    // Ray parallel to triangle?
    if (abs(a) < EPSILON)
        return false;
    
    float f = 1.0f / a;
    float3 s = r.origin - v0;
    float u = f * dot(s, h);
    
    // Intersection outside triangle?
    if (u < 0.0f || u > 1.0f)
        return false;
    
    float3 q = cross(s, edge1);
    float v = f * dot(r.direction, q);
    
    // Intersection outside triangle?
    if (v < 0.0f || u + v > 1.0f)
        return false;
    
    float t = f * dot(edge2, q);
    
    // Intersection outside ray range?
    if (t < t_min || t > t_max)
        return false;
    
    rec.t = t;
    rec.p = r.origin + r.direction * t;
    
    // Calculate face normal
    float3 face_normal = normalize(cross(edge1, edge2));
    
    // Determine front face
    float dot_normal_direction = dot(face_normal, r.direction);
    rec.front_face = dot_normal_direction < 0.0f;
    rec.normal = rec.front_face ? face_normal : -face_normal;
    rec.material_index = tri.material_index;
    
    return true;
}

// Scene intersection
bool hit_scene(Ray r, float t_min, float t_max, inout HitRecord rec)
{
    HitRecord temp_rec;
    bool hit_anything = false;
    float closest_so_far = t_max;
    
    for (uint i = 0; i < sphere_count; ++i)
    {
        if (hit_sphere(spheres[i], r, t_min, closest_so_far, temp_rec))
        {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    for (uint j = 0; j < triangle_count; ++j)
    {
        if (hit_triangle(triangles[j], r, t_min, closest_so_far, temp_rec))
        {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }
    
    return hit_anything;
}

// Environment light
float3 environment_light_sky(Ray r)
{
    float3 sky_color_zenith = float3(0.5f, 0.7f, 1.0f);  // white_blue
    float3 sky_color_horizon = float3(1.0f, 1.0f, 1.0f); // white
    float sky_brightness = 0.4f;
    
    // Smoothstep
    float t = saturate((r.direction.y + 0.6f) / 0.8f);
    t = t * t * (3.0f - 2.0f * t);
    
    float3 light = lerp(sky_color_horizon, sky_color_zenith, t);
    return clamp(light, 0.0f, 1.0f) * sky_brightness;
}
float3 environment_light_darkness(Ray r)
{
    return float3(0.0f, 0.0f, 0.0f);
}

// Refract function
float3 refract_vec(float3 uv, float3 n, float etai_over_etat)
{
    float cos_theta = min(dot(-uv, n), 1.0f);
    float3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    float3 r_out_parallel = -sqrt(abs(1.0f - dot(r_out_perp, r_out_perp))) * n;
    return r_out_perp + r_out_parallel;
}

// Trace ray (iterative, not recursive)
float3 trace_ray(Ray r, inout uint seed)
{
    float3 ray_color = float3(1.0f, 1.0f, 1.0f);
    float3 incoming_light = float3(0.0f, 0.0f, 0.0f);
    
    for (uint bounce = 0; bounce < config.ray_bounces; ++bounce)
    {
        HitRecord hit;
        if (hit_scene(r, 0.01f, 1e30f, hit))
        {
            // Don't bounce if ray has no color
            if (dot(ray_color, ray_color) < MIN_RAY_COLOR_THRESHOLD)
                break;
            
            // Read material
            GPUMaterial mat = materials[hit.material_index];
            
            if (mat.type == MATERIAL_LIGHT)
            {
                // Don't bounce off lights
                incoming_light += mat.emitted_color.xyz * ray_color;
                break;
            }
            
            r.origin = hit.p;
            
            bool can_gloss_bounce = mat.gloss_probability >= rand_pcg(seed);
            bool can_refract = mat.refraction_probability >= rand_pcg(seed);
            
            float3 diffuse_dir = normalize(hit.normal + random_in_unit_sphere(seed));
            
            if (!can_gloss_bounce && can_refract)
            {
                float refraction_ratio = hit.front_face ? (1.0f / mat.refraction_index) : mat.refraction_index;
                float3 refraction_dir = refract_vec(r.direction, hit.normal, refraction_ratio);
                
                // Define next bounce
                r.direction = refraction_dir + diffuse_dir * (1.0f - mat.smoothness);
                
                // Refraction color
                ray_color *= mat.color.xyz;
                continue;
            }
            
            // New directions
            float3 specular_dir = reflect(r.direction, hit.normal);
            
            // Define next bounce
            float blend = mat.smoothness;
            if (mat.gloss_probability > 0.0f)
            {
                blend = mat.smoothness * (can_gloss_bounce ? 1.0f : 0.0f);
            }
            r.direction = lerp(diffuse_dir, specular_dir, blend);
            
            // Calculate color for this hit
            incoming_light += mat.emitted_color.xyz * ray_color;
            ray_color *= lerp(mat.color.xyz, mat.gloss_color.xyz, can_gloss_bounce ? 1.0f : 0.0f);
        }
        else
        {
            float3 env_light = environment_light_darkness(r);
            incoming_light += env_light * ray_color;
            break;
        }
    }
    
    return incoming_light;
}

// Get ray from camera
Ray get_camera_ray(float u, float v, inout uint seed)
{
    Ray r;
    
    float3 rd = camera.lens_radius * random_in_unit_disk(seed);
    float3 offset = camera.u.xyz * rd.x + camera.v.xyz * rd.y;
    
    // Shoot rays from the plane that is proportionally smaller to focus plane
    // 0.0f - perspective camera
    // 1.0f - orthographic camera
    float3 c_horizontal = camera.horizontal.xyz * camera.type;
    float3 c_vertical = camera.vertical.xyz * camera.type;
    float3 c_lower_left_corner = camera.look_from.xyz - c_horizontal / 2.0f - c_vertical / 2.0f;
    float3 cpo = c_lower_left_corner + c_horizontal * u + c_vertical * v;
    float3 fpo = camera.lower_left_corner.xyz + camera.horizontal.xyz * u + camera.vertical.xyz * v;
    float3 fpf = fpo - camera.w.xyz * camera.dist_to_focus;
    r.origin = cpo - offset;
    r.direction = normalize(fpf - cpo + offset);
    
    return r;
}

// Reinhard tone mapping
float3 reinhard_extended(float3 hdr_color, float white_point)
{
    float3 numerator = hdr_color * (1.0f + (hdr_color / (white_point * white_point)));
    return numerator / (1.0f + hdr_color);
}

// Fragment shader (per-pixel)
float3 fragment(uint2 pixel_coord, inout uint seed)
{
    float3 sum_colors = float3(0.0f, 0.0f, 0.0f);
    
    for (uint i = 0; i < config.rays_per_pixel; ++i)
    {
        // Anti-aliasing with ray variance
        float u = (float(pixel_coord.x) + rand_pcg(seed)) / (float(config.width) - 1.0f);
        float v = (float(pixel_coord.y) + rand_pcg(seed)) / (float(config.height) - 1.0f);
        
        // Trace the ray
        Ray r = get_camera_ray(u, v, seed);
        sum_colors += trace_ray(r, seed);
    }
    
    float3 hdr_color = sum_colors / float(config.rays_per_pixel);
    return hdr_color;
}

// Main compute shader
[numthreads(8, 8, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
    // Check bounds
    if (threadID.x >= config.width || threadID.y >= config.height)
        return;
    
    // Each pixel has a unique seed
    uint seed = threadID.y * config.width + threadID.x;
    
    // Compute HDR color
    float3 hdr_color = fragment(threadID.xy, seed);
    
    // Tone mapping
    float3 ldr_color = saturate(reinhard_extended(hdr_color, config.white_point));

    // Write to output (RGBA format)
    OutputTexture[threadID.xy] = float4(ldr_color, 1.0f);
}
