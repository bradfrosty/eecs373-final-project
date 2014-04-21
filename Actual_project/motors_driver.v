module motor_driver_wrapper (/*** APB3 BUS INTERFACE ***/ 
input PCLK, // clock 
input PRESERN, // system reset 
input PSEL, // peripheral select 
input PENABLE, // distinguishes access phase 
output wire PREADY, // peripheral ready signal 
output wire PSLVERR, // error signal 
input PWRITE, // distinguishes read and write cycles 
input [31:0] PADDR, // I/O address 
input wire [31:0] PWDATA, // data from processor to I/O device (32 bits) 
output reg [31:0] PRDATA, // data to processor from I/O device (32-bits)

output wire [3:0] pulseX,
output wire [3:0] pulseY,
output reg [3:0] LED,
output wire [1:0] motionstateY
);

reg [1:0] DIRX;
reg [1:0] DIRY;

assign PREADY = 1;
assign PSLVERR = 0;

wire WRITE_EN;
assign WRITE_EN = (PSEL && PENABLE && PWRITE);

always@(posedge PCLK) begin
if(~PRESERN) begin
	DIRX = 2'b11;
	DIRY = 2'b11;
end

if (WRITE_EN && PADDR[3:2] == 2'b00) begin
	DIRX = PWDATA[1:0];
	if (PWDATA[1:0] == 2'b01)
		LED[1:0] = 2'b10;
	else if (PWDATA[1:0] == 2'b10)
		LED[1:0] = 2'b01;
	else if (PWDATA[1:0] == 2'b11)
		LED[1:0] = 2'b11;
	else
		LED[1:0] = 2'b00;
end
if (WRITE_EN && PADDR[3:2] == 2'b01) begin
	DIRY = PWDATA[1:0];
	if (PWDATA[1:0] == 2'b01)
		LED[3:2] = 2'b10;
	else if (PWDATA[1:0] == 2'b10)
		LED[3:2] = 2'b01;
	else if (PWDATA[1:0] == 2'b11)
		LED[3:2] = 2'b11;
	else
		LED[3:2] = 2'b00;
end
end


motors_driver d1(.clock(PCLK), .reset(PRESERN), .dirX(DIRX), .dirY(DIRY), .pulseX(pulseX), .pulseY(pulseY), .motionstateY(motionstateY));

endmodule





module motors_driver(clock, reset, dirX, dirY, pulseX, pulseY,motionstateY);


input clock, reset;
//input tableedgeX, tableedgeY;
input [1:0] dirX, dirY;

wire [1:0] dirXtest;
assign dirXtest = dirX;

parameter STOP = 2'b00;
parameter ACCEL = 2'b01;
parameter RUN = 2'b10;
parameter DECEL = 2'b11;

//these sizes are pretty arbitrary
//reg [64:0] position;
//reg [31:0] speed;
//reg [15:0] acceleration;

output reg[3:0] pulseX;
output reg [3:0] pulseY;

reg pulseEnX;
reg pulseEnY;
reg pulseClkX;
reg pulseClkY;

reg [1:0] motionstateX = STOP;
output reg [1:0] motionstateY = STOP;
reg [1:0] nextmotionstateX, nextmotionstateY;
reg [1:0] prevdirX, prevdirY;



//BEST PERFORMANCE
//ACCEL = 13'b1111111111110
//MAXSPEEDX = 16'b1000010110000000
//MINSPEEDX = 18'b100001011000111111

`define CCW  2'b01
`define CW 2'b10
`define NONE 2'b00
`define ACCELX 9'b111110100
`define ACCELY 8'b11111010
`define MAXSPEEDX 15'b101110111000000   //minimum number of clock ticks (17094) between steps for max speed (300 mm/s)
`define MINSPEEDX 17'b11000011010100000 //maximum number of clock ticks (25641	025) between steps fo starting speed (1 mm/s)
`define MAXSPEEDY 16'b1000010011010000
`define MINSPEEDY 17'b11000011010100000		//1000000000000000001

reg [25:0] speedX = `MINSPEEDX;
reg [25:0] speedY = `MINSPEEDY;

//the output of these states is defined below
parameter OFF = 3'b000;			//no motor pulse
parameter STEP1 = 3'b001;		//wire 0 and wire 3
parameter STEP2 = 3'b010;		//wire 0 and wire 2
parameter STEP3 = 3'b011;		//wire 1 and wire 2
parameter STEP4 = 3'b100;		//wire 1 and wire 3



reg [28:0] countX, countY;

//count on both clock edges to generate clock within step delay (without redoing calculations
always @(posedge clock)
begin
	if(countX >= speedX) begin
		pulseClkX <= ~pulseClkX;
		countX <= 0;
	end
	else begin
		countX <= countX + 2;
	end		
end

	
always @(posedge clock)
begin
	if(countY >= speedY) begin
		pulseClkY <= ~pulseClkY;
		countY <= 0;
	end
	else begin
		countY <= countY + 2;
	end
end



//next state logic for motors driving sequence
//this FSM produces the signals sent to the wires of the steppers
reg [2:0] stateX;
reg [2:0] stateY;
reg [2:0] nextstateX, nextstateY;
always@ *
begin
	case (stateX)
	OFF:	begin
			nextstateX = STEP1;
			end
	STEP1: 	begin
			if(dirX == `CW || ((dirX == `CCW || dirX == `NONE) && motionstateX == DECEL)) begin
			nextstateX = STEP2;
			end
			if(dirX == `CCW || ((dirX == `CW || dirX == `NONE) && motionstateX == DECEL)) begin
			nextstateX = STEP4;
			end
			/*else begin
			nextstateX = STEP4;
			end*/
		end
	STEP2: 	begin
			if(dirX == `CW || ((dirX == `CCW || dirX == `NONE) && motionstateX == DECEL)) begin
			nextstateX = STEP3;
			end
			if(dirX == `CCW || ((dirX == `CW || dirX == `NONE) && motionstateX == DECEL)) begin
			nextstateX = STEP1;
			end
			/*else begin
			nextstateX = STEP1;
			end*/
		end
	STEP3: 	begin
			if(dirX == `CW || ((dirX == `CCW || dirX == `NONE) && motionstateX == DECEL)) begin
			nextstateX = STEP4;
			end
			if(dirX == `CCW || ((dirX == `CW || dirX == `NONE) && motionstateX == DECEL))begin
			nextstateX = STEP2;
			end
			/*else begin
			nextstateX = STEP2;
			end*/
		end
	STEP4: 	begin
			if(dirX == `CW || ((dirX == `CCW || dirX == `NONE) && motionstateX == DECEL)) begin
			nextstateX = STEP1;
			end
			if(dirX == `CCW || ((dirX == `CW || dirX == `NONE) && motionstateX == DECEL)) begin
			nextstateX = STEP3;
			end
			/*else begin
			nextstateX = STEP3;
			end*/
		end
	default: nextstateX = OFF;
	endcase

	case (stateY)
	OFF:	begin
			nextstateY = STEP1;
			end
	STEP1: 	begin
			if(dirY == `CW || ((dirY == `CCW || dirY == `NONE) && motionstateY == DECEL)) begin
			nextstateY = STEP2;
			end
			if(dirY == `CCW || ((dirY == `CW || dirY == `NONE) && motionstateY == DECEL)) begin
			nextstateY = STEP4;
			end
			/*else begin
			nextstateY = STEP4;
			end*/
		end
	STEP2: 	begin
			if(dirY == `CW || ((dirY == `CCW || dirY == `NONE) && motionstateY == DECEL)) begin
			nextstateY = STEP3;
			end
			if(dirY == `CCW || ((dirY == `CW || dirY == `NONE) && motionstateY == DECEL)) begin
			nextstateY = STEP1;
			end
			/*else begin
			nextstateY = STEP1;
			end*/
		end
	STEP3: 	begin
			if(dirY == `CW || ((dirY == `CCW || dirY == `NONE) && motionstateY == DECEL)) begin
			nextstateY = STEP4;
			end
			if(dirY == `CCW || ((dirY == `CW || dirY == `NONE) && motionstateY == DECEL)) begin
			nextstateY = STEP2;
			end
			/*else begin
			nextstateY = STEP2;
			end*/
		end
	STEP4: 	begin
			if(dirY == `CW || ((dirY == `CCW || dirY == `NONE) && motionstateY == DECEL)) begin
			nextstateY = STEP1;
			end
			if(dirY == `CCW || ((dirY == `CW || dirY == `NONE) && motionstateY == DECEL)) begin
			nextstateY = STEP3;
			end
			/*else begin
			nextstateY = STEP3;
			end*/
		end
	default: nextstateY = OFF;
	endcase
end


//The motors are clocked based on step delay, a counter (called pulseClk) we are continuously updating
//the pulseClk signal updates after a certain amount of cycles which is calculated in the motion control FSM
always@ (posedge pulseClkX)
begin
	if(~pulseEnX) begin			//pulseEnX is updated in motion control FSM, specifically the STOP state
		stateX <= OFF;
	end
	else begin
		stateX <= nextstateX;
	end
end

always@ (posedge pulseClkY)
begin
	if(~pulseEnY) begin			//pulseEnY is updated in motion control FSM, specifically the STOP state
		stateY <= OFF;
	end
	else begin
		stateY <= nextstateY;
	end
end

//output logic for motor driver FSM
always@ *
begin
	case(stateX)
	OFF: 	begin
			pulseX = 4'b0000;
			end
	STEP1:	begin
			pulseX = 4'b1001;
			end
	STEP2:	begin
			pulseX = 4'b0101;
			end
	STEP3:	begin
			pulseX = 4'b0110;
			end
	STEP4:	begin
			pulseX = 4'b1010;
			end
	//default: pulseX = 0;
	endcase

	case(stateY)
	OFF: 	begin
			pulseY = 4'b0000;
			end
	STEP1:	begin
			pulseY = 4'b1001;
			end
	STEP2:	begin
			pulseY = 4'b0101;
			end
	STEP3:	begin
			pulseY = 4'b0110;
			end
	STEP4:	begin
			pulseY = 4'b1010;
			end
	//default: pulseY = 0;
	endcase
end






//motion control FSM. each state determines how the speed is being controlled
//next state logic for motion controller FSM
always @*
begin
	case(motionstateX)
	STOP:	if(dirX == `CW || dirX == `CCW) begin
				nextmotionstateX = ACCEL;
			end
			else begin
				nextmotionstateX = STOP;
			end
	ACCEL:	if(speedX <= `MAXSPEEDX ) begin
				nextmotionstateX = RUN;
			end
			else if(prevdirX != dirX) begin
				nextmotionstateX = DECEL;
			end
			//else if(tableedgeX) begin
			//	nextmotionstateX = STOP;
		//	end
			else begin
				nextmotionstateX = ACCEL;
			end
	RUN:	if(prevdirX != dirX) begin
				nextmotionstateX = DECEL;
			end
			//else if(tableedgeX) begin
			//	nextmotionstateX = STOP;
			//end
			else begin
				nextmotionstateX = RUN;
			end
	DECEL:	if((speedX >= `MINSPEEDX)/* || tableedgeX*/) begin
				nextmotionstateX = STOP;
			end
			else begin
				nextmotionstateX = DECEL;
			end
	default: nextmotionstateX = STOP;
	endcase
	
	case(motionstateY)
	STOP:	if(dirY == `CW || dirY == `CCW) begin
				nextmotionstateY = ACCEL;
			end
			else begin
				nextmotionstateY = STOP;
			end
	ACCEL:	if(speedY <= `MAXSPEEDY) begin
				nextmotionstateY = RUN;
			end
			else if(prevdirY != dirY) begin
				nextmotionstateY = DECEL;
			end
			//else if(tableedgeY) begin
			//	nextmotionstateY = STOP;
			//end
			else begin
				nextmotionstateY = ACCEL;
			end
	RUN:	if(prevdirY != dirY) begin
				nextmotionstateY = DECEL;
			end
			//else if(tableedgeY) begin
		//		nextmotionstateY = STOP;
			//end
			else begin
				nextmotionstateY = RUN;
			end
	DECEL:	if((speedY >= `MINSPEEDY)/* || tableedgeY*/) begin
				nextmotionstateY = STOP;
			end
			else begin
				nextmotionstateY = DECEL;
			end
	default: nextmotionstateY = STOP;
	endcase
end

//state memory for motion controller FSM
always @(posedge pulseClkX)
begin
	if(~reset) begin
		motionstateX <= STOP;
	end
	else begin
		motionstateX <= nextmotionstateX;
	end
	case(motionstateX)

	STOP: 	speedX <= `MINSPEEDX;
	ACCEL: 	speedX <= speedX - `ACCELX;
	DECEL: 	speedX <= speedX + `ACCELX;
	RUN:	speedX <= `MAXSPEEDX;
	endcase

	prevdirX <= dirX;

end
	
always @(posedge pulseClkY)
begin
	if(~reset) begin
		motionstateY <= STOP;
	end
	else begin
		motionstateY <= nextmotionstateY;
	end

	case(motionstateY)
	STOP: 	speedY <= `MINSPEEDY;
	ACCEL: 	speedY <= speedY - `ACCELY;
	DECEL: 	speedY <= speedY + `ACCELY;
	RUN:	speedY <= `MAXSPEEDY;
	endcase

	prevdirY <= dirY;
end

//output logic for motion control FSM
always @ (posedge clock)
begin
	case(motionstateX)
	STOP:	begin
			pulseEnX <= 0;
			end
	ACCEL:	begin
			pulseEnX <= 1;
			end
	RUN:	begin
			pulseEnX <= 1;
			end
	DECEL:	begin
			pulseEnX <= 1;
			end
	//default: pulseEnX = 0;
	endcase

	case(motionstateY)
	STOP:	begin
			pulseEnY <= 0;
			end
	ACCEL:	begin
			pulseEnY <= 1;
			end
	RUN:	begin
			pulseEnY <= 1;
			end
	DECEL:	begin
			pulseEnY <= 1;
			end
	//default: pulseEnY = 0;
	endcase
end
 
endmodule
