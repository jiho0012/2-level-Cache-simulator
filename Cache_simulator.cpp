#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstring>
#include <ctime>
#include <iomanip>
struct CacheBlock {
    bool dirty;
    uint64_t L1_tag;
    uint64_t L2_tag;
};
void printStats(std::ostream& out, const std::string& workloadName, int L1_capacity, int L1_associativity,
                int L2_capacity, int L2_associativity, int block_size,
                int total_accesses, int read_accesses, int write_accesses,
                int L1_read_misses, int L2_read_misses, int L1_write_misses, int L2_write_misses,
                int L1_clean_eviction, int L2_clean_eviction, int L1_dirty_eviction, int L2_dirty_eviction) {
    out << "-- General Stats --" << std::endl;
    out << "L1 Capacity: " << L1_capacity << std::endl;
    out << "L1 way: " << L1_associativity << std::endl;
    out << "L2 Capacity: " << L2_capacity << std::endl;
    out << "L2 way: " << L2_associativity << std::endl;
    out << "Block Size: " << block_size << std::endl;
    out << "Total accesses: " << total_accesses << std::endl;
    out << "Read accesses: " << read_accesses << std::endl;
    out << "Write accesses: " << write_accesses << std::endl;
    out << "L1 Read misses: " << L1_read_misses << std::endl;
    out << "L2 Read misses: " << L2_read_misses << std::endl;
    out << "L1 Write misses: " << L1_write_misses << std::endl;
    out << "L2 Write misses: " << L2_write_misses << std::endl;
    out << "L1 Read miss rate: " << std::fixed<< std::setprecision(2) << 100.0 * L1_read_misses / read_accesses << "%" << std::endl;
    out << "L2 Read miss rate: " <<std::fixed<<  std::setprecision(2) << 100.0 * L2_read_misses / L1_read_misses << "%" << std::endl;
    out << "L1 Write miss rate: " << std::fixed<< std::setprecision(2) << 100.0 * L1_write_misses / write_accesses << "%" << std::endl;
    out << "L2 Write miss rate: " << std::fixed<<std::setprecision(2) << 100.0 * L2_write_misses / L1_write_misses << "%" << std::endl;
    out << "L1 Clean eviction: " << L1_clean_eviction << std::endl;
    out << "L2 Clean eviction: " << L2_clean_eviction << std::endl;
    out << "L1 Dirty eviction: " << L1_dirty_eviction << std::endl;
    out << "L2 Dirty eviction: " << L2_dirty_eviction << std::endl;
}
int main(int argc, char* argv[]) {
    int L2_capacity = 0;
    int L2_associativity = 0;
    int block_size = 0;
    bool lru = false;
    bool random = false;
    std::string trace_file;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            L2_capacity = std::atoi(argv[++i]);
        } else if (strcmp(argv[i], "-a") == 0) {
            L2_associativity = std::atoi(argv[++i]);
        } else if (strcmp(argv[i], "-b") == 0) {
            block_size = std::atoi(argv[++i]);
        } else if (strcmp(argv[i], "-lru") == 0) {
            lru = true;
        } else if (strcmp(argv[i], "-random") == 0) {
            random = true;
        } else {
            trace_file = argv[i];
        }
    }
    std::srand(std::time(nullptr));

    // Calculate the number of sets in L2 cache
    int L2_num_sets = 1024 * L2_capacity / (block_size * L2_associativity);
    std::vector<std::vector<CacheBlock>> L2(L2_num_sets);

    // Calculate the number of sets in L1 cache
    int L1_capacity = L2_capacity / 4;
    int L1_associativity = (L2_associativity >= 4) ? L2_associativity / 4 : L2_associativity;
    int L1_num_sets = 1024 * L1_capacity / (block_size * L1_associativity);
    std::vector<std::vector<CacheBlock>> L1(L1_num_sets);

    // Open the trace file and process each line
    std::ifstream infile(trace_file);
    std::string operation;
    uint64_t address;
    int read_accesses = 0, write_accesses = 0;
    int L1_read_miss=0, L1_write_miss=0;
    int L2_read_miss=0, L2_write_miss=0;
    int L1_clean_eviction=0, L1_dirty_eviction=0;
    int L2_clean_eviction=0, L2_dirty_eviction=0;
    while (infile >> operation >> std::hex >> address) {
        uint64_t block_address = address >> (int)std::log2(block_size);
        uint64_t L1_index = block_address & (L1_num_sets - 1);
        uint64_t L2_index = block_address & (L2_num_sets - 1);
        uint64_t L1_tag = block_address >> (int)std::log2(L1_num_sets);
        uint64_t L2_tag = block_address >> (int)std::log2(L2_num_sets);


        CacheBlock new_block;
        new_block.L1_tag = L1_tag;
        new_block.L2_tag=L2_tag;
        new_block.dirty = false;
        bool l1_hit = false, l2_hit = false;
        std::vector<CacheBlock>::iterator it1;
        std::vector<CacheBlock>::iterator it2;
        // Check L1 for a hit
        if (!L1[L1_index].empty()) {  // L1[L1_index]가 비어 있지 않은지 확인
 	   for (it1 = L1[L1_index].begin(); it1 != L1[L1_index].end(); ++it1) {
	        if (it1->L1_tag == L1_tag) {
	            l1_hit = true;
	            break;
	        }
	   }
	}
        CacheBlock hit_block1;
        if (l1_hit) {
                hit_block1= *it1;
        }
        if (!L2[L2_index].empty()) {  // L2[L2_index]가 비어 있지 않은지 확인 
           for (it2 = L2[L2_index].begin(); it2 != L2[L2_index].end(); ++it2) {
                if (it2->L2_tag == L2_tag) {
                    l2_hit = true;
                    break;
                }
           }
        }
	CacheBlock hit_block2;
        if (l2_hit) {
                hit_block2 = *it2;
        }
        if (operation == "R") {
            read_accesses++;
        } else {write_accesses++;}
	//missrate 계산
	
	if (l1_hit) { //L1 hit L2 hit //ref 순서교체
                if (!l2_hit) {
		    std::cout << "Debugging: L1 hit L2 miss." << std::endl;
		}
		L1[L1_index].erase(it1);
                if (operation=="W"){
                        hit_block1.dirty=true;
                }
                L1[L1_index].push_back(hit_block1);
                L2[L2_index].erase(it2);
                L2[L2_index].push_back(hit_block2);

        } else {//L1 miss (L1 evict)
		if (operation == "R") { 
			L1_read_miss++;
                } else {
			L1_write_miss++;
		}
		
		if (L1[L1_index].size() >= L1_associativity && !L1[L1_index].empty()) {// L1 Evict a block
	                // Eviction from L1
                        auto evict_it = L1[L1_index].begin(); // 기본적으로는 끝 포인터를 할당
                        if (random) {
                                // Random eviction
                                evict_it = L1[L1_index].begin() + std::rand() % L1[L1_index].size();
                        }
                        // Evict the block if found
                        if (evict_it != L1[L1_index].end()) {
                            if (evict_it->dirty) {
                                L1_dirty_eviction++;
                            	auto update_it2 = L2[L2_index].begin();
                                for (; update_it2 != L2[L2_index].end(); ++update_it2) {
                                        if (update_it2->L2_tag == evict_it->L2_tag) {
                                        	update_it2->dirty = true;  // L2의 블록을 dirty로 설정
                                        	break;
                                    	}
                                }
			    } else {
                                L1_clean_eviction++;
                            }
                        }
                        L1[L1_index].erase(evict_it); // Evict the least recently used
                    }
	if (l2_hit){ //L1 miss L2 hit
		//std::cout << "Debugging: L1 miss L2 hit." << std::endl;
		CacheBlock hit_block2_for_L1 = hit_block2; 
		L2[L2_index].erase(it2);
                L2[L2_index].push_back(hit_block2);
		if (operation == "W") {
                        hit_block2_for_L1.dirty = true;
                }
                L1[L1_index].push_back(hit_block2_for_L1);
	} else { // L1 miss L2 miss (L2 evict)
		if (operation == "R") {
                        L2_read_miss++;
                } else {
			L2_write_miss++;
		}
		if (L2[L2_index].size() >= L2_associativity && !L2[L2_index].empty()) {  // L2 evict
			// Eviction from L2
			auto evict_it = L2[L2_index].begin(); // 기본적으로는 끝 포인터를 할당
			if (random) {
    				// Random eviction
    				evict_it = L2[L2_index].begin() + std::rand() % L2[L2_index].size();
			}
			// Evict the block if found
			if (evict_it != L2[L2_index].end()) {
			    if (evict_it->dirty) {
			        L2_dirty_eviction++;
			    } else {
			        L2_clean_eviction++;
			    }
			}
			// Corresponding eviction in L1
			auto update_it1 = L1[L1_index].begin(); //L2 evict시 L1도 evict
                        
			for (; update_it1 != L1[L1_index].end(); ++update_it1) {
                                if (update_it1->L1_tag == evict_it->L1_tag) {
	                                break;
        	                        }
                        }
			if (update_it1 != L1[L1_index].end()) {
				if (update_it1->dirty) {
					L1_dirty_eviction++;
				} else {
					L1_clean_eviction++;
				}
				L1[L1_index].erase(update_it1);
			}
			L2[L2_index].erase(evict_it);
		}
		CacheBlock new_block_for_L1 = new_block;
                L2[L2_index].push_back(new_block);
                if (operation == "W") {
                        new_block_for_L1.dirty = true;
                }
                L1[L1_index].push_back(new_block_for_L1);
	}	
	}	
	
	}
    // 파일 이름 구성
    std::string baseName = trace_file.substr(0, trace_file.find(".out"));
    std::string outputFileName = baseName + "_" + std::to_string(L2_capacity) + "_" +
                                 std::to_string(L2_associativity) + "_" + std::to_string(block_size) + ".out";

    // 파일 스트림 열기
    std::ofstream outfile(outputFileName);

    // 결과 출력
//    printStats(std::cout, baseName, L1_capacity, L1_associativity, L2_capacity, L2_associativity, block_size,
//               read_accesses + write_accesses, read_accesses, write_accesses,
//               L1_read_miss, L2_read_miss, L1_write_miss, L2_write_miss,
//               L1_clean_eviction, L2_clean_eviction, L1_dirty_eviction, L2_dirty_eviction);

    printStats(outfile, baseName, L1_capacity, L1_associativity, L2_capacity, L2_associativity, block_size,
               read_accesses + write_accesses, read_accesses, write_accesses,
               L1_read_miss, L2_read_miss, L1_write_miss, L2_write_miss,
               L1_clean_eviction, L2_clean_eviction, L1_dirty_eviction, L2_dirty_eviction);

    // 파일 닫기
    outfile.close();


    return 0;
}

