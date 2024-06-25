// DO NOT COMPILE!!!

// NOTE: mostly various bits and peaces of code used to debug, slow, ugly or otherwise undesireable to keep in place

int register_count = 0;
int64_t register_a4_off;
extern s32 registers_idx;
extern u32 registers[];

// alis_loop()

//        if (registers_idx == -1 && alis.script->pc == registers[3] && registers[5] == alis.script->vram_org)
//        {
//            if (register_count)
//                registers_idx = 0;
//
//            register_count++;
//        }
        
        if (registers_idx >= 0 && registers_idx < 3067071)
        {
            if (registers_idx == 0)
            {
                register_a4_off = (int64_t)alis.acc - registers[4];
            }

//            bool change = (registers[registers_idx + 5] == 0x2bb0c || registers[registers_idx + 5] == 0x2b95c || registers[registers_idx + 5] == 0x2ba34) && registers[registers_idx + 5] == alis.script->vram_org;
//            bool change = 1;
            
            
            bool found = true;
            
            // a6
            if (registers[registers_idx + 5] != alis.script->vram_org)
            {
//                printf("\nA6: %.6x ?= %.6x", registers[registers_idx + 5], alis.script->vram_org);
                // sleep(0);
                
                // if (0x2a8f2 == registers[registers_idx + 5])
                
                found = false;
                u32 loop = alis.nbent;
                for (int i = 0; i < loop; i++)
                {
                    sAlisScriptLive *s = alis.live_scripts[i];
                    if (s && s->vram_org)
                    {
                        if (s->vram_org == registers[registers_idx + 5])
                        {
                            if (0x2a8f2 == registers[registers_idx + 5])
                            {
                                alis.script = s;
                                alis.fallent = 0;
                                alis.fseq = 0;
                                alis.acc = alis.acc_org;
                                alis.script->pc = get_0x08_script_ret_offset(alis.script->vram_org);
                                alis.script->vacc_off = get_0x0a_vacc_offset(alis.script->vram_org);
                            }
                            found = true;
                        }
                    }
                    else
                    {
                        loop++;
                    }
                }
                
                if (found == false)
                {
//                    loop = alis.nbent;
//                    printf("\nA6: %.6x ?= %.6x", registers[registers_idx + 5], alis.script->vram_org);
//                    for (int i = 0; i < loop; i++)
//                    {
//                        sAlisScriptLive *s = alis.live_scripts[i];
//                        if (s && s->vram_org)
//                        {
//                            printf("\n    %.6x", s->vram_org);
//                        }
//                        else
//                        {
//                            loop++;
//                        }
//                    }

                    sleep(0);
                }
            }
            
            if (found)
            {
                bool change = 0x2a8f2 == alis.script->vram_org && registers[registers_idx + 5] == alis.script->vram_org;
                //            bool change = 0x2a8f2 == alis.script->vram_org;
                
                // d4
                if ((u16)registers[registers_idx + 0] != (u16)alis.script->vacc_off)
                {
                    sleep(0);
                    //                if (change)
                    //                {
                    //                    printf("\nD4: %.6x ?= %.6x", (u16)registers[registers_idx + 0], (u16)alis.script->vacc_off);
                    //                    alis.script->vacc_off = registers[registers_idx + 0];
                    //                }
                }
                
                // d6
                if ((u16)registers[registers_idx + 1] != (u16)alis.varD6)
                {
                    sleep(0);
                    //                if (change)
                    //                {
                    //                    printf("\nD6: %.6x ?= %.6x", (u16)registers[registers_idx + 1], (u16)alis.varD6);
                    //                    alis.varD6 = registers[registers_idx + 1];
                    //                }
                }
                
                // d7
                if ((u16)registers[registers_idx + 2] != (u16)alis.varD7)
                {
                    sleep(0);
                    //                if (change)
                    //                {
                    //                    printf("\nD7: %.6x ?= %.6x", (u16)registers[registers_idx + 2], (u16)alis.varD7);
                    //                    alis.varD7 = registers[registers_idx + 2];
                    //                }
                }
                
                // a3
                if (registers[registers_idx + 3] != alis.script->pc)
                {
                    sleep(0);
                    if (change && (
                        0x68def == registers[registers_idx + 3] ||
                        0x68dfe == registers[registers_idx + 3] ||
                        0x69235 == registers[registers_idx + 3] ||
                        0x69244 == registers[registers_idx + 3] ||
                        0x6926f == registers[registers_idx + 3] ||
                        0x692a7 == registers[registers_idx + 3]
                        ))
                        
                        
//                        0x6884a != registers[registers_idx + 3] && 0x6892b != registers[registers_idx + 3] && 0x6876e != registers[registers_idx + 3] &&
//                        0x6877a != registers[registers_idx + 3] && 0x687bb != registers[registers_idx + 3] && 0x6882e != registers[registers_idx + 3] &&
//                        0x68a3e != registers[registers_idx + 3] && 0x6885e != registers[registers_idx + 3] && 0x68c94 != registers[registers_idx + 3] &&
//                        0x69301 != registers[registers_idx + 3])
                    {
                        printf("\nA3: %.6x ?= %.6x", registers[registers_idx + 3], alis.script->pc);
                        alis.script->pc = registers[registers_idx + 3];
                    }
                }
                
                // a4
                if (register_a4_off + registers[registers_idx + 4] != (int64_t)alis.acc)
                {
                    sleep(0);
                    //                if (change)
                    //                {
                    //                    printf("\nA4: %.6x ?= %.6llx", registers[registers_idx + 4], (int64_t)alis.acc - register_a4_off);
                    //                    alis.acc = (s16 *)(registers[registers_idx + 4] + register_a4_off);
                    //                }
                }

                registers_idx +=6;
            }
            else
            {
                registers_idx = -1;
            }
        }
