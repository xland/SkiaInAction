#pragma once
#include <include/core/SkCanvas.h>
#include <include/core/SkSurface.h>
#include <include/core/SkPaint.h>
#include <include/core/SkExecutor.h>
#include <include/core/SkTiledImageUtils.h>
#include <src/core/SkLRUCache.h>
#include <webgpu/webgpu_cpp.h>
#include <dawn/native/DawnNative.h>
#include <dawn/dawn_proc.h>
#include <include/gpu/ganesh/GrDirectContext.h>
#include <include/gpu/graphite/BackendTexture.h>
#include <include/gpu/graphite/Context.h>
#include <include/gpu/graphite/ContextOptions.h>
#include <include/gpu/graphite/GraphiteTypes.h>
#include <include/gpu/graphite/Recorder.h>
#include <include/gpu/graphite/Recording.h>
#include <include/gpu/graphite/Surface.h>
#include <include/gpu/graphite/dawn/DawnBackendContext.h>
#include <include/gpu/graphite/dawn/DawnTypes.h>
#include <include/gpu/graphite/dawn/DawnUtils.h>
#include <include/gpu/graphite/ImageProvider.h>
#include <include/gpu/ganesh/SkImageGanesh.h>
#include <include/gpu/graphite/Image.h>
#include <src/gpu/graphite/ContextOptionsPriv.h>


class TestingImageProvider : public skgpu::graphite::ImageProvider {
public:
    TestingImageProvider() : fCache(kDefaultNumCachedImages) {}
    ~TestingImageProvider() override {}
    sk_sp<SkImage> findOrCreate(skgpu::graphite::Recorder* recorder,
        const SkImage* image,
        SkImage::RequiredProperties requiredProps) override {
        if (!requiredProps.fMipmapped) {
            // If no mipmaps are required, check to see if we have a mipmapped version anyway -
            // since it can be used in that case.
            // TODO: we could get fancy and, if ever a mipmapped key eclipsed a non-mipmapped
            // key, we could remove the hidden non-mipmapped key/image from the cache.
            ImageKey mipMappedKey(image, /* mipmapped= */ true);
            auto result = fCache.find(mipMappedKey);
            if (result) {
                return *result;
            }
        }
        ImageKey key(image, requiredProps.fMipmapped);
        auto result = fCache.find(key);
        if (result) {
            return *result;
        }
        sk_sp<SkImage> newImage = SkImages::TextureFromImage(recorder, image, requiredProps);
        if (!newImage) {
            return nullptr;
        }
        result = fCache.insert(key, std::move(newImage));
        SkASSERT(result);
        return *result;
    }
private:
    static constexpr int kDefaultNumCachedImages = 256;
    class ImageKey {
    public:
        ImageKey(const SkImage* image, bool mipmapped) {
            uint32_t flags = mipmapped ? 0x1 : 0x0;
            SkTiledImageUtils::GetImageKeyValues(image, &fValues[1]);
            fValues[kNumValues - 1] = flags;
            fValues[0] = SkChecksum::Hash32(&fValues[1], (kNumValues - 1) * sizeof(uint32_t));
        }
        uint32_t hash() const { return fValues[0]; }
        bool operator==(const ImageKey& other) const {
            for (int i = 0; i < kNumValues; ++i) {
                if (fValues[i] != other.fValues[i]) {
                    return false;
                }
            }
            return true;
        }
        bool operator!=(const ImageKey& other) const { return !(*this == other); }
    private:
        static const int kNumValues = SkTiledImageUtils::kNumImageKeyValues + 2;
        uint32_t fValues[kNumValues];
    };
    struct ImageHash {
        size_t operator()(const ImageKey& key) const { return key.hash(); }
    };
    SkLRUCache<ImageKey, sk_sp<SkImage>, ImageHash> fCache;
};