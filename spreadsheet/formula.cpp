#include "formula.h"

#include "FormulaAST.h"
#include "sheet.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <functional>
#include <optional>
#include <regex>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

FormulaError::FormulaError(Category category)
    : category_(category)
{}

FormulaError::Category FormulaError::GetCategory() const{
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const{
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const{
    if(category_ == FormulaError::Category::Div0){
        return "#DIV/0!"sv;
    }
    else if(category_ == FormulaError::Category::Value){
        return "#VALUE!"sv;
    }
    else {
        return "REF!"sv;
    }
}

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression);
    Value Evaluate(const SheetInterface& sheet) const override;
    std::string GetExpression() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
    static void DeleteDuplicate(std::vector<Position>& cells);
    static std::optional<double> GetDouble(const std::string& text);

    FormulaAST ast_;
};

Formula::Formula(std::string expression)
    try : ast_(ParseFormulaAST(expression))
{

}
catch(const std::exception& ex){
    throw FormulaException(ex.what());
}

FormulaInterface::Value Formula::Evaluate(const SheetInterface& sheet) const{
    try{

        std::function<double(Position)> func = [&sheet](Position pos){
            if(!pos.IsValid()){
                throw FormulaError(FormulaError::Category::Ref);
            }
            if(const CellInterface* ptr_to_cell = sheet.GetCell(pos)){
                auto value = ptr_to_cell->GetValue();
                if(std::holds_alternative<std::string>(value)){
                    std::string str = std::get<std::string>(value);
                    if(auto num = GetDouble(str)){
                        return *num;
                    }
                    else{
                        throw FormulaError(FormulaError::Category::Value);
                    }
                }
                else if(std::holds_alternative<double>(value)){
                    return std::get<double>(value);
                }
                else{
                    throw FormulaError(FormulaError::Category::Value);
                }
            }
            return 0.0;
        };

        return ast_.Execute(func);
    }
    catch(const FormulaError& fe){
        return fe;
    }
}

std::string Formula::GetExpression() const{
    std::ostringstream ss;
    ast_.PrintFormula(ss);
    return ss.str();
}

std::vector<Position> Formula::GetReferencedCells() const{
    const auto& references = ast_.GetCells();
    std::vector<Position> cells(references.begin(), references.end());
    DeleteDuplicate(cells);
    return cells;
}

void Formula::DeleteDuplicate(std::vector<Position>& cells){
    std::sort(cells.begin(), cells.end());
    auto it = std::unique(cells.begin(), cells.end());
    cells.resize(std::distance(cells.begin(), it));
}

std::optional<double> Formula::GetDouble(const std::string& text){
    static const std::regex pattern_double(R"([ \t\n]*(-?)(0|([1-9][0-9]*))(.[0-9]+)?([eE]-?[1-9][0-9]*)?[ \t\n]*)");
    try {
        if(std::regex_match(text, pattern_double)){
            return std::stod(text);
        }
    }  catch (...) {

    }
    return std::nullopt;
}

}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}


