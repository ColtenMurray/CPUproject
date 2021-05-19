/******************************
 * Name: Colten Murray-CLM335
* CS 3339 - Spring 2019
 ******************************/
#include "CPU.h"

const string CPU::regNames[] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                                "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                                "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                                "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"
                               };

CPU::CPU(uint32_t pc, Memory &iMem, Memory &dMem) : pc(pc), iMem(iMem), dMem(dMem)
{
    for(int i = 0; i < NREGS; i++)
    {
        regFile[i] = 0;
    }
    hi = 0;
    lo = 0;
    regFile[28] = 0x10008000;
    regFile[29] = 0x10000000 + dMem.getSize();

    instructions = 0;
    stop = false;
}

void CPU::run()
{
    while(!stop)
    {
        instructions++;

        fetch();
        decode();
        execute();
        mem();
        writeback();
        stat.clock();

        D(printRegFile());
    }
}

void CPU::fetch()
{
    instr = iMem.loadWord(pc);
    pc = pc + 4;
}

/////////////////////////////////////////
// ALL YOUR CHANGES GO IN THIS FUNCTION
/////////////////////////////////////////
void CPU::decode()
{
    uint32_t opcode;      // opcode field
    uint32_t rs, rt, rd;  // register specifiers
    uint32_t shamt;       // shift amount (R-type)
    uint32_t funct;       // funct field (R-type)
    uint32_t uimm;        // unsigned version of immediate (I-type)
    int32_t simm;         // signed version of immediate (I-type)
    uint32_t addr;        // jump address offset field (J-type)
    bool taken;
    bool pTaken;
    uint32_t originalPc;
    uint32_t target;

    opcode = (instr >> 0x1A);
    rs = (instr >> 0x15) & 0x1F;
    rt = (instr >> 0x10) & 0x1F;
    rd = (instr >> 0xB) & 0x1F;
    shamt = (instr >> 0x6) & 0x1F;
    funct = (instr & 0x3F);
    uimm = (instr & 0xFFFF);
    simm = ((signed) uimm << 16) >> 16;
    addr = (instr & 0x3FFFFFF);

    //# Hint: you probably want to give all the control signals some "safe"
    // default value here, and then override their values as necessary in each
    // case statement below!

    //Default Values
    aluSrc1 = regFile[0];
    aluSrc2 = regFile[0];
    destReg = 0;
    writeDest = false;
    opIsLoad = false;
    opIsStore = false;
    opIsMultDiv = false;
    storeData = 0;
    writeData = regFile[0];
    aluOp = ADD;

    D(cout << "  " << hex << setw(8) << pc - 4 << ": ");

    switch(opcode)
    {
    case 0x00:
        switch(funct)
        {
        case 0x00:
            D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
            //rd = rs << shamt
            writeDest = true; destReg = rd; stat.registerDest(rd);
            aluSrc1 = regFile[rs]; stat.registerSrc(rs);
            aluSrc2 = shamt;
            aluOp = SHF_L;
            break;

        case 0x03:
            D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
            // rd = rs >> shamt
            writeDest = true; destReg = rd; stat.registerDest(rd);
            aluSrc1 = regFile[rs]; stat.registerSrc(rs);
            aluSrc2 = shamt;
            aluOp = SHF_R;
            break;

        case 0x08:
            D(cout << "jr " << regNames[rs]);
            //pc = rs
            pc = regFile[rs]; stat.registerSrc(rs);
            break;

        case 0x10:
            D(cout << "mfhi " << regNames[rd]);
            //rd = hi
            writeDest = true; destReg = rd; stat.registerDest(rd);
            aluSrc1 = regFile[REG_HILO]; stat.registerSrc(REG_HILO);
            aluSrc2 = 0xffff0000;     //top 16 bits of register HILO
            aluOp = ADD;
            break;

        case 0x12:
            D(cout << "mflo " << regNames[rd]);
            //rd = lo
            writeDest = true; destReg = rd; stat.registerDest(rd);
            aluSrc1 = regFile[REG_HILO]; stat.registerSrc(REG_HILO);
            aluSrc2 = 0x0000ffff;   //bottom 16 bits of register HILO
            aluOp = AND;
            break;

        case 0x18:
            D(cout << "mult " << regNames[rs] << ", " << regNames[rt]);
            // lo = rs * rt
            opIsMultDiv = true;
            aluSrc1 = regFile[rs]; stat.registerSrc(rs);
            aluSrc2 = regFile[rt]; stat.registerSrc(rt);
            aluOp = MUL;
            break;

        case 0x1a:
            D(cout << "div " << regNames[rs] << ", " << regNames[rt]);
            // hi = rs / rt, lo = rs % rt
            opIsMultDiv = true;
            aluSrc1 = regFile[rs]; stat.registerSrc(rs);
            aluSrc2 = regFile[rt]; stat.registerSrc(rt);
            aluOp = DIV;
            break;

        case 0x21:
            D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
            //rd = rs + rt
            writeDest = true; destReg = rd; stat.registerDest(rd);
            aluSrc1 = regFile[rs]; stat.registerSrc(rs);
            aluSrc2 = regFile[rt]; stat.registerSrc(rt);
            aluOp = ADD;
            break;

        case 0x23:
            D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
            // rd = rs - rt
            writeDest = true; destReg = rd; stat.registerDest(rd);
            aluSrc1 = regFile[rs]; stat.registerSrc(rs);
            aluSrc2 = -regFile[rt]; stat.registerSrc(rt);
            aluOp = ADD;
            break;

        case 0x2a:
            D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
            writeDest = true; destReg = rd; stat.registerDest(rd);
            aluSrc1 = regFile[rs]; stat.registerSrc(rs);
            aluSrc2 = regFile[rt]; stat.registerSrc(rt);
            aluOp = CMP_LT;
            break;

        default:
            cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
        }
        break;

    case 0x02:
        D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1 - pc + 4
        pc = (pc & 0xf0000000) | (addr << 2);
        break;

    case 0x03:
        D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1 - pc + 4
        writeDest = true; destReg = REG_RA; stat.registerDest(REG_RA);// Write PC+4 to $ra
        aluOp = ADD; // ALU passes pc thru unchanged
        aluSrc1 = pc;
        aluSrc2 = regFile[REG_ZERO]; // Reads 0 every time
        pc = (pc & 0xf0000000) | addr << 2;
        break;

    case 0x04:
        D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
        stat.countBranch();
        stat.registerSrc(rs);
        stat.registerSrc(rt);
        originalPc = pc;
        target = pc + (simm << 2);
        pTaken = bpred.predict(pc,target);
        taken = false;
        if (regFile[rs] == regFile[rt])
        {
            pc = pc + (simm << 2);
            stat.countTaken();
            taken = true;
        }
        if(bpred.isMispredict(pTaken, target,taken,pc))
        {
            stat.flush(2);
        }
        bpred.update(originalPc, taken, pc);
        break;

    case 0x05:
        D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
        stat.countBranch();
        stat.registerSrc(rs);
        stat.registerSrc(rt);
        originalPc = pc;
        target = pc + (simm << 2);
        pTaken = bpred.predict(pc,target);
        taken = false;

        if (regFile[rs] != regFile[rt])
        {
            pc = pc + (simm << 2);
            stat.countTaken();
            taken = true;
        }
        if(bpred.isMispredict(pTaken, target,taken,pc))
        {
            stat.flush(2);
        }
        bpred.update(originalPc, taken, pc);
        break;

    case 0x09:
        D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
        //rt = rs + simm
        writeDest = true; destReg = rt; stat.registerDest(rt);
        aluSrc1 = regFile[rs]; stat.registerSrc(rs);
        aluSrc2 = simm;
        aluOp = ADD;
        break;

    case 0x0c:
        D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm);
        //rt = rs & simm
        writeDest = true; destReg = rt; stat.registerDest(rt);
        aluSrc1 = regFile[rs]; stat.registerSrc(rs);
        aluSrc2 = simm;
        aluOp = AND;
        break;

    case 0x0f:
        D(cout << "lui " << regNames[rt] << ", " << dec << simm);
        //rt = sim << 16
        writeDest = true; destReg = rt; stat.registerDest(rt);
        aluOp = SHF_L;
        aluSrc1 = simm;
        aluSrc2 = 16;
        break;

    case 0x1a:
        D(cout << "trap " << hex << addr);
        switch(addr & 0xf)
        {
        case 0x0:
            cout << endl;
            break;
        case 0x1:
            cout << " " << (signed)regFile[rs];
            break;
        case 0x5:
            cout << endl << "? ";
            cin >> regFile[rt];
            break;
        case 0xa:
            stop = true;
            break;
        default:
            cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
            stop = true;
        }
        break;

    case 0x23:
        D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
        stat.countMemOp();
        opIsLoad = true;
        writeDest = true; destReg = rt; stat.registerDest(rt);
        aluOp = ADD;
        aluSrc1 = regFile[rs]; stat.registerSrc(rs);
        aluSrc2 = simm;
        break;

    case 0x2b:
        D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
        stat.countMemOp();
        opIsStore = true;
        storeData = regFile[rt];
        aluOp = ADD;
        aluSrc1 = regFile[rs]; stat.registerSrc(rs);
        aluSrc2 = simm;
        break;

    default:
        cerr << "Not implemented instruction: pc = 0x" << hex << pc - 4 << endl;
    }
    D(cout << endl);
}

