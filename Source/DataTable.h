#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include <string>

//==============================================================================
/**
 * 數據表格類別
 * 用於存儲和載入表格數據，數據會保存在插件狀態中
 */
class DataTable
{
public:
    struct Row
    {
        juce::String name;
        float value1 = 0.0f;
        float value2 = 0.0f;
        juce::String note;
        
        Row() = default;
        Row(const juce::String& n, float v1, float v2, const juce::String& nt = "")
            : name(n), value1(v1), value2(v2), note(nt) {}
    };
    
    DataTable();
    ~DataTable();
    
    // 行管理
    void addRow(const juce::String& name, float value1, float value2, const juce::String& note = "");
    void removeRow(int index);
    void removeRowsBySourceNumber(int sourceNumber);  // 根據 source number 刪除所有相關行
    void clear();
    int getNumRows() const { return static_cast<int>(rows.size()); }
    Row& getRow(int index) { return rows[index]; }
    const Row& getRow(int index) const { return rows[index]; }
    const std::vector<Row>& getAllRows() const { return rows; }
    
    // 更新行數據
    void setRowName(size_t index, const juce::String& name);
    void setRowValue1(size_t index, float value);
    void setRowValue2(size_t index, float value);
    void setRowNote(size_t index, const juce::String& note);
    
    // 狀態保存/載入
    void saveState(juce::MemoryOutputStream& stream) const;
    void loadState(juce::MemoryInputStream& stream);
    
    // 導出為字串（用於顯示或輸出）
    juce::String toString() const;
    
private:
    std::vector<Row> rows;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DataTable)
};

