module difftest_reg (
  input wire clk,
  input wire rst,
  input wire[31:0] regs[31:0]
);
  
endmodule

module difftest_csr (

);

// todo

endmodule

module difftest_exception (

);

endmodule

module difftest_instr_wb (
  input wire clk,
  input wire rst,

  input wire reg_we,
  input wire[4:0] reg_addr,
  input wire[31:0] reg_val 
);

endmodule