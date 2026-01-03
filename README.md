# XVK_Ray
A high-performance path tracer based on Vulkan Ray Tracing API, featuring an implementation of the ReSTIR GI (Path Resampling for Real-Time Path Tracing) algorithm. 
## Rendering Results Comparison

| Scene | 1spp Path Tracing | ReSTIR GI (Temporal) | ReSTIR GI (Spatial) | Reference (Converged) |
| :--- | :---: | :---: | :---: | :---: |
| **Scene1** | <img src="gallery/s1_1spp.png" width="200"><br>**~2.8 ms** | <img src="gallery/s1_temporal.png" width="200"><br>**~2.9 ms** | <img src="gallery/s1_spatial.png" width="200"><br>**~3.3 ms** | <img src="gallery/s1_ref.png" width="200"><br>**Reference** |
| **Scene2** | <img src="gallery/s2_1spp.png" width="200"><br>**~1.2 ms** | <img src="gallery/s2_temporal.png" width="200"><br>**~1.7 ms** | <img src="gallery/s2_spatial.png" width="200"><br>**~2.3 ms** | <img src="gallery/s2_ref.png" width="200"><br>**Reference** |
| **Scene3** | <img src="gallery/s3_1spp.png" width="200"><br>**~2.6 ms** | <img src="gallery/s3_temporal.png" width="200"><br>**~2.8 ms** | <img src="gallery/s3_spatial.png" width="200"><br>**~2.8 ms** | <img src="gallery/s3_ref.png" width="200"><br>**Reference** |
