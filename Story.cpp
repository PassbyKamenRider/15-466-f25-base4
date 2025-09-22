#include "Story.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

bool Story::LoadFromFile(const std::string& path) {
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
            std::string label = line.substr(1, arrow_pos - 1);
            std::string next  = line.substr(arrow_pos + 2);
            current_node.options.push_back({ label, next });
        }
        else {
            if (!current_node.text.empty()) current_node.text += "\n";
            current_node.text += line;
        }
    }

    if (!current_state_id.empty()) {
        nodes[current_state_id] = current_node;
    }

    return true;
}

const StoryNode* Story::GetCurrentNode() const {
    auto it = nodes.find(current_state);
    if (it != nodes.end()) return &it->second;
    return nullptr;
}

bool Story::JumpTo(const std::string& state_id) {
    if (nodes.find(state_id) != nodes.end()) {
        current_state = state_id;
        return true;
    }
    return false;
}

void Story::Reset() {
    current_state = start_state;
}