/**
  * Assignment: memory
  * Operating Systems
  */

// function/class definitions you are going to use
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdint>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>
#include <sys/mman.h>
#include <cstring>

// although it is good habit, you don't have to type 'std::' before many objects by including this line
using namespace std;

// size of the image
const int64_t SIZE = 6384ULL;
const int64_t REPEAT = 21ULL;
const size_t TOTALSIZE = SIZE*SIZE*sizeof(float);

int main(int argc, char* argv[]) {
  float* img = new float[SIZE * SIZE];

  // fill with dummy data
  for (int64_t i = 0; i < SIZE; i++) {
    for (int64_t j = 0; j < SIZE; j++) {
      img[j * SIZE + i] = (2 * j + i) % 32768;
    }
  }

  // this dummy value is needed to avoid compilers eliminating the loop as part of a optimisation
  uint64_t dummy = 0;

  // ADJUST BELOW, BUT keep writing to the dummy variable
  float* res = (float*)mmap(nullptr, TOTALSIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (res == MAP_FAILED) {
      perror("mmap failed");
      delete img;
      return -1; 
  }

  memset(res, 0, TOTALSIZE);

  // Apply an averaging imaging filter to some input image, and write in to an output image.
  // A pixel in the output image is calculated by averaging 9 pixels: the pixel at the same
  // coordinates in the input image, and the adjecent pixels.
  constexpr int block_size = 16 * 1024; 

 if (REPEAT > 0) {
    for (int64_t i = 1; i < SIZE - 1; i += block_size) {
        for (int64_t j = 1; j < SIZE - 1; j += block_size) {
            for (int64_t b_row = i; b_row < std::min(i + block_size, SIZE - 1); ++b_row) {
                for (int64_t b_col = j; b_col < std::min(j + block_size, SIZE - 1); ++b_col) {
                    res[b_row * SIZE + b_col] = 0;

                    for (long k = -1; k <= 1; k++) {
                        for (long l = -1; l <= 1; l++) {
                            int64_t img_row = b_row + k;
                            int64_t img_col = b_col + l;

                            if (img_row >= 0 && img_row < SIZE && img_col >= 0 && img_col < SIZE) {
                                res[b_row * SIZE + b_col] += img[img_row * SIZE + img_col];
                            }
                        }
                    }
                    res[b_row * SIZE + b_col] /= 9;
                }
            }
        }
    }

    for (int64_t r = 0; r < REPEAT; ++r) {
        for (int64_t i = 1; i < SIZE - 1; i++) {
            for (int64_t j = 1; j < SIZE - 1; j++) {
                dummy += res[j * SIZE + i];
            }
        }
    }
}


  if (munmap(res, TOTALSIZE) == -1) {
    perror("munmap failed");
  }
  // ADJUST ABOVE, BUT keep writing to the dummy variable
  delete[] img;

  struct rusage usage;
  std::cout<< "getrusage():                 " << getrusage(RUSAGE_SELF, &usage) << std::endl;
  std::cout << "CPU time:                   " << usage.ru_stime.tv_sec << "." << std::fixed << std::setw(6) << std::setprecision(6) << std::setfill('0') << usage.ru_stime.tv_usec << " s" << std::endl;
  std::cout << "user time:                    " << usage.ru_utime.tv_sec << "." << std::fixed << std::setw(6) << std::setprecision(6) << std::setfill('0') << usage.ru_utime.tv_usec << " s" << std::endl;
  std::cout << "soft page faults:             " << usage.ru_minflt << std::endl;
  std::cout << "hard page faults:             " << usage.ru_majflt << std::endl;
#ifdef __APPLE__
  std::cout << "max memory:                   " << usage.ru_maxrss/1024 << " KiB" << std::endl;
#else
  std::cout << "max memory:                   " << usage.ru_maxrss << " KiB" << std::endl;
#endif
  std::cout << "voluntary context switches:   " << usage.ru_nvcsw << std::endl;
  std::cout << "involuntary context switches: " << usage.ru_nivcsw << std::endl;
  std::cout << "dummy value (ignore):         " << dummy << std::endl; // this value is printed to avoid optimisations
  std::cout << "typical page size:            " << sysconf(_SC_PAGESIZE) << std::endl;
  // ...
  return 0;
}
