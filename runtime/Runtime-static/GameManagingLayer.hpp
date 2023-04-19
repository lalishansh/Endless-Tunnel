#pragma once
// Created by Omen on 03-12-2022.
//

#include "ENGINE/engine.hpp"

#include <Windowing/Module.hpp>
#include <OpenGL/Module.hpp>
#include <DearImGui/Module.hpp>

#include "Renderer.hpp"
#include "Components.hpp"

#include <string>
#include <cstdio>
#include <chrono>

namespace EndlessTunnel
{
	static auto framestart_at = std::chrono::system_clock::now();

	struct {
		const u32 level_progress[12]
			= { 200, 800, 1700, 2400, 3200, 4200, 5500, 6700, 7800, 8100, 9000, 10000 };

		f32 last_frame_time = 0.0f;

		u8                lives         = 0;
		u8                level         = 0;
		u32               high_score    = 0;
		u32               obstacles     = 0;
		f64               score         = 0;
		f32               speed_max     = 0.001f;
		f32               speed         = speed_max;
		std::string       save_file     = "";
		std::string       info_string   = "";
		SETU::ECS::Entity collided_with = entt::null;

		void reset () {
			lives = 6, level = 0, obstacles = 9, score = 0, speed_max = 0.0006f, speed = 0,
			collided_with = entt::null;
		}
	}static game_state;

	static constexpr b8 debug_enable = false;

