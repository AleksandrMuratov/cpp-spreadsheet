#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe);

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression);
    Value Evaluate(const SheetInterface& sheet) const override;
    std::string GetExpression() const override;
	std::vector<Position> GetReferencedCells() const override;

private:
    FormulaAST ast_;
};
