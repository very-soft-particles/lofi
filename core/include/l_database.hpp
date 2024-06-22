// =====================================================================================
//
//       Filename:  l_database.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-05 7:26:08 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_memory.hpp"

namespace lofi {

  namespace tbl {

    template<size_t Size>
    using SparseT = mem::SparseArrayContainerPolicy<uint32_t, Size, Size, mem::StackAllocPolicy>;

    template<size_t Size>
    using DenseT = mem::PackedArrayContainerPolicy<uint32_t, Size, Size, mem::StackAllocPolicy>;

    template<size_t DBID, size_t TableID, size_t ColumnID, class T>
    struct ColumnDescriptor {
      using type = T;
    };

    template<size_t DBID, size_t TableID>
    struct ColumnDescriptorSet {
      using type = List<>;
    };

    template<size_t DBID, size_t TableID>
    using column_descriptors_t = typename ColumnDescriptorSet<DBID, TableID>::type;

    template<size_t DBID, size_t TableID, size_t Idx>
    using column_t = meta::at_t<column_descriptors_t<DBID, TableID>, Idx>;

    template<size_t DBID, size_t TableID>
    struct get_column_size : Value<size_t, ColumnDescriptorSet<DBID, TableID>::size> {};

    template<size_t DBID, size_t TableID, size_t ColumnID, size_t Size>
    struct Column {
      using value_t = column_t<DBID, TableID, ColumnID>;
      value_t value[Size];

      value_t& operator[](u64 index) {
        L_ASSERT(index < Size);
        return value[index];
      }
    };


    template<size_t DBID, size_t TableID, size_t Size, class... Idxs>
    struct columns_generator {
      using type = tuple<Column<DBID, TableID, Idxs::value, Size>...>;
    };
    // for instantiating specific tables outside a database ??
  }		// -----  end of namespace tbl  ----- 

  template<size_t DBID, size_t TableID, size_t Size>
  class Table {
  public:
    using list_t = tbl::column_descriptors_t<DBID, TableID>;
    static constexpr size_t num_fields = list_size<list_t>::value;
    static constexpr size_t size = Size;

    using table_t = Table<DBID, TableID, Size>;
    using columns_t = typename tbl::columns_generator<DBID, size, IdxSequence<list_size<tbl::column_descriptors_t<DBID, TableID>>::value>::type>;
    using sparse_t = typename tbl::SparseT<size>;
    using dense_t = typename tbl::DenseT<size>;
    using index_t = u32;

    Table() = default;

    index_t insert() {
      auto sparse_handle = sparse.add_object();
      sparse[sparse_handle] = dense.add_object();
      dense[sparse[sparse_handle]] = sparse_handle;
      return sparse_handle;
    }

    void remove(index_t index) {
      dense.remove_object(sparse[index]);
      sparse.remove_object(index);
    }

    template<size_t Index>
    meta::at_t<list_t, Index>& get_column() const {
      return columns.template get<Index>();
    }

    template<typename T>
    static constexpr size_t get_column_index() {
      using search_t = typename meta::lift<meta::find_t<T>::template type, list_t>::type;
      return search_t::value;
    }

    template<typename T>
    static constexpr b8 contains() {
      constexpr size_t value = get_column_index<T>();
      if constexpr (value != MAX_u64) {
        return true;
      }
      return false;
    }

  private:
    sparse_t sparse{};
    dense_t dense{};
    columns_t columns{};
  };

  namespace db {

    template<size_t TableSize, class... Ts>
    struct TableDescriptor;

    template<size_t TableSize, class... Ts>
    struct TableDescriptor<TableSize, List<Ts...>> {
      using type = List<Ts...>;
      static constexpr size_t size = TableSize;
    };

    template<size_t SysID>
    struct TableDescriptorSet {
      using type = List<>;
    };

    template<size_t DBID>
    struct info {
      using table_descriptors_t = typename TableDescriptorSet<DBID>::type;
      static constexpr size_t table_count = list_size<table_descriptors_t>::value;

      template<size_t TableID>
      static constexpr size_t table_size = meta::at_t<table_descriptors_t, TableID>::size;

      template<typename Idx>
      using column_type = typename meta::at_t<table_descriptors_t, Idx::value>::value_t;

      template<typename T>
      struct contains_seq { 
        template<typename... Idxs>
        using type = typename meta::lift<meta::find_t<T>::template type, table_descriptors_t>::type;
      };
    };

  
    namespace defaults {
      static constexpr size_t TableSize = 32;
      static constexpr size_t TableIDBitsSize = 10;
      static constexpr size_t ComponentIDBitsSize = 22;
    }		// -----  end of namespace defaults  ----- 

    // struct tables_generator??
    
    template<size_t DBID, class... Idxs>
    struct tables_generator {
      using type = tuple<Table<DBID, Idxs::value, info<DBID>::template table_size<Idxs>>...>;
    };

    template<size_t TableBits = defaults::TableIDBitsSize, size_t ComponentBits = defaults::ComponentIDBitsSize>
    struct CompID {
      uint32_t Table : TableBits;
      uint32_t Component : ComponentBits;
    };

    using ComponentID = CompID<>;

    template<class T, size_t Count, class TableTuple, bool Result = false>
    struct find_component;
  
    template<class T, size_t Count, class Table1, class Table2, class... Tables>
    struct find_component<T, Count, List<Table1, Table2, Tables...>, false> {
      static constexpr ComponentID value = 
                                find_component
                                < T
                                , Count + 1
                                , List<Table2, Tables...>
                                , has_type<T, typename Table2::types_t>::value
                                >::value;
    };
  
    template<class T, size_t Count, class Table, class... Tables>
    struct find_component<T, Count, List<Table, Tables...>, true> {
      static constexpr ComponentID value = {Count, Table::template get_column_index<T>()};
    };

    template<size_t TableDescriptorsID>
    class Database {
    public:
      using tables_t = typename db::tables_generator
              <TableDescriptorsID, typename IdxSequence<db::info<TableDescriptorsID>::table_count>::type>::type;
      
      Database() = default;
      
      template<size_t TableID>
      constexpr auto& get_table() {
        return tables.template get<TableID>();
      }
    
        template<class T>
        static constexpr ComponentID get_component_id() {
          return find_component<T, 0, tables_t>::value;
        }
    
    private:
      tables_t tables{};
    };
  }
}
