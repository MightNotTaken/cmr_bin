#ifndef EDIT_MATERIAL_H__
#define EDIT_MATERIAL_H__
#include <ui.h>
#include <lvgl.h>
#include <vector>
#include <Arduino.h>
#include <JSON.h>

#define MATERIAL_DB_REF  "/material.json"

namespace EditMaterial {
    void setup();
    void setMaterials(JSON materialList);
    void setFurnaces(JSON furnaceList);
    void populateMaterial(std::vector<String> options);
    void populateFurnaces(std::vector<String> options);
    void addMaterial(String material);
    void removeMaterial(String material);
    std::vector<String> getMaterialList();
    void loadMaterials();
    void enableOptions();
    void disableOptions();
};
#endif