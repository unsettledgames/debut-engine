#include <Panels/InspectorPanel.h>
#include <Utils/EditorCache.h>
#include <Debut/Core/Instrumentor.h>

#include <Debut/Rendering/Resources/Mesh.h>
#include <Debut/Rendering/Material.h>
#include <Debut/Scene/Components.h>
#include <Debut/AssetManager/AssetManager.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <yaml-cpp/yaml.h>
#include <imgui.h>

#include <fstream>
#include <filesystem>
#include <sstream>

namespace Debut
{
	void InspectorPanel::OnImGuiRender()
	{
		ImGui::Begin("Inspector");

		if (m_SelectionContext)
			DrawComponents(m_SelectionContext);

		ImGui::End();

		m_PrevSelectionContext = m_SelectionContext;
	}

	void InspectorPanel::DrawComponents(Entity& entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>();

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			memcpy(buffer, tag.Name.c_str(), tag.Name.size());

			if (ImGui::InputText("##", buffer, sizeof(buffer)))
			{
				tag.Name = std::string(buffer);
			}
		}

		ImGui::SameLine();

		ImGui::PushItemWidth(-1);
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddReplaceComponent");
		DrawAddReplaceComponentMenu<TagComponent>();
		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
			{
				glm::vec3 rotDeg = glm::degrees(component.Rotation);
				ImGuiUtils::RGBVec3("Position", { "X", "Y", "Z" }, { &component.Translation.x, &component.Translation.y, &component.Translation.z });
				ImGuiUtils::RGBVec3("Rotation", { "X", "Y", "Z" }, { &rotDeg.x, &rotDeg.y, &rotDeg.z });
				ImGuiUtils::RGBVec3("Scale", { "X", "Y", "Z" }, { &component.Scale.x,&component.Scale.y, &component.Scale.z }, 1);

				component.Rotation = glm::radians(rotDeg);
			});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
			{
				auto& camera = component.Camera;

				const char* projectionTypes[] = { "Perspective", "Orthographic" };
				const char* currProjType = projectionTypes[(int)camera.GetProjectionType()];
				const char* finalProjType = nullptr;

				if (ImGuiUtils::Combo("Projection", projectionTypes, 2, &currProjType, &finalProjType))
					camera.SetProjectionType(SceneCamera::StringToProjType(finalProjType));
				ImGui::Checkbox("Set as primary", &component.Primary);
				ImGui::Dummy({ 0.0f, 0.5f });

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float perspFOV = glm::degrees(camera.GetFOV());
					if (ImGuiUtils::DragFloat("Vertical FOV", &perspFOV, 0.15f))
						camera.SetFOV(glm::radians(perspFOV));

					float perspNear = camera.GetNearPlane();
					if (ImGuiUtils::DragFloat("Near clip", &perspNear, 0.15f))
						camera.SetNearPlane(perspNear);

					float perspFar = camera.GetFarPlane();
					if (ImGuiUtils::DragFloat("Far clip", &perspFar, 0.15f))
						camera.SetFarPlane(perspFar);
				}
				else
				{
					float orthoSize = camera.GetOrthoSize();
					if (ImGuiUtils::DragFloat("Size", &orthoSize, 0.15f))
						camera.SetOrthoSize(orthoSize);

					float orthoNear = camera.GetNearPlane();
					if (ImGuiUtils::DragFloat("Near clip", &orthoNear, 0.15f))
						camera.SetNearPlane(orthoNear);

					float orthoFar = camera.GetFarPlane();
					if (ImGuiUtils::DragFloat("Far clip", &orthoFar, 0.15f))
						camera.SetFarPlane(orthoFar);
				}

				UUID pps = ImGuiUtils::DragDestination("Post processing stack", ".postps", component.PostProcessing);
				if (pps != 0)
					component.PostProcessing = pps;
			});


		DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [&](auto& component)
			{
				ImGuiUtils::StartColumns(2, { 100, (uint32_t)ImGui::GetContentRegionAvail().x - 100 });
				static MeshMetadata meshData; 
				static MaterialMetadata materialData;

				if ((entt::entity)m_PrevSelectionContext != (entt::entity)m_SelectionContext)
				{
					meshData = Mesh::GetMetadata(component.Mesh);
					materialData = Material::GetMetadata(component.Material);
				}

				// Mesh reference
				ImGui::LabelText("##meshlabel", "Mesh");
				ImGui::NextColumn();

				ImGui::Button((meshData.Name + "##mesh").c_str(), { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.2f });
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_DATA"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path pathStr(path);

						if (pathStr.extension() == ".mesh")
						{
							pathStr = pathStr.replace_extension();
							std::ifstream meta(AssetManager::s_MetadataDir + pathStr.string() + ".meta");

							if (meta.good())
							{
								std::stringstream ss;
								ss << meta.rdbuf();
								YAML::Node metaData = YAML::Load(ss.str());

								component.Mesh = metaData["ID"].as<uint64_t>();
							}
						}
					}

					ImGui::EndDragDropTarget();
				}

				ImGui::NextColumn();

				// Material reference
				// Mesh reference
				ImGui::LabelText("##materiallabel", "Material");
				ImGui::NextColumn();

				ImGui::Button((materialData.Name + "##material").c_str(), { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.2f });
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_DATA"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path pathStr(path);

						std::ifstream meta;
						std::ifstream file(pathStr.string());

						if (pathStr.extension() == ".mat")
						{
							if (file.good())
							{
								meta = std::ifstream(pathStr.string() + ".meta");
							}
							else
							{
								pathStr = pathStr.replace_extension();
								meta = std::ifstream(AssetManager::s_MetadataDir + pathStr.string() + ".meta");
							}

							if (meta.good())
							{
								std::stringstream ss;
								ss << meta.rdbuf();
								YAML::Node metaData = YAML::Load(ss.str());

								component.Material = metaData["ID"].as<uint64_t>();
							}
						}
					}

					ImGui::EndDragDropTarget();
				}

				ImGui::NextColumn();

				ImGuiUtils::ResetColumns();
			});


		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
			{
				// Color
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color), 0.15f);
				// Texture
				ImTextureID buttonTexture;
				// Use a blank texture if the user hasn't already set one, otherwise use the one submitted by the user	
				buttonTexture = component.Texture == 0 ?
					(ImTextureID)EditorCache::Textures().Get("assets\\textures\\empty_texture.png")->GetRendererID() :
					(ImTextureID)AssetManager::Request<Texture2D>(component.Texture)->GetRendererID();

				ImGuiUtils::StartColumns(3, { 80, 100, 100 });

				// Accept PNG files to use as textures for the sprite renderer
				ImGui::ImageButton(buttonTexture, ImVec2(64.0f, 64.0f), { 0, 1 }, { 1, 0 });
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_DATA"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path pathStr(path);

						if (pathStr.extension() == ".png" || pathStr.extension() == ".jpg")
						{
							Ref<Texture2D> selectedTexture = AssetManager::Request<Texture2D>(pathStr.string());
							component.Texture = selectedTexture->GetID();
						}
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::NextColumn();

				ImGui::SameLine();
				ImGuiUtils::DragFloat("Tiling factor", &component.TilingFactor, 0.1f);

			});

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
				const char* currBodyType = bodyTypeStrings[(int)component.Type];
				const char* finalBodyType = nullptr;

				if (ImGuiUtils::Combo("Body type", bodyTypeStrings, 3, &currBodyType, &finalBodyType))
					component.Type = Rigidbody2DComponent::StrToRigidbody2DType(finalBodyType);

				ImGui::Checkbox("Fixed rotation", &component.FixedRotation);
			});

		DrawComponent<Rigidbody3DComponent>("Rigidbody 3D", entity, [](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
				const char* currBodyType = bodyTypeStrings[(int)component.Type];
				const char* finalBodyType = nullptr;

				if (ImGuiUtils::Combo("Body type", bodyTypeStrings, 3, &currBodyType, &finalBodyType))
					component.Type = Rigidbody3DComponent::StrToRigidbody3DType(finalBodyType);

				ImGuiUtils::DragFloat("Gravity factor", &component.GravityFactor, 0.01f);
				ImGuiUtils::DragFloat("Mass", &component.Mass, 0.01f);
			});

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
			{
				ImGui::Dummy({ 0.0f, 5.0f });

				ImGuiUtils::RGBVec2("Offset", { "X", "Y" }, { &component.Offset.x, &component.Offset.y });
				ImGuiUtils::RGBVec2("Size", { "X", "Y" }, { &component.Size.x, &component.Size.y });

				ImGuiUtils::VerticalSpace(10);

				UUID material = ImGuiUtils::DragDestination("Physics material", ".physmat2d", component.Material);
				if (material != 0)
					component.Material = material;
			});

		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
			{
				ImGuiUtils::RGBVec2("Offset", { "X", "Y" }, { &component.Offset.x, &component.Offset.y });
				ImGuiUtils::DragFloat("Radius", &component.Radius, 1.0f);

				ImGuiUtils::Separator();

				UUID material = ImGuiUtils::DragDestination("Physics material", ".physmat2d", component.Material);
				if (material != 0)
					component.Material = material;
			});

		DrawComponent<PolygonCollider2DComponent>("Polygon Collider", entity, [](auto& component)
			{
				std::vector<glm::vec2>& points = component.Points;
				ImGuiUtils::RGBVec2("Offset", { "X", "Y" }, { &component.Offset.x, &component.Offset.y });

				bool edited = false;
				static std::string selectedPoint = "";
				static int selectedIdx = -1;

				if (ImGui::TreeNode("Points"))
				{
					if (ImGui::Button("Add", { ImGui::GetContentRegionAvail().x / 2, ImGui::GetTextLineHeight() * 1.5f }))
						component.AddPoint();
					ImGui::SameLine();
					if (ImGui::Button("Remove", { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.5f }) &&
						selectedIdx != -1)
					{
						component.RemovePoint(selectedIdx);
						selectedIdx = -1;
						selectedPoint = "";
					}

					for (uint32_t i = 0; i < points.size(); i++)
					{
						bool selected = false;

						std::stringstream ss;
						ss << "Point " << i;
						if (selectedPoint.compare(ss.str()) == 0)
							selected = true;

						if (ImGui::Selectable(ss.str().c_str(), &selected, 0, { 100, ImGui::GetTextLineHeight() }))
						{
							selectedPoint = ss.str();
							selectedIdx = i;
						}

						ImGui::SameLine();
						ImGuiUtils::RGBVec2("", { "X", "Y" }, { &(points[i].x), &(points[i].y) });
					}
					component.Points = points;
					ImGui::TreePop();
				}

				if (ImGui::Button("Retriangulate", { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight() * 1.5f }))
					component.Triangulate();

				ImGuiUtils::Separator();

				UUID material = ImGuiUtils::DragDestination("Physics material", ".physmat2d", component.Material);
				if (material != 0)
					component.Material = material;
			});


		DrawComponent<BoxCollider3DComponent>("Box Collider 3D", entity, [](auto& component)
			{
				ImGui::Dummy({ 0.0f, 5.0f });

				ImGuiUtils::RGBVec3("Offset", { "X", "Y", "Z" }, { &component.Offset.x, &component.Offset.y, &component.Offset.z });
				ImGuiUtils::RGBVec3("Size", { "X", "Y", "Z" }, { &component.Size.x, &component.Size.y, &component.Size.z });

				ImGuiUtils::Separator();

				UUID material = ImGuiUtils::DragDestination("Material", ".physmat3d", component.Material);
				if (material != 0)
					component.Material = material;
			});

		DrawComponent<SphereCollider3DComponent>("Sphere Collider 3D", entity, [](auto& component)
			{
				ImGui::Dummy({ 0.0f, 5.0f });

				ImGuiUtils::DragFloat("Radius", &component.Radius, 0.01f);
				ImGuiUtils::RGBVec3("Offset", { "X", "Y", "Z" }, { &component.Offset.x, &component.Offset.y, &component.Offset.z });

				ImGuiUtils::Separator();

				UUID material = ImGuiUtils::DragDestination("Material", ".physmat3d", component.Material);
				if (material != 0)
					component.Material = material;
			});

		DrawComponent<MeshCollider3DComponent>("Mesh Collider 3D", entity, [](auto& component)
			{
				ImGuiUtils::RGBVec3("Offset", { "X", "Y", "Z" }, { &component.Offset.x, &component.Offset.y, &component.Offset.z });
				UUID mesh = ImGuiUtils::DragDestination("Mesh", ".mesh", component.Mesh);
				if (mesh != 0)
					component.Mesh = mesh;
				UUID material = ImGuiUtils::DragDestination("Material", ".physmat3d", component.Material);
				if (material != 0)
					component.Material = material;

			});

		DrawComponent<DirectionalLightComponent>("Directional Light", entity, [](auto& component)
			{
				ImGuiUtils::RGBVec3("Direction", { "X", "Y", "Z" }, { &component.Direction.x, &component.Direction.y, &component.Direction.z });
				ImGuiUtils::Color3("Color", { &component.Color.r,&component.Color.g,&component.Color.b });
				ImGuiUtils::DragFloat("Intensity", &component.Intensity, 0.1f);
				ImGui::Checkbox("Cast shadows", &component.CastShadows);
			});

		DrawComponent<PointLightComponent>("Point Light", entity, [&](auto& component)
			{
				ImGuiUtils::Color3("Color", { &component.Color.x, &component.Color.y, &component.Color.z });
				ImGuiUtils::DragFloat("Intensity", &component.Intensity, 0.04f, 0.0f);
				ImGuiUtils::DragFloat("Radius", &component.Radius, 0.05f, 0.0f);
				ImGui::Checkbox("Cast shadows", &component.CastShadows);

				component.Position = entity.Transform().Translation;
			});
	}
}