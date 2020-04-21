#include <iostream>
#include <map>
#include <memory>
#include <stack>

#include "ImGuiDirExplorer.hpp"
#include "DirExplorer/dir_explorer.hpp"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

static std::map<ImGuiID, std::shared_ptr<DirExplorer>> active_dir_explorers;
static std::stack<ImGuiID> dir_explorer_stack;

using ImGuiDirExplorer = std::pair<ImGuiID, std::shared_ptr<DirExplorer>>;

namespace ImGui {
    namespace DirectoryExplorer {
        ImGuiID NewDirExplorer(const std::string& context_name, const std::string& start_path){
            ImGuiDirExplorer dir_explorer;
            dir_explorer.first = ImGui::GetID(context_name.c_str());
            dir_explorer.second.reset(new DirExplorer(context_name, start_path));

            active_dir_explorers.insert(dir_explorer);

            return dir_explorer.first;
        }

        inline bool hasTop(){
            if(dir_explorer_stack.size() == 0)
                return false;
            return true;
        }

        inline bool getTop(ImGuiDirExplorer& fill_in){
            if(!hasTop())
                return false;
            fill_in.first = dir_explorer_stack.top();
            fill_in.second = active_dir_explorers[fill_in.first];
            return true;
        }

        inline bool activeDirExplorerExists(const ImGuiID dir_explorer_id, std::map<ImGuiID, std::shared_ptr<DirExplorer>>::iterator out_it = active_dir_explorers.end()){
            if((out_it = active_dir_explorers.find(dir_explorer_id)) == active_dir_explorers.end()){
                std::cerr << "The dir player id did not refer to an active dir explorer." << std::endl;
                return false;
            }
            return true;
        }

        bool Begin(ImGuiID dir_explorer_id){
            if(!activeDirExplorerExists(dir_explorer_id))
                return false;

            if(!ImGui::BeginChild(dir_explorer_id)){
                ImGui::EndChild();
                return false;
            }

            dir_explorer_stack.push(dir_explorer_id);

            return true;
        }

        void ShowHistoryButtons(){
            static const ImVec4 button_disabled_color{0.0f,0.0f,0.0f,0.0f}; 
            ImGuiDirExplorer dir_explorer;
            if(!getTop(dir_explorer))
                return;
            //ImGui::Item
            bool had_forward_history = dir_explorer.second->hasForwardHistory();
            if(!had_forward_history){
                ImGui::PushStyleColor(ImGuiCol_Button, button_disabled_color);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, button_disabled_color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, button_disabled_color);
            }

            if(ImGui::ArrowButton("back", ImGuiDir_Left))
                dir_explorer.second->goBackward();

            if(!had_forward_history){
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();
            
            bool had_back_history = dir_explorer.second->hasBackwardHistory();
            if(!had_back_history){
                ImGui::PushStyleColor(ImGuiCol_Button, button_disabled_color);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, button_disabled_color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, button_disabled_color);
            }
            
            if(ImGui::ArrowButton("forward", ImGuiDir_Right))
                dir_explorer.second->returnForward();

            if(!had_back_history){
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
            }
        }

        void ShowPathBar(float width){
            ImGuiDirExplorer dir_explorer;
            if(!getTop(dir_explorer))
                return;
            std::string path = dir_explorer.second->getCurrentDir();
            ImGui::PushItemWidth(width);
            if(ImGui::InputText("", &path,ImGuiInputTextFlags_EnterReturnsTrue)){
                dir_explorer.second->swapDir(path);
            }
            ImGui::PopItemWidth();
        }

        void ListDirectory(){
            ImGuiDirExplorer dir_explorer;
            if(!getTop(dir_explorer))
                return;
            for(auto dir_entry : *dir_explorer.second){
                if(ImGui::Selectable(dir_entry.path().filename().string().c_str())){
                    if(dir_entry.is_directory()){
                        dir_explorer.second->swapDir(dir_entry.path().string());
                    }
                    else if(dir_entry.is_regular_file()) {
                            dir_explorer.second->selectChild(dir_entry.path().string());
                    }
                }
            }
        }

        inline bool hasSelectedChild(DirExplorer& dir_explorer){
            return dir_explorer.getSelected().has_filename();
        }

        ChildAction SelectedChildShow(std::string& fill_in){
            ImGuiDirExplorer dir_explorer;
            if(!getTop(dir_explorer))
                return ChildAction_NoneSelected;
            
            ChildAction c_action = ChildAction_None;
            if(hasSelectedChild(*dir_explorer.second)){
                auto path = dir_explorer.second->getSelected();
                ImGui::Text("Selected: %s", path.filename().string().c_str());
                ImGui::SameLine();
                if(ImGui::Button("Open file")) {
                    c_action = ChildAction_OpenFile;
                    fill_in = path.string();
                }
            }
            return c_action;
        }

        void End(){
            if(!hasTop())
                return;
            dir_explorer_stack.pop();
            ImGui::EndChild();
        }

        bool OpenFileDialog(ImGuiID dir_explorer_ctx, std::string& selected_file_name, bool& show){
            if(!show)
                return false;

            auto find_res = active_dir_explorers.find(dir_explorer_ctx);
            if(find_res == active_dir_explorers.end())
                return false; // There is no directory explorer context yet.
            
            auto dir_explorer = find_res->second;
            auto explorer_name = dir_explorer->getExplorerName();
            
            if(!ImGui::IsPopupOpen(explorer_name.c_str()))
                ImGui::OpenPopup(explorer_name.c_str());
            
            bool open = false;
            if(ImGui::BeginPopupModal(explorer_name.c_str(), &show)){
                if(ImGui::DirectoryExplorer::Begin(dir_explorer_ctx)){
                    ImGui::DirectoryExplorer::ShowHistoryButtons();
                    ImGui::SameLine();
                    ImGui::DirectoryExplorer::ShowPathBar();
                    ImGui::DirectoryExplorer::ListDirectory();
                    if(ImGui::DirectoryExplorer::SelectedChildShow(selected_file_name) == ChildAction_OpenFile){
                        open = true;
                    }
                    ImGui::DirectoryExplorer::End();
                }
                ImGui::EndPopup();
            }
            return open;
        }
    }
}

