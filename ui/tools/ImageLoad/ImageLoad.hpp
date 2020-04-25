/**
 * 2020 Jonathan Mendez
 */

#pragma once
#include <functional>

using ImageRID = uintptr_t;

struct ImageResourceData {
    int width = 0;
    int height = 0;
    int num_channels = 0;
    uint8_t* bytes = nullptr;

    ~ImageResourceData(){
        if(bytes)
            delete[] bytes;
    }
};

namespace GPUTexture {
    void openGLUpload(ImageRID& rid, int width, int height, int num_channels, const uint8_t* bytes);
    void openGLFree(const ImageRID& rid);

    /**
     * A seperate thread for texture upload jobs to be appended.
     */
    namespace SideLoader {
        using GPUTextureJob = std::function<void()>;
        void start();
        void addJob(std::function<void()>);
    }
}

class ImageResource {
private:
    ImageResourceData data;

    ImageRID rid = 0;
    
    void freeBytes();
    void freeTexture();
    void loadFILE(FILE* image_file, bool flip = false);
    void loadPATH(const std::string& image_path, bool flip = false);
public:
    ~ImageResource();
    inline ImageResource(){}
    ImageResource(const std::string& image_location, bool to_gpu, bool flip = false);
    void load(const std::string& image_location, bool to_gpu, bool flip = false);
    
    void sendToGPU();

    inline int getWidth() { return this->data.width; }
    inline int getHeight() { return this->data.height; }
    /**
     * Get the resource handle of the image on the gpu.
     */
    inline ImageRID getRID() {
        return this->rid;
    }
};
