#include <ENGINE/engine.hpp>

#include "GameManagingLayer.hpp"

using CONTEXT = struct android_app *;

class DummyComponent: public SETU::Base::ROOT_Component
{
   public:
	inline constexpr void OnAwake(const SETU::ECS::Registry*) noexcept override {};
	inline constexpr void OnUpdate(const SETU::ECS::Registry*) noexcept override {};
	inline constexpr void OnPostUpdate(const SETU::ECS::Registry*) noexcept override {};
	inline constexpr b8 OnEvent(SETU::Event_Base&, const SETU::ECS::Registry*) noexcept override
		{ return false; };
	inline constexpr void OnSleep(const SETU::ECS::Registry*) noexcept override {};
};

#define ROOT_COMPONENTS        (DummyComponent)

#define LAYERS                 (EndlessTunnel::GameManagingLayer)

MAKE_SETU_ENGINE_INSTANCE(Runtime, ROOT_COMPONENTS, LAYERS);

Runtime* application = nullptr;
int main (int argc, const c8 *argv[]){
    if (application == nullptr)
        application = new Runtime;
    Runtime::Setup(std::span (argv, argc));
	Runtime::Live();
    delete application;

    return 0;
}

#if PLATFORM_ANDROID
#include <android_native_app_glue.h>
struct android_app *g_App = nullptr;
extern "C" void android_main (CONTEXT ctx) {
	g_App       = ctx;
	application = new Runtime;

	{ // Wait until app initialized
		b8 initialized = false;

		ctx->userData = &initialized;
		ctx->onAppCmd = [] (CONTEXT app, int32_t appCmd) {
			if (appCmd == APP_CMD_INIT_WINDOW) {
				*((b8 *)app->userData) = true;
				app->userData          = nullptr;
			}
		};

		while (!initialized) { // poll until window created by OS
			int                         out_events;
			struct android_poll_source *out_data;
			while (ALooper_pollAll (0, NULL, &out_events, (void **)&out_data) >= 0)
				if (out_data != NULL) out_data->process (ctx, out_data);
		}
	}

	const c8 *app_name = "app";
	main (1, &app_name);
}
#endif
