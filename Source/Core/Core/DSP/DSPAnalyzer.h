// Copyright 2009 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <array>
#include "Common/CommonTypes.h"

namespace DSP
{
struct SDSP;
}

// Basic code analysis.
namespace DSP::Analyzer
{
// Useful things to detect:
// * Loop endpoints - so that we can avoid checking for loops every cycle.

enum CodeFlags : u8
{
  CODE_NONE = 0,
  CODE_START_OF_INST = 1,
  CODE_IDLE_SKIP = 2,
  CODE_LOOP_START = 4,
  CODE_LOOP_END = 8,
  CODE_UPDATE_SR = 16,
  CODE_CHECK_EXC = 32,
};

class Analyzer
{
public:
  explicit Analyzer(const SDSP& dsp);
  ~Analyzer();

  Analyzer(const Analyzer&) = default;
  Analyzer& operator=(const Analyzer&) = delete;

  Analyzer(Analyzer&&) = default;
  Analyzer& operator=(Analyzer&&) = delete;

  // This one should be called every time IRAM changes - which is basically
  // every time that a new ucode gets uploaded, and never else. At that point,
  // we can do as much static analysis as we want - but we should always throw
  // all old analysis away. Luckily the entire address space is only 64K code
  // words and the actual code space 8K instructions in total, so we can do
  // some pretty expensive analysis if necessary.
  void Analyze();

  // Whether or not the given address indicates the start of an instruction.
  [[nodiscard]] bool IsStartOfInstruction(u16 address) const
  {
    return (GetCodeFlags(address) & CODE_START_OF_INST) != 0;
  }

  // Whether or not the address indicates an idle skip location.
  [[nodiscard]] bool IsIdleSkip(u16 address) const
  {
    return (GetCodeFlags(address) & CODE_IDLE_SKIP) != 0;
  }

  // Whether or not the address indicates the start of a loop.
  [[nodiscard]] bool IsLoopStart(u16 address) const
  {
    return (GetCodeFlags(address) & CODE_LOOP_START) != 0;
  }

  // Whether or not the address indicates the end of a loop.
  [[nodiscard]] bool IsLoopEnd(u16 address) const
  {
    return (GetCodeFlags(address) & CODE_LOOP_END) != 0;
  }

  // Whether or not the address describes an instruction that requires updating the SR register.
  [[nodiscard]] bool IsUpdateSR(u16 address) const
  {
    return (GetCodeFlags(address) & CODE_UPDATE_SR) != 0;
  }

  // Whether or not the address describes instructions that potentially raise exceptions.
  [[nodiscard]] bool IsCheckExceptions(u16 address) const
  {
    return (GetCodeFlags(address) & CODE_CHECK_EXC) != 0;
  }

private:
  // Flushes all analyzed state.
  void Reset();

  // Analyzes a region of DSP memory.
  // Note: start is inclusive, end is exclusive.
  void AnalyzeRange(u16 start_addr, u16 end_addr);

  // Retrieves the flags set during analysis for code in memory.
  [[nodiscard]] u8 GetCodeFlags(u16 address) const { return m_code_flags[address]; }

  // Holds data about all instructions in RAM.
  std::array<u8, 65536> m_code_flags{};

  // DSP context for analysis to be run under.
  const SDSP& m_dsp;
};
}  // namespace DSP::Analyzer
