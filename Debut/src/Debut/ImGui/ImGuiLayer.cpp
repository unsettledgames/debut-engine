#include "Debut/dbtpch.h"
#include "ImGuiLayer.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"
#include "Debut/Core/Application.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include "ImGuizmo.h"
#include <Debut/ImGui/ProgressPanel.h>
#include <Debut/Core/Window.h>
#include <stb_image.h>
#include <stb_image_resize.h>

namespace Debut
{
	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer")
	{

	}

	ImGuiLayer::~ImGuiLayer()
	{

	}
	
	void ImGuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		/*
		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.41f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.51f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.351f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.41f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.251f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.41f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.251f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.251f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.481f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.251f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.31f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.251f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.251f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.251f, 1.0f };

		colors[ImGuiCol_PlotHistogram] = ImVec4{ 0.25f, 0.4505f, 0.751f, 1.0f };
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4{ 0.38f, 0.3805f, 0.481f, 1.0f };*/
	}

	void ImGuiLayer::OnAttach()
	{
		std::vector<FontIcon> iconData = GetFontIcons();

		DBT_PROFILE_FUNCTION();
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/Source_Sans_Pro/SourceSansPro-Regular.ttf", 19);
		io.Fonts->AddFontFromFileTTF("assets/fonts/Source_Sans_Pro/SourceSansPro-Bold.ttf", 21);

		// Pixel data probably refers to the whole atlas area
		int iconRectIDs[64];
		wchar_t start = 57344;
		for (uint32_t i=0; i<64; i++)
			iconRectIDs[i] = io.Fonts->AddCustomRectFontGlyph(io.FontDefault, start + i, 23, 23, 23, {0, -2});
		io.Fonts->Build();

		unsigned char* pixels = nullptr;
		int texWidth, texHeight;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &texWidth, &texHeight);

		for (uint32_t i = 0; i < iconData.size(); i++)
		{
			int rectId = iconRectIDs[i];
			if (const ImFontAtlasCustomRect* rect = io.Fonts->GetCustomRectByIndex(rectId))
			{
				int width, height, channels, desiredChannels = 4;
				unsigned char* textureData = stbi_load(iconData[i].TexturePath.c_str(), &width, &height, &channels, desiredChannels);
				stbir_resize_uint8(textureData, width, height, 0, textureData, rect->Width, rect->Height, 0, 4);
				//memcpy(pixels + rect->Y * rect->Height + rect->X, textureData, width * height * 4);
				// Fill the custom rectangle with red pixels (in reality you would draw/copy your bitmap data here!)
				for (int y = 0; y < rect->Height; y++)
				{
					ImU32* p = (ImU32*)pixels + (rect->Y + y) * texWidth + (rect->X);
					for (int x = 0; x < rect->Width; x++)
						*p++ = IM_COL32(
							textureData[y*4 * rect->Width + x*4 + 0], 
							textureData[y*4 * rect->Width + x*4 + 1], 
							textureData[y*4 * rect->Width + x*4 + 2], 
							textureData[y*4 * rect->Width + x*4 + 3]);
				}

				stbi_image_free(textureData);
			}
		}


		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		SetDarkThemeColors();
	}

	void ImGuiLayer::OnDetach()
	{
		DBT_PROFILE_FUNCTION();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::Begin()
	{
		DBT_PROFILE_FUNCTION();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.SetHandled(e.Handled() || (e.IsInCategory(EventCategory::MouseEvent) && io.WantCaptureMouse));
			e.SetHandled(e.Handled() || (e.IsInCategory(EventCategory::KeyboardEvent) && io.WantCaptureKeyboard));
		}
	}

	void ImGuiLayer::OnImGuiRender()
	{
		ProgressPanel::OnImGuiRender();
		static bool showDemo = true;
		ImGui::ShowDemoWindow(&showDemo);
	}

	void ImGuiLayer::End()
	{
		DBT_PROFILE_FUNCTION();
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();

		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backupContext = glfwGetCurrentContext();

			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();

			glfwMakeContextCurrent(backupContext);
		}
	}

	std::vector<FontIcon> ImGuiLayer::GetFontIcons()
	{
		std::vector<FontIcon> icons;

		icons = {
			// DIRECTORY_ICON
			{L'\ue000', "assets\\icons\\directory.png"},
			// FILE_ICON
			{L'\ue001', "assets\\icons\\file.png"},
			// MENU_ICON
			{L'\ue002', "assets\\icons\\menu.png"},
			// MODEL_ICON
			{L'\ue003', "assets\\icons\\model.png"},
			// MESH_ICON
			{L'\ue004', "assets\\icons\\mesh.png"},
			// MATERIAL_ICON
			{L'\ue005', "assets\\icons\\material.png"},
			// UNIMPORTED_MODEL_ICON
			{L'\ue006', "assets\\icons\\unimported_model.png"},
			// ENITTY ICON
			{L'\ue007', "assets\\icons\\entity.png"}
			/*,
			{'\ue007', "assets\\icons\\model.png"},
			{'\ue008', "assets\\icons\\model.png"},
			{'\ue009', "assets\\icons\\model.png"},
			{'\ue00a', "assets\\icons\\model.png"},
			{'\ue00b', "assets\\icons\\model.png"},
			{'\ue00c', "assets\\icons\\model.png"},
			{'\ue00d', "assets\\icons\\model.png"},
			{'\ue00e', "assets\\icons\\model.png"},
			{'\ue00f', "assets\\icons\\model.png"},
			{'\ue010', "assets\\icons\\model.png"},
			{'\ue011', "assets\\icons\\model.png"},
			{'\ue012', "assets\\icons\\model.png"},*/
		};
		
		return icons;
	}
	
}