//Daniel McGarrity, djm10030, djm10030@nyu.edu
//Melissa Kim, mk5015, mk5015@nyu.edu

#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type = false;
    bool        rd_mem = false;
    bool        wrt_mem = false;
    bool        alu_op = true;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable = false;
    bool        nop;
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem = false;
    bool        wrt_mem = false;
    bool        wrt_enable = false;
    bool        nop;
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable = false;
    bool        nop;
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
public:
    bitset<32> Reg_data;
    RF()
    {
        Registers.resize(32);
        Registers[0] = bitset<32>(0);
    }

    bitset<32> readRF(bitset<5> Reg_addr)
    {
        Reg_data = Registers[Reg_addr.to_ulong()];
        return Reg_data;
    }

    void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
    {
        Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
    }

    void outputRF()
    {
        ofstream rfout;
        rfout.open("RFresult.txt", std::ios_base::app);
        if (rfout.is_open())
        {
            rfout << "State of RF:\t" << endl;
            for (int j = 0; j < 32; j++)
            {
                rfout << Registers[j] << endl;
            }
        }
        else cout << "Unable to open file";
        rfout.close();
    }

private:
    vector<bitset<32> >Registers;
};

class INSMem
{
public:
    bitset<32> Instruction;
    INSMem()
    {
        IMem.resize(MemSize);
        ifstream imem;
        string line;
        int i = 0;
        imem.open("imem.txt");
        if (imem.is_open())
        {
            while (getline(imem, line))
            {
                IMem[i] = bitset<8>(line);
                i++;
            }
        }
        else cout << "Unable to open file";
        imem.close();
    }

    bitset<32> readInstr(bitset<32> ReadAddress)
    {
        string insmem;
        insmem.append(IMem[ReadAddress.to_ulong()].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 1].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 2].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 3].to_string());
        Instruction = bitset<32>(insmem);		//read instruction memory
        return Instruction;
    }

private:
    vector<bitset<8> > IMem;
};

class DataMem
{
public:
    bitset<32> ReadData;
    DataMem()
    {
        DMem.resize(MemSize);
        ifstream dmem;
        string line;
        int i = 0;
        dmem.open("dmem.txt");
        if (dmem.is_open())
        {
            while (getline(dmem, line))
            {
                DMem[i] = bitset<8>(line);
                i++;
            }
        }
        else cout << "Unable to open file";
        dmem.close();
    }

    bitset<32> readDataMem(bitset<32> Address)
    {
        string datamem;
        datamem.append(DMem[Address.to_ulong()].to_string());
        datamem.append(DMem[Address.to_ulong() + 1].to_string());
        datamem.append(DMem[Address.to_ulong() + 2].to_string());
        datamem.append(DMem[Address.to_ulong() + 3].to_string());
        ReadData = bitset<32>(datamem);		//read data memory
        return ReadData;
    }

    void writeDataMem(bitset<32> Address, bitset<32> WriteData)
    {
        DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
        DMem[Address.to_ulong() + 1] = bitset<8>(WriteData.to_string().substr(8, 8));
        DMem[Address.to_ulong() + 2] = bitset<8>(WriteData.to_string().substr(16, 8));
        DMem[Address.to_ulong() + 3] = bitset<8>(WriteData.to_string().substr(24, 8));
    }

