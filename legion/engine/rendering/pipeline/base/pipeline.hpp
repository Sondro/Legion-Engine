#pragma once
#include <rendering/data/model.hpp>
#include <rendering/data/shader.hpp>
#include <rendering/data/framebuffer.hpp>
#include <rendering/components/renderable.hpp>
#include <rendering/components/camera.hpp>
#include <rendering/pipeline/base/renderstage.hpp>

#include <map>
#include <memory>
#include <any>

namespace legion::rendering
{
    class RenderPipelineBase
    {
        friend class Renderer;
    protected:
        sparse_map<id_type, framebuffer> m_framebuffers;
        sparse_map<id_type, std::any> m_metadata;

        static ecs::EcsRegistry* m_ecs;
        static schd::Scheduler* m_scheduler;
        static events::EventBus* m_eventBus;
    public:

        template<typename T>
        bool has_meta(std::string name);

        template<typename T, typename... Args>
        T* create_meta(std::string name, Args&&... args);

        template<typename T>
        T* get_meta(std::string name);

        framebuffer addFramebuffer(const std::string& name, GLenum target = GL_FRAMEBUFFER);
        framebuffer getFramebuffer(const std::string& name);

        virtual void init() LEGION_PURE;

        virtual void render(app::window& context, camera& cam, time::span deltaTime) LEGION_PURE;
    };

    template<typename Self>
    class RenderPipeline : public RenderPipelineBase
    {
        friend class Renderer;
    protected:
        static std::multimap<priority_type, std::unique_ptr<RenderStage>, std::greater<>> m_stages;

    public:
        template<typename StageType, inherits_from<StageType, RenderStage> = 0>
        static void attachStage();

        static void attachStage(std::unique_ptr<RenderStage>&& stage);

        virtual void setup() LEGION_PURE;

        void init() override;
        void render(app::window& context, camera& cam, time::span deltaTime) override;
    };
}

#include <rendering/pipeline/base/pipeline.inl>
