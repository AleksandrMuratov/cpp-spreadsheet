#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
        Cell(Sheet& sheet);
        ~Cell() = default;

        void Set(std::string text);
        void Clear();

        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

        bool IsReferenced() const;
        void ClearCache() const;
private:

        class Impl{
        public:
            virtual Value GetValue() const = 0;
            virtual std::string GetText() const = 0;
            virtual std::vector<Position> GetReferencedCells() const = 0;
        };

        class EmptyImpl : public Impl{
        public:
            Value GetValue() const override;
            std::string GetText() const override;
            std::vector<Position> GetReferencedCells() const override;
        };

        class TextImpl : public Impl{
        public:
            explicit TextImpl(std::string text);
            Value GetValue() const override;
            std::string GetText() const override;
            std::vector<Position> GetReferencedCells() const override;
        private:
            std::string text_;
        };

        class FormulaImpl : public Impl{
        public:
            explicit FormulaImpl(std::string expr, const Sheet& sheet);
            Value GetValue() const override;
            std::string GetText() const override;
            std::vector<Position> GetReferencedCells() const override;
        private:
            std::unique_ptr<FormulaInterface> formula_;
            const Sheet& sheet_;
        };



private:
        Sheet& sheet_;
        std::unique_ptr<Impl> impl_;
        mutable std::optional<Value> cache_value_;
};

