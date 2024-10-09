#include <fstream>
#include <iostream>
#include <string>
#include <cassert>
#include <charconv>

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
  //constexpr const char* i_file_name = "./input/input_file.txt";
  constexpr const char* o_file_name = "./output/output_file.txt";

  std::ifstream ifile(argv[1], std::ios_base::in);
  assert(ifile.is_open());

  sp::MyHashSet<std::string, Station> data(1000);
  std::string city;
  std::string temps;
  double temp {0};

  while(!ifile.eof()) {
    if(!std::getline(ifile, city, ';')) [[unlikely]] {
      std::cout << "Failed to read the city " << city << ", line nu:" << data.size() << '\n';
      break;
    }	

    if(!std::getline(ifile, temps, '\n')) [[unlikely]] {
      std::cout << "Failed to read the tmeps " << city << ", line nu:" << data.size() << '\n';
      break;
    }
    std::from_chars(temps.data(), temps.data()+temps.size(), temp);
    data[city].Add(temp);
  }

  ifile.close();
  std::ofstream ofile(o_file_name, std::ios::app);
  assert(ofile.is_open());
  ofile.clear();
  //std::cout << "Total stations: " << data.size() << '\n';
  std::string oline;
  //for(auto& st: data) {
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
    oline += '\n';
    ofile << oline.c_str();
    //oline.clear();
  }
  ofile.close();
}
