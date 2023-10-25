#include <map>
#include <queue>
#include <iostream>

#include "cache.h"
#include "champsim.h"
#include "sfp.h"

// Default constructor method
SHT_entry::SHT_entry() : valid(false) {
  for (int i = 0; i < NUM_FOOTPRINTS_USED; i++) {
    for (int j = 0; j < FOOTPRINT_SIZE; j++) {
      footprints[i][j] = false;
    }
  }
}

// Default constructor method
AST_entry::AST_entry() : SHTi(-1), fetched_by_SFP(false), active(false) {
  for (int i = 0; i < FOOTPRINT_SIZE; i++) {
    footprint[i] = false;
  }
}

//  Fetches all blocks marked in the footprint of the given SHT entry
void SFP_prefetcher::fetchPredictedFootprint(SHT_entry &entry, uint64_t addr, CACHE* cache){
  uint64_t pf_address;
  for (int i = 0; i < FOOTPRINT_SIZE; i++){
    if (entry.footprints[0][i]){
      // obtain address as start of sector + i * block size
      pf_address = START_OF_SECTOR(addr) | (i << LOG2_BLOCK_SIZE);
      cache->prefetch_line(pf_address, 1, 0);  // 1 means "fill_this_level", we want to prefetch to L1 cache of course. 0 is the metadata (not used)
    } 
  }
}

// Index into table to check if entry is present
bool SFP_prefetcher::hasRecordedFootprint(uint64_t addr, uint64_t ip) {
  uint32_t tag = getTag(addr, ip);
  if (SHT[tag].valid == true) {
    return true;
  }
  return false;
}

void SFP_prefetcher::invalidateRecordedFootprint(uint64_t addr, uint64_t ip) {
  uint32_t tag = getTag(addr, ip);
  SHT[tag].valid = false;
}

bool SFP_prefetcher::isActive(uint64_t sector){
  if (this->AST[sector].active == true) {
    return true;
  }
  return false;
}

// Starts tracking the footprint for this sector.
// !! Does NOT set the flag fetched_by_SFP. It needs to be set manually
void SFP_prefetcher::activateSector(uint64_t sector, uint64_t addr, uint64_t ip) {
  this->AST[sector] = AST_entry();
  this->AST[sector].SHTi = getTag(addr, ip);
  this->AST[sector].active = true;
}

// Updates with a footprint for the new memory request 
void SFP_prefetcher::updateAST(uint64_t sector, uint64_t addr) {
  this->AST[sector].footprint[LINE_IN_SECTOR(addr)] = true;
}

// Removes sector from the AST and transfer the recorded footprint to the SHT
void SFP_prefetcher::deactivateSector(uint64_t sector) {
  uint32_t SHTi = AST[sector].SHTi;
  for (int i = 0; i <= FOOTPRINT_SIZE; i++) {
    this->SHT[SHTi].footprints[0][i] = this->AST[sector].footprint[i];
  }
  this->SHT[SHTi].valid = true;
  // update LRU order (if SHT is full, remove least recently used element)
  if (this->SHT_use_order.size() >= SHT_SIZE) {
    this->SHT.erase(this->SHT_use_order.front());
    this->SHT_use_order.pop();
  }
  this->SHT_use_order.push(SHTi);
  // reset the entry to invalidate it
  this->AST[sector] = AST_entry();
}

//  Simply fetches the next three blocks in the sector 
void SFP_prefetcher::fetchDefaultPrediction(uint64_t addr, CACHE* cache){
  uint64_t pf_address;
  for (int i = 1; i <= 3; i++){
    pf_address = START_OF_BLOCK(addr) + (i << LOG2_BLOCK_SIZE);
    cache->prefetch_line(pf_address, 1, 0);  // 1 means "fill_this_level", we want to prefetch to L1 cache of course. 0 is the metadata (not used)
  }
}
// Recovery mechanism : fetches lines when we notice we did not fetch all the necessary ones at sector activation.
// Acts differently based on how we prefetched when we activated the sector (SPF or default)
void SFP_prefetcher::fetchRecovery(uint64_t sector, uint64_t addr, CACHE* cache) {
  if (this->AST[sector].fetched_by_SFP) {
    // If we used the footprint data to prefetch, we only fetch the necessary block.
    cache->prefetch_line(START_OF_BLOCK(addr), 1, 0);
  }
  else {
    // If we originally used the default predictor, we use it again to recover
    fetchDefaultPrediction(addr, cache);
  }
}