    void outputDataMem()
    {
        ofstream dmemout;
        dmemout.open("dmemresult.txt");
        if (dmemout.is_open())
        {
            for (int j = 0; j < 1000; j++)
            {
                dmemout << DMem[j] << endl;
            }

        }
        else cout << "Unable to open file";
        dmemout.close();
    }

private:
    vector<bitset<8> > DMem;
};

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate << "State after executing cycle:\t" << cycle << endl;

        printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
        printstate << "IF.nop:\t" << state.IF.nop << endl;

        printstate << "ID.Instr:\t" << state.ID.Instr << endl;
        printstate << "ID.nop:\t" << state.ID.nop << endl;

        printstate << "EX.Read_data1:\t" << state.EX.Read_data1 << endl;
        printstate << "EX.Read_data2:\t" << state.EX.Read_data2 << endl;
        printstate << "EX.Imm:\t" << state.EX.Imm << endl;
        printstate << "EX.Rs:\t" << state.EX.Rs << endl;
        printstate << "EX.Rt:\t" << state.EX.Rt << endl;
        printstate << "EX.Wrt_reg_addr:\t" << state.EX.Wrt_reg_addr << endl;
        printstate << "EX.is_I_type:\t" << state.EX.is_I_type << endl;
        printstate << "EX.rd_mem:\t" << state.EX.rd_mem << endl;
        printstate << "EX.wrt_mem:\t" << state.EX.wrt_mem << endl;
        printstate << "EX.alu_op:\t" << state.EX.alu_op << endl;
        printstate << "EX.wrt_enable:\t" << state.EX.wrt_enable << endl;
        printstate << "EX.nop:\t" << state.EX.nop << endl;

        printstate << "MEM.ALUresult:\t" << state.MEM.ALUresult << endl;
        printstate << "MEM.Store_data:\t" << state.MEM.Store_data << endl;
        printstate << "MEM.Rs:\t" << state.MEM.Rs << endl;
        printstate << "MEM.Rt:\t" << state.MEM.Rt << endl;
        printstate << "MEM.Wrt_reg_addr:\t" << state.MEM.Wrt_reg_addr << endl;
        printstate << "MEM.rd_mem:\t" << state.MEM.rd_mem << endl;
        printstate << "MEM.wrt_mem:\t" << state.MEM.wrt_mem << endl;
        printstate << "MEM.wrt_enable:\t" << state.MEM.wrt_enable << endl;
        printstate << "MEM.nop:\t" << state.MEM.nop << endl;

        printstate << "WB.Wrt_data:\t" << state.WB.Wrt_data << endl;
        printstate << "WB.Rs:\t" << state.WB.Rs << endl;
        printstate << "WB.Rt:\t" << state.WB.Rt << endl;
        printstate << "WB.Wrt_reg_addr:\t" << state.WB.Wrt_reg_addr << endl;
        printstate << "WB.wrt_enable:\t" << state.WB.wrt_enable << endl;
        printstate << "WB.nop:\t" << state.WB.nop << endl;
    }
    else cout << "Unable to open file";
    printstate.close();
}


