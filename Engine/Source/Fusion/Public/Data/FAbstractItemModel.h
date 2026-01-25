#pragma once

namespace CE
{
    class FAbstractItemModel;

    STRUCT()
    struct FUSION_API FModelIndex final
    {
        CE_STRUCT(FModelIndex)
    public:

        FModelIndex() {}

        virtual ~FModelIndex() {}

        bool IsValid() const { return model != nullptr; }

        //void* GetDataPtr() const { return data; }

        const Variant& GetData() const { return data; }

        u32 GetRow() const { return row; }

        u32 GetColumn() const { return col; }

        FAbstractItemModel* GetModel() const { return model; }

        SIZE_T GetHash() const;

        bool operator==(const FModelIndex& other) const;
        bool operator!=(const FModelIndex& other) const;

    private:

        Variant data = Variant();
        //void* data = nullptr;
        u32 row = 0;
        u32 col = 0;

        FAbstractItemModel* model = nullptr;

        friend class FAbstractItemModel;
    };

    CLASS(Abstract)
    class FUSION_API FAbstractItemModel : public Object
    {
        CE_CLASS(FAbstractItemModel, Object)
    protected:

        FAbstractItemModel();

    public:

        virtual ~FAbstractItemModel();

        // - Public API -

        //! @brief Override this function and return false if the data is not ready or if it's empty.
        //! That will prevent rendering of any content in the item view.
        virtual bool IsReady() { return true; }

        FModelIndex CreateIndex(u32 row, u32 column, const Variant& data = Variant());

        virtual FModelIndex GetParent(const FModelIndex& index) = 0;

        virtual FModelIndex GetIndex(u32 row, u32 column, const FModelIndex& parent = {}) = 0;

        virtual u32 GetRowCount(const FModelIndex& parent = {}) = 0;
        virtual u32 GetColumnCount(const FModelIndex& parent = {}) = 0;

        virtual void SetData(u32 row, FWidget& rowWidget, const FModelIndex& parent = {}) = 0;

        virtual void SetHeaderData(FWidget& header) {}

        ScriptEvent<void(FAbstractItemModel*)> onModelUpdated;

    };
    
} // namespace CE

#include "FAbstractItemModel.rtti.h"
