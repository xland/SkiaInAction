- 修改：gn\BUILDCONFIG.gn
  - line:133  win_vc = "C:/Program Files/Microsoft Visual Studio/2022/Community/VC"
  - line:141  win_toolchain_version = "14.36.32532"
  - line:144  win_sdk_version = "10.0.22621.0"
  - line:152  clang_win_version = "16"
- 修改：gn\toolchain\BUILD.gn
  - line:45  dlsymutil_pool_depth = 8 
- Debug编译
  - to powershell with administrator
  - python3 tools/git-sync-deps
  - bin/gn gen out/debug --args='clang_win=\"C:\Program Files\LLVM\" cc=\"clang\" cxx=\"clang++\" extra_cflags=[\"/MTd\"] is_official_build=true is_debug=false skia_use_system_expat=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false skia_use_system_harfbuzz=false skia_use_system_icu=false skia_enable_skparagraph=true skia_enable_skshaper=true skia_enable_skunicode=true'
  - ninja -C out/debug
- Release编译
  - bin/gn gen out/release --args='clang_win=\"C:\Program Files\LLVM\" cc=\"clang\" cxx=\"clang++\" extra_cflags=[\"/MT\"] is_debug=false is_official_build=true skia_use_system_expat=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false skia_use_system_harfbuzz=false skia_use_system_icu=false skia_enable_skparagraph=true skia_enable_skshaper=true skia_enable_skunicode=true'
  - ninja -C out/release 
- 异常情况 if python3 error, delete python3
  - to cmd with administrator
  - C:\Users\liuxiaolun\AppData\Local\Microsoft\WindowsApps>del python3.exe





使用Skia库时，提示无法解析的外部符号GrGLInterface::checkError，所以Debug编译时，is_debug=false是错的，这个要验证一下
错的也没关系，你只要用预处理器SK_RELEASE就可以搞定这个问题


可能要更新third_party/external下的库
可能要禁用icu:
```
bin/gn gen out/debug --args='clang_win=\"C:\Program Files\LLVM\" cc=\"clang\" cxx=\"clang++\" extra_cflags=[\"/MTd\"] is_official_build=true is_debug=false skia_use_system_expat=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false skia_use_system_harfbuzz=false skia_use_system_icu=true skia_enable_skparagraph=true skia_enable_skshaper=true skia_enable_skunicode=false skia_use_icu=false'
```
```
bin/gn gen out/release --args='clang_win=\"C:\Program Files\LLVM\" cc=\"clang\" cxx=\"clang++\" extra_cflags=[\"/MT\"] is_debug=false is_official_build=true skia_use_system_expat=false skia_use_system_libjpeg_turbo=false skia_use_system_libpng=false skia_use_system_libwebp=false skia_use_system_zlib=false skia_use_system_harfbuzz=false skia_use_icu=false skia_enable_skparagraph=true skia_enable_skshaper=true skia_enable_skunicode=true'
```

