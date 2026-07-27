# XVK_Ray
一个基于 Vulkan Ray Tracing API 的高性能路径追踪器，实现了 ReSTIR GI (Path Resampling for Real-Time Path Tracing) 算法。

## 渲染结果对比

| 场景 | 1spp 路径追踪 | ReSTIR GI (时间域) | ReSTIR GI (空间域) | 参考图 (收敛) |
| :--- | :---: | :---: | :---: | :---: |
| **Scene1** | <img src="gallery/s1_1spp.png" width="200"><br>**~2.8 ms** | <img src="gallery/s1_temporal.png" width="200"><br>**~2.9 ms** | <img src="gallery/s1_spatial.png" width="200"><br>**~3.3 ms** | <img src="gallery/s1_ref.png" width="200"><br>**Reference** |
| **Scene2** | <img src="gallery/s2_1spp.png" width="200"><br>**~1.2 ms** | <img src="gallery/s2_temporal.png" width="200"><br>**~1.7 ms** | <img src="gallery/s2_spatial.png" width="200"><br>**~2.3 ms** | <img src="gallery/s2_ref.png" width="200"><br>**Reference** |
| **Scene3** | <img src="gallery/s3_1spp.png" width="200"><br>**~2.6 ms** | <img src="gallery/s3_temporal.png" width="200"><br>**~2.8 ms** | <img src="gallery/s3_spatial.png" width="200"><br>**~2.8 ms** | <img src="gallery/s3_ref.png" width="200"><br>**Reference** |
