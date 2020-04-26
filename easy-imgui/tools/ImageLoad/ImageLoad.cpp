/*

MIT License

Copyright (c) 2020 Jonathan Mendez

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 */
#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "ImageLoad.hpp"


#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

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
    
    namespace SideLoader {
        namespace {
            static GLFWwindow* texture_sideload_ctx = nullptr;
            static std::queue<GPUTextureJob> texture_jobs0;
            static std::mutex job_queue_mutex;
            static std::condition_variable texture_job_avail;

            static void waitOnGPUTextureJobs(){
                while(true){
                    std::unique_lock<std::mutex> lock(job_queue_mutex);
                    // Wait indefinitely until a job is available.
                    texture_job_avail.wait(lock, []{ return texture_jobs0.size() > 0; });

                    auto& tex_job = texture_jobs0.front();

                    glfwMakeContextCurrent(texture_sideload_ctx);
                    tex_job();
                    glfwMakeContextCurrent(NULL);

                    texture_jobs0.pop(); // Remove the job from the queue since it has been processed.
                }

                glfwDestroyWindow(texture_sideload_ctx);
                texture_sideload_ctx = nullptr;
            }

        }

        void start() {
            if(texture_sideload_ctx)
                return; // Has already been successfully started.

            // Create a seperate glfw context and share texture resources with the current context.
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            texture_sideload_ctx = glfwCreateWindow(640, 480, "Texture sideloader.", NULL, glfwGetCurrentContext());
            std::thread sideload_thread(waitOnGPUTextureJobs);
            sideload_thread.detach(); // We do not need to wait on the thread any longer.
        }
        
        void addJob(GPUTextureJob job){
            std::lock_guard<std::mutex> lock(job_queue_mutex);
            texture_jobs0.push(job);
            texture_job_avail.notify_one();
        }
    }
}

inline void ImageResource::freeBytes() {
    if(this->data.bytes){
        stbi_image_free(this->data.bytes);
        this->data.bytes = NULL;
    }
}

void ImageResource::freeTexture() {
    GPUTexture::SideLoader::addJob([this](){
        GPUTexture::openGLFree(this->rid);
        this->rid = 0;
    });
}

ImageResource::~ImageResource() {
    this->freeBytes();
    this->freeTexture();
}

ImageResource::ImageResource(const std::string& image_location, bool to_gpu, bool flip) {
    this->load(image_location, flip);
}

void ImageResource::load(const std::string& image_location, bool to_gpu, bool flip) {
    FILE* image_file = fopen(image_location.c_str(), "rb");
    if(image_file == nullptr)
        return;
    this->loadFILE(image_file, flip);
    fclose(image_file);

    if(to_gpu)
        this->sendToGPU();
}

void ImageResource::sendToGPU() {
    if(!this->data.bytes)
        return; // There is no image data to upload to the gpu.
    GPUTexture::SideLoader::addJob([this](){
        GPUTexture::openGLFree(this->rid);
        GPUTexture::openGLUpload(this->rid, this->data.width, this->data.height, this->data.num_channels, this->data.bytes);
        delete[] this->data.bytes;
    });
}

/**
 * Load the image from an open file.
 */
void ImageResource::loadFILE(FILE* image_file, bool flip) {
	this->freeBytes(); // Free any possible bytes from a previous texture, if there were any.
	stbi_set_flip_vertically_on_load(flip);
	this->data.bytes = stbi_load_from_file(image_file, &this->data.width, &this->data.height, &this->data.num_channels, 0);
}
