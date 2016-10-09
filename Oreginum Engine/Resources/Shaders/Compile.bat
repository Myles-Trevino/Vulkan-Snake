@ECHO off
C:/VulkanSDK/1.0.24.0/Bin32/glslangValidator.exe -V "Primitive Vertex.vert"
C:/VulkanSDK/1.0.24.0/Bin32/glslangValidator.exe -V "Primitive Fragment.frag"
move /y frag.spv "Primitive Fragment.spv"
move /y vert.spv "Primitive Vertex.spv"
pause
