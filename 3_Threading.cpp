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
#include <array>
#include <optional>
#include <atomic>
#include <thread>

#include "new_hash_set.hpp"

constexpr bool debug = false;

auto to_chars(float temp)
{
  std::array<char, 20> buffer;
  auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), temp);
  if (result.ec == std::errc())
  {
    // Null-terminate the string manually (std::to_chars does not do this)
    *result.ptr = '\0';
  }
  else
  {
    std::cerr << "Conversion failed for: " << temp << std::endl;
    exit(0);
  }
  return buffer;
}

struct Station
{
  float min{__INT32_MAX__};
  float max{0};
  float mean{0};

  float total{0};
  int total_measurements{0};

  void Add(float temp)
  {
    if (temp < min) [[unlikely]]
      min = temp;

    if (temp > max) [[unlikely]]
      max = temp;

    total += temp;
    total_measurements++;
  }

  void CalculateMean()
  {
    mean = (total / total_measurements);
  }

  Station &operator+=(const Station &p_rhs)
  {
    if (&p_rhs == this)
      return *this;

    if (p_rhs.min < min) [[unlikely]]
      min = p_rhs.min;

    if (p_rhs.max > max) [[unlikely]]
      max = p_rhs.max;

    total += p_rhs.total;
    total_measurements += p_rhs.total_measurements;
    return *this;
  }
};

void Worker(void *mmf, long start, long last, sp::MyHashSet<std::string_view, Station> &data)
{
  static std::atomic_short thread_id = 0;
  if(debug) std::cout << "Thread " << ++thread_id << " started.\n";
  char *mmf_cstr = (char *)mmf;
  if(start != 0) 
  {
    while (*(mmf_cstr + start) != '\n')
    {
      ++start;
    }
  }

  long offset = ++start;
  float temp{0};
  while (offset < last)
  {
    long beg = offset;
    while (*(mmf_cstr + offset) != ';')
      ++offset;
    std::string_view city(mmf_cstr + beg, (size_t)(offset - beg));

    beg = ++offset;
    while (*(mmf_cstr + offset) != '\n')
      ++offset;
    std::string_view temps(mmf_cstr + beg, (size_t)(offset - beg + 1));

    std::from_chars(temps.data(), temps.data() + temps.size(), temp);
    data[city].Add(temp);
  }
}

int main(int argc, char *argv[])
{
  if (argc < 2) [[unlikely]]
  {
    if(debug) std::cout << "usage: analyse <input_file>" << '\n';
    return -1;
  }

  using namespace std::literals;
  auto ifd = open(argv[1], O_RDONLY);
  if (ifd == -1) [[unlikely]]
  {
    return -1;
  }

  struct stat sb;
  auto fstret = fstat(ifd, &sb);
  if (fstret == -1) [[unlikely]]
  {
    return -1;
  }

  void *mmf = mmap(nullptr, (size_t)sb.st_size, PROT_READ, MAP_PRIVATE, ifd, 0);
  if (mmf == MAP_FAILED) [[unlikely]]
  {
    std::cout << "MMAP Failed\n";
    return -1;
  }
  madvise(mmf, (size_t)sb.st_size, MADV_WILLNEED);
  long unit = (size_t)sb.st_size / 6; // number of processors
  std::array<sp::MyHashSet<std::string_view, Station>, 6> datas;
  long offset = 0;

  std::array<std::thread, 6> workers;
  if(debug) std::cout << "Total size of the input file " << sb.st_size << '\n';
  for (int i = 0; i < 6; ++i)
  {
    if(debug) std::cout << "thread " << i << " working on " << offset << " to " << (long)unit * (i + 1) << std::endl;
    std::thread t(Worker, (void *)mmf, (long)offset, (long)unit * (i + 1), std::ref(datas[i]));
    workers[i] = std::move(t);
    offset = unit * (i + 1) + 1;
  }

  for (auto &t : workers)
    t.join();

  sp::MyHashSet<std::string_view, Station> data;
  for (auto &d : datas)
  {
    for (auto i = d.begin(); i != d.end(); ++i)
    {
      auto &station = data[i->first];
      station += (i->second);
    }
  }

  // Writing to output file
  auto ofd = open(argv[2], O_RDWR | O_CREAT, 0666);
  if (ofd == -1) [[unlikely]]
  {
    // if(debug) LOG( << "Failed to open file: errono: " << errno << '=' << '\n';
    return -1;
  }

  auto ret = truncate(argv[2], (long)data.size() * 50 ); // total_lines * 1000);
  if (ret == -1) [[unlikely]]
  {
    // if(debug) LOG( << "Failed to truncate file: errono: " << errno << ' ' << strerror(errno) << '\n';
    return -1;
  }

  struct stat sd2;
  ret = fstat(ofd, &sd2);
  assert(ret != -1);

  void *ommf = mmap(nullptr, data.size() * 50, PROT_WRITE | PROT_READ, MAP_SHARED, ofd, 0);
  if (ommf == MAP_FAILED) [[unlikely]]
  {
    return -1;
  }

  size_t woffset = 0;
  std::string oline;
  oline.reserve(15);
  for (auto i = data.begin(); i != data.end(); ++i)
  {
    auto &st = i->second;
    auto &city = i->first;
    st.CalculateMean();
    oline = city;
    oline += ';';
    oline += to_chars(st.min).data();
    oline += ';';
    oline += to_chars(st.mean).data();
    oline += ';';
    oline += to_chars(st.max).data();
    // if(debug) LOG( << "writing line: " << oline << '\n';
    const auto l = oline.size();
    void *waddr = (void *)((char *)ommf + woffset);
    memcpy(waddr, oline.c_str(), l);
    woffset = woffset + l + 1;
  }

  munmap(mmf, (size_t)sb.st_size);
  close(ifd);
  munmap(ommf, data.size() * 40);
  close(ofd);
}
