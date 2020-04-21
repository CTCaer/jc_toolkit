#include "dir_explorer.hpp"

bool DirExplorer::isChild(const std::filesystem::path& test_it){
    if(test_it.parent_path() != this->curr_dir_path){
        // The child does not exist in the parent directory.
        return false;
    }
    return true;
}

void DirExplorer::changeDir(std::filesystem::path new_dir){
    // For some reason we need to use the c_str representation of the path when swapping so that the directory iteration does not throw an error.
    this->curr_dir_path.swap(std::filesystem::path(new_dir.c_str()).make_preferred());
    this->selected_child = std::filesystem::path();
}

/**
 * Undo a return to a previous directory.
 */
void DirExplorer::returnForward(){
    if(!this->hasBackwardHistory())
        // No back visits were done.
        return;
    
    this->history_forward.push_back(this->curr_dir_path);
    this->changeDir(this->history_backward.back());
    this->history_backward.pop_back();
}

/**
 * Return to the previous directory.
 */
void DirExplorer::goBackward(){
    if(!this->hasForwardHistory())
        // No directories were explored.
        return;
    
    this->history_backward.push_back(this->curr_dir_path);
    this->changeDir(this->history_forward.back());
    this->history_forward.pop_back();
}

/**
 * Swap the current directory out for a new one.
 * Change the directory.
 */
bool DirExplorer::swapDir(const std::string& path){
    if(!std::filesystem::is_directory(path))
        // Not a directory.
        return false;
    this->history_backward.clear();
    this->history_forward.push_back(this->curr_dir_path);
    this->changeDir(path);
    return true;
}

bool DirExplorer::getParentDir(std::string& parent_buf){
    if(!this->curr_dir_path.has_parent_path())
        return false;
    parent_buf = this->curr_dir_path.parent_path().string();
    return true;
}

/**
 * Visit the parent directory; change the current directory to the parent directory.
 * Does nothing if there is no parent directory. 
 */
bool DirExplorer::visitParent() {
    // Check if a parent path exists. 
    if(!this->curr_dir_path.has_parent_path())
        return false; // Does not exist so return false to the calling function.
    
    // A parent path exists so we set the current path to its parent path.
    this->history_forward.push_back(this->curr_dir_path);
    this->changeDir(curr_dir_path.parent_path());
    return true;
}

/**
 * Select a child in the current directory by using its path.
 */
bool DirExplorer::selectChild(const std::string& child){
    if(!this->isChild(child))
        return false;
    this->selected_child = child;
    return true;
}
