- 过滤文件
	.mm,*_unix.cpp,*_android.cpp
- 运行此程序必须重新编译Skia

	把Skia调试库编译成MT，不用MTd了，因为调试模式用不了这三个库：
        allocator_shim
        allocator_base
        allocator_core
	（此工程调试状态运行库也调成了MT）
	
		Skia编译参数如下：

		```
		clang_win = "C:\Program Files\LLVM"
		cc = "clang"
		cxx = "clang++"
		extra_cflags = [ "/MTd" ]
		is_official_build = false
		is_debug = false
		skia_use_system_expat = false
		skia_use_system_libjpeg_turbo = false
		skia_use_system_libpng = false
		skia_use_system_libwebp = false
		skia_use_system_zlib = false
		skia_use_system_harfbuzz = false
		skia_use_icu = false
		skia_use_vulkan = false
		skia_use_direct3d = false
		skia_use_webgpu = false
		skia_use_angle = false
		skia_use_metal = false
		skia_use_ffmpeg = false
		skia_use_dawn = true
		skia_use_gl = true   
		skia_enable_tools = false
		skia_enable_ganesh = true
		skia_enable_graphite = true
		skia_enable_gpu = true
		skia_enable_skparagraph = true
		skia_enable_skshaper = true
		```

		```
		bin/gn gen out/debug
		ninja -C out/debug
		```

		## Release

		```
		clang_win = "C:\Program Files\LLVM"
		cc = "clang"
		cxx = "clang++"
		extra_cflags = [ "/MT" ]
		is_official_build = false
		is_debug = false
		skia_use_system_expat = false
		skia_use_system_libjpeg_turbo = false
		skia_use_system_libpng = false
		skia_use_system_libwebp = false
		skia_use_system_zlib = false
		skia_use_system_harfbuzz = false
		skia_use_icu = false
		skia_use_vulkan = false
		skia_use_direct3d = false
		skia_use_webgpu = false
		skia_use_angle = false
		skia_use_metal = false
		skia_use_ffmpeg = false
		skia_use_dawn = true
		skia_use_gl = true   
		skia_enable_tools = false
		skia_enable_ganesh = true
		skia_enable_graphite = true
		skia_enable_gpu = true
		skia_enable_skparagraph = true
		skia_enable_skshaper = true
		```
		```
		ninja -C out/release
		```

- 很多机器没有Direct12，程序在这些机器上也能运行，但用不了GPU的能力




- 这一大套配置可以让exe小1k
```
    target_link_options(${PROJECT_NAME} PRIVATE /INCREMENTAL:NO)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DEBUG:NONE")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /EHsc")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffunction-sections -fdata-sections")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections")
```

- upx能压缩到5M
```
D:\tools\upx>upx D:\project\SkiaInAction\Dawn\out\build\x64-Release\Release\DrawinWindow.exe
                       Ultimate Packer for eXecutables
                          Copyright (C) 1996 - 2024
UPX 4.2.4       Markus Oberhumer, Laszlo Molnar & John Reiser    May 9th 2024

        File size         Ratio      Format      Name
   --------------------   ------   -----------   -----------
  14869504 ->   5229568   35.17%    win64/pe     DrawinWindow.exe                                                                                                                                                                               Packed 1 file.
```