#include <string>
#include <vector>
#include <unordered_map>

struct StoryOption {
    std::string label;
    std::string next;
};

struct StoryNode {
    std::string text;
    std::vector<StoryOption> options;
};

class Story {
public:
    bool LoadFromFile(const std::string& path);

    const StoryNode* GetCurrentNode() const;
    bool JumpTo(const std::string& state_id);
    void Reset();

private:
    std::string start_state;
    std::string current_state;
    std::unordered_map<std::string, StoryNode> nodes;
};