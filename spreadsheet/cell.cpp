#include "sheet.h"
#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
    : sheet_(sheet)
    , impl_(std::make_unique<EmptyImpl>())
{}

void Cell::Set(std::string text) {
    if(!text.empty()){
        if(text.front() == '=' && text.size() > 1){
            impl_ = std::make_unique<FormulaImpl>(text.substr(1), sheet_);
        }
        else{
            impl_ = std::make_unique<TextImpl>(std::move(text));\
        }
    }
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    if(!cache_value_.has_value()){
        cache_value_ = impl_->GetValue();
    }
    return *cache_value_;
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const{
    return impl_->GetReferencedCells();
}

void Cell::ClearCache() const{
    cache_value_.reset();
}

bool Cell::IsReferenced() const{
    return !GetReferencedCells().empty();
}

Cell::Value Cell::EmptyImpl::GetValue() const{
    using namespace std::literals;
    return 0.0;
}

std::string Cell::EmptyImpl::GetText() const{
    using namespace std::literals;
    return ""s;
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const{
    return {};
}

Cell::TextImpl::TextImpl(std::string text)
    : text_(std::move(text))
{}

Cell::Value Cell::TextImpl::GetValue() const{
    if(text_.front() == '\''){
        return text_.substr(1);
    }
    else{
        return text_;
    }
}

std::string Cell::TextImpl::GetText() const{
    return text_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const{
    return {};
}

Cell::FormulaImpl::FormulaImpl(std::string expr, const Sheet& sheet)
    : formula_(ParseFormula(expr))
    , sheet_(sheet)
{}

Cell::Value Cell::FormulaImpl::GetValue() const{
    auto value = formula_->Evaluate(sheet_);
    if(std::holds_alternative<double>(value)){
        return std::get<double>(value);
    }
    else{
        return std::get<FormulaError>(value);
    }
}

std::string Cell::FormulaImpl::GetText() const{
    using namespace std::literals;
    return "="s + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const{
    return formula_->GetReferencedCells();
}
