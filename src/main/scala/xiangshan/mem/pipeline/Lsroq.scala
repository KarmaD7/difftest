package xiangshan.mem.pipeline

import chisel3._
import chisel3.util._
import xiangshan._
import xiangshan.utils._
import chisel3.util.experimental.BoringUtils
import xiangshan.backend.decode.XSTrap
import xiangshan.mem._
import xiangshan.mem.cache._
import bus.simplebus._

class LsRoqEntry extends XSBundle {
  val paddr = UInt(PAddrBits.W)
  val op = UInt(6.W)
  val wmask = UInt(8.W)
  val data = UInt(XLEN.W)
  val exception = UInt(8.W)
  val miss = Bool()
  val mmio = Bool()
  val store = Bool()
}

// Load/Store Roq (Moq) for XiangShan Out of Order LSU
class LsRoq(implicit val p: XSConfig) extends XSModule with HasMEMConst with NeedImpl{
  val io = IO(new Bundle() {
    val dp1Req = Vec(RenameWidth, Flipped(DecoupledIO(new MicroOp)))
    val moqIdxs = Output(Vec(RenameWidth, UInt(MoqIdxWidth.W)))
    val brqRedirect = Input(Valid(new Redirect))
    val loadIn = Vec(LoadPipelineWidth, Flipped(Valid(new LsPipelineBundle)))
    val storeIn = Vec(StorePipelineWidth, Flipped(Valid(new LsPipelineBundle)))
    val out = Vec(2, DecoupledIO(new ExuOutput)) // writeback store
    val commits = Vec(CommitWidth, Valid(new RoqCommit))
    val scommit = Input(UInt(3.W))
    val forward = Vec(LoadPipelineWidth, Flipped(new LoadForwardQueryIO))
    // val rollback = TODO
    // val miss = new SimpleBusUC(addrBits = VAddrBits, userBits = (new DcacheUserBundle).getWidth)
  })

  val uop = Mem(LSRoqSize, new MicroOp)
  val data = Mem(LSRoqSize, new LsRoqEntry)
  val valid = RegInit(VecInit(List.fill(MoqSize)(false.B)))
  val writebacked = RegInit(VecInit(List.fill(MoqSize)(false.B)))
  val store = Reg(Vec(MoqSize, Bool()))
  val miss = Reg(Vec(MoqSize, Bool()))
  
  val ringBufferHeadExtended = RegInit(0.U(MoqIdxWidth.W))
  val ringBufferTailExtended = RegInit(0.U(MoqIdxWidth.W))
  val ringBufferHead = ringBufferHeadExtended(InnerRoqIdxWidth-1,0)
  val ringBufferTail = ringBufferTailExtended(InnerRoqIdxWidth-1,0)
  val ringBufferEmpty = ringBufferHead === ringBufferTail && ringBufferHeadExtended(InnerMoqIdxWidth)===ringBufferTailExtended(InnerMoqIdxWidth)
  val ringBufferFull = ringBufferHead === ringBufferTail && ringBufferHeadExtended(InnerMoqIdxWidth)=/=ringBufferTailExtended(InnerMoqIdxWidth)
  val ringBufferAllowin = !ringBufferFull 

  // Enqueue at dispatch
  val validDispatch = VecInit((0 until RenameWidth).map(io.dp1Req(_).valid)).asUInt
  XSDebug("(ready, valid): ")
  for (i <- 0 until RenameWidth) {
    val offset = if(i==0) 0.U else PopCount(validDispatch(i-1,0))
    when(io.dp1Req(i).fire()){
      uop(ringBufferHead+offset) := io.dp1Req(i).bits
      valid(ringBufferHead+offset) := true.B
      writebacked(ringBufferHead+offset) := false.B
      store(ringBufferHead+offset) := false.B
    }
    io.dp1Req(i).ready := ringBufferAllowin && !valid(ringBufferHead+offset)
    io.moqIdxs(i) := ringBufferHeadExtended+offset
    XSDebug(false, true.B, "(%d, %d) ", io.dp1Req(i).ready, io.dp1Req(i).valid)
  }
  XSDebug(false, true.B, "\n")

  val firedDispatch = VecInit((0 until CommitWidth).map(io.dp1Req(_).fire())).asUInt
  when(firedDispatch.orR){
    ringBufferHeadExtended := ringBufferHeadExtended + PopCount(firedDispatch)
    XSInfo("dispatched %d insts to moq\n", PopCount(firedDispatch))
  }

  // TODO: dispatch ready depends on roq ready: solve in dispatch 1

  // misprediction recovery

  (0 until MoqSize).map(i => {
    when(uop(i).brTag.needFlush(io.brqRedirect) && valid(i)){
      valid(i) := false.B
    }
  })