// Tag is obtained as last 20 bits of address (excluding address inside cache block and sector ) 
// concatenated with last 12 bits of program counter 
uint32_t getTag(uint64_t addr, uint64_t ip) {
  uint32_t bits_ip = (ip & 0xfff);
  uint32_t bits_address = ((addr >> (LOG2_BLOCK_SIZE + LOG2_FOOTPRINT_SIZE)) & 0xfffff) << 12; 
  return bits_address | bits_ip;
}

// instance the class as a global variable
SFP_prefetcher SFP;

void CACHE::prefetcher_initialize() {}

void CACHE::prefetcher_cycle_operate() {}

uint32_t CACHE::prefetcher_cache_operate(uint64_t addr, uint64_t ip, uint8_t cache_hit, bool useful_prefetch, uint8_t type, uint32_t metadata_in)
{
  // This function is called when a tag is checked in the cache.
  // ARGUMENTS:

  // addr: the address of the packet. If this is the first-level cache, the offset bits are included. 
      // Otherwise, the offset bits are zero. If the cache was configured with “virtual_prefetch”: true, this address will be a virtual address. 
      // Otherwise, this is a physical address.
  // ip: the address of the instruction that initiated the demand. 
      // If the packet is a prefetch from another level, this value will be 0.
  // cache_hit: if this tag check is a hit, this value is nonzero. Otherwise, it is 0.
  // useful_prefetch: if this tag check hit a prior prefetch, this value is true.
  // type: the result of static_cast<std::underlying_type_t<access_type>>(v) for v in: 
      // * access_type::LOAD * access_type::RFO * access_type::PREFETCH * access_type::WRITE * access_type::TRANSLATION
  // metadata_in: the metadata carried along by the packet. (valid/dirty bits ecc.. that will be stored with the block)

  // The function should return metadata that will be stored alongside the block.

  uint64_t sector = addr / (SECTOR_SIZE_blocks* BLOCK_SIZE);

  if (cache_hit) {
    if (SFP.isActive(sector)) {
      SFP.updateAST(sector, addr);
    }
  }
  else {
    if (SFP.isActive(sector)) {
      // This means we either did not prefetch all necessary lines when activating the sector 
      // Or enough time has passed so that a previously prefetched line has been evicted from the cache
      SFP.fetchRecovery(sector, addr, this);
      if (SFP.AST[sector].footprint[LINE_IN_SECTOR(addr)] == true) {
        SFP.deactivateSector(sector);
        SFP.activateSector(sector, addr, ip);
      }
    }
    else {
      SFP.activateSector(sector, addr, ip);
      if (SFP.hasRecordedFootprint(addr, ip)) {
        SFP.fetchPredictedFootprint(SFP.SHT[getTag(addr, ip)], addr, this);
        SFP.invalidateRecordedFootprint(addr, ip);
        SFP.AST[sector].fetched_by_SFP = true;
      }
      else {
        SFP.fetchDefaultPrediction(addr, this);
      }
    }
    SFP.updateAST(sector, addr);
  }
  return metadata_in;
}

uint32_t CACHE::prefetcher_cache_fill(uint64_t addr, uint32_t set, uint32_t way, uint8_t prefetch, uint64_t evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}

void CACHE::prefetcher_final_stats() {
  std::cout << "-------- Custom final stats --------" << std::endl;
  std::cout << "Size of queue : " << SFP.SHT_use_order.size() << std::endl;
  std::cout << "Size of SHT : " << SFP.SHT.size() << std::endl;
  std::cout << "Size of AST : " << SFP.AST.size() << std::endl;
}
