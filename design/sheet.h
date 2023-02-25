#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <vector>
#include <memory>
#include <set>

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

	// Можете дополнить ваш класс нужными полями и методами


private:
    void UpdateSize();
    void UpdateSize(Position new_pos);
	
	bool CycleCheck(Position pos, const std::vector<Position>& references_for_pos) const;
	bool CycleCheck(Position pos_prev, const std::vector<Position>& references, std::set<Position> cells_to_formula) const;
	bool ClearCache(Position pos) const;

    struct Comp{
        bool operator()(const Position& lhs, const Position& rhs) const;
    };
// These two containers are needed to find the minimum PrintableSize 
// and to store arrays of cells depending on the cell in the key of the map
	std::map<Position, std::vector<Position>> no_empty_cells_and_cells dependent_on_;
    std::set<Position, Comp> no_empty_cell_sorted_to_column_;
	
    std::vector<std::vector<std::unique_ptr<Cell>>> spreadsheet_;
    Size size_;
};
