#include <fstream>
#include <iostream>
#include <string>
#include <cassert>
#include <charconv>
#include <fcntl.h>
#include <unistd.h> 
#include <sys/stat.h>
#include <sys/mman.h>
#include <cstring> // for strerror()

#include "new_hash_set.hpp"


struct Station {
  double min {__INT32_MAX__};
  double mean {0};
  double max {0};

  double total {0};
  int total_measurements {0};

   void Add(double temp) {
    if(temp < min) [[unlikely]]
      min = temp;
    
    if (temp > max) [[unlikely]]
      max = temp;

    total += temp;
    total_measurements++;
   }

   void CalculateMean() {
      mean = total / total_measurements;
   }
};

int main(int argc, char* argv[]) {
  if(argc < 2) [[unlikely]] {
    std::cout << "usage: analyse <input_file>" << '\n';
    return -1;
  }

  using namespace std::literals;
  auto ifd = open(argv[1], O_RDONLY);
  if(ifd == -1) [[unlikely]] {
    return -1;
  }

  struct stat sb;
  auto fstret = fstat(ifd, &sb);
  if(fstret == -1) [[unlikely]] {
    return -1;
  }

  void* mmf = mmap(nullptr, (size_t)sb.st_size, PROT_READ, MAP_PRIVATE, ifd, 0);
  if(mmf == MAP_FAILED) [[unlikely]] {
    return -1;
  }

  sp::MyHashSet<std::string, Station> data(1000);
  std::string city;
  std::string temps;
  double temp {0};

  long offset = 0;
  
  char* mmf_cstr = (char*)mmf;
  size_t ln = 0;
  constexpr size_t total_lines = 1000000000;
  //constexpr size_t total_lines = 1000000;
  while(offset < sb.st_size && ln < total_lines) {
    ++ln;
    long beg = offset;
    while(*(mmf_cstr + offset) != ';') ++offset;
    city = std::string(mmf_cstr + beg, (size_t)(offset - beg));

    beg = offset + 1;
    while(*(mmf_cstr + offset) != '\n') ++offset;
    temps = std::string(mmf_cstr + beg, (size_t)(offset - beg + 1));

    std::from_chars(temps.data(), temps.data()+temps.size(), temp);
    data[city].Add(temp);
  }

  assert(munmap(mmf, (size_t)sb.st_size) != -1);
  close(ifd);
  //if(debug) std::cout << "Total stations: " << data.size() << '\n';


  //Writing to output file
  auto ofd = open(argv[2], O_RDWR|O_CREAT, 0666);
  if(ofd == -1) [[unlikely]] {
    //if(debug) LOG( << "Failed to open file: errono: " << errno << '=' << '\n';
    return -1;
  }

  auto ret = truncate(argv[2], total_lines * 100);//total_lines * 1000);
  if(ret == -1) [[unlikely]] {
    //if(debug) LOG( << "Failed to truncate file: errono: " << errno << ' ' << strerror(errno) << '\n';
    return -1;
  }
  
  struct stat sd2;
  ret = fstat(ofd, &sd2);
  assert(ret != -1);

  void* ommf = mmap(nullptr, total_lines * 100, PROT_WRITE | PROT_READ, MAP_SHARED, ofd, 0);
  if(ommf == MAP_FAILED) [[unlikely]] {
    return -1;
  }

  size_t woffset = 0;
  std::string oline;
  for(auto i = data.begin();  i != data.end(); ++i) {
    auto& st = i->second;
    auto& city = i->first;
    st.CalculateMean();
    oline = city;
    oline += ';';
    oline += std::to_string(st.min);
    oline += ';';
    oline += std::to_string(st.mean);
    oline += ';';
    oline += std::to_string(st.max);
    //if(debug) LOG( << "writing line: " << oline << '\n';
    const auto l = oline.size();
    void* waddr = (void*) ((char*)ommf + woffset);
    memcpy(waddr, oline.c_str(), l);
    woffset = woffset + l + 1;
  }
  assert(munmap(mmf, (size_t)sb.st_size) != -1);
  close(ofd);
}
