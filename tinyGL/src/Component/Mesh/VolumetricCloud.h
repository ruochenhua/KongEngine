#pragma once
#include "MeshComponent.h"

namespace Kong
{
    class CloudModel
    {
    public:
        CloudModel();
        void Update();
    private:
        shared_ptr<Shader> perlin_worley_comp_shader;
        shared_ptr<Shader> worley_comp_shader;
        shared_ptr<Shader> cloud_compute_shader;
        // control parameter
        float coverage, cloud_speed, crispiness, curliness, density, absorption;
        float earth_radius, sphere_inner_radius, sphere_outer_radius;
        float perlin_frequency;
        bool enable_god_rays;
        bool enable_powder;
        bool post_process;

        glm::vec3 cloud_color_top, cloud_color_bottom;
        glm::vec3 seed, old_seed;

        GLuint perlin_texture, worley32_texture, weather_texutre;
    };
    
    class VolumetricCloud : public CMeshComponent
    {
    public:
        VolumetricCloud();

        void SimpleDraw() override;

    private:
        GLuint perlin_texture = GL_NONE;
        GLuint worly_texture = GL_NONE;
        GLuint weather_texture = GL_NONE;

        // cloud process
        GLuint cloud_process_fbo = GL_NONE;
        GLuint cloud_process_rbo = GL_NONE;
        GLuint cloud_process_depth_tex = GL_NONE;
        
    };
}

