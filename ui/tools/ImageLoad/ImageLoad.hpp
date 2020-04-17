/**
 * 2020 Jonathan Mendez
 */

#pragma once
#include <memory>

using ImageRID = uintptr_t;

struct ImageResourceData {
    int width = 0;
    int height = 0;
    int num_channels = 0;
    uint8_t* bytes = nullptr;

    ImageResourceData(){}
    
    // Move the bytes pointer only, and replace the original bytes pointer with a nullptr.
    ImageResourceData(ImageResourceData&& ird) :
    width(ird.width),
    height(ird.height),
    num_channels(ird.num_channels),
    bytes(std::exchange(ird.bytes, nullptr))
    {}

    // Delete
    ~ImageResourceData(){
        if(bytes)
            delete[] bytes;
    }
};



class ImageResource {
private:
    ImageResourceData data;

    ImageRID rid = 0;

    void uploadToGPU();
    void freeBytes();
    void loadFILE(FILE* image_file, bool flip = false);
    void loadPATH(const std::string& image_path, bool flip = false);
public:
    ~ImageResource();
    inline ImageResource(){}
    ImageResource(const std::string& image_location, bool flip = false);
    void load(const std::string& image_location, bool flip = false);
    inline int getWidth() { return this->data.width; }
    inline int getHeight() { return this->data.height; }
    /**
     * Get the resource handle of the image on the gpu.
     */
    inline ImageRID getRID() {
        return this->rid;
    }
};

namespace GPUTexture {
    void openGLUpload(ImageRID& rid, int width, int height, int num_channels, const uint8_t* bytes);
    void openGLFree(const ImageRID& rid);

    /**
     * A seperate thread for texture upload jobs to be appended.
     */
    namespace SideLoader {
        void start();
        void uploadTexture(ImageRID& rid, ImageResourceData& texture_data);
        void deleteTexture(ImageRID& rid);
    };
}