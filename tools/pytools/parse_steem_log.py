
# TODO:
# - creer une mathode de parse plus generique, qui prend en compte les opcode/opername/storename/addrname
# - trouver où est chargé le script courant pour avoir l'addr de base

import alis


def handle_op(steem_lines,
              steem_line_idx,
              prefix_str,
              divider,
              op_idx,
              op_table,
              op_occurrences):
    #########################################
    regs_line = steem_lines[steem_line_idx + 1]
    addrs_line = steem_lines[steem_line_idx + 2]
    regs = regs_line.split("  ")
    addrs = addrs_line.split("  ")
    # d0 register contains the opcode in its lower byte
    d0_str = regs[0].split("=")[1]
    op_byte = int(int(d0_str[-2:], 16) / divider)
    # a3 register is the virtual PC address
    vpc = int(addrs[3].split("=")[1], 16) - 1
    op_name = op_table[op_byte]
    output_line = prefix_str + hex(vpc) + ": " + op_name

    # statistics
    if not op_name in op_occurrences.keys():
        op_occurrences[op_name] = 1
    else:
        count = op_occurrences[op_name]
        op_occurrences[op_name] = count + 1

    # next...
    #steem_line_idx += 2
    #op_idx += 1
    return output_line


def print_stats(kind_str,
                op_occurences):
    print()
    print(kind_str + " STATISTICS")
    print()
    sorted_op_occurrences = {key: val for key, val in sorted(op_occurences.items(), key=lambda ele: ele[1], reverse=True)}
    for op_name in sorted_op_occurrences.keys():
        stat_line = op_name + ";" + str(sorted_op_occurrences[op_name])
        print(stat_line)
        # opcode_log.write(stat_line + "\n")


def parse_log(alis_data, steem_log, output_file):
    steem_log = open(steem_log, "r")
    opcode_log = open(output_file, "w")
    steem_lines = steem_log.readlines()
    steem_lines_count = len(steem_lines)
    steem_line_idx = 0

    output_lines = []
    opcode_occurrences = {}
    opername_occurrences = {}
    storename_occurrences = {}
    addname_occurrences = {}

    opcode_idx = 0
    while steem_line_idx < steem_lines_count:
        steem_line = steem_lines[steem_line_idx]
        opcode_bp_addr = hex(alis_data.opcode_bp_addr)[2:]
        opername_bp_addr = hex(alis_data.opername_bp_addr)[2:]
        storename_bp_addr = hex(alis_data.storename_bp_addr)[2:]
        addname_bp_addr = hex(alis_data.addname_bp_addr)[2:]
        storename_bp_addr_2 = hex(alis_data.storename_bp_addr_2)[2:]
        addname_bp_addr_2 = hex(alis_data.addname_bp_addr_2)[2:]

        #########################################
        # FUN_READ_OPCODE
        if "$0" + str(opcode_bp_addr) in steem_line:
            line = handle_op(steem_lines, steem_line_idx, "", 1, opcode_idx,
                             alis_data.opcode_table, opcode_occurrences)
            output_lines.append(line)
            steem_line_idx += 2
            opcode_idx += 1
        #########################################

        #########################################
        # FUN_READ_OPERNAME
        if "$0" + str(opername_bp_addr) in steem_line:
            line = handle_op(steem_lines, steem_line_idx, "\t", 2, opcode_idx,
                             alis_data.opername_table, opername_occurrences)
            output_lines.append(line)
            steem_line_idx += 2
            opcode_idx += 1
        #########################################

        #########################################
        # FUN_READ_STORENAME
        if "$0" + str(storename_bp_addr) in steem_line:
            line = handle_op(steem_lines, steem_line_idx, "\t", 2, opcode_idx,
                             alis_data.storename_table, storename_occurrences)
            output_lines.append(line)
            steem_line_idx += 2
            opcode_idx += 1
        if "$0" + str(storename_bp_addr_2) in steem_line:
            line = handle_op(steem_lines, steem_line_idx, "\t", 2, opcode_idx,
                             alis_data.storename_table, storename_occurrences)
            output_lines.append(line)
            steem_line_idx += 2
            opcode_idx += 1
        #########################################

        #########################################
        # FUN_READ_ADDNAME
        if "$0" + str(addname_bp_addr) in steem_line:
            line = handle_op(steem_lines, steem_line_idx, "\t", 2, opcode_idx,
                             alis_data.addname_table, addname_occurrences)
            output_lines.append(line)
            steem_line_idx += 2
            opcode_idx += 1
        if "$0" + str(addname_bp_addr_2) in steem_line:
            line = handle_op(steem_lines, steem_line_idx, "\t\t\tADDNAME", 2, opcode_idx,
                             alis_data.addname_table, addname_occurrences)
            output_lines.append(line)
            steem_line_idx += 2
            opcode_idx += 1
        #########################################

        # next line
        steem_line_idx += 1

    # print all called opcodes, in order
    print()
    print("HISTORY")
    print()
    for output_line in output_lines:
        print(output_line)
        opcode_log.write(output_line + "\n")

    # print opcode usage statistics
    print_stats("OPCODE", opcode_occurrences)
    print_stats("OPERNAME", opername_occurrences)
    print_stats("STORENAME", storename_occurrences)
    print_stats("ADDNAME", addname_occurrences)

    # close files
    steem_log.close()
    opcode_log.close()

# MAIN
parse_log(alis.ISHAR2_CRELITE, "../Steem.SSE.4.0.2.Debug.Win64.DD/steem.log",
          "./../Steem.SSE.4.0.2.Debug.Win64.DD/logo_animating.txt")
# parse_log(alis.ISHAR2_CRELITE, "./steem-boot to logo.log", "./steem-boot to logo.txt")
# parse_log(alis.ISHAR2_CRELITE, "./steem-logo to ishar.log", "./steem-logo to ishar.txt")
# parse_log(alis.ISHAR2_CRELITE, "./steem-ishar to lang.log", "./steem-ishar to lang.txt")
# parse_log(alis.ISHAR2_CRELITE, "./steem-lang to disk b.log", "./steem-lang to disk b.txt")
# parse_log(alis.ISHAR2_CRELITE, "./steem-disk b to game.log", "./steem-disk b to game.txt")
# parse_log(alis.ISHAR2_CRELITE, "./steem-game step fwd.log", "./steem-game step fwd.txt")

# parse_log(alis.ISHAR2_CRELITE, "./steem-boot to game.log", "./steem-boot to game.txt")

# parse_log(alis.ISHAR2_CRELITE, "./steem-boot to open logo script.log", "./steem-boot to open logo script.txt")
