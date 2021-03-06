#include "DebugDrawSystem.h"
#include "Runtime/ecs/Components/TransformComponent.h"
#include "Runtime/ecs/Components/CameraComponent.h"
#include "Runtime/ecs/Components/ModelComponent.h"
#include "Runtime/ecs/Components/LightComponent.h"
#include "Runtime/Rendering/RenderPass.h"
#include "Runtime/Rendering/Camera.h"
#include "Runtime/Rendering/Mesh.h"
#include "Runtime/Rendering/Model.h"
#include "Runtime/Rendering/VertexBuffer.h"
#include "Runtime/Rendering/IndexBuffer.h"
#include "Runtime/Rendering/Program.h"
#include "Runtime/Rendering/Texture.h"
#include "Runtime/Rendering/Material.h"
#include "Runtime/Rendering/Debug/DebugDraw.h"
#include "Runtime/Assets/AssetManager.h"
#include "../EditorApp.h"
DebugDrawSystem::DebugDrawSystem()
{
	auto& app = Singleton<EditorApp>::getInstance();
	auto& manager = app.getAssetManager();
	manager.load<Shader>("editor://shaders/vs_wf_wireframe", false)
		.then([this, &manager](auto vs)
	{
		manager.load<Shader>("editor://shaders/fs_wf_wireframe", false)
			.then([this, vs](auto fs)
		{
			mProgram = std::make_unique<Program>(vs, fs);
		});
	});
}

