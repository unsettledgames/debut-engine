#pragma once
/***************Externals**************/
#include "glm/glm.hpp"

/************Core**************/
#include "Debut/Core/Core.h"
#include "Debut/Core/Time.h"
#include "Debut/Utils/MathUtils.h"
#include "Debut/Core/Log.h"

// Input
#include "Debut/Core/KeyCodes.h"
#include "Debut/Core/MouseButtonCodes.h"
#include "Debut/Core/Input.h"

// Events
#include "Debut/Events/ApplicationEvent.h"
#include "Debut/Events/Event.h"
#include "Debut/Events/KeyEvent.h"
#include "Debut/Events/MouseEvent.h"

// Only for use by Debut applications
#include "Debut/Core/Application.h"

/*********** Layers ****************/
#include "Debut/Core/Layer.h"
#include "Debut/ImGui/ImGuiLayer.h"

/*********** Render ****************/
#include "Debut/Rendering/Renderer/Renderer.h"
#include "Debut/Rendering/Renderer/Renderer2D.h"
#include "Debut/Rendering/Renderer/RenderCommand.h"

/******** Rendering components *************/
#include "Debut/Rendering/Shader.h"
#include "Debut/Rendering/Texture.h"
#include "Debut/Rendering/SubTexture2D.h"
#include "Debut/Rendering/Structures/Buffer.h"
#include "Debut/Rendering/Structures/VertexArray.h"
#include "Debut/Rendering/Structures/FrameBuffer.h"
#include "Debut/Rendering/Camera.h"

/*********Scene and ECS********************/
#include "Debut/Scene/Scene.h"
#include "Debut/Scene/Entity.h"
#include "Debut/Scene/SceneSerializer.h"
#include "Debut/Scene/ScriptableEntity.h"
#include "Debut/Scene/Components.h"

/************Asset managemenet************/
#include "Debut/AssetManager/AssetCache.h"
#include "Debut/AssetManager/AssetManager.h"

