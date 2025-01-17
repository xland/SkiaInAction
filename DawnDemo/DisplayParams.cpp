#include <include/core/SkExecutor.h>
#include "DisplayParams.h"

DisplayParams::DisplayParams() : fColorType(kN32_SkColorType)
, fColorSpace(nullptr)
, fMSAASampleCount(1) //亚像素样本数量。0表示无硬件抗锯齿。
, fSurfaceProps(0, kRGB_H_SkPixelGeometry)
, fDisableVsync(false)
, fDelayDrawableAcquisition(false)
, fCreateProtectedNativeBackend(false)
{
    //创建2个额外的线程来协助GPU工作，包括软件路径渲染。
    static std::unique_ptr<SkExecutor> gGpuExecutor = SkExecutor::MakeFIFOThreadPool(2); 
    fGrContextOptions.fExecutor = gGpuExecutor.get();
    fGrContextOptions.fAllowPathMaskCaching = true;
    //fGrContextOptions.fGpuPathRenderers = collect_gpu_path_renderers_from_flags();  //todo
    fGrContextOptions.fDisableDriverCorrectnessWorkarounds = false;
    fGrContextOptions.fReduceOpsTaskSplitting = GrContextOptions::Enable::kYes;
    fGrContextOptions.fAllowMSAAOnNewIntel = false;
    fGrContextOptions.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kSkSL;
    fGrContextOptions.fSuppressPrints = true;
    fGrContextOptions.fSupportBilerpFromGlyphAtlas = true;
    fCreateProtectedNativeBackend = false;

    //todo
    //skgpu::graphite::ContextOptionsPriv fPriv;
    //fPriv.fPathRendererStrategy = skgpu::graphite::PathRendererStrategy::kDefault; //kComputeMSAA8
    //fPriv.fStoreContextRefInRecorder = true; //需要使同步readPixels工作
}