
def to_hex(value, size = 2):
    return ('%x' %value).upper().zfill(size)

def print_nintendulator(data):
    string = ''
    params = ''

    mode = data.get_op_code().get_mode()
    args = data.get_args()
    info = data.get_info()
    op = data.get_op_code()
    reg = data.get_registers()

    if mode.uses_arg1():
        params += to_hex(args.arg1) + ' '
    if mode.uses_arg2():
        params += to_hex(args.arg2);
    params = params.ljust(7)

    string += to_hex(info.program_counter, 4) + '  '  + \
              to_hex(args.opcode) + ' ' + params + \
              (op.get_name() + ' ' + data.get_mode_string()).ljust(32) + \
              'A:' + to_hex(reg.accumulator) + \
              ' X:' + to_hex(reg.x_index) + \
              ' Y:' + to_hex(reg.y_index) + \
              ' P:' + to_hex(reg.get_status()) + \
              ' SP:' + to_hex(reg.stack_pointer) + \
              ' CYC:' + str(info.cycles).rjust(3) + \
              ' SL:' + str(info.scan_line)
    return string