void CPU::execute()
{
    aluOut = alu.op(aluOp, aluSrc1, aluSrc2);
}

void CPU::mem()
{
    if(opIsLoad)
        writeData = dMem.loadWord(aluOut);
    else
        writeData = aluOut;

    if(opIsStore)
        dMem.storeWord(storeData, aluOut);
}

void CPU::writeback()
{
    if(writeDest && destReg > 0) // skip if write is to zero register
        regFile[destReg] = writeData;

    if(opIsMultDiv)
    {
        hi = alu.getUpper();
        lo = alu.getLower();
    }
}

void CPU::printRegFile()
{
    cout << hex;
    for(int i = 0; i < NREGS; i++)
    {
        cout << "    " << regNames[i];
        if(i > 0) cout << "  ";
        cout << ": " << setfill('0') << setw(8) << regFile[i];
        if( i == (NREGS - 1) || (i + 1) % 4 == 0 )
            cout << endl;
    }
    cout << "    hi   : " << setfill('0') << setw(8) << hi;
    cout << "    lo   : " << setfill('0') << setw(8) << lo;
    cout << dec << endl;
}

void CPU::printFinalStats()
{
    cout << "Program finished at pc = 0x" << hex << pc << "  ("
         << dec << instructions << " instructions executed)" << endl << endl;

    cout << "Cycles: " << stat.getCycles() << endl
         << "CPI: " << (stat.getCycles()/instructions) << endl << endl
         << "Bubbles: " << stat.getBubbles() << endl
         << "Flushes: " << stat.getFlushes() << endl << endl
         << "Mem ops: " << fixed << setprecision(1) << 100.0 * stat.getMemOps() / instructions << "%" << " of instructions" << endl
         << "Branches: " << fixed << setprecision(1) << 100.0 * stat.getBranches() / instructions << "%" << " of instructions" << endl
         << "% Taken: " << fixed << setprecision(1) << 100.0 * stat.getTaken() / stat.getBranches() << "%";
}
