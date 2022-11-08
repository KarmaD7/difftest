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

/**
 * Headers for Verilog DPI-C difftest interface
 */

#ifndef __DT_INTERFACE_H__
#define __DT_INTERFACE_H__

#include "difftest.h"
#include "runahead.h"

// #ifdef __cplusplus
// extern "C" {
// #endif

#define DIFFTEST_DPIC_FUNC_NAME(name) \
  v_difftest_##name

#define DIFFTEST_DPIC_FUNC_DECL(name) \
  extern "C" void DIFFTEST_DPIC_FUNC_NAME(name)

#define DPIC_ARG_BIT  uint8_t
#define DPIC_ARG_BYTE uint8_t
#define DPIC_ARG_INT  uint32_t
#define DPIC_ARG_LONG uint64_t

// #define DPIC_ARG_BIT  svBit
// #define DPIC_ARG_BYTE char
// #define DPIC_ARG_INT  int32_t
// #define DPIC_ARG_LONG int64_t

// v_difftest_init
extern "C" int v_difftest_init();
extern "C" int v_difftest_step();

// v_difftest_step
// extern "C" int
// #define INTERFACE_STEP
//   DIFFTEST_DPIC_FUNC_DECL(step) (
//   )

// v_difftest_ArchEvent
#define INTERFACE_ARCH_EVENT             \
  DIFFTEST_DPIC_FUNC_DECL(ArchEvent) (   \
    DPIC_ARG_BYTE coreid,                \
    DPIC_ARG_INT  intrNo,                \
    DPIC_ARG_INT  cause,                 \
    DPIC_ARG_LONG exceptionPC,           \
    DPIC_ARG_INT  exceptionInst          \
  )

// v_difftest_BasicInstrCommit
#define INTERFACE_BASIC_INSTR_COMMIT     \
  DIFFTEST_DPIC_FUNC_DECL(BasicInstrCommit) ( \
    DPIC_ARG_BYTE coreid,                \
    DPIC_ARG_BYTE index,                 \
    DPIC_ARG_BIT  valid,                 \
    DPIC_ARG_BYTE special,               \
    DPIC_ARG_BIT  skip,                  \
    DPIC_ARG_BIT  isRVC,                 \
    DPIC_ARG_BIT  rfwen,                 \
    DPIC_ARG_BIT  fpwen,                 \
    DPIC_ARG_BYTE wpdest,                \
    DPIC_ARG_BYTE wdest                  \
  )

// v_difftest_InstrCommit
#define INTERFACE_INSTR_COMMIT           \
  DIFFTEST_DPIC_FUNC_DECL(InstrCommit) ( \
    DPIC_ARG_BYTE coreid,                \
    DPIC_ARG_BYTE index,                 \
    DPIC_ARG_BIT  valid,                 \
    DPIC_ARG_BYTE special,               \
    DPIC_ARG_BIT  skip,                  \
    DPIC_ARG_BIT  isRVC,                 \
    DPIC_ARG_BIT  rfwen,                 \
    DPIC_ARG_BIT  fpwen,                 \
    DPIC_ARG_INT  wpdest,                \
    DPIC_ARG_BYTE wdest,                 \
    DPIC_ARG_LONG pc,                    \
    DPIC_ARG_INT  instr,                 \
    DPIC_ARG_LONG wdata                  \
  )

// v_difftest_BasicTrapEvent
#define INTERFACE_BASIC_TRAP_EVENT       \
  DIFFTEST_DPIC_FUNC_DECL(BasicTrapEvent) (   \
    DPIC_ARG_BYTE coreid,                \
    DPIC_ARG_BIT  valid,                 \
    DPIC_ARG_LONG cycleCnt,              \
    DPIC_ARG_LONG instrCnt,              \
    DPIC_ARG_BIT  hasWFI                 \
  )

// v_difftest_TrapEvent
#define INTERFACE_TRAP_EVENT             \
  DIFFTEST_DPIC_FUNC_DECL(TrapEvent) (   \
    DPIC_ARG_BYTE coreid,                \
    DPIC_ARG_BIT  valid,                 \
    DPIC_ARG_LONG cycleCnt,              \
    DPIC_ARG_LONG instrCnt,              \
    DPIC_ARG_BIT  hasWFI,                \
    DPIC_ARG_BYTE code,                  \
    DPIC_ARG_LONG pc                     \
  )

// v_difftest_CSRState
#define INTERFACE_CSR_STATE              \
  DIFFTEST_DPIC_FUNC_DECL(CSRState) (    \
    DPIC_ARG_BYTE coreid,                \
    DPIC_ARG_BYTE priviledgeMode,        \
    DPIC_ARG_LONG mstatus,               \
    DPIC_ARG_LONG sstatus,               \
    DPIC_ARG_LONG mepc,                  \
    DPIC_ARG_LONG sepc,                  \
    DPIC_ARG_LONG mtval,                 \
    DPIC_ARG_LONG stval,                 \
    DPIC_ARG_LONG mtvec,                 \
    DPIC_ARG_LONG stvec,                 \
    DPIC_ARG_LONG mcause,                \
    DPIC_ARG_LONG scause,                \
    DPIC_ARG_LONG satp,                  \
    DPIC_ARG_LONG mip,                   \
    DPIC_ARG_LONG mie,                   \
    DPIC_ARG_LONG mscratch,              \
    DPIC_ARG_LONG sscratch,              \
    DPIC_ARG_LONG mideleg,               \
    DPIC_ARG_LONG medeleg                \
  )

