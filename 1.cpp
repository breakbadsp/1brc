#include <fstream>
#include <iostream>
#include <string>
#include <cassert>
#include <unordered_map>


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

  std::fstream ifile(argv[1], std::ios_base::in);
  assert(ifile.is_open());

  std::unordered_map<std::string, Station> data;
  std::string city;
  std::string temps;

  while(!ifile.eof()) {
    if(!std::getline(ifile, city, ';')) [[unlikely]] {
      std::cout << "Failed to read the city " << city << ", line nu:" << ln <<  '\n';
      break;
    }	

    if(!std::getline(ifile, temps, '\n')) [[unlikely]] {
      std::cout << "Failed to read the tmeps " << city << ", line nu:" << ln <<  '\n';
      break;
    }

    data[city].Add(std::atof(temps.c_str()));
  }

  ifile.close();
  std::ofstream ofile(o_file_name, std::ios::app);
  assert(ofile.is_open());
  ofile.clear();
  //std::cout << "Total stations: " << data.size() << '\n';
  std::string oline;
  for(auto& st: data) {
    st.second.CalculateMean();
    oline += st.first;
    oline += ';';
    oline += std::to_string(st.second.min);
    oline += ';';
    oline += std::to_string(st.second.mean);
    oline += ';';
    oline += std::to_string(st.second.max);
    oline += '\n';
    ofile << oline.c_str();
    oline.clear();
  }
  ofile.close();
}
