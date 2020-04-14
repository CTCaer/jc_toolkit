/**
 * 2020 Jonathan Mendez
 */

#pragma once
#ifdef ImageLoadASYNC
#include <thread>
#endif

using ImageRID = unsigned int;

struct ImageResourceData {
    int width = 0;
    int height = 0;
    int num_channels = 0;
    uint8_t* bytes = nullptr;
#ifdef ImageLoadASYNC
    std::thread load_thread;
    bool load_thread_active = false;
#endif
    ImageResourceData();
};

class ImageResource {
private:
    ImageResourceData data;

    ImageRID rid = 0;

    void uploadToGPU();
    void unloadFromGPU(ImageRID replace_with = 0);
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