// v_difftest_DebugMode
#define INTERFACE_DM_STATE               \
  DIFFTEST_DPIC_FUNC_DECL(DebugMode) (   \
    DPIC_ARG_BYTE coreid,                \
    DPIC_ARG_BIT  dMode,                 \
    DPIC_ARG_LONG dcsr,                  \
    DPIC_ARG_LONG dpc,                   \
    DPIC_ARG_LONG dscratch0,             \
    DPIC_ARG_LONG dscratch1              \
  )

// v_difftest_IntWriteback
#define INTERFACE_INT_WRITEBACK          \
  DIFFTEST_DPIC_FUNC_DECL(IntWriteback) (\
    DPIC_ARG_BYTE coreid,                \
    DPIC_ARG_BIT  valid,                 \
    DPIC_ARG_INT  dest,                  \
    DPIC_ARG_LONG data                   \
  )

// v_difftest_ArchIntRegState
#define INTERFACE_INT_REG_STATE          \
  DIFFTEST_DPIC_FUNC_DECL(ArchIntRegState) ( \
    DPIC_ARG_BYTE coreid,                \
    DPIC_ARG_LONG gpr_0,                 \
    DPIC_ARG_LONG gpr_1,                 \
    DPIC_ARG_LONG gpr_2,                 \
    DPIC_ARG_LONG gpr_3,                 \
    DPIC_ARG_LONG gpr_4,                 \
    DPIC_ARG_LONG gpr_5,                 \
    DPIC_ARG_LONG gpr_6,                 \
    DPIC_ARG_LONG gpr_7,                 \
    DPIC_ARG_LONG gpr_8,                 \
    DPIC_ARG_LONG gpr_9,                 \
    DPIC_ARG_LONG gpr_10,                \
    DPIC_ARG_LONG gpr_11,                \
    DPIC_ARG_LONG gpr_12,                \
    DPIC_ARG_LONG gpr_13,                \
    DPIC_ARG_LONG gpr_14,                \
    DPIC_ARG_LONG gpr_15,                \
    DPIC_ARG_LONG gpr_16,                \
    DPIC_ARG_LONG gpr_17,                \
    DPIC_ARG_LONG gpr_18,                \
    DPIC_ARG_LONG gpr_19,                \
    DPIC_ARG_LONG gpr_20,                \
    DPIC_ARG_LONG gpr_21,                \
    DPIC_ARG_LONG gpr_22,                \
    DPIC_ARG_LONG gpr_23,                \
    DPIC_ARG_LONG gpr_24,                \
    DPIC_ARG_LONG gpr_25,                \
    DPIC_ARG_LONG gpr_26,                \
    DPIC_ARG_LONG gpr_27,                \
    DPIC_ARG_LONG gpr_28,                \
    DPIC_ARG_LONG gpr_29,                \
    DPIC_ARG_LONG gpr_30,                \
    DPIC_ARG_LONG gpr_31                 \
  )

// v_difftest_StoreEvent
#define INTERFACE_STORE_EVENT            \
  DIFFTEST_DPIC_FUNC_DECL(StoreEvent) (  \
    DPIC_ARG_BYTE coreid,                \
    DPIC_ARG_BYTE index,                 \
    DPIC_ARG_BIT  valid,                 \
    DPIC_ARG_LONG storeAddr,             \
    DPIC_ARG_LONG storeData,             \
    DPIC_ARG_BYTE storeMask              \
  )

// v_difftest_PtwEvent
#define INTERFACE_PTW_EVENT              \
  DIFFTEST_DPIC_FUNC_DECL(PtwEvent) (    \
    DPIC_ARG_BYTE coreid,                \
    DPIC_ARG_BIT  resp,                  \
    DPIC_ARG_LONG addr,                  \
    DPIC_ARG_LONG data_0,                \
    DPIC_ARG_LONG data_1,                \
    DPIC_ARG_LONG data_2,                \
    DPIC_ARG_LONG data_3                 \
  )

INTERFACE_BASIC_INSTR_COMMIT;
INTERFACE_ARCH_EVENT;
INTERFACE_INSTR_COMMIT;
INTERFACE_BASIC_TRAP_EVENT;
INTERFACE_TRAP_EVENT;
INTERFACE_CSR_STATE;
INTERFACE_INT_WRITEBACK;
INTERFACE_INT_REG_STATE;
INTERFACE_STORE_EVENT;
INTERFACE_PTW_EVENT;

#endif