void DebugDrawSystem::frameRender(ecs::EntityManager &entities, ecs::EventManager &events, ecs::TimeDelta dt)
{	
	auto& app = Singleton<EditorApp>::getInstance();
	auto& editState = app.getEditState();

	auto& editorCamera = editState.camera;
	auto& selected = editState.selectionData.object;
	if (!editorCamera || 
		!editorCamera.has_component<CameraComponent>())
		return;

	const auto cameraComponentRef = editorCamera.component<CameraComponent>();
	const auto cameraComponent = cameraComponentRef.lock();
	const auto surface = cameraComponent->getOutputBuffer();
	auto& camera = cameraComponent->getCamera();
	const auto& view = camera.getView();
	const auto proj = camera.getProj();
	const auto cameraPos = camera.getPosition();

	RenderPass pass("DebugDrawPass");
	pass.bind(surface.get());
	gfx::setViewTransform(pass.id, &view, &proj);
	ddRAII dd(pass.id);
	const std::uint32_t colorGrid = 0xff606060;
	auto drawGrid = [](std::uint32_t gridColor, float height, float heightIntervals, std::uint32_t gridSize, std::uint32_t sizeIntervals, std::uint32_t iteration, std::uint32_t maxIterations)
	{
		std::uint32_t detailGridColor = gridColor;

		bool shouldRender = true;
		if (iteration + 1 != maxIterations)
		{
			const auto iterationHeight = heightIntervals * float(iteration + 1);
			const float factor = math::clamp(height, 0.0f, iterationHeight) / iterationHeight;
			std::uint32_t r = (gridColor) & 0xff;
			std::uint32_t g = (gridColor >> 8) & 0xff;
			std::uint32_t b = (gridColor >> 16) & 0xff;
			std::uint32_t a = (gridColor >> 24) & 0xff;
			a = static_cast<std::uint32_t>(math::lerp(255.0f, 0.0f, factor));
			if (a < 10)
				shouldRender = false;

			detailGridColor = r + (g << 8) + (b << 16) + (a << 24);
		}


		if (shouldRender)
		{
			auto step = (sizeIntervals * iteration);
			step = step ? step : 1;
			ddPush();
			ddSetState(true, false, true);
			ddSetColor(detailGridColor);
			math::vec3 center = { 0.0f, 0.0f, 0.0f };
			ddDrawGrid(Axis::Y, &center, gridSize / step, float(step));
			ddPop();
		}

	};

	if (editState.showGrid)
	{
		static const auto farClip = 200;
		static const auto height = 40.0f;
		static const auto divison = 10;
		const auto iterations = math::power_of_n_round_down(farClip, divison);
		for (std::uint32_t i = 0; i < iterations; ++i)
		{
			drawGrid(colorGrid, math::abs(cameraPos.y), height, farClip, divison, i, iterations);
		}
	}
	

	if (!selected || !selected.is_type<ecs::Entity>())
		return;

	auto selectedEntity = selected.get_value<ecs::Entity>();

	if (!selectedEntity ||
		!selectedEntity.has_component<TransformComponent>())
		return;


	auto& transformComponent = *selectedEntity.component<TransformComponent>().lock();
	const auto& worldTransform = transformComponent.getTransform();

	if (selectedEntity.has_component<CameraComponent>() && selectedEntity != editorCamera)
	{
		auto& cameraComponent = *selectedEntity.component<CameraComponent>().lock();
		auto& selectedCamera = cameraComponent.getCamera();
		const auto viewProj = selectedCamera.getProj() * selectedCamera.getView();
		ddPush();
		ddSetColor(0xffffffff);
		ddSetTransform(nullptr);
		ddDrawFrustum(&viewProj);
		ddPop();
	}

	if (selectedEntity.has_component<LightComponent>())
	{
		auto& lightComponent = *selectedEntity.component<LightComponent>().lock();
		auto& light = lightComponent.getLight();
		if (light.lightType == LightType::Spot)
		{
			auto adjacent = light.spotData.range;
			{
				auto tanAngle = math::tan(math::radians(light.spotData.spotOuterAngle));
				// oposite = tan * adjacent
				auto oposite = tanAngle * adjacent;
				ddPush();
				ddSetColor(0xff00ff00);
				ddSetWireframe(true);
				math::vec3 from = transformComponent.getPosition();
				math::vec3 to = from + transformComponent.getZAxis() * adjacent;
				ddDrawCone(&to, &from, oposite);
				ddPop();
			}
			{
				auto tanAngle = math::tan(math::radians(light.spotData.spotInnerAngle));
				// oposite = tan * adjacent
				auto oposite = tanAngle * adjacent;
				ddPush();
				ddSetColor(0xff00ffff);
				ddSetWireframe(true);
				math::vec3 from = transformComponent.getPosition();
				math::vec3 to = from + transformComponent.getZAxis() * adjacent;
				ddDrawCone(&to, &from, oposite);
				ddPop();
			}
		}
		else if (light.lightType == LightType::Point)
		{
			auto radius = light.pointData.range;
			ddPush();
			ddSetColor(0xff00ff00);
			ddSetWireframe(true);
			math::vec3 center = transformComponent.getPosition();
			ddDrawCircle(Axis::X, center.x, center.y, center.z, radius);
			ddDrawCircle(Axis::Y, center.x, center.y, center.z, radius);
			ddDrawCircle(Axis::Z, center.x, center.y, center.z, radius);
			ddPop();
		}
		else if (light.lightType == LightType::Directional)
		{
			ddPush();
			ddSetLod(UINT8_MAX);
			ddSetColor(0xff00ff00);
			ddSetWireframe(true);
			math::vec3 from1 = transformComponent.getPosition();
			math::vec3 to1 = from1 + transformComponent.getZAxis() * 2.0f;
			ddDrawCylinder(&from1, &to1, 0.1f);
			math::vec3 from2 = to1;
			math::vec3 to2 = from2 + transformComponent.getZAxis() * 1.5f;;
			ddDrawCone(&from2, &to2, 0.5f);
			ddPop();
		}
	}
	
	if (selectedEntity.has_component<ModelComponent>())
	{
		const auto& modelComponent = *selectedEntity.component<ModelComponent>().lock();
		const auto& model = modelComponent.getModel();
		if (!model.isValid())
			return;

		const auto hMesh = model.getLod(0);
		if (!hMesh)
			return;
		const auto& frustum = camera.getFrustum();
		const auto& bounds = hMesh->aabb;

		// Test the bounding box of the mesh
		if (math::frustum::testOBB(frustum, bounds, worldTransform))
		{
			if (editState.wireframeSelection)
			{
				const float u_params[8] =
				{
					1.0f, 1.0f, 0.0f, 0.5f, //r,g,b,a
					1.0f, 0.0f, 0.0f, 0.0f  //thickness, unused, unused, unused
				};
				if (!mProgram)
					return;

				mProgram->beginPass();
				mProgram->setUniform("u_params", u_params, 2);
				const auto material = model.getMaterialForGroup({});
				if (!material)
					return;
				const std::uint64_t state = material->getRenderStates(false, false, false)
					| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
					;
				hMesh->submit(pass.id, mProgram->handle, worldTransform, state);
			}
			else
			{
				ddPush();
				ddSetColor(0xff00ff00);
				ddSetTransform(&worldTransform);
				ddDrawAabb(&bounds.min, &bounds.max);
				ddPop();
			}
		}
	}
}
