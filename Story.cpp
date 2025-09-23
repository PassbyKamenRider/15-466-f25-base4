#include "Story.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

bool Story::LoadFromFile(std::string path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open story file: " << path << std::endl;
        return false;
    }

    std::string line;
    std::string current_state_id;
    StoryNode current_node;
    bool first_state = true;

    while (std::getline(ifs, line)) {
        if (line.empty()) continue;

        if (line.rfind("->", 0) == 0) {
            if (!current_state_id.empty()) {
                nodes[current_state_id] = current_node;
                current_node = StoryNode();
            }

            current_state_id = line.substr(2);

            if (first_state) {
                start_state = current_state_id;
                current_state = start_state;
                first_state = false;
            }
        }
        else if (line.rfind("+", 0) == 0) {
            size_t arrow_pos = line.find("->");
            std::string name = line.substr(1, arrow_pos - 1);
            std::string next  = line.substr(arrow_pos + 2);
            current_node.options.push_back({ name, next });
        }
        else {
            current_node.text += "\n";
            current_node.text += line;
        }
    }

    nodes[current_state_id] = current_node;

    return true;
}

StoryNode* Story::GetCurrentNode() {
    auto it = nodes.find(current_state);
    if (it != nodes.end()) return &it->second;
    return nullptr;
}

void Story::JumpTo(std::string stateName) {
    current_state = stateName;
}