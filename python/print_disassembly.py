
def to_hex(value, size = 2):
    return ('%x' %value).upper().zfill(size)

def print_nintendulator(data):
    string = ''
    params = ''

    mode = data.opcode.mode
    args = data.args
    info = data.info
    op = data.opcode
    reg = data.registers

    if mode.uses_arg1:
        params += to_hex(args.arg1) + ' '
    if mode.uses_arg2:
        params += to_hex(args.arg2);
    params = params.ljust(7)

    string += to_hex(info.program_counter, 4) + '  '  + \
              to_hex(args.opcode) + ' ' + params + \
              (op.name + ' ' + data.mode_string).ljust(32) + \
              'A:' + to_hex(reg.accumulator) + \
              ' X:' + to_hex(reg.x_index) + \
              ' Y:' + to_hex(reg.y_index) + \
              ' P:' + to_hex(reg.get_status()) + \
              ' SP:' + to_hex(reg.stack_pointer) + \
              ' CYC:' + str(info.cycles).rjust(3) + \
              ' SL:' + str(info.scan_line)
    return string