with open("disassembled.txt", "r") as f:
    for _ in range(30):
        line = f.readline()
        if not line: break
        print(line.strip())
