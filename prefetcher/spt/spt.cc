#include <algorithm>
#include <array>
#include <map>
#include <optional>
#include <iostream>

#include "cache.h"

namespace
{
struct spt_prefetcher {
  struct spt_entry {
    uint64_t ip = 0;           
    uint64_t last_cl_addr = 0;
    int8_t valid = 0;   

    auto tag() const { return ip; }
  };

  constexpr static int SPT_SIZE = 1024;

  std::array<spt_entry, SPT_SIZE> prediction_table = {{0}};

  int current_id = 0;

  std::optional<spt_entry> check_hit(uint64_t ip)
  {
    
    auto hit = std::find_if(prediction_table.begin(), prediction_table.end(), [ip](auto& entry) { return entry.tag() == ip; });

    if(hit != std::end(prediction_table)) {
      return *hit;
    } else {
      return std::nullopt;
    }
  }

  void update(uint64_t ip, uint64_t cl_addr)
  {
    auto entry = std::find_if(prediction_table.begin(), prediction_table.end(), [ip](auto& elem) { return elem.tag() == ip; });
    entry->last_cl_addr = cl_addr;
    entry->valid = 1;
  }

  void insert(uint64_t ip, uint64_t cl_addr)
  {
    int fill_id = (current_id+1) % SPT_SIZE;
    
    prediction_table[fill_id].ip = ip;
    prediction_table[fill_id].last_cl_addr = cl_addr;
    prediction_table[fill_id].valid = 1;

    current_id++;

  }


public:
  void prefetch(uint64_t ip, uint64_t cl_addr, CACHE* cache)
  {
    auto hit = check_hit(ip);

    if(hit.has_value()){
      if(hit->valid){
        int64_t stride = static_cast<int64_t>(cl_addr) - static_cast<int64_t>(hit->last_cl_addr);
        if(stride != 0) {
          auto addr_delta = stride;
          auto pf_address = static_cast<uint64_t>(cl_addr + addr_delta);
          // Only prefetch if address + stride goes outside of the current block
          if (pf_address >> LOG2_BLOCK_SIZE != cl_addr >> LOG2_BLOCK_SIZE) 
          {
            cache->prefetch_line(pf_address, 1, 0);
          }
        }
      }
      update(ip, cl_addr);
    }else{
      insert(ip, cl_addr);
    }
    
  }

};
spt_prefetcher prefetcher;
} // namespace

void CACHE::prefetcher_initialize() {}

void CACHE::prefetcher_cycle_operate() {}

uint32_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, bool useful_prefetch, uint8_t type, uint32_t metadata_in)
{
  if (static_cast<std::underlying_type_t<access_type>>(type)  == static_cast<std::underlying_type_t<access_type>>(access_type::LOAD))
  {
    prefetcher.prefetch(ip, addr, this);
  }
  return metadata_in;
}

uint32_t CACHE::prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}

void CACHE::prefetcher_final_stats() {
    std::cout << "Total number of elements ever stored in prediction table :" << prefetcher.current_id << std::endl;
}