int main()
{

    bitset<32> branch_address;

    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
    stateStruct state, newState;

    newState.IF.nop = 0, newState.ID.nop = 1, newState.EX.nop = 1, newState.MEM.nop = 1, newState.WB.nop = 1;
    state.IF.nop = 0, state.ID.nop = 1, state.EX.nop = 1, state.MEM.nop = 1, state.WB.nop = 1;


    int cycle = 0;

    while (1) {
        /* --------------------- WB stage --------------------- */


        if (!state.WB.nop) {
            if (state.WB.wrt_enable == 1)// load
            {
                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
            }
        }

        newState.WB.nop = state.MEM.nop;

        /* --------------------- MEM stage --------------------- */


        if (!newState.MEM.nop) {


            if (state.MEM.wrt_mem == 1) { //store; write data to mem
                //cout << state.MEM.Store_data << endl;
                //cout << state.MEM.ALUresult << endl;
                myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
                newState.WB.Wrt_data = state.MEM.Store_data;
                //cout << "WE MADE IT TO WRITE DATA MEM" << endl;
                //cout << "address: " << state.MEM.ALUresult << endl;
                //cout << "data: " << state.MEM.Store_data << endl;
            }
            if (state.MEM.rd_mem == 1) {//load; read data from mem
                newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
            }
            if (state.MEM.rd_mem == 0 && state.MEM.wrt_mem == 0) {
                newState.WB.Wrt_data = state.MEM.ALUresult;
            }

            //update newState values
            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            newState.WB.wrt_enable = state.MEM.wrt_enable;
            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Rt = state.MEM.Rt;

        }
            
        
        newState.MEM.nop = state.EX.nop;

       
        /* --------------------- EX stage --------------------- */
        bitset<32> value2;
        bitset<32> value1;
        

        if (!newState.EX.nop) {

            bitset<32> value1 = myRF.readRF(state.EX.Rs);
            bitset<32> value2 = myRF.readRF(state.EX.Rt);

            //Forwarding 
            if (state.EX.wrt_enable == 1 && state.WB.wrt_enable == 1){
                if (state.WB.Wrt_reg_addr == state.EX.Rs && state.WB.nop == 0){ //MEM-EX
                    value1 = state.WB.Wrt_data;
                    //cout << "MEM to EX1\n";
                }
                if (state.WB.Wrt_reg_addr == state.EX.Rt && state.WB.nop == 0){ //MEM-EX
                    value2 = state.WB.Wrt_data;
                    //cout << "MEM to EX2\n";

                }
                if (state.MEM.Wrt_reg_addr == state.EX.Rs && state.MEM.nop == 0){ //EX-EX
                    value1 = state.MEM.ALUresult;
                    //state.EX.Read_data1 = state.MEM.ALUresult;
                    //cout << "EX to EX1\n";

                }
                if (state.MEM.Wrt_reg_addr == state.EX.Rt && state.MEM.nop == 0) { //EX-EX
                    value2 = state.MEM.ALUresult;
                    //state.EX.Read_data2 = state.MEM.ALUresult;
                    //cout << "EX to EX2\n";
                }
            }

            //fwd if sw uses address prev instruction writes to
            if (state.EX.is_I_type == 1 && state.EX.wrt_mem == 1 && state.MEM.wrt_enable == 1) {
                if (state.MEM.Wrt_reg_addr == state.EX.Rs) {
                    //cout << "sw rs == addu rd \n";
        
                    value1 = state.MEM.ALUresult;
                }
            }

            //fwd if sw uses address 2 prev instruction writes to
            if (state.EX.is_I_type == 1 && state.EX.wrt_mem == 1 && state.WB.wrt_enable == 1) {
                if (state.WB.Wrt_reg_addr == state.EX.Rs) {
                    //cout << "sw rs == addu rd \n";

                    value1 = state.WB.Wrt_data;
                }
            }
            

            //maths time :DDD
            //addu
            if (state.EX.alu_op == 1) {
               // bitset<32> value1 = myRF.readRF(state.EX.Rs);
                //bitset<32> value2 = myRF.readRF(state.EX.Rt);
                //cout << value1 << " + " << value2 << endl;
                newState.MEM.ALUresult = bitset<32>(value1.to_ulong() + value2.to_ulong());
            
            }

            //subu
            if (state.EX.alu_op == 0) {
                //bitset<32> value1 = myRF.readRF(state.EX.Rs);
               // bitset<32> value2 = myRF.readRF(state.EX.Rt);
                newState.MEM.ALUresult = bitset<32>(value1.to_ulong() - value2.to_ulong());
                
            }

            //lw,sw (CHECK WERE DOING THE WHOLE SIGN EXTEND THINGY RIGHT AT SOME POINT OK THANKS-->yes)
            if (state.EX.is_I_type == 1) {
                string stringImm = state.EX.Imm.to_string();
                if (stringImm[0] == '0') {
                    stringImm = "0000000000000000" + stringImm;
                }
                else {
                    stringImm = "1111111111111111" + stringImm;
                }
                bitset<32>extendedImmediate(stringImm);
                int extImmInt = extendedImmediate.to_ulong() + value1.to_ulong(); //myRF.readRF(state.EX.Rs).to_ulong();
                newState.MEM.ALUresult = bitset<32>(extImmInt);
            }        


            newState.MEM.Store_data = state.EX.Read_data2; 


            //fwd if sw uses address prev instruction writes to
            if (state.EX.is_I_type == 1 && state.EX.wrt_mem == 1 && state.MEM.wrt_enable == 1) {
                if (state.MEM.Wrt_reg_addr == state.EX.Rt) {
                    //cout << "sw rt == addu rd \n";
                    
                    newState.MEM.Store_data = state.MEM.ALUresult;
                }
            }

            //fwd if sw uses address 2 prev instruction writes to
            if (state.EX.is_I_type == 1 && state.EX.wrt_mem == 1 && state.WB.wrt_enable == 1) {
                if (state.WB.Wrt_reg_addr == state.EX.Rt) {
                    //cout << "sw rt == addu rd \n";

                    newState.MEM.Store_data = state.WB.Wrt_data;
                }
            }
            

            //rs, rt
            newState.MEM.Rs = state.EX.Rs;
            newState.MEM.Rt = state.EX.Rt;

            //wrt reg addr
            newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;

            //wrt enable
            newState.MEM.wrt_enable = state.EX.wrt_enable;

            //rdmem, wrtmem
            newState.MEM.rd_mem = state.EX.rd_mem;
            newState.MEM.wrt_mem = state.EX.wrt_mem;
            //cout<<"Update EX states";

            
        }


        newState.EX.nop = state.ID.nop;
        

        /* --------------------- ID stage --------------------- */
            

        string instrType = "";
        int IDStall = 0;

        if (!newState.ID.nop) {
            //decode state.if.instr
            string instrToString = state.ID.Instr.to_string();

            if (instrToString.substr(0, 6) == "000000" && instrToString.substr(26, 6) == "100011") {
                instrType = "subu";
                //cout << "subu" << endl;
            }
            if (instrToString.substr(0, 6) == "000000" && instrToString.substr(26, 6) == "100001") {
                instrType = "addu";
                //cout << "addu" << endl;

            }
            if (instrToString.substr(0, 6) == "100011") {
                instrType = "lw";
                //cout << "lw" << endl;

            }
            if (instrToString.substr(0, 6) == "101011") {
                instrType = "sw";
                //cout << "sw" << endl;

            }
            if (instrToString.substr(0, 6) == "000100") {
                instrType = "beq";
            }
            

            //aluOp: 1 for addu, lw, sw; 0 for subu
            if (instrType == "subu") {
                newState.EX.alu_op = 0;
            }
            if (instrType == "addu" || instrType == "lw" || instrType == "sw") {
                newState.EX.alu_op = 1;
            }

            //is i-type
            if (instrType == "lw" || instrType == "sw" || instrType == "beq") {
                newState.EX.is_I_type = 1;
            }
            else {
                newState.EX.is_I_type = 0;
            }

            //wrtenable (if instr updates rf): 1 for addu, subu, lw; 0 for beq, sw and halt. 
            if (instrType == "addu" || instrType == "subu" || instrType == "lw") {
                newState.EX.wrt_enable = 1;
            }
            else {
                newState.EX.wrt_enable = 0;
            }

            //rdmem
            if (instrType == "lw")
                newState.EX.rd_mem = 1;
            else {
                newState.EX.rd_mem = 0;
            }
            //wrmem
            if (instrType == "sw")
                newState.EX.wrt_mem = 1;
            else {
                newState.EX.wrt_mem = 0;
            }

            //wrtreg address: assigned even if not needed, will just be discarded next cycle.
            if (instrType == "addu" || instrType == "subu") {
                newState.EX.Wrt_reg_addr = bitset<5>(instrToString.substr(16, 5));
            }
            else{
                newState.EX.Wrt_reg_addr = bitset<5>(instrToString.substr(11, 5));
                //cout << "lw wrt reg addr = " << newState.EX.Wrt_reg_addr << endl;
            }

            //rs, rt
            newState.EX.Rs = bitset<5>(instrToString.substr(6, 5));
            newState.EX.Rt = bitset<5>(instrToString.substr(11, 5));
            

            //readdata 1, readdata 2  from rf 
            newState.EX.Read_data1 = myRF.readRF(bitset<5>(instrToString.substr(6, 5)));
            newState.EX.Read_data2 = myRF.readRF(bitset<5>(instrToString.substr(11, 5)));

            string immediate = instrToString.substr(16, 16);
            newState.EX.Imm = bitset<16>(immediate);

            // stalls

			// Add-Sub stall (if reading in data same time as used in addu/subu)
			if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1 && instrToString.substr(0, 6) == "000000" && state.EX.nop == 0){
				if (state.EX.Rt == newState.EX.Rt || state.EX.Rt == newState.EX.Rs){
					newState.EX.nop = 1;
                    IDStall = 1;
                    //cout << "ID stall1\n";
				}
			}

			// Load-Store stall (if storing data same time as result from addu/subu written back)

            

            

            if (instrType == "beq") {
                
                if (newState.EX.Read_data1 != newState.EX.Read_data2) {
                    //ADD IN BRANCH ADDRESS: branch address = 14 bits extend, imm, 2 bits 0
                    string BA;
                    if(immediate[0] == '0'){
                        BA = "00000000000000" + immediate + "00";
                    }
                    if (immediate[0] == '1') {
                        BA = "11111111111111" + immediate + "00";
                    }
                    
                    newState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + bitset<32>(BA).to_ulong() - 4ULL);
                    newState.ID.nop = 1;
                }
            }
        }

        

        if (instrType != "beq") {
            newState.ID.nop = state.IF.nop;
        }

        /* --------------------- IF stage --------------------- */
                
        
        if (newState.IF.nop == 0 && IDStall == 0) {
            newState.ID.Instr = myInsMem.readInstr(newState.IF.PC);
            newState.IF.PC = bitset<32>(newState.IF.PC.to_ulong() + 4ULL);            

        //halt
        //cout << " instruction" <<newState.ID.Instr << endl;
        if (newState.ID.Instr == 0xFFFFFFFF && newState.ID.nop == 0) {
            newState.IF.PC = bitset<32>(newState.IF.PC.to_ulong() - 4ULL);
            newState.IF.nop = 1;
            newState.ID.nop = 1;
        }         
            
         //load add stall
        if(state.EX.is_I_type == 0 && state.EX.rd_mem == 1 && state.EX.nop == 0 && state.ID.Instr.to_string().substr(0, 6) == "000000"){ 
            if (state.EX.Rt == newState.EX.Rt || state.EX.Rt == newState.EX.Rs){
                newState.IF = state.IF;
                newState.ID = state.ID;
                //cout << "IF stall1\n";
            }
        }
            //load store stall
        if (state.EX.Rt == newState.EX.Rt && state.EX.is_I_type == 1 && state.EX.rd_mem == 1 && state.ID.Instr.to_string().substr(0, 6) == "101011"){
                newState.IF = state.IF;
                newState.ID = state.ID;
                //cout << "IF stall2\n";
            }
        }

        //cout << "IF NOP: " << state.IF.nop << " ID NOP: " << state.ID.nop << " EXE NOP: " << state.EX.nop << " MEM NOP: " << state.MEM.nop << " WB NOP: " << state.WB.nop << endl;

        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;
            
        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
        cycle++;
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */

    }
    //cout << "WE MADE IT HERE";

    myRF.outputRF(); // dump RF;	
    myDataMem.outputDataMem(); // dump data mem 

    return 0;
}