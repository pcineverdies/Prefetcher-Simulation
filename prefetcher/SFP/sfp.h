#include <map>
#include <queue>

#include "cache.h"
#include "champsim.h"


#define NUM_FOOTPRINTS_USED 1 // right now this is not used, it would be used to swap between 1 and 2 recorded footprints per entry
#define FOOTPRINT_SIZE 32  // equivalent to sector size
#define LOG2_FOOTPRINT_SIZE 5
#define SECTOR_SIZE_blocks FOOTPRINT_SIZE
#define LOG2_SECTOR_SIZE LOG2_FOOTPRINT_SIZE

#define SHT_WAYS 4
#define SHT_SIZE 1024
#define LOG2_SHT_SIZE 10
#define AST_WAYS 1
#define AST_SIZE 32
#define LOG2_AST_SIZE 5

#define RECOVERY_BLOCKS 2

#define START_OF_SECTOR(addr) ((addr >> (LOG2_BLOCK_SIZE + LOG2_SECTOR_SIZE)) << (LOG2_BLOCK_SIZE + LOG2_SECTOR_SIZE))
#define START_OF_BLOCK(addr) ((addr >> LOG2_BLOCK_SIZE) << LOG2_BLOCK_SIZE)
#define LINE_IN_SECTOR(addr) ((addr >> LOG2_BLOCK_SIZE) % SECTOR_SIZE_blocks)

struct SHT_entry{
    bool valid;
    bool footprints[NUM_FOOTPRINTS_USED][FOOTPRINT_SIZE];
    // Default constructor method
    SHT_entry();
    uint64_t getSetIndex();
    uint32_t getTag();
};

struct AST_entry{
    uint32_t SHTi; // index inside the SHT
    uint64_t sector; //number (address) of sector currently stored 
    bool fetched_by_SFP; // flag to choose the appropriate recovery mechanism in case of miss
    bool active = false;
    bool footprint[FOOTPRINT_SIZE];
    // Default constructor method
    AST_entry();
    uint64_t getSetIndex();
    uint32_t getTag();

};

// Spatial Footprint Predictor
class SFP_prefetcher {
    public:
        // Spatial Footprint History Table
        // Used to store the previously recorded footprints.
        // Each entry consists of one (or more, depends on the configuration) of the previously saved footprints
        std::map<uint32_t, SHT_entry> SHT;
        // Active Sector Table
        // Used to record the footprint while a sector is active in the cache
        // Indexed by the sector address
        std::map<uint64_t, AST_entry> AST;
        std::queue<uint32_t> SHT_use_order; // queue used to keep track of least recently used SHT entry (the one to evict when table is full)


        // Constructor
        SFP_prefetcher();
        //  Fetches all blocks marked in the footprint of the given SHT entry
        void fetchPredictedFootprint(SHT_entry &entry, uint64_t addr, CACHE* cache);

        // Index into table to check if entry is present
        bool hasRecordedFootprint(uint64_t addr, uint64_t ip) ;

        void invalidateRecordedFootprint(uint64_t addr, uint64_t ip) ;

        bool isActive(uint64_t sector);

        // Starts tracking the footprint for this sector.
        // !! Does NOT set the flag fetched_by_SFP. It needs to be set manually
        void activateSector(uint64_t sector, uint64_t addr, uint64_t ip) ;

        // Updates with a footprint for the new memory request 
        void updateAST(uint64_t sector, uint64_t addr);

        // Removes sector from the AST and transfer the recorded footprint to the SHT
        void deactivateSector(uint64_t sector);

        //  Simply fetches the next three blocks in the sector 
        void fetchDefaultPrediction(uint64_t addr, CACHE* cache);
        // Recovery mechanism : fetches lines when we notice we did not fetch all the necessary ones at sector activation.
        // Acts differently based on how we prefetched when we activated the sector (SPF or default)
        void fetchRecovery(uint64_t sector, uint64_t addr, CACHE* cache);
};

// Tag is obtained as last 20 bits of address (excluding address inside cache block and sector ) 
// concatenated with last 12 bits of program counter 
uint32_t getTag(uint64_t addr, uint64_t ip) ;
