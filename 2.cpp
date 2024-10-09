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

  //
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
  if(argc < 2) {
    std::cout << "usage: analyse <input_file>\n";
    return -1;
  }

  using namespace std::literals;
  //constexpr const char* o_file_name = "./output/output_file.txt";

  //if(argc > 2) [[unlikely]]
    //o_file_name = argv[1];


  auto ifd = open(argv[1], O_RDONLY);
  assert(ifd != -1);

  struct stat sb;
  assert(fstat(ifd, &sb) != -1);

  void* mmf = mmap(nullptr, (size_t)sb.st_size, PROT_READ, MAP_PRIVATE, ifd, 0);
  assert(mmf != MAP_FAILED);

  sp::MyHashSet<std::string, Station> data(1000);
  std::string city;
  std::string temps;
  double temp {0};

  long offset = 0;
  
  char* mmf_cstr = (char*)mmf;
  size_t ln = 0;
  //constexpr size_t total_lines = 1000000000;
  constexpr size_t total_lines = 1000000;
  while(offset < sb.st_size && ln < total_lines) {
    ++ln;
    long beg = offset;
    while(*(mmf_cstr + offset) != ';') ++offset;
    city = std::string(mmf_cstr + beg, (size_t)(offset - beg));
    //if(debug) std::cout << "city:" << city << ' ';

    beg = offset + 1;
    while(*(mmf_cstr + offset) != '\n') ++offset;
    temps = std::string(mmf_cstr + beg, (size_t)(offset - beg + 1));
    //if(debug) std::cout << " temps:" << temps;

    std::from_chars(temps.data(), temps.data()+temps.size(), temp);
    data[city].Add(temp);
    //++offset;
  }

  assert(munmap(mmf, (size_t)sb.st_size) != -1);
  close(ifd);
  //std::cout << "Total stations: " << data.size() << '\n';


  //Writing to output file
  //auto ofd = open(o_file_name, O_RDONLY| O_CREAT, 0666);
  auto ofd = open(argv[2], O_RDWR| O_CREAT, 0666);
  if(ofd == -1) {
    std::cout << "Failed to open file: errono: " << errno << '=' << '\n';
    return -1;
  }

  auto ret = truncate(argv[2], total_lines * 100);//total_lines * 1000);
  if(ret == -1) {
    std::cout << "Failed to truncate file: errono: " << errno << ' ' << strerror(errno) << '\n';
    return -1;
  }
  
  struct stat sd2;
  ret = fstat(ofd, &sd2);
  assert(ret != -1);

  void* ommf = mmap(nullptr, total_lines * 100, PROT_WRITE | PROT_READ, MAP_SHARED, ofd, 0);
  if(ommf == MAP_FAILED) {
    std::cout << "MMAP failed, errno:" << errno << ' ' << strerror(errno) << '\n';
    return -1;
  }
  //std::cout << "wa: " << std::hex <<  ommf << '\n';


  size_t woffset = 0;
  //char* wr = (char*)ommf;
  std::string oline;
  //std::cout << "Total stations: " << data.size() << '\n';
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
    //std::cout << "writing line: " << oline << '\n';
    const auto l = oline.size();
    void* waddr = (void*) ((char*)ommf + woffset);
    memcpy(waddr, oline.c_str(), l);
    woffset = woffset + l + 1;
  }
  assert(munmap(mmf, (size_t)sb.st_size) != -1);
  close(ofd);
}
