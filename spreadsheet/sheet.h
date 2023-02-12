#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <vector>
#include <memory>
#include <set>
#include <map>

class Sheet : public SheetInterface {
public:

    Sheet();
    ~Sheet() = default;

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    bool CycleCheck(Position pos, const std::vector<Position> references_down) const;
    bool CycleCheck(const std::vector<Position>& references, std::set<Position>& cells_to_formula) const;
    void ClearCache(Position pos) const;
private:
    void UpdateSize();
    void UpdateSize(Position new_pos);
    const std::set<Position>& GetReferencesUp(Position pos) const;
    const std::vector<Position> GetReferencesDown(Position pos) const;
    void AddRefToCell(Position cell, Position ref);
    void DeleteDependencies(Position pos);
    std::unique_ptr<Cell> TryCreateCell(Position pos, std::string text);

    struct Comp{
        bool operator()(const Position& lhs, const Position& rhs) const;
    };

    // These three containers are needed to find the minimum PrintableSize
    // and to store arrays of cells depending on the cell in the key of the map
    std::map<Position, std::set<Position>> cells_and_cells_dependent_on_;
    std::set<Position> no_empty_cell_sorted_to_row_;
    std::set<Position, Comp> no_empty_cell_sorted_to_column_;

    //std::set<Position> no_empty_cells_;
    //std::set<Position, Comp> no_empty_cell_sorted_to_column_;
    std::vector<std::vector<std::unique_ptr<Cell>>> spreadsheet_;
    Size size_;
};
