# GPU Reference Renderer Implementation Guide

## Overview
This document describes the GPU-based reference renderer implementation that accelerates ray tracing using DirectX 11 compute shaders.

## Architecture

### Components
1. **gpu_reference_renderer.h/cpp** - Main renderer class
   - Inherits from `async_renderer_base`
   - Manages DirectX 11 resources
   - Handles scene data upload and result readback

2. **raytracer.hlsl** - HLSL compute shader
   - Performs ray tracing on GPU
   - Uses 8x8 thread groups for parallel processing
   - Implements iterative (non-recursive) ray tracing

### Key Features
- **GPU Parallelism**: One thread per pixel using compute shaders
- **Scene Data Upload**: Spheres, materials, and camera data transferred to GPU buffers
- **Random Number Generation**: PCG hash-based RNG for GPU
- **Iterative Ray Tracing**: Converted from recursive CPU implementation
- **Tone Mapping**: Reinhard extended tone mapping performed on GPU

## Data Structures

### GPU-Friendly Formats
All data structures are aligned for GPU constant buffers (16-byte alignment):

```cpp
struct GPUMaterial {
    vec3 color;                  // 12 bytes
    float smoothness;            // 4 bytes
    vec3 emitted_color;          // 12 bytes
    float gloss_probability;     // 4 bytes
    vec3 gloss_color;            // 12 bytes
    float refraction_probability;// 4 bytes
    float refraction_index;      // 4 bytes
    uint32_t type;               // 4 bytes
    vec3 padding;                // 12 bytes (alignment)
};

struct GPUSphere {
    vec3 origin;                 // 12 bytes
    float radius;                // 4 bytes
    uint32_t material_index;     // 4 bytes
    vec3 padding;                // 12 bytes (alignment)
};

struct GPUTriangle {
    vec3 v0;                     // Vertex 0 (12 bytes)
    vec3 v1;                     // Vertex 1 (12 bytes)
    vec3 v2;                     // Vertex 2 (12 bytes)
    uint32_t material_index;     // Material index (4 bytes)
    float padding[3];            // 12 bytes (alignment)
};

struct GPUBVHNode {
    float aabb_min[3];           // AABB minimum corner (12 bytes)
    uint32_t left_or_first;      // Left child index or first triangle index (4 bytes)
    float aabb_max[3];           // AABB maximum corner (12 bytes)
    uint32_t count;              // 0 = internal node, >0 = leaf triangle count (4 bytes)
};

struct GPUCamera {
    vec3 look_from;              // Camera position
    float lens_radius;           // For depth of field
    vec3 lower_left_corner;      // Viewport corner
    vec3 horizontal;             // Viewport horizontal span
    vec3 vertical;               // Viewport vertical span
    vec3 u, v, w;                // Camera basis vectors
    float dist_to_focus;         // Focus distance
    float type;                  // Camera type (perspective=0)
};

struct GPUConfig {
    uint32_t width;              // Image width
    uint32_t height;             // Image height
    uint32_t rays_per_pixel;     // AA samples
    uint32_t ray_bounces;        // Max bounce depth
    float white_point;           // Tone mapping parameter
};
```

## Shader Implementation

### Thread Organization
- Thread groups: 8x8 (64 threads per group)
- One thread per pixel
- Dispatch dimensions: `(width + 7) / 8` x `(height + 7) / 8`

### Ray Tracing Algorithm
1. **Fragment**: Generate rays per pixel with anti-aliasing
2. **Trace Ray**: Iterative loop for ray bounces
   - Scene intersection (spheres and triangles)
   - Material evaluation (universal/light)
   - Color accumulation
   - Next bounce calculation
3. **Environment Light**: Sky gradient for missed rays
4. **Tone Mapping**: Reinhard extended on HDR result

### Triangle Intersection
Uses the Möller-Trumbore algorithm for fast ray-triangle intersection:
- Efficiently computes barycentric coordinates
- Early exit for parallel rays
- Backface culling based on ray direction
- Face normal computed from cross product of edges

### BVH Acceleration Structure
A Bounding Volume Hierarchy (BVH) is built on the CPU and uploaded to the GPU for accelerated triangle intersection:
- **CPU-side construction**: Top-down recursive subdivision using midpoint splitting along the longest AABB axis
- **Leaf nodes**: Contain up to 4 triangles each
- **GPU traversal**: Iterative stack-based traversal (max stack depth: 32)
- **AABB test**: Slab method for fast ray-AABB intersection
- **Fallback**: Linear search is used when no BVH is available (e.g., sphere-only scenes)
- **Data layout**: BVH nodes stored in a StructuredBuffer at register t1, triangles reordered to match BVH leaf ordering

