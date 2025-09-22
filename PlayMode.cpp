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

std::string GetUserNameCrossPlatform() {
	#ifdef _WIN32
		char* buffer = nullptr;
		size_t size = 0;
		if (_dupenv_s(&buffer, &size, "USERNAME") == 0 && buffer != nullptr) {
			std::string name(buffer);
			free(buffer);
			return name;
		}
	#else
		const char* name = std::getenv("USER");
		if (name) return std::string(name);
	#endif
		return "Player";
}

std::string PlayMode::ReplaceUsername(const std::string& text) {
    std::string result = text;
    size_t pos = 0;
    while ((pos = result.find('@', pos)) != std::string::npos) {
        result.replace(pos, 1, userName);
        pos += userName.size();
    }
    return result;
}

PlayMode::PlayMode() : scene(*hexapod_scene), text1(32.f), text2(32.f), text3(32.f) {
	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	if (!story.LoadFromFile(data_path("story.txt"))) {
		throw std::runtime_error("Failed to load story");
	}

	userName = GetUserNameCrossPlatform();

	const StoryNode* node = story.GetCurrentNode();
	if (node) {
		text1.Set_Text(ReplaceUsername(node->text));
		if (node->options.size() > 0) text2.Set_Text(node->options[0].label);
		if (node->options.size() > 1) text3.Set_Text(node->options[1].label);
	}
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
			const StoryNode* node = story.GetCurrentNode();
            if (node && currentOption < (int)node->options.size()) {
                std::string next = node->options[currentOption].next;
                if (story.JumpTo(next)) {
                    const StoryNode* newNode = story.GetCurrentNode();
                    if (newNode) {
                        text1.Set_Text(newNode->text);
                        if (newNode->options.size() > 0) text2.Set_Text(newNode->options[0].label);
                        else text2.Set_Text("");
                        if (newNode->options.size() > 1) text3.Set_Text(newNode->options[1].label);
                        else text3.Set_Text("");
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

	const StoryNode* node = story.GetCurrentNode();
	if (node) {
		text1.Render_Text(100, 650, glm::vec2(1280, 720), glm::vec3(1.0, 1.0, 1.0));

		if (node->options.size() > 0) {
			glm::vec3 color = (currentOption == 0) ? glm::vec3(1.0f, 1.0f, 0.0f)
			                                      : glm::vec3(1.0f, 1.0f, 1.0f);
			text2.Render_Text(100, 600, glm::vec2(1280, 720), color);
		}

		if (node->options.size() > 1) {
			glm::vec3 color = (currentOption == 1) ? glm::vec3(1.0f, 1.0f, 0.0f)
			                                      : glm::vec3(1.0f, 1.0f, 1.0f);
			text3.Render_Text(100, 550, glm::vec2(1280, 720), color);
		}
	}

	GL_ERRORS();
}