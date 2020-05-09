#include <list>
#include "com_builder_ui.hpp"
#include "ui_helpers.hpp"
#include "TP/TP.hpp"

namespace ConCom {
    namespace UI {
        static std::stringstream cmd_seq_out;
        void commands(){
            static std::list<int> command_sequence;
            auto insert_com = [](auto& li, auto& it){
                ImGui::PushID(&it);
                ImGui::Selectable("[-]");
                ImGui::PopID();
                if(ImGui::BeginDragDropTarget()){
                    if(auto payload = ImGui::AcceptDragDropPayload("con_com")){
                        IM_ASSERT(payload->DataSize == sizeof(*it));
                        li.insert(it, *(const int*)payload->Data);
                    }
                    ImGui::EndDragDropTarget();
                }
            };
            int com_idx = 0;
            auto it = command_sequence.begin();

            ImGui::Text("Command Sequence [%ld]", command_sequence.size());
            if(ImGui::Button("Run")){
                cmd_seq_out.clear();
                TP::add_job([&](){
                    for(auto& command: command_sequence){
                        cmd_seq_out << "Command-" << command << " executed." << std::endl;
                    }
                });
            }
            // Insert at the beginning of the command sequence.
            insert_com(command_sequence, it);
            while(it != command_sequence.end()){
                ImGui::Selectable(std::string(std::to_string(com_idx) + "Command-" + std::to_string(*it)).c_str());
                // Replace the command.
                if(ImGui::BeginDragDropTarget()){
                    if(auto payload = ImGui::AcceptDragDropPayload("con_com")){
                        IM_ASSERT(payload->DataSize == sizeof(*it));
                        *it = *(const int*)payload->Data;
                    }
                    ImGui::EndDragDropTarget();
                }
                
                it++;
                com_idx++;
                insert_com(command_sequence, it);
            }
        }

        void command_list() {
            for(int i = 0; i < 10; i++){
                ImGui::Selectable(std::string("Command-" + std::to_string(i)).c_str());
                if(ImGui::BeginDragDropSource()){
                    ImGui::SetDragDropPayload("con_com", &i, sizeof(i));
                    ImGui::EndDragDropSource();
                }
            }
        }

        void page() {
            ImGui::Columns(2);
            command_list();
            ImGui::NextColumn();
            commands();
            ImGui::Columns(1);
            ImGui::Text("Command Sequence Output: %s", cmd_seq_out.str().c_str());
        }
    }
}