### Random Number Generation
Uses PCG (Permuted Congruential Generator) hash:
```hlsl
uint pcg_hash(uint seed) {
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}
```

## Usage

### Building
1. Open RayTracer.sln in Visual Studio 2022
2. Ensure DirectX 11 SDK is installed
3. Build configuration: x64 Debug or Release
4. HLSL shader is compiled at runtime (not pre-compiled)

### Running
1. Launch the application
2. Select "GPU Reference (DirectX 11)" from renderer dropdown
3. Configure scene and render settings
4. Click "Render" to start GPU-accelerated rendering

### Supported Features
- ✅ Sphere primitives
- ✅ Static mesh primitives (triangles)
- ✅ BVH acceleration structure for triangles
- ✅ Universal materials (diffuse, specular, refraction)
- ✅ Emissive materials (lights)
- ✅ Perspective camera
- ✅ Depth of field
- ✅ Anti-aliasing
- ✅ Multiple ray bounces
- ✅ Tone mapping

### Limitations
- Rectangles (xy_rect, xz_rect, yz_rect) not yet supported
- Maximum 256 spheres and materials
- Maximum 16,384 triangles per scene
- Maximum 16 ray bounces (can be increased)
- Orthographic camera not implemented
- Requires DirectX 11 compatible GPU

## Performance Considerations

### Memory Layout
- Constant buffers are 16-byte aligned
- Scene buffer: ~65KB (256 spheres + 256 materials)
- Triangle buffer: Variable size structured buffer (48 bytes per triangle)
- BVH buffer: Variable size structured buffer (32 bytes per node, up to ~2× triangle count nodes)
- Camera buffer: ~128 bytes
- Config buffer: ~32 bytes
- Output texture: width × height × 16 bytes (RGBA32F)

### Optimization Opportunities
1. ~~Use BVH/acceleration structure for scene intersection~~ ✅ Implemented
2. Add support for rectangle primitives
3. Implement wavefront path tracing for better GPU utilization
4. Add texture support
5. Pre-compile shader for faster startup

## Integration Points

### Renderer Registration
```cpp
// factories.h
enum class renderer_type {
    // ... existing types
    gpu_reference
};

// factories.cpp
async_renderer_base* object_factory::spawn_renderer(renderer_type type) {
    if (type == renderer_type::gpu_reference) {
        return new gpu_reference_renderer();
    }
    // ...
}
```

### Camera Data Export
Added getter methods to camera class:
```cpp
class camera {
public:
    const vec3& get_look_from() const;
    float get_lens_radius() const;
    const vec3& get_lower_left_corner() const;
    // ... other getters
};
```

### Static Mesh Support
Triangle data is uploaded to GPU via structured buffer:
```cpp
// Extract triangles from static_mesh objects
for (const auto& face : mesh->faces) {
    GPUTriangle gpu_tri{};
    gpu_tri.v0 = face.vertices[0];
    gpu_tri.v1 = face.vertices[1];
    gpu_tri.v2 = face.vertices[2];
    gpu_tri.material_index = mat_index;
    gpu_triangles.push_back(gpu_tri);
}

// Create structured buffer for GPU access
StructuredBuffer<GPUTriangle> triangles : register(t0);
```

The mesh's `pre_render()` method transforms triangles from model space to world space before GPU upload.

## Testing

### Validation
1. Compare output with CPU reference renderer
2. Verify visual quality matches
3. Check performance improvement (should be significantly faster)
4. Test with various scene configurations:
   - Scenes with only spheres
   - Scenes with only static meshes
   - Mixed scenes with both spheres and meshes
   - Complex meshes (icospheres, cubes, etc.)
5. Verify correct handling of materials and lighting on mesh surfaces
6. Test with different mesh transformations (rotation, scale, translation)

### Known Issues
None at this time.

## Future Enhancements
1. ~~BVH acceleration structure for both spheres and triangles~~ ✅ BVH for triangles implemented
2. Support for rectangle primitives (xy_rect, xz_rect, yz_rect)
3. Texture mapping for meshes
4. Pre-compiled shader pipeline
5. Orthographic camera support
6. Multiple light importance sampling
7. Volumetric rendering
8. GPU-side denoising
9. BVH acceleration for spheres
