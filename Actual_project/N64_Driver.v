module N64_Driver(
/*** APB3 BUS INTERFACE ***/ 
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

/*** I/O ports ***/
inout wire N64_data,
output wire FABINT,
output wire test
);

assign PREADY = 1'b1;
assign PSLVERR = 1'b0;


reg [31:0] dataReg = 0;
reg [8:0] commandReg = 0;
reg N64_enable = 0;


reg N64_reg = 0;
reg [31:0] N64_reg_out = 0;
reg [31:0] bits_recieved = 0;
reg [31:0] bits_sent = 0;
reg sending_data = 0;
reg recieving_data = 0;

reg recieve_counter_reset = 0;
reg [31:0] recieve_counter = 0;

wire WRITE_EN;
assign WRITE_EN = (PENABLE && PWRITE && PSEL);

parameter interupt_period = 1000000000;
//FABINT TIMER
reg [32:0] interupt_timer = 0;
reg interupt_timer_reset = 0;

always@* begin
if (WRITE_EN && (PADDR[3:2] == 2'b01))
	N64_enable <= PWDATA[0];
end


always@(posedge PCLK) begin
if (interupt_timer_reset) begin
	interupt_timer <= 0;
	interupt_timer_reset <= 0;
end
else if (N64_enable) begin
	interupt_timer <= interupt_timer + 1;
	if (interupt_timer == interupt_period) 
		interupt_timer_reset <= 1;
end
end

assign FABINT = (interupt_timer == interupt_period) ? 1'b1 : 1'b0;
assign test = (interupt_timer == interupt_period) ? 1'b1 : 1'b0;





//recieving timer
always@(posedge PCLK) begin
if(recieve_counter_reset)
	recieve_counter <= 0;
else if (recieving_data)
	recieve_counter <= recieve_counter + 1;
end

//get command from user

reg send_counter_reset = 0;
reg [31:0] send_counter = 0;
//sending timer
always@(posedge PCLK) begin
if(send_counter_reset)
	send_counter <= 0;
else if (sending_data)
	send_counter <= send_counter + 1;
end


//sync data
reg [2:0] syncer = 3'b111;
always@(posedge PCLK) begin
if(N64_data) begin
    syncer[0] <= 1'b1;
    syncer[1] <= 1'b1;
    syncer[2] <= 1'b1;
end
else begin
    syncer[0] <= 1'b0;
    syncer[1] <= syncer[0];
    syncer[2] <= syncer[1];
end
end

//start timer
always@(posedge PCLK) begin
if (~PRESERN) begin
	recieving_data <= 0;
	bits_recieved <= 0;
end
else begin
	if (syncer[1] == 1'b0 && syncer[2] == 1'b1 && ~sending_data) begin
		recieve_counter_reset <= 1;
		if (~recieving_data) begin
			recieving_data <= 1;
			bits_recieved <= 0;
		end
	end
	else
		recieve_counter_reset <= 0;

	if (bits_recieved >= 33 && recieving_data) begin
		PRDATA <= N64_reg_out;
		recieving_data <= 0;
		bits_recieved <= 0;
	end
end


//grab data

if (recieving_data) begin
	if (recieve_counter == 200)
		N64_reg_out <= (N64_reg_out << 1);
	else if (recieve_counter == 201) begin
		if (bits_recieved != 33)
			N64_reg_out[0] <= N64_data;
		bits_recieved <= bits_recieved + 1;
	end
end








if (~PRESERN)
	sending_data <= 0;
else begin
	if (WRITE_EN && PADDR[3:2] == 2'b00)
		commandReg <= PWDATA[8:0];
	if (WRITE_EN && ~sending_data) begin
		sending_data <= 1;
		recieving_data <= 0;
		send_counter_reset <= 1;
		bits_sent <= 0;
		PRDATA <= N64_reg_out;
	end

	
	if (sending_data) begin
		if (send_counter < 100)
			N64_reg <= 0;
		else if (send_counter >= 100 && send_counter < 300) begin
			if (commandReg[8-bits_sent])
				N64_reg <= 1;
			else
				N64_reg <= 0;
		end
		else 
			N64_reg <= 1;

		// up bits_sent at 300
		if (send_counter == 300) begin
			if (bits_sent == 8) begin
				sending_data <= 0;
				recieving_data <= 1;
			end
			bits_sent <= bits_sent + 1;
		end

		if (send_counter == 400)
			send_counter_reset <= 1;
		else
			send_counter_reset <= 0;
		
	end

end
end

assign N64_data = (N64_reg) ? 1'bz : 1'b0;
endmodule