  // writeback load
  (0 until LoadPipelineWidth).map(i => {
    when(io.loadIn(i).fire()){
      // when(io.loadIn(i).miss){
      //   writebacked(io.loadIn(i).bits.moqIdx) := true.B
      //   data(io.loadIn(i).bits.uop.moqIdx).paddr := io.loadIn(i).bits.paddr
      //   data(io.loadIn(i).bits.uop.moqIdx).wmask := io.loadIn(i).bits.mask
      //   data(io.loadIn(i).bits.uop.moqIdx).data := io.loadIn(i).bits.data
      //   data(io.loadIn(i).bits.uop.moqIdx).miss := true.B
      //   data(io.loadIn(i).bits.uop.moqIdx).mmio := io.loadIn(i).bits.mmio
      //   data(io.loadIn(i).bits.uop.moqIdx).store := false.B
      //   XSInfo("load miss write to lsroq pc 0x%x vaddr %x paddr %x miss %x mmio %x roll %x\n", 
      //     io.loadIn(i).bits.uop.cf.pc, 
      //     io.loadIn(i).bits.vaddr, 
      //     io.loadIn(i).bits.paddr, 
      //     io.loadIn(i).bits.miss, 
      //     io.loadIn(i).bits.mmio, 
      //     io.loadIn(i).bits.rollback
      //   )
      // }.otherwise{
        assert(!io.loadIn(i).bits.miss)
        valid(io.loadIn(i).bits.uop.moqIdx) := false.B // do not writeback to lsroq
        XSInfo(io.loadIn(i).valid, "load hit write to cbd idx %d pc 0x%x vaddr %x paddr %x data %x miss %x mmio %x roll %x\n", 
          io.loadIn(i).bits.uop.moqIdx, 
          io.loadIn(i).bits.uop.cf.pc, 
          io.loadIn(i).bits.vaddr, 
          io.loadIn(i).bits.paddr, 
          io.loadIn(i).bits.data, 
          io.loadIn(i).bits.miss, 
          io.loadIn(i).bits.mmio, 
          io.loadIn(i).bits.rollback
        )
      // }
    }
  })

  // writeback store
  (0 until StorePipelineWidth).map(i => {
    when(io.storeIn(i).fire()){
      writebacked(io.storeIn(i).bits.uop.moqIdx) := true.B
      data(io.storeIn(i).bits.uop.moqIdx).paddr := io.storeIn(i).bits.paddr
      data(io.storeIn(i).bits.uop.moqIdx).wmask := io.storeIn(i).bits.mask
      data(io.storeIn(i).bits.uop.moqIdx).data := io.storeIn(i).bits.data
      data(io.storeIn(i).bits.uop.moqIdx).miss := io.storeIn(i).bits.miss
      data(io.storeIn(i).bits.uop.moqIdx).mmio := io.storeIn(i).bits.mmio
      data(io.storeIn(i).bits.uop.moqIdx).store := true.B
      XSInfo("store write to lsroq idx %d pc 0x%x vaddr %x paddr %x miss %x mmio %x roll %x\n", 
        io.storeIn(i).bits.uop.moqIdx, 
        io.storeIn(i).bits.uop.cf.pc, 
        io.storeIn(i).bits.vaddr, 
        io.storeIn(i).bits.paddr, 
        io.storeIn(i).bits.miss, 
        io.storeIn(i).bits.mmio, 
        io.storeIn(i).bits.rollback
      )
    }
  })

  // commit store to cdb
  // TODO: how to select 2 from 64?
  // just randomly pick a store, write it back to cdb
  // TODO: scommit
  (0 until StorePipelineWidth).map(i => {
      io.out(i) := DontCare
      io.out(i).valid := false.B
  })

  (0 until MoqSize).map(i => {
    when(valid(i) && writebacked(i) && store(i)){
      io.out(0).bits.uop := uop(i)
      io.out(0).bits.data := data(i).data
      io.out(0).bits.redirectValid := false.B
      io.out(0).bits.debug.isMMIO := data(i).mmio
      valid(i) := false.B
    }
  })

  // cache miss request
  // io.miss := DontCare
  // val missRefillSelVec = VecInit(
  //   (0 until MoqSize).map(i => valid(i) && writebacked(i) && miss(i))
  // )
  // val missRefillSel = OHToUInt(missRefillSelVec.asUInt)
  // io.miss.req.valid := missRefillSelVec.orR
  // io.miss.req.bits.addr := data(missRefillSel).paddr
  // when(io.fire()){
  //   writebacked(missRefillSel) := false.B
  //   miss(missRefillSel) := false.B
  //   // TODO: re-exec missed inst
  // }

  // load forward query

  // TODO
  // def needForward(taddr: UInt, tmask: UInt, tmoqIdx: UInt, saddr: UInt, smask: UInt, smoqIdx: UInt) = {
  //   taddr(PAddrBits-1, 3) === saddr(PAddrBits-1, 3) && 
  //   (tmask & smask).orR && 
  //   tmoqIdx smoqIdx && 
  //   valid(i) &&
  //   writebacked(i) &&
  //   store(i)
  // }

  // (0 until LoadPipelineWidth).map(i => {
  //   // val forward = Vec(LoadPipelineWidth, Flipped(new LoadForwardQueryIO))
  //   io.forward(i).paddr
  //   io.forward(i).mask
  //   io.forward(i).moqIdx
  //   io.forward(i).pc

  //   io.forward(i).forwardData = forwardData
  //   io.forward(i).forwardMask = forwardMask
  // })

  // store backward query and rollback

  // val rollback = 
  (0 until StorePipelineWidth).map(i => {
    when(io.storeIn(i).valid){

    }
  })

}
