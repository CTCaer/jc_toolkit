/**
 * 2020 Jonathan Mendez
 */
#include <iostream>
#include <string>

#include "ImageLoad.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"

namespace GPUTexture {
    void openGLUpload(ImageRID& rid, int width, int height, int num_channels, const uint8_t* bytes){
        unsigned int pixel_fmt_src;
        switch(num_channels){
            case 1:
                pixel_fmt_src = GL_R8;
                break;
            case 2:
                pixel_fmt_src = GL_RG8;
                break;
            case 3: // RGB
                pixel_fmt_src = GL_RGB;
                break;
            case 4: // RGBA
                pixel_fmt_src = GL_RGBA;
                break;
            default:
                return;
        }
        GLuint tex_id = 0;
        glGenTextures(1, &tex_id);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        /*
        // Texture filtering.
        */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Without this there are crashes when deleting and assigning a new texture.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, pixel_fmt_src, GL_UNSIGNED_BYTE, bytes);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        rid = tex_id;
    }

    /**
     * Does not modify the value of rid.
     * "glDeleteTextures silently ignores 0's and names that do not correspond to existing textures." - khronos.org
     */
    void openGLFree(const ImageRID& rid){
        glDeleteTextures(1, (GLuint*)&rid);
    }
}

#ifdef ImageLoadASYNC
/**
 * Set a bool to its opposite value on destruction.
 * The size of this is 2 bytes.
 */
struct ScopedBool {
    bool* scoped_bool;
    bool set_val;
    ScopedBool(bool& bool_ref) :
    scoped_bool{&bool_ref},
    set_val{!bool_ref}
    {}
    ~ScopedBool() {
        *this->scoped_bool = this->set_val;
    }
};
#endif

#ifdef ImageLoadASYNC
static GLFWwindow* image_upload_in_bg_ctx = nullptr;
#endif

ImageResourceData::ImageResourceData() {
#ifdef ImageLoadASYNC
    if(image_upload_in_bg_ctx == nullptr){
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        image_upload_in_bg_ctx = glfwCreateWindow(640, 480, "image upload in bg ctx", NULL, glfwGetCurrentContext());
    }
#endif
}

inline void ImageResource::freeBytes() {
    if(this->data.bytes){
        stbi_image_free(this->data.bytes);
        this->data.bytes = NULL;
    }
}

inline void ImageResource::unloadFromGPU(ImageRID swap_val) {
    std::swap(swap_val, this->rid);
    if(swap_val)
        GPUTexture::openGLFree(swap_val);
}

ImageResource::~ImageResource() {
    this->freeBytes();
    this->unloadFromGPU();
}

ImageResource::ImageResource(const std::string& image_location, bool flip) {
    this->load(image_location, flip);
}

void ImageResource::load(const std::string& image_location, bool flip) {
    #ifdef ImageLoadASYNC
    // Check if there is a thread already loading an image.
    if(this->data.load_thread_active){
        // TODO: If we supply a uri to an image that does not exist, then this->uploadToGPU() will never get called, thus leading to the thread that was attempting to load the image never being called to join the main thread and leading to an indefinite "false" image loading process (Which is in this code block.)
        // There is a thread loading an image.
        std::cout << "An image is already in the process of being loaded." << std::endl;
        return; // Do not continue.
    }
    
    this->data.load_thread_active = true;
    this->data.load_thread = std::thread(&ImageResource::loadPATH, this, image_location, flip);
    this->data.load_thread.detach();
#else
    this->loadPATH(image_location, flip);
#endif
}

/**
 * Load the image from a local path.
 */
void ImageResource::loadPATH(const std::string& image_path, bool flip) {
#ifdef ImageLoadASYNC
    ScopedBool change_bool_on_out_of_scope(this->data.load_thread_active);
#endif
    FILE* image_file = fopen(image_path.c_str(), "rb");
    if(image_file == nullptr)
        return;
    this->loadFILE(image_file, flip);
    fclose(image_file);
    this->uploadToGPU();
}

/**
 * Load the image from an open file.
 */
void ImageResource::loadFILE(FILE* image_file, bool flip) {
	this->freeBytes(); // Free any possible bytes from a previous texture, if there were any.
	stbi_set_flip_vertically_on_load(flip);
	this->data.bytes = stbi_load_from_file(image_file, &this->data.width, &this->data.height, &this->data.num_channels, 0);
}

/**
 * Upload the image to the gpu as a texture.
 */
void ImageResource::uploadToGPU() {
    if(!this->data.bytes)
        return; // There is no image data to upload to the gpu.
    /**
     * TODO: Pass the image's bitmap data to a callback since various
     * graphics frameworks upload the data differently.
     */
    ImageRID new_img_rid = 0;
#ifdef ImageLoadASYNC
    glfwMakeContextCurrent(image_upload_in_bg_ctx);
#endif
    GPUTexture::openGLUpload(new_img_rid, this->data.width, this->data.height, this->data.num_channels, this->data.bytes);

#ifdef ImageLoadASYNC
    glfwMakeContextCurrent(NULL);
#endif
    
    this->unloadFromGPU(new_img_rid);
    this->freeBytes(); // We no longer need the image data in memory.
}
