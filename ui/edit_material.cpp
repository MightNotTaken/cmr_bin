#include "edit_material.hpp"
#include <algorithm>
#include <database.h>
#include "cellular.hpp"
#include <JSON.h>
#include <mac.h>
#include "data-source.hpp"
#include "furnace-ctrl.hpp"

namespace EditMaterial {
    std::vector<String> furnaces;
    std::vector<String> materials;
    bool saving = false;

    String getSelectedFurnace() {
        uint16_t sel = lv_dropdown_get_selected(ui_furnaceOption);
        return furnaces[sel];
    }
    
    String getSelectedMaterial() {
        uint16_t sel = lv_dropdown_get_selected(ui_materialOption);
        return materials[sel];
    }
    void setup() {
        dataSource.on("mat-up", [](String data) {
            enableOptions();
            lv_label_set_text(ui_saveButtonLabel, "Saved");
            saving = false;
            FurnaceCtrl::setMaterial(FurnaceCtrl::getMac(getSelectedFurnace()), getSelectedMaterial());
            lv_scr_load(ui_homeScreen); 

        });
        lv_obj_add_event_cb(ui_editCloseButton, [](lv_event_t * e) {
            if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
                saving = false;
                lv_label_set_text(ui_saveButtonLabel, "Save");
                lv_scr_load(ui_homeScreen); 
            }
        }, LV_EVENT_ALL, NULL);
        lv_obj_add_event_cb(ui_saveButton, [](lv_event_t * e) {
            if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
                if (Cellular::internetConnected && !saving) {
                    saving = true;
                    lv_label_set_text(ui_saveButtonLabel, "Saving...");
                    JSON data("[]");
                    data.push_back(
                        FurnaceCtrl::getMac(getSelectedFurnace())
                    );
                    data.push_back(getSelectedMaterial());
                    data.push_back(mac());
                    disableOptions();
                    Cellular::emit("cmr:mat-up", data.toString());
                }
            }
        }, LV_EVENT_ALL, NULL);
        lv_obj_add_event_cb(ui_editMaterialScreen, [](lv_event_t * e) {
            populateMaterial(materials);
            populateFurnaces(furnaces);
            enableOptions();
            lv_label_set_text(ui_saveButtonLabel, "Save");
        }, LV_EVENT_SCREEN_LOAD_START, NULL);
    }

    
    void disableOptions() {
        lv_obj_add_state(ui_furnaceOption, LV_STATE_DISABLED);
        lv_obj_add_state(ui_materialOption, LV_STATE_DISABLED);
    }

    void enableOptions() {
        lv_obj_clear_state(ui_furnaceOption, LV_STATE_DISABLED);
        lv_obj_clear_state(ui_materialOption, LV_STATE_DISABLED);
    }

    void populateMaterial(std::vector<String> options) {
        String list = "";
        for (size_t i = 0; i < options.size(); i++) {
            list += options[i];
            if (i != options.size() - 1) {
                list += "\n";
            }
        }
        lv_dropdown_set_options(ui_materialOption, list.c_str());
    }

    void populateFurnaces(std::vector<String> options) {
        String list = "";
        for (size_t i = 0; i < options.size(); i++) {
            list += options[i];
            if (i != options.size() - 1) {
                list += "\n";
            }
        }
        lv_dropdown_set_options(ui_furnaceOption, list.c_str());
    }

    void setMaterials(JSON materialList) {
        materials.clear();
        for (int i=0; i<materialList.size(); i++) {
            materials.push_back(materialList[i].toString());    
        }
    }
    
    void setFurnaces(JSON furnaceList) {
        furnaces.clear();
        for (int i=0; i<furnaceList.size(); i++) {
            furnaces.push_back(furnaceList[i].toString());    
        }
    }

    void saveMaterials() {
        JSON list("[]");
        for (auto material: materials) {
            list.push_back(material);
        }
        database.writeFile(MATERIAL_DB_REF, list.toString());
    }

    void addMaterial(String material) {
        // avoid duplicates (optional)
        if (std::find(materials.begin(), materials.end(), material) == materials.end()) {
            materials.push_back(material);
        }
        saveMaterials();

    }

    void removeMaterial(String material) {
        auto it = std::find(materials.begin(), materials.end(), material);
        if (it != materials.end()) {
            materials.erase(it);
        }
        saveMaterials();
    }


    std::vector<String> getMaterialList() {
        return materials;
    }

    void loadMaterials() {
        materials.clear();
        if (database.hasFile(MATERIAL_DB_REF)) {
            database.readFile(MATERIAL_DB_REF);
            JSON list(database.payload());
            for (int i=0; i<list.size(); i++) {
                materials.push_back(list[i].toString());
            }
        }
        populateMaterial(materials);
    }

};