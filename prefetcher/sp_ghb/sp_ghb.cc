#include <algorithm>
#include <array>
#include <map>
#include <optional>

#include "cache.h"

namespace
{
struct spt_ghb_prefetcher {
  struct ghb_entry {
    uint64_t last_cl_addr = 0;
    struct ghb_entry *next = nullptr;
    struct ghb_entry *prev = nullptr;
    int index_table_id = 0;
  };

  struct index_table_entry {
    uint64_t ip = 0;
    struct ghb_entry *ghb_entry_pointer = nullptr;

    auto tag() const { return ip;}
  };

  constexpr static int GHB_SIZE = 1024; // Size of the global history buffer
  constexpr static int INDEX_TABLE_SIZE = 512; // Size of the index table
  constexpr static int NB_STRIDES_COMPUTED = 2; // When computing the stride, we look at the last NB_STRIDES_COMPUTED strides, if there are all equal, we prefetch

  std::array<ghb_entry, GHB_SIZE> global_history_buffer = {{0, nullptr, nullptr, 0}};
  std::array<index_table_entry, INDEX_TABLE_SIZE> index_table = {{0, nullptr}};


  int current_ghb_id = 0; 

  std::optional<ghb_entry> check_hit(uint64_t ip)
  {

    int hit_id = ip % INDEX_TABLE_SIZE;

    if(index_table[hit_id].ghb_entry_pointer != nullptr){
      return std::optional<ghb_entry>{*index_table[hit_id].ghb_entry_pointer};
    }else{
      return std::nullopt;
    }
  }

  /// Remove an element from a linked list stored in the GHB array
  void pop_at(int id)
  {
    ghb_entry *next_ghb_entry = global_history_buffer[id].next;
    ghb_entry *prev_ghb_entry = global_history_buffer[id].prev;
    int index_table_id = global_history_buffer[id].index_table_id;

    if(next_ghb_entry != nullptr){
      next_ghb_entry->prev = prev_ghb_entry;
    }else{
      if(prev_ghb_entry != nullptr){
        prev_ghb_entry->next = nullptr;
      }else{
        index_table[index_table_id].ghb_entry_pointer = nullptr;
      }
    }

    if(prev_ghb_entry != nullptr){
      prev_ghb_entry->next = next_ghb_entry;
    }else{
      index_table[index_table_id].ghb_entry_pointer = next_ghb_entry;
    }
  }

  void insert(uint64_t ip, uint64_t cl_addr, ghb_entry *next)
  {
    int ghb_fill_id = (current_ghb_id+1) % GHB_SIZE;

    pop_at(ghb_fill_id);
    
    global_history_buffer[ghb_fill_id].last_cl_addr = cl_addr;
    global_history_buffer[ghb_fill_id].next = next;

    int index_table_fill_id = ip % INDEX_TABLE_SIZE;
    index_table[index_table_fill_id].ghb_entry_pointer = &global_history_buffer[ghb_fill_id];

    current_ghb_id++;

  }

  void insert(uint64_t ip, uint64_t cl_addr)
  {
    int ghb_fill_id = (current_ghb_id+1) % GHB_SIZE;

    pop_at(ghb_fill_id);
    
    global_history_buffer[ghb_fill_id].last_cl_addr = cl_addr;
    global_history_buffer[ghb_fill_id].next = nullptr;

    int index_table_fill_id = ip % INDEX_TABLE_SIZE;
    index_table[index_table_fill_id].ghb_entry_pointer = &global_history_buffer[ghb_fill_id];

    current_ghb_id++;

  }

  std::optional<int64_t> compute_stride(uint64_t cl_addr, ghb_entry hit)
  {
    uint64_t current_addr = cl_addr;
    int64_t strides[NB_STRIDES_COMPUTED];
    for(int i = 0; i < NB_STRIDES_COMPUTED; i++){
      int64_t stride = static_cast<int64_t>(current_addr) - static_cast<int64_t>(hit.last_cl_addr);
      strides[i] = stride;
      current_addr = hit.last_cl_addr;
      hit = *hit.next;
    }
    if(std::all_of(strides, strides+NB_STRIDES_COMPUTED, [strides](int x){ return x==strides[0];}) && strides[0] != 0){
      return std::optional<int64_t>{strides[0]};
    }else{
      return std::nullopt;
    }
  }


public:
  void prefetch(uint64_t ip, uint64_t cl_addr, CACHE* cache)
  {
    auto hit = check_hit(ip);

    if(hit.has_value()){
      
      auto stride = compute_stride(cl_addr, *hit);
      if(stride.has_value()){
        auto addr_delta = *stride;
        auto pf_address = static_cast<uint64_t>(cl_addr + addr_delta);

        cache->prefetch_line(pf_address, 1, 0);
      }
      insert(ip, cl_addr, &*hit);
    }else{
      insert(ip, cl_addr);
    }
    
  }

};
spt_ghb_prefetcher prefetcher;
} // namespace

void CACHE::prefetcher_initialize() {}

void CACHE::prefetcher_cycle_operate() {}

uint32_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, bool useful_prefetch, uint8_t type, uint32_t metadata_in)
{
  if(static_cast<std::underlying_type_t<access_type>>(type)  == static_cast<std::underlying_type_t<access_type>>(access_type::LOAD)){
    prefetcher.prefetch(ip, addr, this);
  }
  return metadata_in;
}

uint32_t CACHE::prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}

void CACHE::prefetcher_final_stats() {}
