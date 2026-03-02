# Ares RT

Ares RT is a raytracing engine developed on top of the Lightwave Framework. It is a project part of the Computer Graphics course at Saarland University lectured by Prof. Dr.-Ing. Philipp Slusallek. 

![Blender Scene rendered with Ares RT](https://mohammedsaad114.github.io/rendering-competition/render.jpg)

---

## Features
### Functionalities Provided by Lightwave
* Modularity
  * Modern APIs flexible enough for sophisticated algorithms
  * Shapes, materials, etc are implemented as plugins
* Basic math library
  * Vector operations
  * Matrix operations
  * Transforms
* File I/O
  * An XML parser for the lightwave scene format
  * Reading and writing various image formats
  * Reading and representing triangle meshes
  * Streaming images to the [tev](https://github.com/Tom94/tev) image viewer
* Multi-threading
  * Rendering is parallelized across all available cores
  * Parallelized scene loading (image loading, BVH building, etc)
* BVH acceleration structure
  * Data-structure and traversal is supplied by us
  * Split-in-the-middle build is supplied as well
* Useful utilities
  * Thread-safe logger functionality
  * Assert statements that provide extra context
  * An embedded profiler to identify bottlenecks of your code
  * Random number generators
* A Blender exporter
  * Exports a scene that is built in Blender to a format accepted by lightwave

### Features Provided by Ares RT
* BSDFs:
  * Diffuse
  * Conductor
  * Rough Conductor
  * Dielectric
  * Principled
* Camera Models
  * Basic Perspective Camera
* Emissions
  * Lambertian Emission
* Integrators
  * Albedo
  * Normals
  * Direct Lighting
  * Path Tracing
* Lights:
  * Environment Map
  * Area Lights
  * Point Light
  * Directional Light
* Postprocesses:
  * Bloom 
  * Image denoising using [Intel&reg; Open Image Denoise](https://www.openimagedenoise.org/)
  * Tonemap 
* Basic Primitives
  * Sphere
  * Rectangles
  * Triangle/Mesh
* Acceleration Structures:
  * SAH Bounding Volume Hierarchy
* Shading Normals
* Textures:
  * Checkerboard Texture
  * Image Texture
* Sampling:
  * BSDF Sampling 
  * Next Event Estimation (NEE)

### Examples
Some of the implemented features are showcased [here](https://mohammedsaad114.github.io/rendering-competition/).

---

## Copyright & Credits
Lightwave was written by [Alexander Rath](https://graphics.cg.uni-saarland.de/people/rath.html), with contributions from [Ömercan Yazici](https://graphics.cg.uni-saarland.de/people/yazici.html) and [Philippe Weier](https://graphics.cg.uni-saarland.de/people/weier.html).
Many of our design decisions were heavily inspired by [Nori](https://wjakob.github.io/nori/), a great educational renderer developed by Wenzel Jakob.
We would also like to thank the teams behind our dependencies: [ctpl](https://github.com/vit-vit/CTPL), [miniz](https://github.com/richgel999/miniz), [stb](https://github.com/nothings/stb), [tinyexr](https://github.com/syoyo/tinyexr), [tinyformat](https://github.com/c42f/tinyformat), [pcg32](https://github.com/wjakob/pcg32), and [catch2](https://github.com/catchorg/Catch2).
