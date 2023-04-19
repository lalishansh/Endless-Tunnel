#pragma once

// Created by Omen on 04-12-2022.
//

#include <ENGINE/Base/Component.hpp>

namespace EndlessTunnel
{

	class Renderer : public SETU::Base::ROOT_Component
	{
	  public:
		static i32   texture_type;
		virtual void OnAwake (const SETU::ECS::Registry *registry) override;
		virtual void OnUpdate (const SETU::ECS::Registry *registry) override;
		virtual b8   OnEvent (SETU::Event_Base &e, const SETU::ECS::Registry *registry) override;
		virtual void OnPostUpdate (const SETU::ECS::Registry *registry) override;
		virtual void OnSleep (const SETU::ECS::Registry *registry) override;
	};

} // namespace EndlessTunnel

// EOF: ENDLESSTUNNEL_RENDERER_HPP
