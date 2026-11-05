#pragma once
#include <array>
#include <atomic>
#include <cstdint>
namespace plate {
// Bounded reader: audio keeps its last coherent snapshot during a preset write.
// There is no retry/spin, lock, allocation or tree access on the audio thread.
class ParameterTransactions {
 std::atomic<uint32_t> writers{0},generation{0};
public:
 static_assert(std::atomic<uint32_t>::is_always_lock_free);
 class Write {
  ParameterTransactions& owner;
 public:
  explicit Write(ParameterTransactions& o):owner(o){++owner.writers;}
  ~Write(){++owner.generation;--owner.writers;}
  Write(const Write&)=delete;Write& operator=(const Write&)=delete;
 };
 template<std::size_t N> bool capture(const std::array<std::atomic<float>*,N>& raw,std::array<float,N>& result)const noexcept {
  if(writers.load()!=0)return false;
  const auto before=generation.load();std::array<float,N> candidate{};
  for(std::size_t i=0;i<N;++i)candidate[i]=raw[i]->load();
  // Order matters: finish checking the writer count BEFORE the generation.
  if(writers.load()!=0||generation.load()!=before)return false;
  result=candidate;return true;
 }
};
}
