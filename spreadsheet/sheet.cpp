#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::Sheet()
    : no_empty_cell_sorted_to_column_(Comp())
{}

void Sheet::SetCell(Position pos, std::string text) {
    std::unique_ptr<Cell> new_cell = TryCreateCell(pos, text);
    auto refs = new_cell->GetReferencedCells();
    if(!CycleCheck(pos, refs)){
        throw CircularDependencyException("Сycle check was not successful"s);
    }
    UpdateSize(pos);
    DeleteDependencies(pos);
    auto& ptr_to_cell = spreadsheet_[pos.row][pos.col];
    ptr_to_cell.reset(new_cell.release());
    for(const Position& cell : ptr_to_cell->GetReferencedCells()){
        AddRefToCell(cell, pos);
    }
    no_empty_cell_sorted_to_column_.insert(pos);
    no_empty_cell_sorted_to_row_.insert(pos);
    ClearCache(pos);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if(!pos.IsValid()){
        throw InvalidPositionException("Position of cell is not valid"s);
    }
    if(pos.col >= size_.cols || pos.row >= size_.rows){
        return nullptr;
    }
    return spreadsheet_[pos.row][pos.col].get();
}
CellInterface* Sheet::GetCell(Position pos) {
    if(!pos.IsValid()){
        throw InvalidPositionException("Position of cell is not valid"s);
    }
    if(pos.col >= size_.cols || pos.row >= size_.rows){
        return nullptr;
    }
    return spreadsheet_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
    if(!pos.IsValid()){
        throw InvalidPositionException("Position of cell is not valid"s);
    }
    if(pos.row < size_.rows && pos.col < size_.cols){
        ClearCache(pos);
        auto& ptr_to_cell = spreadsheet_[pos.row][pos.col];
        ptr_to_cell.reset();
        ptr_to_cell = std::make_unique<Cell>(*this);
        no_empty_cell_sorted_to_row_.erase(pos);
        no_empty_cell_sorted_to_column_.erase(pos);
        UpdateSize();
    }
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::ClearCache(Position pos) const{
    if(const Cell* ptr_to_cell = dynamic_cast<const Cell*>(GetCell(pos))){
        ptr_to_cell->ClearCache();
        for(const Position& ref : GetReferencesUp(pos)){
            ClearCache(ref);
        }
    }
}

const std::set<Position>& Sheet::GetReferencesUp(Position pos) const{
    auto it = cells_and_cells_dependent_on_.find(pos);
    static const std::set<Position> empty_refs;
    return (it == cells_and_cells_dependent_on_.end())
            ? empty_refs
            : it->second;
}

const std::vector<Position> Sheet::GetReferencesDown(Position pos) const{
    if(auto ptr_to_cell = GetCell(pos)){
        return ptr_to_cell->GetReferencedCells();
    }
    return {};
}

void Sheet::DeleteDependencies(Position pos){
    for(Position ref : GetReferencesDown(pos)){
        cells_and_cells_dependent_on_[ref].erase(pos);
    }
}

void Sheet::AddRefToCell(Position cell, Position ref){
    auto& refs = cells_and_cells_dependent_on_[cell];
    refs.insert(ref);
}

bool Sheet::CycleCheck(Position pos, const std::vector<Position> references_down) const{
    std::set<Position> cells_for_check_cycle;
    cells_for_check_cycle.insert(pos);
    return CycleCheck(references_down, cells_for_check_cycle);
}

bool Sheet::CycleCheck(const std::vector<Position>& references, std::set<Position>& cells_to_formula) const{
    for(const Position& ref : references){
        if(!cells_to_formula.insert(ref).second){
            return false;
        }
        const std::vector<Position> references_down = GetReferencesDown(ref);
        if(!CycleCheck(references_down, cells_to_formula)){
            return false;
        }
    }
    return true;
}

void Sheet::UpdateSize(){
    if(!no_empty_cell_sorted_to_row_.empty()){
        const Position& pos_back_row = *no_empty_cell_sorted_to_row_.rbegin();
        size_.rows = pos_back_row.row + 1;
        const Position& pos_back_col = *no_empty_cell_sorted_to_column_.rbegin();
        size_.cols = pos_back_col.col + 1;
    }
    else{
        size_.rows = 0;
        size_.cols = 0;
    }
}

void Sheet::UpdateSize(Position new_pos){
    if(new_pos.col >= size_.cols){
        for(auto& row : spreadsheet_){
            row.resize(new_pos.col + 1);
            for(int i = size_.cols; i < static_cast<int>(row.size()); ++i){
                row[i] = std::make_unique<Cell>(*this);
            }
        }
        size_.cols = new_pos.col + 1;
    }
    if(new_pos.row >= size_.rows){
        spreadsheet_.resize(new_pos.row +1);
        for(int i = size_.rows; i < new_pos.row + 1; ++i){
            auto& row = spreadsheet_[i];
            row.resize(size_.cols);
            for(int j = size_.cols; j < static_cast<int>(row.size()); ++j){
                row[j] = std::make_unique<Cell>(*this);
            }
        }
        size_.rows = new_pos.row + 1;
    }
}

std::unique_ptr<Cell> Sheet::TryCreateCell(Position pos, std::string text){
    if(!pos.IsValid()){
        throw InvalidPositionException("Position of cell is not valid"s);
    }
    std::unique_ptr<Cell> new_cell = std::make_unique<Cell>(*this);
    new_cell->Set(text);
    return new_cell;
}

void Sheet::PrintValues(std::ostream& output) const {
    for(int row = 0; row < size_.rows; ++row){
        bool is_first = true;
        for(int col = 0; col < size_.cols; ++col){
            if(!is_first){
                output << '\t';
            }
            is_first = false;
            if(auto ptr_to_cell = GetCell(Position{row, col})){
                if(ptr_to_cell->GetText().empty()){
                    continue;
                }
                const auto& value = ptr_to_cell->GetValue();
                if(std::holds_alternative<std::string>(value)){
                    output << std::get<std::string>(value);
                }
                else if(std::holds_alternative<double>(value)){
                    output << std::get<double>(value);
                }
                else{
                    output << std::get<FormulaError>(value);
                }
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for(int row = 0; row < size_.rows; ++row){
        bool is_first = true;
        for(int col = 0; col < size_.cols; ++col){
            if(!is_first){
                output << '\t';
            }
            is_first = false;
            if(auto ptr_to_cell = GetCell(Position{row, col})){
                output << ptr_to_cell->GetText();
            }
        }
        output << '\n';
    }
}

bool Sheet::Comp::operator()(const Position& lhs, const Position& rhs) const {
    return std::tie(lhs.col, lhs.row) < std::tie(rhs.col, rhs.row);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