	static std::string g_String;
	static SETU::ECS::Registry *g_P_Registry;
	static SETU::ECS::Entity g_RootEntity;
	static SETU::ECS::Entity g_CameraParent;
	static SETU::ECS::Entity g_Camera;
	static void DebugUI (void *gui_call_ptr) {
        ImGui::Begin("Console");
        ImGui::TextUnformatted(g_String.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY (1.0f);
		ImGui::End();
        ImGui::Begin ("start screen");
        {
            if (ImGui::Button ("Hide DebugUI")) *(void **)(gui_call_ptr) = nullptr;
            ImGui::InputInt("TextureType", &Renderer::texture_type);
            {
                auto &transform = g_P_Registry->get<DataComponents::Transform> (g_RootEntity);
                ImGui::DragFloat3 ("posn", &transform.position[0], 0.1f);
                ImGui::DragFloat3 ("rotn", &transform.rotation[0], 0.1f);
                ImGui::DragFloat3 ("scale", &transform.scale[0], 0.1f);
            }
            {
                auto &transform = g_P_Registry->get<DataComponents::Transform> (g_Camera);
                auto  posn = glm::vec3 (transform.GetTransform()[3]);
                ImGui::Text ("camera posn: {%f, %f, %f}", posn[0], posn[1], posn[2]);
            }
            {
                auto &transform = g_P_Registry->get<DataComponents::Transform> (g_CameraParent);
                ImGui::DragFloat3 ("c_posn", &transform.position[0], 0.1f);
                ImGui::DragFloat3 ("c_rotn", &transform.rotation[0], 0.1f);
                ImGui::DragFloat3 ("c_scale", &transform.scale[0], 0.1f);
            }
            if (ImGui::CollapsingHeader ("Show Groups")) {
                u32 group_idx = 0;
                g_P_Registry->view<DataComponents::CubeArranger, DataComponents::Transform>().each (
                    [&] (
                        const SETU::ECS::Entity e, DataComponents::CubeArranger &cube_arranger,
                        DataComponents::Transform &transform
                    ) {
                        std::string name = "group #" + std::to_string (group_idx);
                        ImGui::PushID (group_idx);
                        if (ImGui::CollapsingHeader (name.c_str())) {
                            ImGui::Indent();
                            ImGui::NewLine();
                            for (u32 i = 0; i < DataComponents::cube_mat_dimension; ++i) {
                                for (u32 j = 0; j < DataComponents::cube_mat_dimension; ++j) {
                                    std::string temp
                                        = std::to_string (i) + ',' + std::to_string (j);
                                    ImGui::SameLine();
                                    ImGui::Checkbox (temp.c_str(), &cube_arranger.matrix[i][j]);
                                }
                                ImGui::NewLine();
                            }
                            ImGui::DragFloat3 ("grp posn", &transform.position[0], 0.1f);
                            ImGui::DragFloat3 ("grp rotn", &transform.rotation[0], 0.1f);
                            ImGui::DragFloat3 ("grp scale", &transform.scale[0], 0.1f);
                            ImGui::Unindent();
                        }
                        ImGui::PopID();
                        ++group_idx;
                    }
                );
            }
        }
        ImGui::End();
	};

	class GameManagingLayer
	{

	  public:
		static thread_local GameManagingLayer* s_CurrentCTX;
	  private:
		SETU::ECS::Registry m_Registry;

		SETU::Window           m_Window;
		SETU::OpenGL           m_OpenGl;
		SETU::DearImGui_OpenGL m_ImGuiOpenGl;

		Renderer m_Renderer;

		void(*m_StartScreen)(void*) = [](void*){

			// start screen
			auto screen_size = ImGui::GetMainViewport()->Size;
			//ImGui::SetNextWindowFocus();
			ImGui::SetNextWindowPos({ screen_size.x / 3, screen_size.y / 3 });
			ImGui::Begin("start screen", nullptr, ImGuiWindowFlags_NoMove |
			                                           ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
			{
				if (ImGui::Button ("Start Game", { screen_size.x / 3, screen_size.y * .1f })) {
					game_state.reset();
					s_CurrentCTX->clear_scene();
					s_CurrentCTX->create_scene();
					s_CurrentCTX->m_ImGuiOpenGl.GUICall = s_CurrentCTX->m_RunningScreen;
					Renderer::texture_type = 0;
				}
				if (ImGui::Button ("About Me", { screen_size.x / 3, screen_size.y * .1f }))
					s_CurrentCTX->m_ImGuiOpenGl.GUICall = s_CurrentCTX->m_AboutScreen;

				auto str = "High Score: " + std::to_string(game_state.high_score);

				ImGui::SetCursorPosX (
					(ImGui::GetWindowWidth() - ImGui::CalcTextSize (str.c_str()).x) * .5f
				);
				ImGui::TextUnformatted (str.c_str());
			}
			ImGui::End();
		};

		void(*m_RunningScreen)(void*) = [](void*){
					ImGui::SetNextWindowPos ({ 10, 10 });
					ImGui::Begin (
						"scr", NULL,
						ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
							| ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDocking
							| ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize
					);
					ImGui::GetFont()->FontSize *= .5f;
					ImGui::Text (
						"Score: %ld, Lives: %d", u64 (game_state.score), i32 (game_state.lives)
					);
					ImGui::GetFont()->FontSize *= 2.f;
					ImGui::NewLine();
					if (!game_state.info_string.empty()){
						ImGui::TextUnformatted(game_state.info_string.c_str());
					}
					ImGui::End();
		};

		void(*m_AboutScreen)(void*) = [](void*){
					const char* text=
R"(
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc
venenatis ante neque, et malesuada dui vehicula a. Cras a
volutpat mauris, vitae lacinia odio. Duis id fringilla purus,
quis luctus nisi. Nullam placerat ipsum id justo pellentesque
blandit. In lobortis, enim ac lobortis fermentum, magna ante
scelerisque turpis, eu porta eros felis at massa. Suspendisse
potenti.
)";
					auto size = ImGui::GetMainViewport()->Size;
					auto ct_size = ImGui::CalcTextSize(text);
					ImGui::SetNextWindowPos({(size.x - ct_size.x)*.5f, (size.y - ct_size.y)*.4f });
					ImGui::Begin (
						"scr", NULL, ImGuiWindowFlags_NoDecoration |
							ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
							| ImGuiWindowFlags_NoDocking
							| ImGuiWindowFlags_AlwaysAutoResize
					);
					//ImGui::GetFont()->FontSize *= .5f;
					if (ImGui::Button("< Go Back", {-1, ct_size.y*.2f}))
						s_CurrentCTX->m_ImGuiOpenGl.GUICall = s_CurrentCTX->m_StartScreen;
					ImGui::TextUnformatted (text);
					//ImGui::GetFont()->FontSize *= 2.f;
					ImGui::End();
		};

		void(*m_GameOverScreen)(void*) = [](void*) {
			if (game_state.high_score < game_state.score) {
				FILE *fptr = nullptr;
				fptr = fopen (game_state.save_file.c_str(), "w+");
				fprintf (fptr, "hs: %ld", i64 (game_state.score));
				game_state.high_score = game_state.score;
				fclose (fptr);
			}

			auto size = ImGui::GetMainViewport()->Size;

			const char *text    = "!! GAME OVER !!";
			auto        ct_size = ImGui::CalcTextSize (text);
			ct_size.x *= 4.3, ct_size.y *= 4;
			ImGui::SetNextWindowPos ({ (size.x - ct_size.x) * .5f, (size.y - ct_size.y) * .4f });
			ImGui::SetNextWindowSize ({ ct_size.x, ct_size.y * 4.f });
			ImGui::Begin (
				"scr", NULL,
				ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoDecoration
					/*| ImGuiWindowFlags_NoBackground */
					| ImGuiWindowFlags_AlwaysAutoResize
			);
			ImGui::GetFont()->FontSize *= .25f;
			ImGui::TextUnformatted (text);
			ImGui::NewLine();
			ImGui::NewLine();
			ImGui::GetFont()->FontSize *= 4.f;
			if (ImGui::Button ("Go Home <", { -1, -1 }))
				s_CurrentCTX->m_ImGuiOpenGl.GUICall = s_CurrentCTX->m_StartScreen;
			ImGui::End();
		};
	  public:
		void Deploy () {
			s_CurrentCTX = this;

			m_Window.OnAwake (&m_Registry);
			m_OpenGl.OnAwake (&m_Registry);
			m_ImGuiOpenGl.OnAwake (&m_Registry);
			m_Renderer.OnAwake (&m_Registry);

			create_scene();
		}
		void create_scene(){
			auto create_cube = [&](SETU::ECS::Entity parent, const glm::vec3 posn,
			                        const glm::vec3 scale, const u32 idx = 0,
			                        DataComponents::Cube::TYPE type =
			                            DataComponents::Cube::TYPE::BLOCKADE
			                        ) {
				auto entity = m_Registry.create();

				auto &transform = m_Registry.emplace<DataComponents::Transform> (entity, parent);
				transform.OnAwake (entity, m_Registry);
				transform.position = posn;
				transform.scale = scale;

				auto &cube_component = m_Registry.emplace<DataComponents::Cube> (entity, type, idx);
				cube_component.OnAwake (entity, transform, m_Registry);
			};
			auto root = m_Registry.create();
			m_Registry.emplace<DataComponents::Transform>(root);
			auto camera_root = m_Registry.create();
			m_Registry.emplace<DataComponents::Transform>(camera_root);
			auto camera = m_Registry.create();
			m_Registry.emplace<DataComponents::Transform>(camera, camera_root);
			m_Registry.emplace<DataComponents::Camera>(camera);
			g_Camera = camera;
			m_Registry.emplace<DataComponents::Script> (
				camera,
				DataComponents::Script{
					.on_update =
						[] (const SETU::ECS::Entity entity, DataComponents::Transform &transform,
			                const SETU::ECS::Registry &registry, u64 &data) {
							// check collision
							glm::mat4 inv_transform = glm::inverse (
								registry.get<DataComponents::Transform> (transform.parent)
									.GetTransform()
							);
							auto          real_posn = glm::vec3 (transform.GetTransform()[3]);
							constexpr f32 radius    = .2f;
							bool update = false;
							registry.view<DataComponents::Transform, DataComponents::Cube>()
								.each ([&] (
										   const SETU::ECS::Entity                entity_cube,
										   const DataComponents::Transform &transform_cube,
										   const DataComponents::Cube            &cube
									   ) {
									glm::vec3 real_posn_cube (transform_cube.GetTransform()[3]);
									auto real_scale_cube_z =
										glm::length (glm::vec3 (transform_cube.GetTransform()[2]));
									if ((real_posn.z - real_scale_cube_z*.5f - radius) > real_posn.z)
										return;
									glm::vec3 real_scale_cube{
											glm::length (glm::vec3 (transform_cube.GetTransform()[0])),
											glm::length (glm::vec3 (transform_cube.GetTransform()[1])),
											real_scale_cube_z
										};

									glm::vec3 min_xy = real_posn_cube - (real_scale_cube*.5f);
									glm::vec3 max_xy = min_xy + real_scale_cube;
									if (real_posn.z < min_xy.z || real_posn.z > max_xy.z)
										return;
									switch (cube.Type) {
									case DataComponents::Cube::TYPE::WALL: {
										min_xy += 2*radius;
										max_xy -= 2*radius;

										if (real_posn.x < min_xy.x || max_xy.x < real_posn.x
						                    || real_posn.y < min_xy.y || max_xy.y < real_posn.y)
										{
										// 	g_String += "## HIT-wall ##\n";
											real_posn.x
												= std::clamp (real_posn.x, min_xy.x, max_xy.x);
											real_posn.y
												= std::clamp (real_posn.y, min_xy.y, max_xy.y);
											update = true;
										}
									} break;
									case DataComponents::Cube::TYPE::BLOCKADE: {
										auto SqDistPointAABB = [](glm::vec3 p, glm::vec3 b_min,
						                                           glm::vec3 b_max) {
											float sqDist = 0.0f;
											for (int i = 0; i < 3; i++) {
												// for each axis count any excess distance outside box extents
												float v = p[i];
												if (v < b_min[i])sqDist += (b_min[i] - v) * (b_min[i] - v);
												if (v > b_max[i])sqDist += (v - b_max[i]) * (v - b_max[i]);
											}
											return sqDist;
										};
										{
											// Compute squared distance between sphere center and AABB the sqrt(dist) is fine to use as well, but this is faster.
											float sqDist = SqDistPointAABB (real_posn, min_xy,
							                                                max_xy);

											// Sphere and AABB intersect if the (squared) distance between them is less than the (squared) sphere radius.
											if(sqDist <= radius * radius) {
												game_state.speed = -abs(game_state.speed);
												if (game_state.collided_with != transform.parent)
													game_state.collided_with = transform.parent,
													--game_state.lives;
												// 	g_String += "## HIT-$$$- ##\n";
											}
										}

									} break;
									default: break;
									};
								});

							if (update)
								transform.position = inv_transform*glm::vec4(real_posn, 1);
							else if (data) {
								union my_data {
									struct {
										f32 x, y;
									};
									u64 data_cast;
								};
								my_data tap_dirn{};
								tap_dirn.data_cast = data;
								//// g_String           += '{' + std::to_string (tap_dirn.x) + ','
					            //          + std::to_string (tap_dirn.y) + "} tap_dirn\n";
								transform.position.x
									+= tap_dirn.x * 10 * game_state.last_frame_time;
								transform.position.y
									-= tap_dirn.y * 10 * game_state.last_frame_time;

							}
						},
					.on_event =
						[] (SETU::Event_Base &event, const SETU::ECS::Entity entity,
			                DataComponents::Transform &transform,
			                const SETU::ECS::Registry &registry, u64 &data) -> b8 {
						SETU::Event_DispatchByType<SETU::Event_Input_TouchStatePressed> (
							event,
							[&] (SETU::Event_Input_TouchStatePressed &e) {
								auto      size  = ImGui::GetMainViewport()->Size;
								auto      coord = e.Get_TouchPosition();
								glm::vec2 dirn{ size.x * .5f - coord.first,
						                        size.y * .5f - coord.second };
								dirn = glm::normalize (dirn);

								union my_data {
									struct {
										f32 x, y;
									};
									u64 data_cast;
								};
								my_data pack{};
								pack.x = dirn.x, pack.y = dirn.y;
								data     = pack.data_cast;
								auto m_p = e.Get_TouchPosition();
							// 	g_String += std::to_string (m_p.first) + ", "
					        //         + std::to_string (m_p.second) + "-TP\n";
								return true;
							}
						);
						SETU::Event_DispatchByType<SETU::Event_Input_TouchStateDragged> (
							event,
							[&] (SETU::Event_Input_TouchStateDragged &e) {
								auto      size  = ImGui::GetMainViewport()->Size;
								auto      coord = e.Get_TouchPosition();
								glm::vec2 dirn{ size.x * .5f - coord.first,
						                        size.y * .5f - coord.second };
								dirn = glm::normalize (dirn);

								union my_data {
									struct {
										f32 x, y;
									};
									u64 data_cast;
								};
								my_data pack{};
								pack.x = dirn.x, pack.y = dirn.y;
								data     = pack.data_cast;
								auto m_p = e.Get_TouchPosition();
								//// g_String += std::to_string (m_p.first) + ", "
					            //     + std::to_string (m_p.second) + "-TD\n";
								return true;
							}
						);
						SETU::Event_DispatchByType<SETU::Event_Input_TouchStateReleased> (
							event,
							[&] (SETU::Event_Input_TouchStateReleased &e) {
								union my_data {
									struct {
										f32 x, y;
									};
									u64 data_cast;
								};
								my_data pack{};
								pack.x = 0, pack.y = 0;
								data = pack.data_cast;
							// 	g_String += std::to_string (e.Get_FingerTouchID()) + "-TR\n";
								return true;
							}
						);
						Event_DispatchByType<SETU::Event_Input_KeyboardKeyPressed> (
							event,
							[&] (SETU::Event_Input_KeyboardKeyPressed &e) {
							// 	g_String += std::string() + c8 (e.Get_KeyCode()) + "-Down\n";
								return true;
							}
						);
						SETU::Event_DispatchByType<SETU::Event_Input_KeyboardKeyReleased> (
							event,
							[&] (SETU::Event_Input_KeyboardKeyReleased &e) {
							// 	g_String += std::string() + c8 (e.Get_KeyCode()) + "-Up\n";
								return true;
							}
						);
						SETU::Event_DispatchByType<SETU::Event_Input_KeyboardKeyRepeated> (
							event,
							[&] (SETU::Event_Input_KeyboardKeyRepeated &e) {
							// 	g_String += std::string() + c8 (e.Get_KeyCode()) + "-Re\n";
								return true;
							}
						);
						// move
						return false;
					}
				}
			);


			static constexpr u32  num_tunnel_groups = 6;
			for (u32 i = 0; i < num_tunnel_groups; ++i) {
				static constexpr float   len_tunnel_part  = 20.f;
				static constexpr u32 num_tunnel_parts = 1;
				auto          group            = m_Registry.create();
				m_Registry.emplace<DataComponents::Transform> (group, root).position = glm::vec3 (
					0.f, 0.f, (-len_tunnel_part / 2) - (i * num_tunnel_parts * len_tunnel_part)
				);
				m_Registry.emplace<DataComponents::CubeArranger> (group);
				m_Registry.emplace<DataComponents::Script>(
					group, DataComponents::Script{
							   .on_update = [](const SETU::ECS::Entity entity,
				                               DataComponents::Transform& transform, const
				                               SETU::ECS::Registry& registry, u64 &data) {
								    transform.position.z += game_state.speed*ImGui::GetIO()
					                                                             .Framerate;
									b8 *array = (b8*)registry.get<DataComponents::CubeArranger>
									    (entity).matrix;

									if (transform.position.z > 11) {
										transform.position.z -= num_tunnel_groups * num_tunnel_parts
						                                      * len_tunnel_part;
										constexpr u32 N = DataComponents::cube_mat_dimension
						                                * DataComponents::cube_mat_dimension;
										u32 M = game_state.obstacles;
										game_state.collided_with = entt::null;

										for (unsigned i = 0, m = M; i < N; ++i)
											if (rand() % (N - i) < m) {
												array[i] = true;
												--m;
											} else array[i] = false;
									}
						}});

				for (u32 j = 0; j < num_tunnel_parts; ++j) {
					const float offsetZ = j * len_tunnel_part;

					static_assert (DataComponents::cube_mat_dimension == 5);
					create_cube (group, { -2.f, -2.f, 0.f - offsetZ }, glm::vec3 (.9f), 0);
					create_cube (group, { -1.f, -2.f, 0.f - offsetZ }, glm::vec3 (.9f), 1);
					create_cube (group, { 0.f, -2.f, 0.f - offsetZ }, glm::vec3 (.9f), 2);
					create_cube (group, { 1.f, -2.f, 0.f - offsetZ }, glm::vec3 (.9f), 3);
					create_cube (group, { 2.f, -2.f, 0.f - offsetZ }, glm::vec3 (.9f), 4);

					create_cube (group, { -2.f, -1.f, 0.f - offsetZ }, glm::vec3 (.9f), 5);
					create_cube (group, { -1.f, -1.f, 0.f - offsetZ }, glm::vec3 (.9f), 6);
					create_cube (group, { 0.f, -1.f, 0.f - offsetZ }, glm::vec3 (.9f), 7);
					create_cube (group, { 1.f, -1.f, 0.f - offsetZ }, glm::vec3 (.9f), 8);
					create_cube (group, { 2.f, -1.f, 0.f - offsetZ }, glm::vec3 (.9f), 9);

					create_cube (group, { -2.f, 0.f, 0.f - offsetZ }, glm::vec3 (.9f), 10);
					create_cube (group, { -1.f, 0.f, 0.f - offsetZ }, glm::vec3 (.9f), 11);
					create_cube (group, { 0.f, 0.f, 0.f - offsetZ }, glm::vec3 (.9f), 12);
					create_cube (group, { 1.f, 0.f, 0.f - offsetZ }, glm::vec3 (.9f), 13);
					create_cube (group, { 2.f, 0.f, 0.f - offsetZ }, glm::vec3 (.9f), 14);

					create_cube (group, { -2.f, 1.f, 0.f - offsetZ }, glm::vec3 (.9f), 15);
					create_cube (group, { -1.f, 1.f, 0.f - offsetZ }, glm::vec3 (.9f), 16);
					create_cube (group, { 0.f, 1.f, 0.f - offsetZ }, glm::vec3 (.9f), 17);
					create_cube (group, { 1.f, 1.f, 0.f - offsetZ }, glm::vec3 (.9f), 18);
					create_cube (group, { 2.f, 1.f, 0.f - offsetZ }, glm::vec3 (.9f), 19);

					create_cube (group, { -2.f, 2.f, 0.f - offsetZ }, glm::vec3 (.9f), 20);
					create_cube (group, { -1.f, 2.f, 0.f - offsetZ }, glm::vec3 (.9f), 21);
					create_cube (group, { 0.f, 2.f, 0.f - offsetZ }, glm::vec3 (.9f), 22);
					create_cube (group, { 1.f, 2.f, 0.f - offsetZ }, glm::vec3 (.9f), 23);
					create_cube (group, { 2.f, 2.f, 0.f - offsetZ }, glm::vec3 (.9f), 24);
					create_cube (
						group, { 0.f, 0.f, 0.f - offsetZ }, glm::vec3 (DataComponents::cube_mat_dimension, DataComponents::cube_mat_dimension, len_tunnel_part), 0,
						DataComponents::Cube::TYPE::WALL
					);
				}
			}

			g_CameraParent        = camera_root;
			m_ImGuiOpenGl.GUICall = m_StartScreen;
			{
				char *data_save_dir = nullptr;
				SETU::Window::Handle (SETU::Window::platform_enum::SAVE_PATH, &data_save_dir);

				game_state.save_file = std::string (data_save_dir) + "/score.dat";
				FILE *fptr = nullptr;
				fptr = fopen (game_state.save_file.c_str(), "r+");
				if (fptr == nullptr) { // if file does not exist, create it
					fptr = fopen (game_state.save_file.c_str(), "w+");
					fprintf(fptr, "hs: %ld", i64(0));
				}else {
					i64  hs = 0;
					fscanf(fptr, "hs: %ld", &hs);
					game_state.high_score = hs;
				}
				fclose(fptr);
			}
			if constexpr (debug_enable) {
				g_P_Registry           = &m_Registry;
				g_RootEntity           = root;
				g_Camera               = camera;
				m_ImGuiOpenGl.GUICall  = DebugUI;
				m_ImGuiOpenGl.UserData = &m_ImGuiOpenGl.GUICall;
			}
		}

		void Update () {
			{
				auto now = std::chrono::system_clock::now();
				auto delta = now - framestart_at;
				framestart_at = now;
				game_state.last_frame_time = std::chrono::duration<float>(delta).count();
			}
			m_Window.OnUpdate (&m_Registry);
			m_OpenGl.OnUpdate (&m_Registry);
			m_ImGuiOpenGl.OnUpdate (&m_Registry);
			m_Renderer.OnUpdate (&m_Registry);

			if (game_state.lives > 0) {
				m_Registry.view<DataComponents::Transform>().each (
					[&] (const SETU::ECS::Entity entity, DataComponents::Transform &transform) {
						transform.OnUpdate (entity, m_Registry);
					}
				);

				m_Registry.view<DataComponents::Script, DataComponents::Transform>().each (
					[&] (
						const SETU::ECS::Entity entity, DataComponents::Script &script,
						DataComponents::Transform &transform
					) { script.OnUpdate (entity, transform, m_Registry); }
				);

				m_Registry.view<DataComponents::Cube, DataComponents::Transform>().each (
					[&] (
						const SETU::ECS::Entity entity, DataComponents::Cube &cube,
						DataComponents::Transform &transform
					) { cube.OnUpdate (entity, transform, m_Registry); }
				);

				m_Registry.view<DataComponents::Camera, DataComponents::Transform>().each (
					[&] (
						const SETU::ECS::Entity entity, DataComponents::Camera &camera,
						DataComponents::Transform &transform
					) { camera.OnUpdate (entity, transform, m_Registry); }
				);

				if constexpr (debug_enable) {
					static f32 wait_for_5_sec = 0;
					if (m_ImGuiOpenGl.GUICall == nullptr) {
						wait_for_5_sec += game_state.last_frame_time;
						if (wait_for_5_sec > 5) {
							wait_for_5_sec         = 0;
							m_ImGuiOpenGl.GUICall  = DebugUI;
							m_ImGuiOpenGl.UserData = &m_ImGuiOpenGl.GUICall;
						}
					}
				}

				{
					static f32 chaos_start = 300.f;
					if (game_state.score > chaos_start) {
						game_state.info_string = "chaos ongoing!";

						Renderer::texture_type = 2;
						if (game_state.score > chaos_start + 300.f) {
							game_state.info_string = "! phew !";
							chaos_start            += fabs (3000);
							Renderer::texture_type = 1;
						}
					}else if (game_state.score > chaos_start-300.f)
						game_state.info_string = "chaos incoming!";
				}

				if (game_state.score > game_state.level_progress[game_state.level]) {
					game_state.level++;
					game_state.info_string = "! levelUp to " + std::to_string(game_state.level) + " !";
					if (game_state.level == 2 || game_state.level == 6 || game_state.level == 8)
						Renderer::texture_type = 0;
					game_state.speed_max *= 1.1f;
					game_state.obstacles += (game_state.level & 1);
				};

				if (game_state.level > 1) {
					const float rotn_speed
						= game_state.speed * game_state.last_frame_time * game_state.level;
					m_Registry.get<DataComponents::Transform> (g_CameraParent).rotation.z
						+= rotn_speed;
				}

				if (game_state.speed < game_state.speed_max)
					game_state.speed += game_state.speed_max * 0.1f;
				game_state.score += game_state.speed * 10000 * game_state.last_frame_time;
			} else {
				if (m_ImGuiOpenGl.GUICall == m_RunningScreen) {
					m_ImGuiOpenGl.GUICall = m_GameOverScreen;
				}
			}

			m_Renderer.OnPostUpdate (&m_Registry);
			m_ImGuiOpenGl.OnPostUpdate (&m_Registry);
			m_OpenGl.OnPostUpdate (&m_Registry);
			m_Window.OnPostUpdate (&m_Registry);
		}

		b8 Event (SETU::Event_Base &e) {
			bool handled = false;
			if (!handled) handled |= m_Window.OnEvent (e, &m_Registry);
			if (!handled) handled |= m_OpenGl.OnEvent (e, &m_Registry);
			if (!handled) handled |= m_ImGuiOpenGl.OnEvent (e, &m_Registry);
			if (!handled) handled |= m_Renderer.OnEvent (e, &m_Registry);
			if (!handled) {
				m_Registry.view<DataComponents::Script, DataComponents::Transform>().each(
				[&](const SETU::ECS::Entity entity, DataComponents::Script& script,
			         DataComponents::Transform& transform) {
						if (!handled)
							handled |= script.OnEvent (e, entity, transform, m_Registry);
				});
			}
			return handled;
		}

		void clear_scene(){
			m_Registry.clear();

		}
		void Dispose () {
			m_Renderer.OnSleep (&m_Registry);
			m_ImGuiOpenGl.OnSleep (&m_Registry);
			m_OpenGl.OnSleep (&m_Registry);
			m_Window.OnSleep (&m_Registry);

			clear_scene();
		}
	};

} // namespace EndlessTunnel

// EOF: ENDLESSTUNNEL_GAMEMANAGINGLAYER_HPP
