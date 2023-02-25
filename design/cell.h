class Cell : public CellInterface {
public:
    Cell(const SheetInterface& sheet);
    ~Cell() = default;

    void Set(std::string text);
    void Clear();
    Value GetValue() const override;
    std::string GetText() const override;
	std::vector<Position> GetReferencedCells() const override;
	void ClearCache() const;
	bool CacheIsValid() const;
	
private:
    class Impl{
    public:
        virtual Value GetValue(const SheetInterface&) const = 0;
        virtual std::string GetText() const = 0;
		virtual std::vector<Position> GetReferencedCells() const = 0;
    };

    class EmptyImpl : public Impl{
    public:
        Value GetValue([[maybe_unused]] const SheetInterface& sheet) const override;
        std::string GetText() const override;
		std::vector<Position> GetReferencedCells() const override;
    };

    class TextImpl : public Impl{
    public:
        explicit TextImpl(std::string text);
        Value GetValue([[maybe_unused]] const SheetInterface& sheet) const override;
        std::string GetText() const override;
		std::vector<Position> GetReferencedCells() const override;
    private:
        std::string text_;
    };

    class FormulaImpl : public Impl{
    public:
        explicit FormulaImpl(std::string expr);
        Value GetValue([[maybe_unused]] const SheetInterface& sheet) const override;
        std::string GetText() const override;
		std::vector<Position> GetReferencedCells() const override;
    private:
        std::unique_ptr<FormulaInterface> formula_;
    };

private:
	const SheetInterface& sheet_;
    std::unique_ptr<Impl> impl_;
	mutable std::optional<Value> cache_value_;
};
