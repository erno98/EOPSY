import io

with open('commands', mode='w') as f:
    for i in range(64):
        f.write('READ ' + str(int((i+1)*16e3)) + '\n')