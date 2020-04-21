#pragma once
#include <filesystem>
#include <deque>
#include <string>

class DirExplorer {
    std::string name;
    std::filesystem::path curr_dir_path;
    // The history of navigating to other directories.
    std::deque<std::filesystem::path> history_forward;
    // The history of navigating backwards from the forward history.
    std::deque<std::filesystem::path> history_backward;
    std::filesystem::path selected_child;

    bool isChild(const std::filesystem::path& test_it);
    void changeDir(std::filesystem::path new_dir);
public:
    inline DirExplorer(const std::string& explorer_name = "", const std::string& start_directory = ""):
    name(explorer_name),
    curr_dir_path(start_directory)
    {}

    void returnForward();
    void goBackward();
    bool swapDir(const std::string& path);
    bool getParentDir(std::string& parent_buf);
    bool visitParent();
    bool selectChild(const std::string& child);

    inline bool hasBackwardHistory(){
        return this->history_backward.size() > 0;
    }
    inline bool hasForwardHistory(){
        return this->history_forward.size() > 0;
    }
    inline std::string getCurrentDir() {
        return this->curr_dir_path.string();
    }
    inline std::filesystem::directory_iterator begin() {
        return std::filesystem::directory_iterator(this->curr_dir_path);
    }
    inline const std::filesystem::directory_iterator end() {
        return std::filesystem::directory_iterator();
    }
    inline std::filesystem::path getSelected(){
        return this->selected_child;
    }
    inline const std::string getExplorerName(){
        return this->name;
    }
};
