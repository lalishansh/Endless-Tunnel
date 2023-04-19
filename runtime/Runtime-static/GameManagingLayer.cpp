//
// Created by Omen on 03-12-2022.
//

#include "GameManagingLayer.hpp"

namespace EndlessTunnel
{
	thread_local GameManagingLayer* GameManagingLayer::s_CurrentCTX = nullptr;
} // namespace EndlessTunnel
