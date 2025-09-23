#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <cstdlib>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;
	});
});

//
std::string GetUserName() {
	#ifdef _WIN32
		char* pValue = nullptr;
		size_t len = 0;
		if (_dupenv_s(&pValue, &len, "USERNAME") == 0 && pValue != nullptr) {
			std::string name(pValue);
			free(pValue);
			return name;
		}
	#else
		const char* name = std::getenv("USER");
		if (name) return std::string(name);
	#endif
		return "Player";
}

std::string PlayMode::ReplaceUsername(std::string text) {
    size_t pos = text.find('@');
    if (pos != std::string::npos) {
        text.replace(pos, 1, userName);
    }
    return text;
}

PlayMode::PlayMode() : scene(*hexapod_scene), mainText(32.f), optionText1(32.f), optionText2(32.f) {
	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	if (!story.LoadFromFile(data_path("story.txt"))) {
		throw std::runtime_error("Failed to load story");
	}

	userName = GetUserName();

	const StoryNode* node = story.GetCurrentNode();
	mainText.Set_Text(ReplaceUsername(node->text));
	optionText1.Set_Text(node->options[0].name);
	optionText2.Set_Text(node->options[1].name);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_ESCAPE) {
			SDL_SetWindowRelativeMouseMode(Mode::window, false);
			return true;
		} else if (evt.key.key == SDLK_A) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_D) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_W) {
			currentOption = (currentOption - 1 + 2) % 2;
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_S) {
			currentOption = (currentOption + 1) % 2;
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_RETURN) {
			auto node = story.GetCurrentNode();
            if (node && currentOption < (int)node->options.size()) {
                std::string next = node->options[currentOption].next;
				story.JumpTo(next);
                auto nextNode = story.GetCurrentNode();
				if (nextNode) {
					mainText.Set_Text(ReplaceUsername(nextNode->text));
					if (nextNode->options.size() != 0) {
						optionText1.Set_Text(nextNode->options[0].name);
						optionText2.Set_Text(nextNode->options[1].name);
					} else {
						optionText1.Set_Text("");
						optionText2.Set_Text("");
					}
				}
            }
			return true;
		}
	} else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_A) {
			left.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_D) {
			right.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_W) {
			up.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_S) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto node = story.GetCurrentNode();
	int lineCount = mainText.GetLineCount();
	float lineHeight = 50.0f;

	mainText.Render_Text(100, 650, glm::vec2(1280, 720), glm::vec3(1.0, 1.0, 1.0));

	if (node->options.size() > 0) {
		glm::vec3 color= glm::vec3(1.0f, 1.0f, 1.0f);
		if (currentOption == 0) {
			color = glm::vec3(1.0f, 1.0f, 0.0f);
		}
		optionText1.Render_Text(100, 650 - lineCount * lineHeight, glm::vec2(1280, 720), color);
	}

	if (node->options.size() > 1) {
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
		if (currentOption == 1) {
			color = glm::vec3(1.0f, 1.0f, 0.0f);
		}
		optionText2.Render_Text(100, 600 - lineCount * lineHeight, glm::vec2(1280, 720), color);
	}

	GL_ERRORS();
}