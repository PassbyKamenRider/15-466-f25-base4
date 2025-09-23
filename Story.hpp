#include <string>
#include <vector>
#include <unordered_map>

struct StoryOption {
    std::string name;
    std::string next;
};

struct StoryNode {
    std::string text;
    std::vector<StoryOption> options;
};

class Story {
public:
    bool LoadFromFile(std::string path);

    StoryNode* GetCurrentNode();
    void JumpTo(std::string stateName);

private:
    std::string start_state;
    std::string current_state;
    std::unordered_map<std::string, StoryNode> nodes;
};