#include "DataTable.h"

//==============================================================================
DataTable::DataTable()
{
    // Default example rows
    addRow("Example 1", 0.5f, -0.3f, "Note");
    addRow("Example 2", -0.2f, 0.8f, "");
}

DataTable::~DataTable()
{
}

//==============================================================================
void DataTable::addRow(const juce::String& name, float value1, float value2, const juce::String& note)
{
    rows.emplace_back(name, value1, value2, note);
}

void DataTable::removeRow(int index)
{
    if (index >= 0 && index < static_cast<int>(rows.size()))
    {
        rows.erase(rows.begin() + index);
    }
}

void DataTable::removeRowsBySourceNumber(int sourceNumber)
{
    juce::String sourceStr = juce::String(sourceNumber);
    for (int i = static_cast<int>(rows.size()) - 1; i >= 0; --i)
    {
        // 檢查 name 或 note 中是否包含 source number
        if (rows[i].name.contains(sourceStr) || rows[i].note.contains(sourceStr))
        {
            rows.erase(rows.begin() + i);
        }
    }
}

void DataTable::clear()
{
    rows.clear();
}

//==============================================================================
void DataTable::setRowName(size_t index, const juce::String& name)
{
    if (index < rows.size())
        rows[index].name = name;
}

void DataTable::setRowValue1(size_t index, float value)
{
    if (index < rows.size())
        rows[index].value1 = value;
}

void DataTable::setRowValue2(size_t index, float value)
{
    if (index < rows.size())
        rows[index].value2 = value;
}

void DataTable::setRowNote(size_t index, const juce::String& note)
{
    if (index < rows.size())
        rows[index].note = note;
}

//==============================================================================
void DataTable::saveState(juce::MemoryOutputStream& stream) const
{
    stream.writeInt(static_cast<int>(rows.size()));
    for (const auto& row : rows)
    {
        stream.writeString(row.name);
        stream.writeFloat(row.value1);
        stream.writeFloat(row.value2);
        stream.writeString(row.note);
    }
}

void DataTable::loadState(juce::MemoryInputStream& stream)
{
    rows.clear();
    int numRows = stream.readInt();
    for (int i = 0; i < numRows; ++i)
    {
        juce::String name = stream.readString();
        float value1 = stream.readFloat();
        float value2 = stream.readFloat();
        juce::String note = stream.readString();
        addRow(name, value1, value2, note);
    }
}

//==============================================================================
juce::String DataTable::toString() const
{
    juce::String result;
    for (size_t i = 0; i < rows.size(); ++i)
    {
        const auto& row = rows[i];
        result += juce::String(i + 1) + ". " + row.name + 
                  " | " + juce::String(row.value1, 3) + 
                  ", " + juce::String(row.value2, 3);
        if (row.note.isNotEmpty())
            result += " | " + row.note;
        result += "\n";
    }
    return result;
}

