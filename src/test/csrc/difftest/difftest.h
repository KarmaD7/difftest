/***************************************************************************************
* Copyright (c) 2020-2021 Institute of Computing Technology, Chinese Academy of Sciences
* Copyright (c) 2020-2021 Peng Cheng Laboratory
*
* XiangShan is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __DIFFTEST_H__
#define __DIFFTEST_H__

#include "common.h"
#include "refproxy.h"

#define DIFFTEST_CORE_NUMBER  NUM_CORES

enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };
enum { REF_TO_DUT, DUT_TO_REF };
enum { REF_TO_DIFFTEST, DUT_TO_DIFFTEST };
enum { ICACHEID, DCACHEID };
// DIFFTEST_TO_DUT ~ REF_TO_DUT ~ REF_TO_DIFFTEST
// DIFFTEST_TO_REF ~ DUT_TO_REF ~ DUT_TO_DIFFTEST
#define CP printf("%s: %d\n", __FILE__, __LINE__);fflush( stdout );

#define DEBUG_MEM_REGION(v, f) (f <= (DEBUG_MEM_BASE + 0x1000) && \
        f >= DEBUG_MEM_BASE && \
        v)
#define IS_LOAD_STORE(instr) (((instr & 0x7f) == 0x03) || ((instr & 0x7f) == 0x23))
#define IS_TRIGGERCSR(instr) (((instr & 0x7f) == 0x73) && ((instr & (0xff0 << 20)) == (0x7a0 << 20)))
#define IS_DEBUGCSR(instr) (((instr & 0x7f) == 0x73) && ((instr & (0xffe << 20)) == (0x7b0 << 20))) // 7b0 and 7b1
#ifdef DEBUG_MODE_DIFF
#define DEBUG_MODE_SKIP(v, f, instr) DEBUG_MEM_REGION(v, f) && \
(IS_LOAD_STORE(instr) || IS_TRIGGERCSR(instr))
#else
#define DEBUG_MODE_SKIP(v, f, instr) false
#endif

// Difftest structures
// trap events: self-defined traps
typedef struct {
  uint8_t  valid = 0;
  uint8_t  code = 0;
  uint32_t pc = 0;
  uint64_t cycleCnt = 0;
  uint64_t instrCnt = 0;
  uint8_t  hasWFI = 0;
} trap_event_t;

// architectural events: interrupts and exceptions
// whose priority should be higher than normal commits
typedef struct {
  uint32_t interrupt = 0;
  uint32_t exception = 0;
  uint32_t exceptionPC = 0;
  uint32_t exceptionInst = 0;
} arch_event_t;

typedef struct {
  uint8_t  valid = 0;
  uint32_t pc;
  uint32_t inst;
  uint8_t  skip;
  uint8_t  isRVC;
  uint8_t  fused;
  uint8_t  rfwen;
  uint8_t  fpwen;
  uint32_t wpdest;
  uint8_t  wdest;
} instr_commit_t;

typedef struct {
  uint32_t gpr[32];
} arch_reg_state_t;

typedef struct __attribute__((packed)) {
  uint32_t this_pc;
  uint32_t mstatus;
  uint32_t mcause;
  uint32_t mepc;
  uint32_t sstatus;
  uint32_t scause;
  uint32_t sepc;
  uint32_t satp;
  uint32_t mip;
  uint32_t mie;
  uint32_t mscratch;
  uint32_t sscratch;
  uint32_t mideleg;
  uint32_t medeleg;
  uint32_t mtval;
  uint32_t stval;
  uint32_t mtvec;
  uint32_t stvec;
  uint32_t priviledgeMode;
} arch_csr_state_t;

typedef struct __attribute__((packed)) {
  uint32_t debugMode;
  uint32_t dcsr;
  uint32_t dpc;
  uint32_t dscratch0;
  uint32_t dscratch1;
} debug_mode_t;

#ifndef DEBUG_MODE_DIFF
const int DIFFTEST_NR_REG = (sizeof(arch_reg_state_t) + sizeof(arch_csr_state_t)) / sizeof(uint64_t);
#else
const int DIFFTEST_NR_REG = (sizeof(arch_reg_state_t) + sizeof(arch_csr_state_t) + sizeof(debug_mode_t)) / sizeof(uint64_t);
#endif

typedef struct {
  uint8_t  valid = 0;
  uint32_t addr;
  uint32_t data;
  uint8_t  mask;
} store_event_t;

typedef struct {
  uint8_t  resp = 0;
  uint32_t addr;
  uint32_t data[4];
} ptw_event_t;

typedef struct {
  trap_event_t      trap;
  arch_event_t      event;
  instr_commit_t    commit[DIFFTEST_COMMIT_WIDTH];
  arch_reg_state_t  regs;
  arch_csr_state_t  csr;
  debug_mode_t      dmregs;
  store_event_t     store[DIFFTEST_STORE_WIDTH];
  ptw_event_t       ptw;
} difftest_core_state_t;

enum retire_inst_type {
  RET_NORMAL=0,
  RET_INT,
  RET_EXC
};

class DiffState {
public:
  DiffState();
  void record_group(uint32_t pc, uint64_t count) {
    retire_group_pc_queue [retire_group_pointer] = pc;
    retire_group_cnt_queue[retire_group_pointer] = count;
    retire_group_pointer = (retire_group_pointer + 1) % DEBUG_GROUP_TRACE_SIZE;
  };
  void record_inst(uint32_t pc, uint32_t inst, uint8_t en, uint8_t dest, uint32_t data, bool skip) {
    retire_inst_pc_queue   [retire_inst_pointer] = pc;
    retire_inst_inst_queue [retire_inst_pointer] = inst;
    retire_inst_wen_queue  [retire_inst_pointer] = en;
    retire_inst_wdst_queue [retire_inst_pointer] = dest;
    retire_inst_wdata_queue[retire_inst_pointer] = data;
    retire_inst_skip_queue[retire_inst_pointer] = skip;
    retire_inst_type_queue[retire_inst_pointer] = RET_NORMAL;
    retire_inst_pointer = (retire_inst_pointer + 1) % DEBUG_INST_TRACE_SIZE;
  };
  void record_abnormal_inst(uint32_t pc, uint32_t inst, uint32_t abnormal_type, uint32_t cause) {
    retire_inst_pc_queue   [retire_inst_pointer] = pc;
    retire_inst_inst_queue [retire_inst_pointer] = inst;
    retire_inst_wdata_queue[retire_inst_pointer] = cause; // write cause to data queue to save space
    retire_inst_type_queue[retire_inst_pointer] = abnormal_type;
    retire_inst_pointer = (retire_inst_pointer + 1) % DEBUG_INST_TRACE_SIZE;
  };
  void display(int coreid);

private:
  int retire_group_pointer = 0;
  uint32_t retire_group_pc_queue[DEBUG_GROUP_TRACE_SIZE] = {0};
  uint32_t retire_group_cnt_queue[DEBUG_GROUP_TRACE_SIZE] = {0};

  int retire_inst_pointer = 0;
  uint32_t retire_inst_pc_queue[DEBUG_INST_TRACE_SIZE] = {0};
  uint32_t retire_inst_inst_queue[DEBUG_INST_TRACE_SIZE] = {0};
  uint32_t retire_inst_wen_queue[DEBUG_INST_TRACE_SIZE] = {0};
  uint32_t retire_inst_wdst_queue[DEBUG_INST_TRACE_SIZE] = {0};
  uint32_t retire_inst_wdata_queue[DEBUG_INST_TRACE_SIZE] = {0};
  uint32_t retire_inst_type_queue[DEBUG_INST_TRACE_SIZE] = {0};
  bool retire_inst_skip_queue[DEBUG_INST_TRACE_SIZE] = {0};
};

class Difftest {
public:
  // Difftest public APIs for testbench
  // Its backend should be cross-platform (NEMU, Spike, ...)
  // Initialize difftest environments
  Difftest(int coreid);
  DIFF_PROXY *proxy = NULL;
  uint32_t num_commit = 0; // # of commits if made progress
  bool has_commit = false;
  // Trigger a difftest checking procdure
  virtual int step();
  void update_nemuproxy(int, size_t);
  inline bool get_trap_valid() {
    return dut.trap.valid;
  }
  inline int get_trap_code() {
    return dut.trap.code;
  }
  void display();

  // Difftest public APIs for dut: called from DPI-C functions (or testbench)
  // These functions generally do nothing but copy the information to core_state.
  inline trap_event_t *get_trap_event() {
    return &(dut.trap);
  }
  inline arch_event_t *get_arch_event() {
    return &(dut.event);
  }
  inline instr_commit_t *get_instr_commit(uint8_t index) {
    return &(dut.commit[index]);
  }
  inline arch_csr_state_t *get_csr_state() {
    return &(dut.csr);
  }
  inline arch_reg_state_t *get_arch_reg_state() {
    return &(dut.regs);
  }
  inline store_event_t *get_store_event(uint8_t index) {
    return &(dut.store[index]);
  }
  inline ptw_event_t *get_ptw_event() {
    return &(dut.ptw);
  }
  inline difftest_core_state_t *get_dut() {
    return &dut;
  }
  inline difftest_core_state_t *get_ref() {
    return &ref;
  }
  inline debug_mode_t *get_debug_state() {
    return &(dut.dmregs);
  }

#ifdef DEBUG_REFILL
  void save_track_instr(uint32_t instr) {
    track_instr = instr;
  }
#endif

#ifdef DEBUG_MODE_DIFF
  void debug_mode_copy(uint32_t addr, size_t size, uint32_t data) {
    proxy->debug_mem_sync(addr, &data, size);
  }
#endif

protected:
  const uint64_t firstCommit_limit = 10000;
  const uint64_t stuck_limit = 5000;

  int id;
  difftest_core_state_t dut;
  difftest_core_state_t ref;
  uint32_t *ref_regs_ptr = (uint32_t*)&ref.regs;
  uint32_t *dut_regs_ptr = (uint32_t*)&dut.regs;

  bool progress = false;
  uint64_t ticks = 0;
  uint64_t last_commit = 0;

  uint32_t nemu_this_pc;
  DiffState *state = NULL;
#ifdef DEBUG_REFILL
  uint32_t track_instr = 0;
#endif

  void update_last_commit() { last_commit = ticks; }
  int check_timeout();
  void do_first_instr_commit();
  void do_interrupt();
  void do_exception();
  void do_instr_commit(int index);
  int do_store_check();
  int do_refill_check(int cacheid);
  int do_irefill_check();
  int do_drefill_check();
  int do_golden_memory_update();
  // inline uint64_t *ref_regs_ptr() { return (uint64_t*)&ref.regs; }
  // inline uint64_t *dut_regs_ptr() { return (uint64_t*)&dut.regs; }
  inline uint32_t get_commit_data(int i) {
    uint32_t result = dut.regs.gpr[dut.commit[i].wpdest];
    return result;
  }
  inline bool has_wfi() {
    return dut.trap.hasWFI;
  }

  void raise_trap(int trapCode);
  void clear_step();
};

extern Difftest **difftest;
int difftest_init();
int difftest_step();
int difftest_state();
int init_nemuproxy(size_t);

#endif
